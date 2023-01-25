#include <company_functions.h>
#include <company_cached.h>
#include <company.h>
#include <company_definitions.h>
#include <logger.h>
#include <json_array.h>
#include <k_utils.h>
#include <string.h>
#include <ctype.h>

//
// Prototypes
//

// Company

static DaosId_t getCompanyId(const void* item);
static void freeCompanyItem(void* item);
static uint8_t* companyItemToBinary(uint8_t* byteStream, const void* item);
static int binaryToCompanyItem(const uint8_t* byteStream, void** item, int fileVersion);
static uint32_t companyItemBinarySize(const void* item);
static struct json_object* companyToJson(const void* item);
static int jsonToCompany(void** item, const struct json_object* obj);
static int jsonToCompanyList(void** lst, const struct json_object* obj, const char* jsonKey);
static void freeCompanyList(void* lst);
static uint32_t companyListCount(const void* lst);
static void* getItemFromCompanyList(void* lst, uint32_t index);
static bool companyShouldBeCached(const void* item);
static bool isCompanyFile(const char* filename);
static bool isCompanyDestinationServerCorrect(const void* item, DaosCount_t numServers, const DaosId_t serverId);

// CompanyCached

static DaosId_t getCompanyCachedId(const void* item);
static void* generateCompanyCached(const void* item);
static void freeCompanyCachedList(void** items, uint32_t numItems);
static void freeCompanyCachedItem(void* item);
static int binaryToCompanyCachedItem(const uint8_t* byteStream, void** item, int fileVersion);
static DaosCount_t generateCompanyCacheIndex(DaosId_t id, DaosCount_t numServers);

//
// Variables
//

static ItemFunctions_t companyFunctions = 
{
    // Persistent item

    .getPersistentItemId          = getCompanyId,
    .freePersistentItem           = freeCompanyItem,
    .persistentItemToBinary       = companyItemToBinary,
    .binaryToPersistentItem       = binaryToCompanyItem,
    .persistentItemBinarySize     = companyItemBinarySize,
    .persistentItemToJson         = companyToJson,
    .jsonToPersistentItem         = jsonToCompany,
    .jsonToPersistentList         = jsonToCompanyList,
    .freePersistentList           = freeCompanyList,
    .persistentListCount          = companyListCount,
    .getItemFromPersistentList    = getItemFromCompanyList,
    .persistentItemShouldBeCached = companyShouldBeCached,
    .isItemFile                   = isCompanyFile,
    .isDestinationServerCorrect   = isCompanyDestinationServerCorrect,

    // Cached item

    .getCachedItemId    = getCompanyCachedId,
    .generateCachedItem = generateCompanyCached,
    .freeCachedList     = freeCompanyCachedList,
    .freeCachedItem     = freeCompanyCachedItem,
    .binaryToCachedItem = binaryToCompanyCachedItem,
    .generateCacheIndex = generateCompanyCacheIndex
};

//
// External interface
//

const ItemFunctions_t* getCompanyFunctions()
{
    return &companyFunctions;
}

//
// Local functions
//

//
// Company
//

static DaosId_t getCompanyId(const void* item)
{
    const struct Company* company = (const struct Company*) item;

    return company->id;
}

static void freeCompanyItem(void* item)
{
    struct Company* company = (struct Company*) item;
    if(company != NULL)
    {
        freeCompany(company);
        free(company);
    }
}

static uint8_t* companyItemToBinary(uint8_t* byteStream, const void* item)
{
    const struct Company* company = (const struct Company*) item;

    return companyToBinary(byteStream, company);
}

static int binaryToCompanyItem(const uint8_t* byteStream, void** item, int fileVersion)
{
    struct Company* company = (struct Company*) malloc(sizeof(struct Company));
    if(company == NULL) { return 1; }

    initCompany(company);

    const uint8_t* resultingByteStream = binaryToCompany(byteStream, company, fileVersion);
    if(resultingByteStream == NULL)
    {
        free(company);
        return 2;
    }

    *item = company;

    return 0;
}

static uint32_t companyItemBinarySize(const void* item)
{
    const struct Company* company = (const struct Company*) item;

    return companyBinarySize(company);
}

static struct json_object* companyToJson(const void* item)
{
    const struct Company* company = (const struct Company*) item;

    return marshallCompany(company);
}

static int jsonToCompany(void** item, const struct json_object* obj)
{
    struct Company* company = (struct Company*) malloc(sizeof(struct Company));
    if(company == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.", sizeof(struct Company));
        return 1;
    }

    initCompany(company);

    bool ret = unmarshallCompany(company, obj);
    if(ret == false)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallCompany() failed.");
        freeCompanyItem(company);
        return 2;
    }

    *item = company;

    return 0;
}

static int jsonToCompanyList(void** lst, const struct json_object* obj, const char* jsonKey)
{
    bool ret = false;
    CompanyKvec_t* companies = (CompanyKvec_t*) malloc(sizeof(CompanyKvec_t));
    if(companies == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.", sizeof(CompanyKvec_t));
        return 1;
    }

    kv_init(*companies);
    JSON_GET_COMPANY_ARRAY(obj, jsonKey, *companies, ret);

    if(ret == false)
    {
        freeCompanyList(companies);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: JSON_GET_COMPANY_ARRAY() failed.");
        return 2;
    }

    *lst = companies;

    return 0;
}

static void freeCompanyList(void* lst)
{
    CompanyKvec_t* companies = lst;

    FREE_KVEC_ADDR(*companies, freeCompany);
    free(companies);
}

static uint32_t companyListCount(const void* lst)
{
    const CompanyKvec_t* companies = lst;

    return kv_size(*companies);
}

static void* getItemFromCompanyList(void* lst, uint32_t index)
{
    CompanyKvec_t* companies = lst;

    return &(kv_A(*companies, index));
}

static bool companyShouldBeCached(const void* item)
{
    return true;
}

static bool isCompanyFile(const char* filename)
{
    static const size_t companyFilePrefixLength = strlen(COMPANY_FILE_PREFIX);
    static const size_t companyFileExtensionLength = strlen(COMPANY_FILE_EXTENSION);
    static const size_t companyFilenameLength = companyFilePrefixLength + COMPANY_NUM_ID_DIGITS
        + 1 + companyFileExtensionLength;

    if(strlen(filename) != companyFilenameLength) { return false; }

    const char* filenamePtr = filename;

    if(strncmp(filenamePtr, COMPANY_FILE_PREFIX, companyFilePrefixLength)) { return false; }

    filenamePtr += companyFilePrefixLength;

    uint16_t i = 0;
    for(; i < COMPANY_NUM_ID_DIGITS; ++i) {
        if(!isdigit(filenamePtr[i])) { return false; }
    }

    filenamePtr += COMPANY_NUM_ID_DIGITS;

    return !strcmp(filenamePtr, "." COMPANY_FILE_EXTENSION);
}

//
// CompanyCached
//

static DaosId_t getCompanyCachedId(const void* item)
{
    const struct CompanyCached* companyCached = (const struct CompanyCached*) item;

    return companyCached->id;
}

static void* generateCompanyCached(const void* item)
{
    const struct Company* company = (const struct Company*) item;

    struct CompanyCached* companyCached
        = (struct CompanyCached*) malloc(sizeof(struct CompanyCached));
    if(companyCached == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (company.id = %lu) - malloc(size = %zu bytes) failed.",
            company->id, sizeof(struct CompanyCached));
        return NULL;
    }

    initCompanyCached(companyCached);

    if(!companyToCompanyCached(companyCached, company))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (company.id = %lu) - companyToCompanyCached() failed.",
            company->id);
        freeCompanyCached(companyCached);
        free(companyCached);
        return NULL;
    }

    return companyCached;
}

static void freeCompanyCachedList(void** items, uint32_t numItems)
{
    struct CompanyCached** list = (struct CompanyCached**) items;

    if(list == NULL) { return; }

    for(uint32_t i = 0; i < numItems; ++i)
    {
        if(list[i] != NULL)
        {
            freeCompanyCached(list[i]);
            free(list[i]);
        }
    }

    free(list);
}

static void freeCompanyCachedItem(void* item)
{
    struct CompanyCached* company = (struct CompanyCached*) item;
    if(company != NULL)
    {
        freeCompanyCached(company);
        free(company);
    }
}

static int binaryToCompanyCachedItem(const uint8_t* byteStream, void** item, int fileVersion)
{
    struct CompanyCached* company = (struct CompanyCached*) malloc(sizeof(struct CompanyCached));
    if(company == NULL) { return 1; }

    initCompanyCached(company);

    const uint8_t* resultingByteStream = binaryToCompanyCached(byteStream, company);
    if(resultingByteStream == NULL)
    {
        free(company);
        return 2;
    }

    *item = company;

    return 0;
}

static DaosCount_t generateCompanyCacheIndex(DaosId_t id, DaosCount_t numServers)
{
    return id;
}

static bool isCompanyDestinationServerCorrect(const void* item, DaosCount_t numServers, const DaosId_t serverId)
{
    return true;
}
