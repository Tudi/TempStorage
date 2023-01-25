#include <cache_engine.h>
#include <daos.h>
#include <logger.h>
#include <company_cached_comparison.h>
#include <company_test_data.h>
#include <company_functions.h>
#include <company_definitions.h>
#include <assert_mt.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

//
// Constants
//

static const char* companyDirectory      = "./";
static const DaosCount_t numServers      = 2;
static const DaosCount_t numItemsPerFile = 24;
static const uint16_t updatePeriodInSecs = 2;
static const DaosCount_t cacheCapacity   = 24;
static const uint16_t numThreads         = 1;

//
// Variables
//

static Daos_t companyDaos = Daos_NULL;
static CacheEngine_t companyCacheEngine = CacheEngine_NULL;

static struct Company company1, company2;
static struct CompanyCached companyCached1, companyCached2;

static pthread_t serverThreadId, clientThreadId;

//
// Prototypes
//

int test_CompanyCacheEngine_succeeds();
void test_CompanyCacheEngine_setUp();
void test_CompanyCacheEngine_cleanup();

void* serverThread(void* arg);
void* clientThread(void* arg);

void serverThreadCleanupHandler(void *arg);
void clientThreadCleanupHandler(void *arg);

//
// Tests
//

int main(int argc, char* argv[])
{
    atexit(test_CompanyCacheEngine_cleanup);

    return test_CompanyCacheEngine_succeeds();
}

int test_CompanyCacheEngine_succeeds()
{
    LOG_MESSAGE(INFO_LOG_MSG, "Test started...");

    test_CompanyCacheEngine_setUp();

    companyDaos = daos_init(COMPANY_DAOS_VERSION, "./", COMPANY_FILE_PREFIX, COMPANY_FILE_EXTENSION,
        COMPANY_NUM_ID_DIGITS, numItemsPerFile, getCompanyFunctions());
    ASSERT_MT_INT_EQUAL(0, daos_isNull(companyDaos));

    bool isNewItem = false;

    ASSERT_MT_INT_EQUAL(0, daos_saveItem(companyDaos, &company1, &isNewItem));
    ASSERT_MT_TRUE(isNewItem);

    companyCacheEngine = cacheEngine_load(getCompanyFunctions(), companyDaos,
        companyDirectory, numServers, cacheCapacity, numThreads);
    ASSERT_MT_INT_EQUAL(0, cacheEngine_isNull(companyCacheEngine));

    LOG_MESSAGE(INFO_LOG_MSG, "Starting server thread...");
    ASSERT_MT_INT_EQUAL(0, pthread_create(&serverThreadId, NULL, serverThread, NULL));
    LOG_MESSAGE(INFO_LOG_MSG, "Started server thread.");

    LOG_MESSAGE(INFO_LOG_MSG, "Starting client thread...");
    ASSERT_MT_INT_EQUAL(0, pthread_create(&clientThreadId, NULL, clientThread, NULL));
    LOG_MESSAGE(INFO_LOG_MSG, "Started client thread.");

    sleep(updatePeriodInSecs + 2);

    LOG_MESSAGE(INFO_LOG_MSG, programError ? "Test failed." : "Test passed.");

    return programError ? EINVAL : 0;
}

void test_CompanyCacheEngine_setUp()
{
    initCompany(&company1);
    initCompany(&company2);

    initTestData_Company1(&company1);
    initTestData_Company2(&company2);

    initCompanyCached(&companyCached1);
    initCompanyCached(&companyCached2);

    initTestData_CompanyCached1(&companyCached1);
    initTestData_CompanyCached2(&companyCached2);
}

void test_CompanyCacheEngine_cleanup()
{
    cacheEngine_stop(companyCacheEngine);

    pthread_join(clientThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread joined.");
    pthread_join(serverThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread joined.");

    cacheEngine_free(companyCacheEngine);
    daos_free(companyDaos);

    freeCompanyCached(&companyCached2);
    freeCompanyCached(&companyCached1);

    freeCompany(&company2);
    freeCompany(&company1);
}

void* serverThread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread - begin.");

    pthread_cleanup_push(serverThreadCleanupHandler, NULL);

    int ret = cacheEngine_run(companyCacheEngine, updatePeriodInSecs);
    ASSERT_MT_INT_EQUAL(0, ret);

    pthread_cleanup_pop(1);

    return NULL;
}

void* clientThread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread - begin.");

    // Check statistics.

    DaosCount_t numPersistentItems = 0;
    DaosId_t minPersistentId       = INT32_MAX;
    DaosId_t maxPersistentId       = 0;

    ASSERT_MT_INT_EQUAL(0, cacheEngine_getInfo(companyCacheEngine, &numPersistentItems,
        &minPersistentId, &maxPersistentId));

    ASSERT_MT_INT_EQUAL(1, numPersistentItems);
    ASSERT_MT_INT_EQUAL(company1.id, minPersistentId);
    ASSERT_MT_INT_EQUAL(company1.id, maxPersistentId);

    // Store items.

    ASSERT_MT_INT_EQUAL(0, cacheEngine_storeItem(companyCacheEngine, &company2, true));
    sleep(updatePeriodInSecs + 1);

    pthread_cleanup_push(clientThreadCleanupHandler, NULL);

    CachedItems_t* cachedItems = cacheEngine_lockItemsForRead(companyCacheEngine);
    struct CompanyCached** companyCachedList = (struct CompanyCached**) cachedItems->items;

    DaosCount_t cacheIndex1 = company1.id;
    ASSERT_MT_TRUE(cacheIndex1 < cacheCapacity);

    DaosCount_t cacheIndex2 = company2.id;
    ASSERT_MT_TRUE(cacheIndex2 < cacheCapacity);

    for(DaosCount_t i = 0; i < cachedItems->capacity; ++i)
    {
        if((i != cacheIndex1) && (i != cacheIndex2)) {
            ASSERT_MT_NULL(companyCachedList[i]);
        } else {
            ASSERT_MT_NOT_NULL(companyCachedList[i]);
        }
    }

    assert_CompanyCached_equal(&companyCached1, companyCachedList[cacheIndex1]);
    assert_CompanyCached_equal(&companyCached2, companyCachedList[cacheIndex2]);

    // Check if cache would detect an ID too large for insertion.

    uint32_t idBackup = company1.id;
    company1.id = cacheCapacity;
    ASSERT_MT_FALSE(cacheEngine_ItemIdCanStoreResult(companyCacheEngine, &company1));

    company1.id = cacheCapacity - 1;
    ASSERT_MT_TRUE(cacheEngine_ItemIdCanStoreResult(companyCacheEngine, &company1));
    company1.id = idBackup;

    // Check statistics.

    numPersistentItems = 0;
    minPersistentId    = INT32_MAX;
    maxPersistentId    = 0;

    ASSERT_MT_INT_EQUAL(0, cacheEngine_getInfo(companyCacheEngine, &numPersistentItems,
        &minPersistentId, &maxPersistentId));

    ASSERT_MT_INT_EQUAL(2, numPersistentItems);
    ASSERT_MT_INT_EQUAL(company1.id, minPersistentId);
    ASSERT_MT_INT_EQUAL(company2.id, maxPersistentId);

    pthread_cleanup_pop(1);

    return NULL;
}

void serverThreadCleanupHandler(void *arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread - end.");
}

void clientThreadCleanupHandler(void *arg)
{
    cacheEngine_unlockItemsForRead(companyCacheEngine);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread - end.");
}
