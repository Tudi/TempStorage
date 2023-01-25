#include <daos.h>
#include <company.h>
#include <company_cached.h>
#include <company_comparison.h>
#include <company_cached_comparison.h>
#include <company_test_data.h>
#include <company_functions.h>
#include <company_definitions.h>
#include <utils.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct Company expectedCompany1, expectedCompany2;
static struct CompanyCached expectedCompanyCached1, expectedCompanyCached2;
static struct Company* company = NULL;

static const ItemFunctions_t* itemFunctions = NULL;
static Daos_t companyDaos                   = Daos_NULL;
static FileTableEntry_t* fileTable          = NULL;
static uint8_t* fileContents                = NULL;
static void** itemsFromFile                 = NULL;
static DaosCount_t numItemsFromFile         = 0;

#define COMPANIES_DIRECTORY         "./"
#define INVALID_COMPANIES_DIRECTORY "./invalid_data/"

int test_company_daos_setUp(void** state)
{
    initCompany(&expectedCompany1);
    initCompany(&expectedCompany2);

    initCompanyCached(&expectedCompanyCached1);
    initCompanyCached(&expectedCompanyCached2);

    company = NULL;

    itemFunctions    = getCompanyFunctions();
    companyDaos      = Daos_NULL;
    fileTable        = NULL;
    fileContents     = NULL;
    itemsFromFile    = NULL;
    numItemsFromFile = 0;

    return 0;
}

int test_company_daos_tearDown(void** state)
{
    itemFunctions->freeCachedList(itemsFromFile, numItemsFromFile);
    itemFunctions    = NULL;
    itemsFromFile    = NULL;
    numItemsFromFile = 0;

    free(fileContents);
    free(fileTable);
    daos_free(companyDaos);

    if(company != NULL)
    {
        freeCompany(company);
        free(company);
        company = NULL;
    }

    freeCompanyCached(&expectedCompanyCached2);
    freeCompanyCached(&expectedCompanyCached1);

    freeCompany(&expectedCompany2);
    freeCompany(&expectedCompany1);

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

#define BINARY_FILE_ARRAY_SIZE ( 2 + 4 + 4 + 8 * 8 \
    + 3 + COMPANY_1_BINARY_SIZE + 3 \
    + 3 * 6 \
    + 3 + COMPANY_2_BINARY_SIZE + 3 \
    + 3 * 6 )

void test_company_daos_functions_succeed(void** state)
{
    uint8_t expectedBinArray[BINARY_FILE_ARRAY_SIZE] = {
        // File version
        0x8, 0x0,

        // Capacity
        0x08, 0x00, 0x00, 0x00, 

        // Number of items in file
        0x2, 0x00, 0x00, 0x00, 

        // File table
        0x4a, 0x00, 0x00, 0x00, 0x5f, 0x01, 0x00, 0x00,
        0xa9, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xaf, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xb5, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xbb, 0x01, 0x00, 0x00, 0x37, 0x01, 0x00, 0x00,
        0xf2, 0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xf8, 0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xfe, 0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,

        // Company1 record

        // Header
        0xcc, 0xaa, 0x99,

        // Data
        COMPANY_1_BINARY

        // Trailer
        0x99, 0xaa, 0xcc,

        // Empty companies
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,

        // Company2 record

        // Header
        0xcc, 0xaa, 0x99,

        // Data
        COMPANY_2_BINARY

        // Trailer
        0x99, 0xaa, 0xcc,

        // Empty companies
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc, 0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
        0xcc, 0xaa, 0x99, 0x99, 0xaa, 0xcc,
    }; // Little endian

    initTestData_Company1(&expectedCompany1);
    initTestData_Company2(&expectedCompany2);

    initTestData_CompanyCached1(&expectedCompanyCached1);
    initTestData_CompanyCached2(&expectedCompanyCached2);

    // daos_init()

    DaosCount_t numItemsPerFile = 8;

    companyDaos = daos_init(COMPANY_DAOS_VERSION, COMPANIES_DIRECTORY, COMPANY_FILE_PREFIX, COMPANY_FILE_EXTENSION,
        COMPANY_NUM_ID_DIGITS, numItemsPerFile, getCompanyFunctions());

    assert_false(daos_isNull(companyDaos));

    // daos_saveItem(), daos_getItem()

    bool isNewItem = false;

    assert_int_equal(0, daos_saveItem(companyDaos, &expectedCompany1, &isNewItem));
    assert_true(isNewItem);
    assert_int_equal(0, daos_getItem(companyDaos, expectedCompany1.id, (void**) &company));
    assert_Company_equal(&expectedCompany1, company);

    freeCompany(company);
    free(company);
    company = NULL;
    isNewItem = false;

    assert_int_equal(0, daos_saveItem(companyDaos, &expectedCompany2, &isNewItem));
    assert_true(isNewItem);
    assert_int_equal(0, daos_getItem(companyDaos, expectedCompany2.id, (void**) &company));
    assert_Company_equal(&expectedCompany2, company);

    // daos_getFileVersion(), daos_getFileTable()
 
    const char* companyFilename = COMPANIES_DIRECTORY COMPANY_FILE_PREFIX "000000002." COMPANY_FILE_EXTENSION;

    DaosFileVersion_t libraryVersion = 0;
    DaosFileVersion_t fileVersion    = 0;

    assert_int_equal(0, daos_getFileVersion(companyDaos, companyFilename, &libraryVersion, &fileVersion));
    assert_int_equal(COMPANY_DAOS_VERSION, libraryVersion);
    assert_int_equal(libraryVersion, fileVersion);

    fileVersion = 0;

    DaosCount_t fileCapacity = 0;
    DaosCount_t numCompaniesInFile = 0;

    assert_int_equal(0, daos_getFileTable(companyDaos, companyFilename, &fileVersion, &fileCapacity,
        &numCompaniesInFile, &fileTable));
    assert_int_equal(libraryVersion, fileVersion);
    assert_int_equal(numItemsPerFile, fileCapacity);
    assert_int_equal(2, numCompaniesInFile);
    assert_non_null(fileTable);

    assert_file_contents_equal(companyFilename, expectedBinArray, BINARY_FILE_ARRAY_SIZE);

    // daos_load()

    assert_int_equal(0, daos_loadAllCachedItemsFromFile(companyDaos, companyFilename,
        &itemsFromFile, &numItemsFromFile));
    assert_non_null(itemsFromFile);
    assert_int_equal(2, numItemsFromFile);

    DaosId_t itemId = itemFunctions->getCachedItemId(itemsFromFile[0]);
    struct CompanyCached* companyCached = (struct CompanyCached*) itemsFromFile[0];

    assert_int_equal(COMPANY_1_ID, itemId);
    assert_CompanyCached_equal(&expectedCompanyCached1, companyCached);

    itemId = itemFunctions->getCachedItemId(itemsFromFile[1]);
    companyCached = (struct CompanyCached*) itemsFromFile[1];

    assert_int_equal(COMPANY_2_ID, itemId);
    assert_CompanyCached_equal(&expectedCompanyCached2, companyCached);
}

void test_company_daos_load_fails(void** state)
{
    const char* incorrectVersionCompanyFilename = INVALID_COMPANIES_DIRECTORY "company_999777555.dat";

    DaosCount_t numItemsPerFile = 4;

    companyDaos = daos_init(COMPANY_DAOS_VERSION, INVALID_COMPANIES_DIRECTORY, COMPANY_FILE_PREFIX,
        COMPANY_FILE_EXTENSION, COMPANY_NUM_ID_DIGITS, numItemsPerFile, getCompanyFunctions());

    assert_false(daos_isNull(companyDaos));

    assert_int_not_equal(0, daos_loadAllCachedItemsFromFile(companyDaos, incorrectVersionCompanyFilename,
        &itemsFromFile, &numItemsFromFile));
    assert_null(itemsFromFile);
    assert_int_equal(0, numItemsFromFile);
}
