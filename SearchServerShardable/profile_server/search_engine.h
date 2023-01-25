#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <search_criteria.h>
#include <composite_score.h>
#include <cache_engine.h>
#include <binary_heap.h>
#include <json_object.h>
#include <stdint.h>

// Types

typedef
struct
{
    void* s;
} SearchEngine_t;

typedef uint32_t SearchId_t;

// Constants

#define SearchEngine_NULL ((SearchEngine_t) { .s = NULL })

// Macros

#define SEARCH_ID_TO_NETWORK htonl
#define NETWORK_TO_SEARCH_ID ntohl

// Search engine

SearchEngine_t searchEngine_init();
int searchEngine_start(SearchEngine_t searchEngine, CacheEngine_t profileCache,
    CacheEngine_t companyCache, uint16_t numThreads, uint16_t searchResultsExpirationInMinutes);
void searchEngine_stop(SearchEngine_t searchEngine);
bool searchEngine_isNull(SearchEngine_t searchEngine);

// Searches

int searchEngine_startSearch(SearchEngine_t searchEngine, struct SearchCriteria* searchCriteria,
    DaosCount_t profilesLimit, SearchId_t* searchId, int64_t searchDuration);
int searchEngine_getSearchStatus(SearchEngine_t searchEngine, SearchId_t searchId,
    uint8_t* completionPercentage);
int searchEngine_getSearchResults(SearchEngine_t searchEngine, SearchId_t searchId,
    struct json_object** resultsJsonObj);
int searchEngine_removeSearchResults(SearchEngine_t searchEngine, SearchId_t searchId);

#endif // SEARCH_ENGINE_H
