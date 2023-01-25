#include <daos.h>
#include <profile_persistent.h>
#include <profile_persistent_comparison.h>
#include <profile_cached_comparison.h>
#include <profile_test_data.h>
#include <profile_functions.h>
#include <profile_definitions.h>
#include <company.h>
#include <company_cached.h>
#include <company_comparison.h>
#include <company_cached_comparison.h>
#include <company_test_data.h>
#include <company_functions.h>
#include <company_definitions.h>
#include <utils.h>
#include <assert_mt.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h> 
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

bool programError = false;

static Daos_t profileDaos = Daos_NULL;
static Daos_t companyDaos = Daos_NULL;

int test_daos_V7_setUp(void** state)
{
    profileDaos = Daos_NULL;
    companyDaos = Daos_NULL;

    return 0;
}

int test_daos_V7_tearDown(void** state)
{
    daos_free(profileDaos);
    daos_free(companyDaos);

    return 0;
}

static const DaosCount_t numItemsPerFile = 24;

static void test_daos_V7_detect_convert_company_succeeds()
{
    static struct Company company1;
    static struct CompanyCached companyCached1;
    initCompany(&company1);
    initTestData_Company1(&company1);
    initCompanyCached(&companyCached1);
    initTestData_CompanyCached1(&companyCached1);

    // copy a V7 database file to the bin dir
    system("cp ../data/company_000000000.dat ./company_000000000.dat");

    // load the V7 DB file. This should convert it to latest version
    companyDaos = daos_init(COMPANY_DAOS_VERSION, "./", COMPANY_FILE_PREFIX, COMPANY_FILE_EXTENSION,
        COMPANY_NUM_ID_DIGITS, numItemsPerFile, getCompanyFunctions());
    assert_false(daos_isNull(companyDaos));

    // test if file version is properly detected and file content gets upgraded
    void** items = NULL;
    DaosCount_t numItems = 0;
    int loadCompanyErr = daos_loadAllCachedItemsFromFile(companyDaos, "./company_000000000.dat", &items, &numItems);
    assert_true(loadCompanyErr == 0);
    assert_true(numItems == 1);

    static struct Company* company1File;
    assert_int_equal(0, daos_getItem(companyDaos, companyCached1.id, (void**)&company1File));

    // check if cached item content is as expected
	company1File->numEmployees = company1.numEmployees; // !! V7 is missing this data
    assert_Company_equal(company1File, &company1);

    DaosCount_t cacheIndex1 = 0;
    ASSERT_MT_TRUE(cacheIndex1 < numItems);
    ASSERT_MT_NOT_NULL(items[cacheIndex1]);
	((struct CompanyCached*)items[cacheIndex1])->numEmployees = companyCached1.numEmployees; // !! V7 is missing this data
    assert_CompanyCached_equal(&companyCached1, items[cacheIndex1]);

    // cleanup
    freeCompanyCached(&companyCached1);
    freeCompany(&company1);
    freeCompany(company1File);
    free(company1File);
    getCompanyFunctions()->freeCachedList(items, numItems);
}

static void test_daos_V7_detect_convert_profile_succeeds()
{
    static struct ProfilePersistent profilePersistent1, profilePersistent3, profilePersistent5;
    static struct ProfileCached profileCached1, profileCached3, profileCached5;
    initProfilePersistent(&profilePersistent1);
    initProfilePersistent(&profilePersistent3);
    initProfilePersistent(&profilePersistent5);
    initTestData_ProfilePersistent1(&profilePersistent1);
    initTestData_ProfilePersistent3(&profilePersistent3);
    profilePersistent3.logicalDelete = true;
    initTestData_ProfilePersistent5(&profilePersistent5);
    initProfileCached(&profileCached1);
    initProfileCached(&profileCached3);
    initProfileCached(&profileCached5);
    initTestData_ProfileCached1(&profileCached1);
    initTestData_ProfileCached2(&profileCached3);
    initTestData_ProfileCached5(&profileCached5);

    // copy a V7 database file to the bin dir
    system("cp ../data/profile_000000000.dat ./profile_000000000.dat");
    system("cp ../data/profile_000000001.dat ./profile_000000001.dat");

    // load the V7 DB file. This should convert it to latest version
    profileDaos = daos_init(PROFILE_DAOS_VERSION, "./", PROFILE_FILE_PREFIX, PROFILE_FILE_EXTENSION,
        PROFILE_NUM_ID_DIGITS, numItemsPerFile, getProfileFunctions());
    assert_false(daos_isNull(profileDaos));

    // test if file version is properly detected and file content gets upgraded
    void** items = NULL;
    DaosCount_t numItems = 0;
    int loadProfile1Err = daos_loadAllCachedItemsFromFile(profileDaos, "./profile_000000000.dat", &items, &numItems);
    assert_true(loadProfile1Err == 0);
    assert_true(numItems == 2); // prof 1, id 16, prof 3 id 22

    static struct ProfilePersistent* prof1File, * prof3File;
    assert_int_equal(0, daos_getItem(profileDaos, profilePersistent1.id, (void**)&prof1File));
    assert_int_equal(0, daos_getItem(profileDaos, profilePersistent3.id, (void**)&prof3File));

    // check if cached item content is as expected
    assert_ProfilePersistent_equal(prof1File, &profilePersistent1);
    assert_ProfilePersistent_equal(prof3File, &profilePersistent3);

    DaosCount_t cacheIndex1 = 0;
    DaosCount_t cacheIndex3 = 1;
    ASSERT_MT_TRUE(cacheIndex1 < numItems);
    ASSERT_MT_NOT_NULL(items[cacheIndex1]);
    ASSERT_MT_TRUE(cacheIndex3 < numItems);
    ASSERT_MT_TRUE(items[cacheIndex3] == NULL); // this is marked as deleted. It will not have a cached version !

    assert_ProfileCached_equal(&profileCached1, items[cacheIndex1]);

    // cleanup
    getProfileFunctions()->freeCachedList(items, numItems);

    int loadProfile2Err = daos_loadAllCachedItemsFromFile(profileDaos, "./profile_000000001.dat", &items, &numItems);
    assert_true(loadProfile2Err == 0);
    assert_true(numItems == 1); // prof 5 id 26

    static struct ProfilePersistent* prof5File;
    assert_int_equal(0, daos_getItem(profileDaos, profilePersistent5.id, (void**)&prof5File));

    // check if cached item content is as expected
    assert_ProfilePersistent_equal(prof5File, &profilePersistent5);

    DaosCount_t cacheIndex5 = 0;
    ASSERT_MT_TRUE(cacheIndex5 < numItems);
    ASSERT_MT_NOT_NULL(items[cacheIndex5]);

    assert_ProfileCached_equal(&profileCached5, items[cacheIndex5]);

    // cleanup
    getProfileFunctions()->freeCachedList(items, numItems);

    freeProfileCached(&profileCached5);
    freeProfileCached(&profileCached3);
    freeProfileCached(&profileCached1);
    freeProfilePersistent(&profilePersistent5);
    freeProfilePersistent(&profilePersistent3);
    freeProfilePersistent(&profilePersistent1);
    freeProfilePersistent(prof1File);
    freeProfilePersistent(prof3File);
    freeProfilePersistent(prof5File);
    free(prof1File);
    free(prof3File);
    free(prof5File);
}

void test_daos_V7_detect_convert_item_succeeds(void** state)
{
    logger_setLogLevel(DEBUG_LOG_MSG);

    test_daos_V7_detect_convert_company_succeeds();
    test_daos_V7_detect_convert_profile_succeeds();
}