#include <cache_engine.h>
#include <daos.h>
#include <logger.h>
#include <profile_cached_comparison.h>
#include <profile_test_data.h>
#include <profile_functions.h>
#include <profile_definitions.h>
#include <assert_mt.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

//
// Constants
//

static const char* profileDirectory      = "./";
static const DaosCount_t numServers      = 2;
static const DaosCount_t numItemsPerFile = 24;
static const uint16_t updatePeriodInSecs = 2;
static const DaosCount_t cacheCapacity   = 14;
static const uint16_t numThreads         = 1;

//
// Variables
//

static Daos_t profileDaos = Daos_NULL;
static CacheEngine_t profileCacheEngine = CacheEngine_NULL;

static struct ProfilePersistent profilePersistent1, profilePersistent2, profilePersistent3,
    profilePersistent4, profilePersistent5;
static struct ProfileCached profileCached1, profileCached2, profileCached5;

static pthread_t serverThreadId, clientThreadId;

//
// Prototypes
//

int test_ProfileCacheEngine_succeeds();
void test_ProfileCacheEngine_setUp();
void test_ProfileCacheEngine_cleanup();

void* serverThread(void* arg);
void* clientThread(void* arg);

void serverThreadCleanupHandler(void *arg);
void clientThreadCleanupHandler(void *arg);

//
// Tests
//

int main(int argc, char* argv[])
{
    atexit(test_ProfileCacheEngine_cleanup);

    return test_ProfileCacheEngine_succeeds();
}

int test_ProfileCacheEngine_succeeds()
{
    LOG_MESSAGE(INFO_LOG_MSG, "Test started...");

    test_ProfileCacheEngine_setUp();

    profileDaos = daos_init(PROFILE_DAOS_VERSION, "./", PROFILE_FILE_PREFIX, PROFILE_FILE_EXTENSION,
        PROFILE_NUM_ID_DIGITS, numItemsPerFile, getProfileFunctions());
    ASSERT_MT_INT_EQUAL(0, daos_isNull(profileDaos));

    bool isNewItem = false;

    ASSERT_MT_INT_EQUAL(0, daos_saveItem(profileDaos, &profilePersistent1, &isNewItem));
    ASSERT_MT_TRUE(isNewItem);

    isNewItem = false;
    ASSERT_MT_INT_EQUAL(0, daos_saveItem(profileDaos, &profilePersistent3, &isNewItem));
    ASSERT_MT_TRUE(isNewItem);

    isNewItem = false;
    ASSERT_MT_INT_EQUAL(0, daos_saveItem(profileDaos, &profilePersistent5, &isNewItem));
    ASSERT_MT_TRUE(isNewItem);

    profileCacheEngine = cacheEngine_load(getProfileFunctions(), profileDaos,
        profileDirectory, numServers, cacheCapacity, numThreads);
    ASSERT_MT_INT_EQUAL(0, cacheEngine_isNull(profileCacheEngine));

    // Check statistics.

    DaosCount_t numPersistentItems = 0;
    DaosId_t minPersistentId       = INT32_MAX;
    DaosId_t maxPersistentId       = 0;

    ASSERT_MT_INT_EQUAL(0, cacheEngine_getInfo(profileCacheEngine, &numPersistentItems,
        &minPersistentId, &maxPersistentId));

    ASSERT_MT_INT_EQUAL(3, numPersistentItems);
    ASSERT_MT_INT_EQUAL(profilePersistent1.id, minPersistentId);
    ASSERT_MT_INT_EQUAL(profilePersistent5.id, maxPersistentId);

    // Run server.

    LOG_MESSAGE(INFO_LOG_MSG, "Starting server thread...");
    ASSERT_MT_INT_EQUAL(0, pthread_create(&serverThreadId, NULL,
        serverThread, NULL));
    LOG_MESSAGE(INFO_LOG_MSG, "Started server thread.");

    // Run client.

    LOG_MESSAGE(INFO_LOG_MSG, "Starting client thread...");
    ASSERT_MT_INT_EQUAL(0, pthread_create(&clientThreadId, NULL, clientThread, NULL));
    LOG_MESSAGE(INFO_LOG_MSG, "Started client thread.");

    sleep(updatePeriodInSecs + 4);

    LOG_MESSAGE(INFO_LOG_MSG, programError ? "Test failed." : "Test passed.");

    return programError ? EINVAL : 0;
}

void test_ProfileCacheEngine_setUp()
{
    initProfilePersistent(&profilePersistent1);
    initProfilePersistent(&profilePersistent2);
    initProfilePersistent(&profilePersistent3);
    initProfilePersistent(&profilePersistent4);
    initProfilePersistent(&profilePersistent5);

    initTestData_ProfilePersistent1(&profilePersistent1);
    initTestData_ProfilePersistent2(&profilePersistent2);

    initTestData_ProfilePersistent3(&profilePersistent3);
    profilePersistent3.logicalDelete = true;
    initTestData_ProfilePersistent4(&profilePersistent4);
    profilePersistent4.logicalDelete = true;
    initTestData_ProfilePersistent5(&profilePersistent5);

    initProfileCached(&profileCached1);
    initProfileCached(&profileCached2);
    initProfileCached(&profileCached5);

    initTestData_ProfileCached1(&profileCached1);
    initTestData_ProfileCached2(&profileCached2);
    initTestData_ProfileCached5(&profileCached5);
}

void test_ProfileCacheEngine_cleanup()
{
    cacheEngine_stop(profileCacheEngine);

    pthread_join(clientThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread joined.");
    pthread_join(serverThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread joined.");

    cacheEngine_free(profileCacheEngine);
    daos_free(profileDaos);

    freeProfileCached(&profileCached5);
    freeProfileCached(&profileCached2);
    freeProfileCached(&profileCached1);

    freeProfilePersistent(&profilePersistent5);
    freeProfilePersistent(&profilePersistent4);
    freeProfilePersistent(&profilePersistent3);
    freeProfilePersistent(&profilePersistent2);
    freeProfilePersistent(&profilePersistent1);
}

void* serverThread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread - begin.");

    pthread_cleanup_push(serverThreadCleanupHandler, NULL);

    int ret = cacheEngine_run(profileCacheEngine, updatePeriodInSecs);
    ASSERT_MT_INT_EQUAL(0, ret);

    pthread_cleanup_pop(1);

    return NULL;
}

void* clientThread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread - begin.");

    // Ensure that all cache indexes fall within the maximum range.

    DaosCount_t cacheIndex1 = profilePersistent1.id / numServers;
    ASSERT_MT_TRUE(cacheIndex1 < cacheCapacity);

    DaosCount_t cacheIndex2 = profilePersistent2.id / numServers;
    ASSERT_MT_TRUE(cacheIndex2 < cacheCapacity);

    DaosCount_t cacheIndex3 = profilePersistent3.id / numServers;
    ASSERT_MT_TRUE(cacheIndex3 < cacheCapacity);

    DaosCount_t cacheIndex4 = profilePersistent4.id / numServers;
    ASSERT_MT_TRUE(cacheIndex4 < cacheCapacity);

    DaosCount_t cacheIndex5 = profilePersistent5.id / numServers;
    ASSERT_MT_TRUE(cacheIndex5 < cacheCapacity);

    // Test if all profiles were loaded.

    pthread_cleanup_push(clientThreadCleanupHandler, NULL);

    CachedItems_t* cachedItems = cacheEngine_lockItemsForRead(profileCacheEngine);
    struct ProfileCached** profileCachedList = (struct ProfileCached**) cachedItems->items;

    ASSERT_MT_NOT_NULL(profileCachedList[cacheIndex1]);
    assert_ProfileCached_equal(&profileCached1, profileCachedList[cacheIndex1]);
    ASSERT_MT_NOT_NULL(profileCachedList[cacheIndex5]);
    assert_ProfileCached_equal(&profileCached5, profileCachedList[cacheIndex5]);

    pthread_cleanup_pop(1);

    // Save new profiles.

    ASSERT_MT_INT_EQUAL(0, cacheEngine_storeItem(profileCacheEngine, &profilePersistent2, true));
    ASSERT_MT_INT_EQUAL(0, cacheEngine_storeItem(profileCacheEngine, &profilePersistent4, true));

    // Update profile to be deleted.

    profilePersistent5.logicalDelete = true;
    ASSERT_MT_INT_EQUAL(0, cacheEngine_storeItem(profileCacheEngine, &profilePersistent5, false));

    sleep(updatePeriodInSecs + 1);

    // Test all entries in the cache.

    pthread_cleanup_push(clientThreadCleanupHandler, NULL);

    CachedItems_t* cachedItems = cacheEngine_lockItemsForRead(profileCacheEngine);
    struct ProfileCached** profileCachedList = (struct ProfileCached**) cachedItems->items;

    for(DaosCount_t i = 0; i < cachedItems->capacity; ++i)
    {
        if((i == cacheIndex1) || (i == cacheIndex2)) {
            ASSERT_MT_NOT_NULL(profileCachedList[i]);
        } else if((i == cacheIndex3) || (i == cacheIndex4) || (i == cacheIndex5)) {
            // Making it clear that they shouldn't be cached.
            ASSERT_MT_NULL(profileCachedList[i]);
        } else {
            ASSERT_MT_NULL(profileCachedList[i]);
        }
    }

    assert_ProfileCached_equal(&profileCached1, profileCachedList[cacheIndex1]);
    assert_ProfileCached_equal(&profileCached2, profileCachedList[cacheIndex2]);

    // Check if cache would detect an ID too large for insertion.

    uint32_t idBackup = profilePersistent1.id;
    profilePersistent1.id = cacheCapacity * numServers;
    ASSERT_MT_FALSE(cacheEngine_ItemIdCanStoreResult(profileCacheEngine, &profilePersistent1));

    profilePersistent1.id = cacheCapacity * numServers - 1;
    ASSERT_MT_TRUE(cacheEngine_ItemIdCanStoreResult(profileCacheEngine, &profilePersistent1));
    profilePersistent1.id = idBackup;

    // Check statistics.

    DaosCount_t numPersistentItems = 0;
    DaosId_t minPersistentId       = INT32_MAX;
    DaosId_t maxPersistentId       = 0;

    ASSERT_MT_INT_EQUAL(0, cacheEngine_getInfo(profileCacheEngine, &numPersistentItems,
        &minPersistentId, &maxPersistentId));

    ASSERT_MT_INT_EQUAL(5, numPersistentItems);
    ASSERT_MT_INT_EQUAL(profilePersistent1.id, minPersistentId);
    ASSERT_MT_INT_EQUAL(profilePersistent5.id, maxPersistentId);

    pthread_cleanup_pop(1);

    return NULL;
}

void serverThreadCleanupHandler(void *arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread - end.");
}

void clientThreadCleanupHandler(void *arg)
{
    cacheEngine_unlockItemsForRead(profileCacheEngine);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread - end.");
}
