#include <cache_engine.h>
#include <logger.h>
#include <daos.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/eventfd.h>
#include <pthread.h>
#include <stdlib.h>

//
// Types
//

typedef
struct
{
    void*       item;
    DaosId_t    id;
    DaosCount_t destIndex;
} PendingItem_t;

typedef
struct
{
    PendingItem_t* items;
    DaosCount_t    numItems;
    DaosCount_t    capacity;
} PendingItemsList_t;

typedef
struct
{
    int endEvent;

    DaosCount_t numServers;

    pthread_rwlock_t itemsRWLock;
    CachedItems_t    items;

    pthread_mutex_t pendingItemsMutex;
    PendingItemsList_t pendingItems;

    ItemFunctions_t itemFunctions;
    Daos_t daos;

    DaosId_t    minPersistentId;
    DaosId_t    maxPersistentId;
    DaosCount_t persistentCount;
} CacheEngineData_t;

typedef
struct
{
    Daos_t daos;

    const char* path;
    size_t      pathLength;
    DaosCount_t numServers;

    pthread_mutex_t* mutex;
    DIR* directory;

    const ItemFunctions_t* itemFunctions;
    CachedItems_t*         items;

    DaosId_t    minPersistentId;
    DaosId_t    maxPersistentId;
    DaosCount_t persistentCount;
} CacheLoadingArg_t;

//
// Constants
//

#define MAX_PENDING_CAPACITY (16*1024)

#define ITEM_FILE_PATH_MAX_LENGTH 256

//
// Local prototypes
//

// CacheEngineData_t

static CacheEngineData_t* initCacheEngineData();
static void freeCacheEngineData(CacheEngineData_t* engineData);

// Load and save operations
 
static bool storeItemIntoCachedItems(const ItemFunctions_t* itemFunctions, CachedItems_t* items,
    void* item, DaosId_t id, DaosCount_t destIndex);
static void commitPendingItems(const ItemFunctions_t* itemFunctions, PendingItemsList_t* pendingItems,
    CachedItems_t* items);

// Cache loading

static int loadCachedItemsFromDirectory(CacheEngineData_t* engineData,
    const char* path, DaosCount_t capacity, uint16_t numThreads);
static int loadCachedItemsFromFile(Daos_t daos, const ItemFunctions_t* itemFunctions,
    const char* filename, DaosCount_t numServers, CachedItems_t* items,
    DaosId_t* minPersistentId, DaosId_t* maxPersistentId, DaosCount_t* persistentCount);
static void* loadingThread(void* arg);
static void loadingThreadCleanup(void *arg);

// CachedItems_t

static void zeroCachedItems(CachedItems_t* items);
static bool allocateCachedItems(CachedItems_t* items, uint32_t capacity);
static void freeCachedItems(const ItemFunctions_t* itemFunctions, CachedItems_t* items);

// PendingItemsList_t

static void zeroPendingItems(PendingItemsList_t* items);
static bool allocatePendingItems(PendingItemsList_t* items, uint32_t capacity);
static void freePendingItems(const ItemFunctions_t* itemFunctions, PendingItemsList_t* items);

//
// External interface
//

CacheEngine_t cacheEngine_load(const ItemFunctions_t* itemFunctions, Daos_t daos,
    const char* directory, DaosCount_t numServers, DaosCount_t capacity, uint16_t numThreads)
{
    CacheEngine_t engine = CacheEngine_NULL;

    if(itemFunctions == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for itemFunctions.");
        return engine;
    }

    if(daos_isNull(daos))
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for daos.");
        return engine;
    }

    if((numServers == 0) || (capacity == 0))
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid zero values (numServers = %lu, capacity = %lu).",
            numServers, capacity);
        return engine;
    }

    CacheEngineData_t* engineData = initCacheEngineData();
    if(engineData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: initCacheEngineData() failed.");
        return engine;
    }

    engineData->endEvent = eventfd(0, EFD_NONBLOCK);
    if(engineData->endEvent < 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: eventfd() failed. errno = %d (\"%s\").", errno, strerror(errno));
        freeCacheEngineData(engineData);
        return engine;
    }

    engineData->numServers = numServers;

    engineData->itemFunctions = *itemFunctions;
    engineData->daos = daos;

    int ret = loadCachedItemsFromDirectory(engineData, directory, capacity, numThreads);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: loadCachedItemsFromDirectory() returned %d.", ret);
        freeCacheEngineData(engineData);
        return engine;
    }

    if(!allocatePendingItems(&engineData->pendingItems, MAX_PENDING_CAPACITY))
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: allocatePendingItems(pendingItems, capacity = %lu) failed.",
            MAX_PENDING_CAPACITY);
        freeCacheEngineData(engineData);
        return engine;
    }

    engine.c = engineData;

    return engine;
}

void cacheEngine_free(CacheEngine_t engine)
{
    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;
    freeCacheEngineData(engineData);
}

bool cacheEngine_isNull(CacheEngine_t engine)
{
    return engine.c == NULL;
}

int cacheEngine_run(CacheEngine_t engine, uint16_t updatePeriod)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Started...");

    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;
    if(engineData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for engine.");
        return 1;
    }

    fd_set masterSet;

    FD_ZERO(&masterSet);
    FD_SET(engineData->endEvent, &masterSet);
    int maxSd = engineData->endEvent;

    struct timeval masterSelectTimeout = { .tv_sec = updatePeriod, .tv_usec = 0 };

    while(true)
    {
        fd_set workingSet = masterSet;
        struct timeval selectTimeout = masterSelectTimeout;

        int sd = select(maxSd + 1, &workingSet, NULL, NULL, &selectTimeout);
        if(sd < 0)
        {
            if(errno == EINTR)
            {
                LOG_MESSAGE(INFO_LOG_MSG, "select() returned EINTR.");
                break;
            }

            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: select() failed. errno = %d (\"%s\").",
                errno, strerror(errno));
            return 2;
        }
        else if(sd > 0 && FD_ISSET(engineData->endEvent, &workingSet))
        {
            uint64_t eventCounter = 0;
            read(engineData->endEvent, &eventCounter, sizeof(eventCounter));
            break;
        }

        pthread_rwlock_wrlock(&engineData->itemsRWLock);
        pthread_mutex_lock(&engineData->pendingItemsMutex);

        commitPendingItems(&engineData->itemFunctions, &engineData->pendingItems, &engineData->items);

        pthread_mutex_unlock(&engineData->pendingItemsMutex);
        pthread_rwlock_unlock(&engineData->itemsRWLock);
    }

    LOG_MESSAGE(INFO_LOG_MSG, "End");

    return 0;
}

void cacheEngine_stop(CacheEngine_t engine)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Begin.");

    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;
    if(engineData != NULL)
    {
        uint64_t eventCounter = 1;
        write(engineData->endEvent, &eventCounter, sizeof(eventCounter));
    }

    LOG_MESSAGE(INFO_LOG_MSG, "End.");
}

CachedItems_t* cacheEngine_lockItemsForRead(CacheEngine_t engine)
{
    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;

    pthread_rwlock_rdlock(&engineData->itemsRWLock);

    return &engineData->items;
}

void cacheEngine_unlockItemsForRead(CacheEngine_t engine)
{
    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;

    pthread_rwlock_unlock(&engineData->itemsRWLock);
}

bool cacheEngine_ItemIdCanStoreResult(CacheEngine_t engine, const void* item)
{
    CacheEngineData_t* engineData = (CacheEngineData_t*)engine.c;

    DaosId_t persistentItemId = engineData->itemFunctions.getPersistentItemId(item);
    DaosCount_t destIndex     = engineData->itemFunctions.generateCacheIndex(persistentItemId, engineData->numServers);

    return destIndex < engineData->items.capacity;
}

int cacheEngine_storeItem(CacheEngine_t engine, const void* item, bool isNewItem)
{
    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;
    if(engineData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for engine.");
        return 1;
    }

    DaosId_t persistentItemId = engineData->itemFunctions.getPersistentItemId(item);
    DaosCount_t destIndex     = engineData->itemFunctions.generateCacheIndex(persistentItemId, engineData->numServers);
    void* cachedItem          = NULL;

    pthread_rwlock_wrlock(&engineData->itemsRWLock);

    if((persistentItemId < engineData->minPersistentId) || (engineData->minPersistentId == 0)) {
        engineData->minPersistentId = persistentItemId;
    }

    if(persistentItemId > engineData->maxPersistentId) { engineData->maxPersistentId = persistentItemId; }
    if(isNewItem == true) { ++engineData->persistentCount; }

    pthread_rwlock_unlock(&engineData->itemsRWLock);

    if(engineData->itemFunctions.persistentItemShouldBeCached(item) == true)
    {
        cachedItem = engineData->itemFunctions.generateCachedItem(item);
        if(cachedItem == NULL)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - generateCachedItem() failed.", persistentItemId);
            return 2;
        }
    }
    else
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Item (id = %lu) will not generate a cached item.", persistentItemId);
    }

    pthread_mutex_lock(&engineData->pendingItemsMutex);

    if(engineData->pendingItems.numItems == engineData->pendingItems.capacity)
    {
        pthread_mutex_unlock(&engineData->pendingItemsMutex);
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - pendingItems is full (capacity = %lu).",
            persistentItemId, engineData->pendingItems.capacity);
        engineData->itemFunctions.freeCachedItem(cachedItem);
        return 3;
    }

    engineData->pendingItems.items[engineData->pendingItems.numItems].item      = cachedItem;
    engineData->pendingItems.items[engineData->pendingItems.numItems].id        = persistentItemId;
    engineData->pendingItems.items[engineData->pendingItems.numItems].destIndex = destIndex;
    ++(engineData->pendingItems.numItems);

    pthread_mutex_unlock(&engineData->pendingItemsMutex);

    return 0;
}

int cacheEngine_getInfo(CacheEngine_t engine, DaosCount_t* numPersistentItems,
    DaosId_t* minPersistentId, DaosId_t* maxPersistentId)
{
    CacheEngineData_t* engineData = (CacheEngineData_t*) engine.c;
    if(engineData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for engine.");
        return 1;
    }

    pthread_rwlock_rdlock(&engineData->itemsRWLock);

    *numPersistentItems = engineData->persistentCount;
    *minPersistentId    = engineData->minPersistentId;
    *maxPersistentId    = engineData->maxPersistentId;

    pthread_rwlock_unlock(&engineData->itemsRWLock);

    return 0;
}

//
// Local functions
//

//
// CacheEngineData_t
//

static CacheEngineData_t* initCacheEngineData()
{
    CacheEngineData_t* engineData = (CacheEngineData_t*) malloc(sizeof(CacheEngineData_t));
    if(engineData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(size = %zu bytes) failed.", sizeof(CacheEngineData_t));
        return engineData;
    }

    engineData->endEvent   = -1;
    engineData->numServers = 0;

    pthread_rwlock_init(&engineData->itemsRWLock, NULL);
    zeroCachedItems(&engineData->items);

    pthread_mutex_init(&engineData->pendingItemsMutex, NULL);
    zeroPendingItems(&engineData->pendingItems);

    engineData->itemFunctions = ITEM_FUNCTIONS_ZERO_VALUE;
    engineData->daos          = Daos_NULL;

    engineData->minPersistentId = 0;
    engineData->maxPersistentId = 0;
    engineData->persistentCount = 0;

    return engineData;
}

static void freeCacheEngineData(CacheEngineData_t* engineData)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    if(engineData != NULL)
    {
        freePendingItems(&engineData->itemFunctions, &engineData->pendingItems);
        pthread_mutex_destroy(&(engineData->pendingItemsMutex));
        freeCachedItems(&engineData->itemFunctions, &engineData->items);
        pthread_rwlock_destroy(&(engineData->itemsRWLock));

        if(engineData->endEvent < 0) { close(engineData->endEvent); }
        free(engineData);
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
}

//
// Load and save operations
//

static bool storeItemIntoCachedItems(const ItemFunctions_t* itemFunctions, CachedItems_t* items,
    void* item, DaosId_t id, DaosCount_t destIndex)
{
    if(destIndex >= items->capacity)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: index (%lu) for id (%lu) is too large for "
            "cached items (capacity = %lu).", destIndex, id, items->capacity);
        return false;
    }

    if(items->items[destIndex] != NULL)
    {
        itemFunctions->freeCachedItem(items->items[destIndex]);
        --(items->numItems);
    }

    items->items[destIndex] = item;
    ++(items->numItems);

    if(destIndex > items->largestIndexInUse) { items->largestIndexInUse = destIndex; }

    return true;
}

static void commitPendingItems(const ItemFunctions_t* itemFunctions,
    PendingItemsList_t* pendingItems, CachedItems_t* items)
{
    while(pendingItems->numItems > 0)
    {
        DaosCount_t index = pendingItems->numItems - 1;

        PendingItem_t* pendingItem = &pendingItems->items[index];

        if(!storeItemIntoCachedItems(itemFunctions, items, pendingItem->item, pendingItem->id, pendingItem->destIndex))
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: storeItemIntoCachedItems() failed."
               " Discarding cached item (id = %lu).", pendingItem->id);
            itemFunctions->freeCachedItem(pendingItem->item);
        }

        pendingItem->item      = NULL;
        pendingItem->id        = 0;
        pendingItem->destIndex = 0;

        --(pendingItems->numItems);
    }
}

//
// Cache loading
//

static int loadCachedItemsFromDirectory(CacheEngineData_t* engineData,
    const char* path, DaosCount_t capacity, uint16_t numThreads)
{
    LOG_MESSAGE(INFO_LOG_MSG, "directory = \"%s\", number of loading threads = %hu - Begin.", path, numThreads);

    // Prepare resources.

    size_t pathLength = strlen(path);

    // Considering last '/' plus one character for filename.
    if(pathLength >= ITEM_FILE_PATH_MAX_LENGTH - 2)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: directory's (\"%s\") length (%zu) is greater than maximum (%z).",
            path, pathLength, ITEM_FILE_PATH_MAX_LENGTH - 2);
        return 1;
    }

    DIR* directory = opendir(path);
    if(directory == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: opendir(\"%s\") failed. errno = %d (\"%s\").",
            path, errno, strerror(errno));
        return 2;
    }

    if(!allocateCachedItems(&engineData->items, capacity))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: allocateCachedItems() failed.");
        closedir(directory);
        return 3;
    }

    pthread_mutex_t loadingMutex;

    pthread_mutex_init(&loadingMutex, NULL);

    pthread_t* threadIds = (pthread_t*) malloc(numThreads * sizeof(pthread_t));
    if(threadIds == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(threadIds, size = %zu bytes) failed.",
            numThreads * sizeof(pthread_t));
        freeCachedItems(&engineData->itemFunctions, &engineData->items);
        closedir(directory);
        return 4;
    }

    CacheLoadingArg_t* loadingArgs = (CacheLoadingArg_t*) malloc(numThreads * sizeof(CacheLoadingArg_t));
    if(loadingArgs == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(loadingArgs, size = %zu bytes) failed.",
            numThreads * sizeof(CacheLoadingArg_t));
        free(threadIds);
        freeCachedItems(&engineData->itemFunctions, &engineData->items);
        closedir(directory);
        return 5;
    }

    // Create loading threads.

    for(uint16_t i = 0; i < numThreads; ++i)
    {
        loadingArgs[i] = (CacheLoadingArg_t) { .daos = engineData->daos, .path = path, .pathLength = pathLength,
            .numServers = engineData->numServers, .mutex = &loadingMutex, .directory = directory,
            .itemFunctions = &engineData->itemFunctions, .items = &engineData->items,
            .minPersistentId = 0, .maxPersistentId = 0, .persistentCount = 0
        };

        int createRet = pthread_create(&threadIds[i], NULL, loadingThread, &loadingArgs[i]);
        if(createRet != 0) 
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: pthread_create(thread = %hu) returned %d."
                " errno = %d (\"%s\").", i, createRet, errno, strerror(errno));

            for(uint16_t j = 0; j < i; ++j)
            {
                pthread_cancel(threadIds[j]);
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Loading thread %hu cancelled.", j);
            }

            for(uint16_t j = 0; j < i; ++j)
            {
                pthread_join(threadIds[j], NULL);
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Loading thread %hu joined.", j);
            }

            free(loadingArgs);
            free(threadIds);
            freeCachedItems(&engineData->itemFunctions, &engineData->items);
            closedir(directory);
            return 6;
        }
    }

    // Wait for threads to finish their work.

    int ret = 0;

    for(uint16_t i = 0; i < numThreads; ++i)
    {
        void* threadRet = NULL;

        pthread_join(threadIds[i], &threadRet);
        LOG_MESSAGE(INFO_LOG_MSG, "Loading thread %hu joined.", i);

        if(threadRet != NULL)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: loading thread %hu returned %d.", i, (int) (intptr_t) threadRet);
            ret = 7;
        }
        else
        {
            if((loadingArgs[i].minPersistentId < engineData->minPersistentId) || (engineData->minPersistentId == 0)) {
                engineData->minPersistentId = loadingArgs[i].minPersistentId;
            }

            if(loadingArgs[i].maxPersistentId > engineData->maxPersistentId) {
                engineData->maxPersistentId = loadingArgs[i].maxPersistentId;
            }

            engineData->persistentCount += loadingArgs[i].persistentCount;
        }
    }

    free(loadingArgs);
    free(threadIds);
    closedir(directory);

    if(ret == 0)
    {
        LOG_MESSAGE(INFO_LOG_MSG, "End.");
    }
    else
    {
        freeCachedItems(&engineData->itemFunctions, &engineData->items);
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Failed.");
    }

    return ret;
}

static int loadCachedItemsFromFile(Daos_t daos, const ItemFunctions_t* itemFunctions,
    const char* filename, DaosCount_t numServers, CachedItems_t* items,
    DaosId_t* minPersistentId, DaosId_t* maxPersistentId, DaosCount_t* persistentCount)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Loading file \"%s\".", filename);

    void** itemsFromFile         = NULL;
    DaosCount_t numItemsFromFile = 0;

    int ret = daos_loadAllCachedItemsFromFile(daos, filename, &itemsFromFile, &numItemsFromFile);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_loadAllItemsFromFile(\"%s\") returned %d.", filename, ret);
        return 1;
    }

    for(DaosCount_t i = 0; i < numItemsFromFile; ++i)
    {
        if(itemsFromFile[i] != NULL)
        {
            DaosId_t    itemId    = itemFunctions->getCachedItemId(itemsFromFile[i]);
            DaosCount_t destIndex = itemFunctions->generateCacheIndex(itemId, numServers);

            if(!storeItemIntoCachedItems(itemFunctions, items, itemsFromFile[i], itemId, destIndex))
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: storeItemIntoCachedItems(id = %lu) failed.", itemId);
                itemFunctions->freeCachedList(itemsFromFile, numItemsFromFile);
                return 2;
            }
            
            if((itemId < *minPersistentId) || (*minPersistentId == 0)) {
                *minPersistentId = itemId;
            }

            if(itemId > *maxPersistentId) { *maxPersistentId = itemId; }

            itemsFromFile[i] = NULL;
        }
    }
    
    *persistentCount += numItemsFromFile;

    itemFunctions->freeCachedList(itemsFromFile, numItemsFromFile);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Finished loading file.");

    return 0;
}

#define NUM_FILES_TO_LOAD 100

static void* loadingThread(void* arg)
{
    CacheLoadingArg_t* loadingArg = (CacheLoadingArg_t*) arg;

    LOG_MESSAGE(INFO_LOG_MSG, "Cache loading (path = \"%s\") started...", loadingArg->path);

    DaosCount_t numFilesLoaded = 0;

    // Preparing path.

    char filesToLoad[NUM_FILES_TO_LOAD][ITEM_FILE_PATH_MAX_LENGTH + 1];

    char endSlash = '\0';
    size_t pathLength = loadingArg->pathLength;

    if(loadingArg->path[pathLength - 1] != '/')
    {
        endSlash = '/';
        ++pathLength;
    }

    for(uint16_t i = 0; i < NUM_FILES_TO_LOAD; ++i)
    {
        strcpy(&filesToLoad[i][0], loadingArg->path);
        filesToLoad[i][loadingArg->pathLength]     = endSlash;
        filesToLoad[i][loadingArg->pathLength + 1] = '\0';
    }

    size_t filenameMaxLength = ITEM_FILE_PATH_MAX_LENGTH - pathLength;

    // Processing files.

    bool finishedReadingDir = false;

    while(finishedReadingDir == false)
    {
        intptr_t readingDirRet = 0;
        uint16_t filesToLoadCount = 0;

        pthread_cleanup_push(loadingThreadCleanup, loadingArg->mutex);
        pthread_mutex_lock(loadingArg->mutex);

        while(filesToLoadCount < NUM_FILES_TO_LOAD)
        {
            struct dirent* dirEntry = readdir(loadingArg->directory);
            if(dirEntry == NULL)
            {
                finishedReadingDir = true;
                break;
            }

            if(loadingArg->itemFunctions->isItemFile(dirEntry->d_name))
            {
                size_t filenameLength = strlen(dirEntry->d_name);

                if(filenameLength <= filenameMaxLength)
                {
                    strcpy(&filesToLoad[filesToLoadCount][pathLength], dirEntry->d_name);
                    ++filesToLoadCount;
                }
                else
                {
                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: full path's length (%zu) for"
                        " file name (\"%s\") is greater than maximum (%zu).", dirEntry->d_name,
                        pathLength + filenameLength, ITEM_FILE_PATH_MAX_LENGTH);
                    readingDirRet = 1;
                }
            }
        }

        pthread_cleanup_pop(1);

        if(readingDirRet != 0) { return (void*) readingDirRet; }

        for(uint16_t i = 0; i < filesToLoadCount; ++i)
        {
            int ret = loadCachedItemsFromFile(loadingArg->daos, loadingArg->itemFunctions,
                &filesToLoad[i][0], loadingArg->numServers, loadingArg->items,
                &loadingArg->minPersistentId, &loadingArg->maxPersistentId, &loadingArg->persistentCount);
            if(ret != 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: loadCachedItemsFromFile(\"%s\") returned %d.",
                    filesToLoad[i], ret);
                return (void*) 2;
            }
        }

        numFilesLoaded += filesToLoadCount;
    }

    LOG_MESSAGE(INFO_LOG_MSG, "Cache loading finished. Loaded %lu files.", numFilesLoaded);

    return NULL;
}

static void loadingThreadCleanup(void *arg)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*) arg;

    pthread_mutex_unlock(mutex);
}

//
// CachedItems_t
//

static void zeroCachedItems(CachedItems_t* items)
{
    items->items             = NULL;
    items->numItems          = 0;
    items->capacity          = 0;
    items->largestIndexInUse = 0;
}

static bool allocateCachedItems(CachedItems_t* items, uint32_t capacity)
{
    zeroCachedItems(items);

    items->items = (void**) calloc(capacity, sizeof(void*));
    if(items->items == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: calloc(capacity = %lu, item size = %zu bytes) failed.",
            capacity, sizeof(void*));
        return false;
    }

    items->capacity = capacity;

    return true;
}

static void freeCachedItems(const ItemFunctions_t* itemFunctions, CachedItems_t* items)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    if(items != NULL)
    {
        for(DaosCount_t i = 0; i < items->capacity; ++i) {
            itemFunctions->freeCachedItem(items->items[i]);
        }

        free(items->items);
        zeroCachedItems(items);
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
}

//
// PendingItemsList_t
//

static void zeroPendingItems(PendingItemsList_t* items)
{
    items->items    = NULL;
    items->numItems = 0;
    items->capacity = 0;
}

static bool allocatePendingItems(PendingItemsList_t* items, uint32_t capacity)
{
    zeroPendingItems(items);

    items->items = (PendingItem_t*) calloc(capacity, sizeof(PendingItem_t));
    if(items->items == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: calloc(capacity = %lu, item size = %zu bytes)"
            " failed.", capacity, sizeof(PendingItem_t));
        return false;
    }

    items->capacity = capacity;

    return true;
}

static void freePendingItems(const ItemFunctions_t* itemFunctions, PendingItemsList_t* items)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    if(items != NULL)
    {
        for(DaosCount_t i = 0; i < items->capacity; ++i)
        {
            itemFunctions->freeCachedItem(items->items[i].item);
            items->items[i].item      = NULL;
            items->items[i].id        = 0;
            items->items[i].destIndex = 0;
        }

        free(items->items);
        zeroPendingItems(items);
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
}
