#ifndef CACHE_ENGINE_H
#define CACHE_ENGINE_H

#include <daos_definitions.h>
#include <daos.h>
#include <item_functions.h>

// Types

typedef
struct
{
    void**      items;
    DaosCount_t numItems;
    DaosCount_t capacity;
    DaosCount_t largestIndexInUse;
} CachedItems_t;

typedef
struct
{
    void* c;
} CacheEngine_t;

// Constants

#define CacheEngine_NULL ((CacheEngine_t) { .c = NULL })

// Functions

CacheEngine_t cacheEngine_load(const ItemFunctions_t* itemFunctions, Daos_t daos,
    const char* directory, DaosCount_t numServers, DaosCount_t capacity, uint16_t numThreads);
void cacheEngine_free(CacheEngine_t engine);
bool cacheEngine_isNull(CacheEngine_t engine);

int cacheEngine_run(CacheEngine_t engine, uint16_t updatePeriod);
void cacheEngine_stop(CacheEngine_t engine);

CachedItems_t* cacheEngine_lockItemsForRead(CacheEngine_t engine); 
void cacheEngine_unlockItemsForRead(CacheEngine_t engine); 

int cacheEngine_storeItem(CacheEngine_t engine, const void* item, bool newItem);
bool cacheEngine_ItemIdCanStoreResult(CacheEngine_t engine, const void* item);

int cacheEngine_getInfo(CacheEngine_t engine, DaosCount_t* numPersistentItems,
    DaosId_t* minPersistentId, DaosId_t* maxPersistentId);

#endif // CACHE_ENGINE_H
