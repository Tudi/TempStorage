#include <daos.h>
#include <profile_persistent.h>
#include <profile_cached.h>
#include <profile_persistent_comparison.h>
#include <profile_cached_comparison.h>
#include <profile_test_data.h>
#include <profile_functions.h>
#include <profile_definitions.h>
#include <utils.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct ProfilePersistent expectedProfile1, expectedProfile2;
static struct ProfileCached expectedProfileCached1, expectedProfileCached2;
static struct ProfilePersistent* profile = NULL;

static const ItemFunctions_t* itemFunctions = NULL;
static Daos_t profileDaos                   = Daos_NULL;
static FileTableEntry_t* fileTable          = NULL;
static uint8_t* fileContents                = NULL;
static void** itemsFromFile                 = NULL;
static DaosCount_t numItemsFromFile         = 0;

#define PROFILES_DIRECTORY         "./"
#define INVALID_PROFILES_DIRECTORY "./invalid_data/"

int test_profile_daos_setUp(void** state)
{
    initProfilePersistent(&expectedProfile1);
    initProfilePersistent(&expectedProfile2);

    initProfileCached(&expectedProfileCached1);
    initProfileCached(&expectedProfileCached2);

    profile = NULL;

    itemFunctions    = getProfileFunctions();
    profileDaos      = Daos_NULL;
    fileTable        = NULL;
    fileContents     = NULL;
    itemsFromFile    = NULL;
    numItemsFromFile = 0;

    return 0;
}

int test_profile_daos_tearDown(void** state)
{
    itemFunctions->freeCachedList(itemsFromFile, numItemsFromFile);
    itemFunctions    = NULL;
    itemsFromFile    = NULL;
    numItemsFromFile = 0;

    free(fileContents);
    free(fileTable);
    daos_free(profileDaos);

    if(profile != NULL)
    {
        freeProfilePersistent(profile);
        free(profile);
        profile = NULL;
    }

    freeProfileCached(&expectedProfileCached2);
    freeProfileCached(&expectedProfileCached1);

    freeProfilePersistent(&expectedProfile2);
    freeProfilePersistent(&expectedProfile1);

    return 0;
}

static void assert_file_contents_equal(const char* filename, uint8_t* expectedContents,
    size_t expectedContentsSize)
{
    FILE* f = fopen(filename, "rb");

    assert_non_null(f);

    size_t fileSize = 0;

    bool fileContentsRead = !getFileContents(f, &fileContents, &fileSize);
    fclose(f);

    assert_true(fileContentsRead); 
    assert_int_equal(expectedContentsSize, fileSize);
    assert_memory_equal(expectedContents, fileContents, expectedContentsSize);
}

#define BINARY_FILE_ARRAY_SIZE ( 2 + 4 + 4 + 16 * 8 + 9 * 6 \
    + 3 + PROFILE_PERSISTENT_1_BINARY_SIZE + 3 \
    + 4 * 6 \
    + 3 + PROFILE_PERSISTENT_2_BINARY_SIZE + 3 \
    + 6 )

void test_profile_daos_functions_succeed(void** state)
{
    uint8_t expectedBinArray[BINARY_FILE_ARRAY_SIZE] = {
        // File version
        0x8, 0x0,

        // Capacity
        0x10, 0x00, 0x00, 0x00, 

        // Number of items in file
        0x2, 0x00, 0x00, 0x00, 

        // File table
        0x8A, 0x00, 0x00, 0x00, 0x42, 0x04, 0x00, 0x00,
        0xcc, 0x04, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xd2, 0x04, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xd8, 0x04, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xde, 0x04, 0x00, 0x00, 0x94, 0x01, 0x00, 0x00,
        0x72, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x78, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x7e, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x84, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x8a, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x90, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x96, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x9c, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xa2, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xa8, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xae, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,

        // ProfilePersistent1 record

        // Header
        0xcc, 0xaa, 0x99,

        // Data
        PROFILE_PERSISTENT_1_BINARY

        // Trailer
        0x99, 0xaa, 0xcc,

        // Empty profiles
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,

        // ProfilePersistent2 record

        // Header
        0xcc, 0xaa, 0x99,

        // Data
        PROFILE_PERSISTENT_2_BINARY

        // Trailer
        0x99, 0xaa, 0xcc,

        // Empty profiles
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
    }; // Little endian

    initTestData_ProfilePersistent1(&expectedProfile1);
    initTestData_ProfilePersistent2(&expectedProfile2);

    initTestData_ProfileCached1(&expectedProfileCached1);
    initTestData_ProfileCached2(&expectedProfileCached2);

    // daos_init()

    DaosCount_t numItemsPerFile = 16;

    profileDaos = daos_init(PROFILE_DAOS_VERSION, PROFILES_DIRECTORY, PROFILE_FILE_PREFIX, PROFILE_FILE_EXTENSION,
        PROFILE_NUM_ID_DIGITS, numItemsPerFile, getProfileFunctions());

    assert_false(daos_isNull(profileDaos));

    // daos_saveItem(), daos_getItem()

    bool isNewItem = false;

    assert_int_equal(0, daos_saveItem(profileDaos, &expectedProfile1, &isNewItem));
    assert_true(isNewItem);
    assert_int_equal(0, daos_getItem(profileDaos, expectedProfile1.id, (void**) &profile));
    assert_ProfilePersistent_equal(&expectedProfile1, profile);

    freeProfilePersistent(profile);
    free(profile);
    profile = NULL;
    isNewItem = false;

    assert_int_equal(0, daos_saveItem(profileDaos, &expectedProfile2, &isNewItem));
    assert_true(isNewItem);
    assert_int_equal(0, daos_getItem(profileDaos, expectedProfile2.id, (void**) &profile));
    assert_ProfilePersistent_equal(&expectedProfile2, profile);

    // daos_getFileVersion(), daos_getFileTable()

    const char* profileFilename = PROFILES_DIRECTORY PROFILE_FILE_PREFIX "000000001." PROFILE_FILE_EXTENSION;

    DaosFileVersion_t libraryVersion = 0;
    DaosFileVersion_t fileVersion    = 0;

    assert_int_equal(0, daos_getFileVersion(profileDaos, profileFilename, &libraryVersion, &fileVersion));
    assert_int_equal(PROFILE_DAOS_VERSION, libraryVersion);
    assert_int_equal(libraryVersion, fileVersion);

    fileVersion = 0;

    DaosCount_t fileCapacity = 0;
    DaosCount_t numProfilesInFile = 0;

    assert_int_equal(0, daos_getFileTable(profileDaos, profileFilename, &fileVersion, &fileCapacity,
        &numProfilesInFile, &fileTable));
    assert_int_equal(libraryVersion, fileVersion);
    assert_int_equal(numItemsPerFile, fileCapacity);
    assert_int_equal(2, numProfilesInFile);
    assert_non_null(fileTable);

    assert_file_contents_equal(profileFilename, expectedBinArray, BINARY_FILE_ARRAY_SIZE);

    // daos_load()

    assert_int_equal(0, daos_loadAllCachedItemsFromFile(profileDaos, profileFilename,
        &itemsFromFile, &numItemsFromFile));
    assert_non_null(itemsFromFile);
    assert_int_equal(2, numItemsFromFile);

    DaosId_t itemId = itemFunctions->getCachedItemId(itemsFromFile[0]);
    struct ProfileCached* profileCached = (struct ProfileCached*) itemsFromFile[0];

    assert_int_equal(PROFILE_PERSISTENT_1_ID, itemId);
    assert_ProfileCached_equal(&expectedProfileCached1, profileCached);

    itemId = itemFunctions->getCachedItemId(itemsFromFile[1]);
    profileCached = (struct ProfileCached*) itemsFromFile[1];

    assert_int_equal(PROFILE_PERSISTENT_2_ID, itemId);
    assert_ProfileCached_equal(&expectedProfileCached2, profileCached);
}

void test_profile_daos_load_fails(void** state)
{
    const char* incorrectVersionProfileFilename = INVALID_PROFILES_DIRECTORY "profile_999888777.dat";

    DaosCount_t numItemsPerFile = 4;

    profileDaos = daos_init(PROFILE_DAOS_VERSION, INVALID_PROFILES_DIRECTORY, PROFILE_FILE_PREFIX,
        PROFILE_FILE_EXTENSION, PROFILE_NUM_ID_DIGITS, numItemsPerFile, getProfileFunctions());

    assert_false(daos_isNull(profileDaos));

    assert_int_not_equal(0, daos_loadAllCachedItemsFromFile(profileDaos, incorrectVersionProfileFilename,
        &itemsFromFile, &numItemsFromFile));
    assert_null(itemsFromFile);
    assert_int_equal(0, numItemsFromFile);
}
