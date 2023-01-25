#include <search_engine.h>
#include <search/composite_score.h>
#include <scoring/composite_score.h>
#include <binary_heap.h>
#include <k_utils.h>
#include <list.h>
#include <mt_queue.h>
#include <daos_definitions.h>
#include <profiling.h>
#include <logger.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

//
// Types
//

typedef
struct {
    uint16_t expiration;

    pthread_mutex_t mutex;
    List_t list;
} SearchResultsGroup_t;

typedef
struct {
    SearchId_t nextSearchId;

    CacheEngine_t profileCache;
    CacheEngine_t companyCache;
    MtQueue_t searchQueue;

    SearchResultsGroup_t searchResults;

    pthread_t searchResultsThreadId;
    bool searchResultsThreadStarted;

    uint16_t numSearchThreads;
    pthread_t* searchThreadIds;
} SearchEngineData_t;

typedef
struct
{
    SearchId_t searchId;

    struct SearchCriteria* searchCriteria;
    DaosCount_t profilesLimit;
    CacheEngine_t profileCache;
    CacheEngine_t companyCache;

    pthread_mutex_t mutex;
    time_t completionTime;

    uint16_t numPendingScoreLists;
    uint16_t numScoreListsInUse;
    uint16_t totalNumScoreLists;
    BinaryHeap_t* scoreListsGroup;
    uint32_t numProfilesScored; // only a limited amount of profiles will be returned out of the max possible

    struct json_object* resultsJsonObj;

    // used by Xuan to profile how much time scoring client spent on search
    // value is stored in nanoseconds
    int64_t searchThreadsStarted;
    int64_t searchDuration;
} SearchResults_t;

typedef
struct
{
    SearchResults_t* searchResults;

    DaosCount_t rangeLow;
    DaosCount_t rangeHigh;

    BinaryHeap_t scoreList;
} SearchData_t;

//
// Constants
//

#define SEARCH_QUEUE_SIZE_PER_THREAD 8

const char* compositeScore_Key = "scores";
const char* totalProfiles_Key = "total_scored";

//
// Prototypes
//

// SearchEngineData_t

static void cleanupSearchEngineData(SearchEngineData_t* data);

// Search preparation

static int prepareSearch(SearchEngineData_t* data, struct SearchCriteria* searchCriteria,
    DaosCount_t profilesLimit, SearchId_t* searchId, int64_t searchDuration);
static List_t prepareSearchThreadArgs(struct SearchCriteria* searchCriteria,
    DaosCount_t numProfiles, SearchResults_t* searchResults, uint16_t numThreads);

// Threads

static void* searchResultsThread(void* arg);
static void* searchThread(void* arg);

static void cacheEngineThreadCleanup(void *arg);
static void searchQueueThreadCleanup(void *arg);
static void mutexThreadCleanup(void *arg);

// SearchResults_t

static SearchResults_t* initSearchResults(SearchId_t searchId,
    struct SearchCriteria* searchCriteria, DaosCount_t profilesLimit,
    CacheEngine_t profileCache, CacheEngine_t companyCache, uint16_t numScoreLists);
static void freePartialSearchResults(SearchResults_t* results);
static void freeSearchResults(SearchResults_t* results);

static int compareSearchResultsKeys(void* key1, void* key2);
static void freeSearchResultsItem(void* arg);
static void* keyFromSearchResults(void* value);

// SearchData_t

static int compareSearchDataKeys(void* key1, void* key2);
static void freeSearchData(void* arg);
static void* keyFromSearchData(void* value);

// struct CompositeScore

static int compareCompositeScoresAscendingOrder(void* arg1, void* arg2);
static int compareCompositeScoresDescendingOrder(void* arg1, void* arg2);
static void freeCompositeScoreItem(void* arg);

static void mergeCompositeScores(BinaryHeap_t resultingScoreList,
    BinaryHeap_t* scoreListsGroup, uint16_t numScoreLists);

// Others

static int mergeScoresAndGenerateJson(BinaryHeap_t* scoresGroup, uint16_t scoresGroupSize,
    DaosCount_t resultsLimit, struct json_object** resultsJsonObj);

//
// External interface
//

//
// Search engine
//

SearchEngine_t searchEngine_init()
{
    SearchEngine_t searchEngine = SearchEngine_NULL;

    SearchEngineData_t* data = malloc(sizeof(SearchEngineData_t));
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(size = %zu bytes) failed.",
            sizeof(SearchEngineData_t));
        return searchEngine;
    }

    data->nextSearchId = 1;
    data->profileCache = CacheEngine_NULL;
    data->companyCache = CacheEngine_NULL;
    data->searchQueue  = MtQueue_NULL;

    data->searchResults.expiration = 0;
    data->searchResults.list       = List_NULL;

    data->numSearchThreads = 0;
    data->searchThreadIds  = NULL;
    data->searchResultsThreadStarted = false;

    searchEngine.s = data;

    return searchEngine;
}

int searchEngine_start(SearchEngine_t searchEngine, CacheEngine_t profileCache,
    CacheEngine_t companyCache, uint16_t numThreads, uint16_t searchResultsExpirationInMinutes)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Begin.");

    SearchEngineData_t* data = (SearchEngineData_t*) searchEngine.s;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for searchEngine.");
        return 1;
    }

    data->profileCache = profileCache;
    data->companyCache = companyCache;

    data->searchQueue = mtQueue_init(numThreads * SEARCH_QUEUE_SIZE_PER_THREAD, freeSearchData);
    if(mtQueue_isNull(data->searchQueue))
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: mtQueue_init() failed.");
        return 2;
    }

    data->searchResults.expiration = searchResultsExpirationInMinutes * 60;

    data->searchResults.list = list_init(compareSearchResultsKeys, freeSearchResultsItem,
        keyFromSearchResults);
    if(list_isNull(data->searchResults.list))
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: list_init(searchResults) failed.");
        cleanupSearchEngineData(data);
        return 3;
    }

    pthread_mutex_init(&data->searchResults.mutex, NULL);

    if(pthread_create(&data->searchResultsThreadId, NULL, searchResultsThread,
        &data->searchResults) != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: pthread_create(searchResultsThread) "
            "failed. errno = %d (\"%s\").", errno, strerror(errno));
        cleanupSearchEngineData(data);
        return 4;
    }

    data->searchResultsThreadStarted = true;

    data->searchThreadIds = (pthread_t*) malloc(numThreads * sizeof(pthread_t));
    if(data->searchThreadIds == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(searchThreadIds, size = %zu bytes)"
            " failed.", numThreads * sizeof(pthread_t));
        cleanupSearchEngineData(data);
        return 5;
    }

    for(uint16_t i = 0; i < numThreads; ++i)
    {
        if(pthread_create(&data->searchThreadIds[i], NULL, searchThread, &data->searchQueue) != 0)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: pthread_create(searchThread = %hu) "
                "failed. errno = %d (\"%s\").", i, errno, strerror(errno));
            data->numSearchThreads = i;
            cleanupSearchEngineData(data);
            return 6;
        }
    }

    data->numSearchThreads = numThreads;

    LOG_MESSAGE(INFO_LOG_MSG, "Succeeded");
    return 0;
}

void searchEngine_stop(SearchEngine_t searchEngine)
{
    SearchEngineData_t* data = (SearchEngineData_t*) searchEngine.s;

    if(data != NULL) {
        cleanupSearchEngineData(data);
    }
}

bool searchEngine_isNull(SearchEngine_t searchEngine)
{
    return searchEngine.s == NULL;
}

//
// Searches
//

int searchEngine_startSearch(SearchEngine_t searchEngine, struct SearchCriteria* searchCriteria,
    DaosCount_t numProfiles, SearchId_t* searchId, int64_t searchDuration)
{
    SearchEngineData_t* data = (SearchEngineData_t*) searchEngine.s;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for searchEngine.");
        return 1;
    }

    int ret = prepareSearch(data, searchCriteria, numProfiles, searchId, searchDuration);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: prepareSearch() returned %d.", ret);
        return 2;
    }

    return 0;
}

int searchEngine_getSearchStatus(SearchEngine_t searchEngine, SearchId_t searchId,
    uint8_t* completionPercentage)
{
    SearchEngineData_t* data = (SearchEngineData_t*) searchEngine.s;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for searchEngine.");
        return 1;
    }

    int ret = 0;

    pthread_cleanup_push(mutexThreadCleanup, &data->searchResults.mutex);

    pthread_mutex_lock(&data->searchResults.mutex);

    ListIterator_t it = list_search(data->searchResults.list, (void*) (uint64_t) searchId);
    if(list_end(&it) == false)
    {
        SearchResults_t* results = (SearchResults_t*) list_iteratorValue(&it);

        if(results->numPendingScoreLists == 0)
        {
            *completionPercentage = 100;
        }
        else
        {
            *completionPercentage
                = ((results->numScoreListsInUse - results->numPendingScoreLists) * 100)
                / results->numScoreListsInUse;
        }
    }
    else
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchId = %lu not found.", searchId);
        ret = 2;
    }

    pthread_cleanup_pop(1);

    return ret;
} 

int searchEngine_getSearchResults(SearchEngine_t searchEngine, SearchId_t searchId,
    struct json_object** resultsJsonObj)
{
    int64_t searchMergeDurationStart = GetProfilingStamp();

    SearchEngineData_t* data = (SearchEngineData_t*) searchEngine.s;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for searchEngine.");
        return 1;
    }

    SearchResults_t* results = NULL;

    pthread_cleanup_push(mutexThreadCleanup, &data->searchResults.mutex);

    pthread_mutex_lock(&data->searchResults.mutex);

    ListIterator_t it = list_search(data->searchResults.list, (void*) (uint64_t) searchId);
    if(list_end(&it) == false) {
        results = (SearchResults_t*) list_iteratorValue(&it);
    }

    pthread_cleanup_pop(1);

    if(results == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: no search found with id = %lu.", searchId);
        return 2;
    }

    int ret = 0;

    pthread_cleanup_push(mutexThreadCleanup, &results->mutex);

    pthread_mutex_lock(&results->mutex);

    if(results->numPendingScoreLists > 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: search (id = %lu) not finished yet. %lu ranges"
            " to complete.", searchId, results->numPendingScoreLists);
        ret = 3;
    }
    else if(results->resultsJsonObj == NULL)
    {
        int mergeRet = mergeScoresAndGenerateJson(results->scoreListsGroup,
            results->numScoreListsInUse, results->profilesLimit, &results->resultsJsonObj);
        if(mergeRet == 0)
        {
            freePartialSearchResults(results);
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: mergeScoresAndGenerateJson() returned %d."
                " SearchId = %lu.", mergeRet, searchId);
            ret = 4;
        }

        // Add total results possible. Not the same value as total values returned
        struct json_object* jObj = json_object_new_int(results->numProfilesScored);
        json_object_object_add(results->resultsJsonObj, totalProfiles_Key, jObj);

        // only count this process once
        int64_t searchMergeDuration = GetProfilingStamp() - searchMergeDurationStart;
        results->searchDuration += searchMergeDuration;
        if (results->searchDuration > WARNING_ON_SEARCH_TIME_EXCEED_MS * MS_TO_NS)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention : Unexpectedly long search detected for id %u! Search duration %lld ms. Limit %u ms", 
                searchId, results->searchDuration / MS_TO_NS, WARNING_ON_SEARCH_TIME_EXCEED_MS);
        }
        LOG_MESSAGE(DEBUG_LOG_MSG, "Search id %lu took %lld ms to process.", searchId, results->searchDuration / MS_TO_NS);
        ProfilingAddSectionTimeThreadSafeWithMax(PE_SEARCH_REQUEST_FULL, results->searchDuration, 1);
    }

    *resultsJsonObj = results->resultsJsonObj;

    pthread_cleanup_pop(1);

    return ret;
}

int searchEngine_removeSearchResults(SearchEngine_t searchEngine, SearchId_t searchId)
{
    SearchEngineData_t* data = (SearchEngineData_t*) searchEngine.s;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for searchEngine.");
        return 1;
    }

    int ret = 0;

    pthread_cleanup_push(mutexThreadCleanup, &data->searchResults.mutex);

    pthread_mutex_lock(&data->searchResults.mutex);

    ListIterator_t it = list_search(data->searchResults.list, (void*) (uint64_t) searchId);
    if(list_end(&it) == false)
    {
        list_deleteItem(&it);
    }
    else
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchId = %lu not found among completed searches.",
            searchId);
        ret = 2;
    }

    pthread_cleanup_pop(1);

    return ret;
}

//
// Local functions
//

//
// SearchEngineData_t
//

static void cleanupSearchEngineData(SearchEngineData_t* data)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    for(uint16_t i = 0; i < data->numSearchThreads; ++i)
    {
        pthread_cancel(data->searchThreadIds[i]);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Search thread %hu cancelled.", i);
    }

    for(uint16_t i = 0; i < data->numSearchThreads; ++i)
    {
        pthread_join(data->searchThreadIds[i], NULL);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Search thread %hu joined.", i);
    }

    if(data->searchResultsThreadStarted == true)
    {
        pthread_cancel(data->searchResultsThreadId);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Search results thread cancelled.");

        pthread_join(data->searchResultsThreadId, NULL);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Search results thread joined.");
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "list_free(searchResults list).");
    list_free(data->searchResults.list);

    pthread_mutex_destroy(&data->searchResults.mutex);

    LOG_MESSAGE(DEBUG_LOG_MSG, "mtQueue_free(searchQueue).");
    mtQueue_free(data->searchQueue);

    LOG_MESSAGE(DEBUG_LOG_MSG, "free(searchThreadIds).");
    free(data->searchThreadIds);
    free(data);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
}

//
// Search preparation
//

static int prepareSearch(SearchEngineData_t* data, struct SearchCriteria* searchCriteria,
    DaosCount_t profilesLimit, SearchId_t* searchId, int64_t searchDuration)
{
    int64_t searchSetupDurationStart = GetProfilingStamp();
    DaosCount_t numCachedProfiles = 0;

    pthread_cleanup_push(cacheEngineThreadCleanup, &data->profileCache);

    CachedItems_t* profileCachedList = cacheEngine_lockItemsForRead(data->profileCache);

    numCachedProfiles = profileCachedList->largestIndexInUse + 1;

    pthread_cleanup_pop(1);

    SearchResults_t* searchResults = initSearchResults(data->nextSearchId, searchCriteria,
        profilesLimit, data->profileCache, data->companyCache, data->numSearchThreads);
    if(searchResults == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: initSearchResults() failed.");
        freeSearchCriteria(searchCriteria);
        free(searchCriteria);
        return 1;
    }

    List_t searchDataList = prepareSearchThreadArgs(searchCriteria, numCachedProfiles,
        searchResults, data->numSearchThreads);
    if(list_isNull(searchDataList))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: prepareSearchThreadArgs() failed.");
        freeSearchResults(searchResults);
        return 2;
    }

    bool ret = false;

    pthread_cleanup_push(mutexThreadCleanup, &data->searchResults.mutex);

    pthread_mutex_lock(&data->searchResults.mutex);
    
    ret = list_addItem(data->searchResults.list, searchResults);

    // Add the time it took us to parse the search request JSON + start up the worked threads
    int64_t searchSetupDuration = GetProfilingStamp() - searchSetupDurationStart;
    searchResults->searchDuration += searchDuration;
    searchResults->searchDuration += searchSetupDuration;
    searchResults->searchThreadsStarted = GetProfilingStamp(); // last thread will get the search duration based on this

    pthread_cleanup_pop(1);

    if(ret == false)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: addItemToList() failed.");
        freeSearchResults(searchResults);
        return 3;
    }

    ListIterator_t sd = list_begin(searchDataList);

    while(list_end(&sd) == false)
    {
        pthread_cleanup_push(searchQueueThreadCleanup, &data->searchQueue);

        mtQueue_push(data->searchQueue, list_iteratorPop(&sd));

        pthread_cleanup_pop(0);
    }

    list_free(searchDataList);

    *searchId = data->nextSearchId++;

    if(data->nextSearchId == 0) { ++(data->nextSearchId); }

    return 0;
}

static List_t prepareSearchThreadArgs(struct SearchCriteria* searchCriteria,
    DaosCount_t numProfiles, SearchResults_t* searchResults, uint16_t numThreads)
{
    List_t searchDataList = list_init(compareSearchDataKeys, freeSearchData, keyFromSearchData);
    if(list_isNull(searchDataList))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: list_init(searchDataList) failed.");
        return searchDataList;
    }

    DaosCount_t rangeSize = numProfiles / numThreads;
    if(numProfiles % numThreads != 0) { rangeSize += 1; }

    DaosCount_t nextRangeLow = 0;

    for(uint16_t i = 0; i < numThreads; ++i)
    {
        DaosCount_t rangeHigh = nextRangeLow + rangeSize - 1;

        if(rangeHigh >= numProfiles) { rangeHigh = 0; }

        SearchData_t* searchData = (SearchData_t*) malloc(sizeof(SearchData_t));
        if(searchData == NULL)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(searchData = %hu, size = %zu bytes)"
                " failed.", i, sizeof(SearchData_t));
            list_free(searchDataList);
            return List_NULL;
        }

        searchData->searchResults = searchResults;
        searchData->rangeLow      = nextRangeLow;
        searchData->rangeHigh     = rangeHigh;

        searchResults->scoreListsGroup[i] = binaryHeap_init(compareCompositeScoresAscendingOrder,
            freeCompositeScoreItem, searchResults->profilesLimit);
        if(binaryHeap_isNull(searchResults->scoreListsGroup[i]))
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: binaryHeap_init(scores, index = %hu,"
                " profilesLimit = %lu) failed.", i, searchResults->profilesLimit);
            free(searchData);
            freeSearchResults(searchResults);

            list_free(searchDataList);
            return List_NULL;
        }

        ++(searchResults->numScoreListsInUse);
        ++(searchResults->numPendingScoreLists);

        searchData->scoreList = searchResults->scoreListsGroup[i];

        if(list_addItem(searchDataList, searchData) == false)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: addItemToList(searchData = %hu) failed.", i);
            free(searchData);

            list_free(searchDataList);
            return List_NULL;
        }

        if(rangeHigh == 0) { break; }

        nextRangeLow = rangeHigh + 1;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "numScoreListsInUse = %lu, numPendingScoreLists = %lu.",
        searchResults->numScoreListsInUse, searchResults->numPendingScoreLists);

    return searchDataList;
}

//
// Threads
//

static void* searchResultsThread(void* arg)
{
    SearchResultsGroup_t* resultsGroup = (SearchResultsGroup_t*) arg;

    while(true)
    {
        sleep(60);

        time_t now = time(NULL);

        pthread_cleanup_push(mutexThreadCleanup, &resultsGroup->mutex);

        pthread_mutex_lock(&resultsGroup->mutex);

        for(ListIterator_t it = list_begin(resultsGroup->list); list_end(&it) == false;
            list_iteratorNext(&it))
        {
            SearchResults_t* results = (SearchResults_t*) list_iteratorValue(&it);

            if(results->numPendingScoreLists == 0)
            {
                if(results->completionTime == INT64_MIN)
                {
                    results->completionTime = now;
                }
                else
                {
                    if(difftime(now, results->completionTime) > resultsGroup->expiration)
                    {
                        LOG_MESSAGE(INFO_LOG_MSG, "SearchId %d result timed out. Deleting it.",
                            results->searchId);
                        list_deleteItem(&it);
                    }
                }
            }
        }

        pthread_cleanup_pop(1);
    }

    return NULL;
}

static void* searchThread(void* arg)
{
    MtQueue_t* searchQueue = (MtQueue_t*) arg;

    while(true)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Search thread waiting for next search request...");

        SearchData_t* searchData = NULL;

        pthread_cleanup_push(searchQueueThreadCleanup, searchQueue);

        searchData = (SearchData_t*) mtQueue_pop(*searchQueue);

        pthread_cleanup_pop(0);

        // Perform search.
        StartInlinedProfilingThreadSafe(PE_THREADED_SEARCH);

        size_t totalProfilesScored = 0;
        ScoringCompositeScore scoringSessionStore;
        // set default values for structure
        initScoringCompositeScore(&scoringSessionStore);

        CachedItems_t* cachedItems = NULL;

        pthread_cleanup_push(cacheEngineThreadCleanup, &searchData->searchResults->companyCache);
        cachedItems = cacheEngine_lockItemsForRead(searchData->searchResults->companyCache);
        scoringSessionStore.companyCachedList = (const struct CompanyCached**)cachedItems->items;
        scoringSessionStore.companyCachedLargestId = cachedItems->largestIndexInUse;

        pthread_cleanup_push(cacheEngineThreadCleanup, &searchData->searchResults->profileCache);
        cachedItems = cacheEngine_lockItemsForRead(searchData->searchResults->profileCache);
        struct ProfileCached** profileCachedList = (struct ProfileCached**) cachedItems->items;

        // Initialize filters inside scoring module
        setScoringCompositeScoreSearchCriteria(&scoringSessionStore,
            searchData->searchResults->searchCriteria, 0);

        DaosCount_t rangeHigh = searchData->rangeHigh != 0
            ? searchData->rangeHigh : cachedItems->largestIndexInUse;

        LOG_MESSAGE(INFO_LOG_MSG, "SearchId %d, thread performing search on profiles"
            " [rangeLow = %lu, rangeHigh = %lu].", searchData->searchResults->searchId, 
            searchData->rangeLow, rangeHigh);

        for(DaosCount_t profileIndex = searchData->rangeLow; profileIndex <= rangeHigh;
            profileIndex++)
        {
            if (profileCachedList[profileIndex] == NULL) {
                continue;
            }

            scoringSessionStore.profile = profileCachedList[profileIndex];
            runCompareReturnCode compareResult = runCompare(&scoringSessionStore);

            StartInlinedProfilingThreadSafe(PE_THREADED_SORT);
            if(compareResult == RCRC_NO_ERRORS && scoringSessionStore.scores.total > 0.0f)
            {
                totalProfilesScored++;
                struct CompositeScore* score
                    = (struct CompositeScore*) malloc(sizeof(struct CompositeScore));
                if(score != NULL)
                {
                    initCompositeScore(score);

                    score->role = searchData->searchResults->searchCriteria->role;
                    score->profile = scoringSessionStore.profile->id;
                    score->total = (int16_t) (scoringSessionStore.scores.total * 10000);
                    score->heuristicScore
                        = (int16_t) (scoringSessionStore.scores.heuristicScore * 10000);
                    score->companyScore
                        = (int16_t) (scoringSessionStore.scores.companyScore * 10000);
                    score->experienceScore
                        = (int16_t) (scoringSessionStore.scores.experienceScore * 10000);
                    score->skillsScore = (int16_t) (scoringSessionStore.scores.skillsScore * 10000);
                    score->jobTitleScore
                        = (int16_t) (scoringSessionStore.scores.jobTitleScore * 10000);
                    score->relevantExperience
                        = (int32_t) (scoringSessionStore.scores.relevantExperience);
                    score->srcProfile = scoringSessionStore.profile;

                    binaryHeap_pushItemAndDeleteFront(searchData->scoreList, score);
                }
            }
            EndInlinedProfilingCalcMaxThreadSafe(PE_THREADED_SORT);
        }

        // generate filter explain info
        StartInlinedProfilingThreadSafe(PE_THREADED_EXPLAIN);
        for (int32_t resIndex = binaryHeap_size(searchData->scoreList) - 1; resIndex >= 0; resIndex--)
        {
            struct CompositeScore* score = binaryHeap_getElementByIndex(searchData->scoreList, resIndex);
            if (score == NULL)
            {
                continue;
            }
            scoringSessionStore.profile = score->srcProfile;
            scoringSessionStore.scores.relevantExperience = (float)score->relevantExperience / 10000.0f;
            runCompare_explain(&scoringSessionStore);
            // take ownership of the explain info
            kv_move(score->filterExplained, scoringSessionStore.filter.filterExplained);
        }
        EndInlinedProfilingCalcMaxThreadSafe(PE_THREADED_EXPLAIN);

        pthread_cleanup_pop(1);
        pthread_cleanup_pop(1);

        freeScoringCompositeScore(&scoringSessionStore);

        pthread_cleanup_push(mutexThreadCleanup, &searchData->searchResults->mutex);

        pthread_mutex_lock(&searchData->searchResults->mutex);

        --(searchData->searchResults->numPendingScoreLists);
        searchData->searchResults->numProfilesScored += totalProfilesScored;

        if (searchData->searchResults->numPendingScoreLists == 0)
        {
            int64_t searchThreadDuration = GetProfilingStamp() - searchData->searchResults->searchThreadsStarted;
            searchData->searchResults->searchDuration += searchThreadDuration;
        }

        pthread_cleanup_pop(1);

        free(searchData);

        LOG_MESSAGE(DEBUG_LOG_MSG, "Search thread finished performing search.");
        EndInlinedProfilingCalcMaxThreadSafe(PE_THREADED_SEARCH);
    }

    return NULL;
}

static void cacheEngineThreadCleanup(void *arg)
{
    CacheEngine_t* cacheEngine = (CacheEngine_t*) arg;

    cacheEngine_unlockItemsForRead(*cacheEngine);
}

static void searchQueueThreadCleanup(void *arg)
{
    MtQueue_t* searchQueue = (MtQueue_t*) arg;

    MtQueueThreadCleanup(*searchQueue);
}

static void mutexThreadCleanup(void *arg)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*) arg;

    pthread_mutex_unlock(mutex);
}

//
// SearchResults_t
//

static SearchResults_t* initSearchResults(SearchId_t searchId,
    struct SearchCriteria* searchCriteria, DaosCount_t profilesLimit,
    CacheEngine_t profileCache, CacheEngine_t companyCache, uint16_t numScoreLists)
{
    SearchResults_t* results = malloc(sizeof(SearchResults_t));
    if(results == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(results, size = %zu bytes) failed.",
            sizeof(SearchResults_t));
        return NULL;
    }

    results->searchDuration = 0;

    results->searchId = searchId;

    results->searchCriteria = searchCriteria;
    results->profilesLimit  = profilesLimit;
    results->profileCache   = profileCache;
    results->companyCache   = companyCache;

    pthread_mutex_init(&results->mutex, NULL);
    results->completionTime = INT64_MIN;

    results->numPendingScoreLists = 0;
    results->numScoreListsInUse   = 0;

    results->scoreListsGroup = (BinaryHeap_t*) calloc(sizeof(BinaryHeap_t), numScoreLists);
    if(results->scoreListsGroup == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (scoreListsGroup, size = %zu bytes) failed.",
            sizeof(BinaryHeap_t) * numScoreLists);
        freeSearchResults(results);
        return NULL;
    }

    results->totalNumScoreLists = numScoreLists;
    results->numProfilesScored = 0;

    results->resultsJsonObj = NULL;

    return results;
}

static void freePartialSearchResults(SearchResults_t* results)
{
    for(uint16_t i = 0; i < results->numScoreListsInUse; ++i) {
        binaryHeap_free(results->scoreListsGroup[i]);
    }

    free(results->scoreListsGroup);
    results->scoreListsGroup      = NULL;
    results->numPendingScoreLists = 0;
    results->numScoreListsInUse   = 0;
    results->totalNumScoreLists   = 0;

    if(results->searchCriteria != NULL)
    {
        freeSearchCriteria(results->searchCriteria);
        free(results->searchCriteria);
        results->searchCriteria = NULL;
    }
}

static void freeSearchResults(SearchResults_t* results)
{
    freePartialSearchResults(results);

    json_object_put(results->resultsJsonObj);

    pthread_mutex_destroy(&results->mutex);

    free(results);
}

static int compareSearchResultsKeys(void* key1, void* key2)
{
    SearchId_t searchId1 = (SearchId_t) (int64_t) key1;
    SearchId_t searchId2 = (SearchId_t) (int64_t) key2;

    return searchId2 - searchId1;
}

static void freeSearchResultsItem(void* arg)
{
    SearchResults_t* searchResults = (SearchResults_t*) arg;

    freeSearchResults(searchResults);
}

static void* keyFromSearchResults(void* value)
{
    SearchResults_t* searchResults = (SearchResults_t*) value;

    return (void*) (uint64_t) searchResults->searchId;
}

//
// SearchData_t
//

static int compareSearchDataKeys(void* key1, void* key2)
{
    return (int) ((int64_t) key1 - (int64_t) key2);
}

static void freeSearchData(void* arg)
{
    SearchData_t* searchData = (SearchData_t*) arg;

    free(searchData);
}

static void* keyFromSearchData(void* value)
{
    SearchData_t* searchData = (SearchData_t*) value;

    return (void*) (int64_t) searchData->rangeLow; 
}

//
// CompositeScoreList_t
//

static int compareCompositeScoresAscendingOrder(void* arg1, void* arg2)
{
    struct CompositeScore* score1 = (struct CompositeScore*) arg1;
    struct CompositeScore* score2 = (struct CompositeScore*) arg2;

    return score1->total - score2->total;
}

static int compareCompositeScoresDescendingOrder(void* arg1, void* arg2)
{
    struct CompositeScore* score1 = (struct CompositeScore*) arg1;
    struct CompositeScore* score2 = (struct CompositeScore*) arg2;

    return score2->total - score1->total;
}

static void freeCompositeScoreItem(void* arg)
{
    struct CompositeScore* score = (struct CompositeScore*) arg;

    freeCompositeScore(score);
    free(score);
}

static void mergeCompositeScores(BinaryHeap_t resultingScoreList,
    BinaryHeap_t* scoreListsGroup, uint16_t numScoreLists)
{
    uint64_t scoresTotal = 0;

    for(uint16_t i = 0; i < numScoreLists; ++i) {
        scoresTotal += binaryHeap_size(scoreListsGroup[i]);
    }

    while(scoresTotal > binaryHeap_capacity(resultingScoreList))
    {
        uint16_t minIndex = 0;
        struct CompositeScore* minScore = NULL;

        for(uint16_t i = 0; i < numScoreLists; ++i)
        {
            if(binaryHeap_size(scoreListsGroup[i]) == 0) { continue; }

            struct CompositeScore* score = binaryHeap_getFront(scoreListsGroup[i]);
        
            if(minScore == NULL || compareCompositeScoresAscendingOrder(minScore, score) > 0)
            {
                minIndex = i;
                minScore = score;
            }
        }

        binaryHeap_deleteFront(scoreListsGroup[minIndex]);

        --scoresTotal;
    }

    for(uint16_t i = 0; i < numScoreLists; ++i)
    {
        while(binaryHeap_size(scoreListsGroup[i]))
        {
            binaryHeap_addItem(resultingScoreList, binaryHeap_getFront(scoreListsGroup[i]));
            binaryHeap_popFront(scoreListsGroup[i]);
        }
    }
}

//
// Others
//

static int mergeScoresAndGenerateJson(BinaryHeap_t* scoresGroup, uint16_t scoresGroupSize,
    DaosCount_t resultsLimit, struct json_object** resultsJsonObj)
{
    // Merge scores.

    LOG_MESSAGE(DEBUG_LOG_MSG, "Combining scores...");

    BinaryHeap_t finalScores = binaryHeap_init(compareCompositeScoresDescendingOrder,
        freeCompositeScoreItem, resultsLimit);
    if(binaryHeap_isNull(finalScores))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: binaryHeap_init() failed.");
        return 1;
    }

    mergeCompositeScores(finalScores, scoresGroup, scoresGroupSize);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Finished combining scores.");

    // Encode results.

    LOG_MESSAGE(DEBUG_LOG_MSG, "Encoding results...");

    *resultsJsonObj = json_object_new_object();
    if(*resultsJsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_object_new_object() failed.");
        binaryHeap_free(finalScores);
        return 2;
    }

    struct json_object* arrObj = json_object_new_array_ext(binaryHeap_size(finalScores));

    size_t finalScoresSize = binaryHeap_size(finalScores);
    for(uint16_t i = 0; i < finalScoresSize; ++i)
    {
        json_object_array_add(arrObj,
            marshallCompositeScore(binaryHeap_getFront(finalScores)));
        binaryHeap_deleteFront(finalScores);
    }

    binaryHeap_free(finalScores);
    json_object_object_add(*resultsJsonObj, compositeScore_Key, arrObj);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Finished encoding results.");

    return 0;
}
