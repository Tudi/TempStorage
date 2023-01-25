#include <company.h>
#include <company_definitions.h>
#include <daos_versioned.h>
#include <binary_utils.h>
#include <binary_specialized_array.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <json_specialized_array.h>
#include <stdlib.h>

static const char* id_Key                     = "id";
static const char* parentId_Key               = "parent_id";
static const char* industries_Key             = "industries";
static const char* stage_Key                  = "stage";
static const char* numEmployees_Key           = "number_employees";
static const char* name_Key                   = "name";
static const char* domain_Key                 = "domain";
static const char* headquartersCity_Key       = "headquarters_city";
static const char* headquartersState_Key      = "headquarters_state";
static const char* headquartersZipcode_Key    = "headquarters_zipcode";
static const char* url_Key                    = "url";
static const char* description_Key            = "description";
static const char* crunchbaseUrl_Key          = "crunchbase_url";
static const char* crunchbaseHeadquarters_Key = "crunchbase_headquarters";
static const char* headquartersCountry_Key    = "headquarters_country";
static const char* facebookUrl_Key            = "facebook_url";
static const char* twitterUrl_Key             = "twitter_url";
static const char* linkedinUrl_Key            = "linkedin_url";
static const char* linkedinUser_Key           = "linkedin_user";
static const char* lastCachedAt_Key           = "last_cached_at";
static const char* logoUrl_Key                = "logo_url";
static const char* companyIndustries_Key      = "company_industries";

void initCompany(struct Company* company)
{
    company->id                     = 0;
    company->parentId               = 0;
    kv_init(company->parentIndustryIds);
    company->stage                  = NULL;
    company->numEmployees           = -1;
    company->name                   = NULL;
    company->domain                 = NULL;
    company->headquartersCity       = NULL;
    company->headquartersState      = NULL;
    company->headquartersZipcode    = NULL;
    company->url                    = NULL;
    company->description            = NULL;
    company->crunchbaseUrl          = NULL;
    company->crunchbaseHeadquarters = NULL;
    company->headquartersCountry    = NULL;
    company->facebookUrl            = NULL;
    company->twitterUrl             = NULL;
    company->linkedinUrl            = NULL;
    company->linkedinUser           = NULL;
    company->lastCachedAt           = getDate1_time_t();
    company->logoUrl                = NULL;
    kv_init(company->industries);
}

void freeCompany(struct Company* company)
{
    kv_destroy(company->parentIndustryIds);
    kv_init(company->parentIndustryIds);

    free(company->stage);
    company->stage = NULL;
    free(company->name);
    company->name = NULL;
    free(company->domain);
    company->domain = NULL;
    free(company->headquartersCity);
    company->headquartersCity = NULL;
    free(company->headquartersState);
    company->headquartersState = NULL;
    free(company->headquartersZipcode);
    company->headquartersZipcode = NULL;
    free(company->url);
    company->url = NULL;
    free(company->description);
    company->description = NULL;
    free(company->crunchbaseUrl);
    company->crunchbaseUrl = NULL;
    free(company->crunchbaseHeadquarters);
    company->crunchbaseHeadquarters = NULL;
    free(company->headquartersCountry);
    company->headquartersCountry = NULL;
    free(company->facebookUrl);
    company->facebookUrl = NULL;
    free(company->twitterUrl);
    company->twitterUrl = NULL;
    free(company->linkedinUrl);
    company->linkedinUrl = NULL;
    free(company->linkedinUser);
    company->linkedinUser = NULL;
    free(company->logoUrl);
    company->logoUrl = NULL;

    for(size_t i = 0; i < kv_size(company->industries); ++i) {
        freeCompanyIndustry(&(kv_A(company->industries, i)));
    }
    kv_destroy(company->industries);
    kv_init(company->industries);
}

struct json_object* marshallCompany(const struct Company* company)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, id_Key, json_object_new_int(company->id));
        json_object_object_add(obj, parentId_Key, json_object_new_int(company->parentId));

        JSON_ADD_INT32_ARRAY(obj, industries_Key, company->parentIndustryIds);

        json_object_object_add(obj, stage_Key, json_object_new_string(company->stage));
        json_object_object_add(obj, numEmployees_Key, json_object_new_int(company->numEmployees));
        json_object_object_add(obj, name_Key, json_object_new_string(company->name));
        json_object_object_add(obj, domain_Key, json_object_new_string(company->domain));
        json_object_object_add(obj, headquartersCity_Key, json_object_new_string(company->headquartersCity));
        json_object_object_add(obj, headquartersState_Key, json_object_new_string(company->headquartersState));
        json_object_object_add(obj, headquartersZipcode_Key, json_object_new_string(company->headquartersZipcode));
        json_object_object_add(obj, url_Key, json_object_new_string(company->url));
        json_object_object_add(obj, description_Key, json_object_new_string(company->description));
        json_object_object_add(obj, crunchbaseUrl_Key, json_object_new_string(company->crunchbaseUrl));
        json_object_object_add(obj, crunchbaseHeadquarters_Key,
            json_object_new_string(company->crunchbaseHeadquarters));
        json_object_object_add(obj, headquartersCountry_Key, json_object_new_string(company->headquartersCountry));
        json_object_object_add(obj, facebookUrl_Key, json_object_new_string(company->facebookUrl));
        json_object_object_add(obj, twitterUrl_Key, json_object_new_string(company->twitterUrl));
        json_object_object_add(obj, linkedinUrl_Key, json_object_new_string(company->linkedinUrl));
        json_object_object_add(obj, linkedinUser_Key, json_object_new_string(company->linkedinUser));
        json_object_object_add(obj, lastCachedAt_Key, marshallTime(company->lastCachedAt));
        json_object_object_add(obj, logoUrl_Key, json_object_new_string(company->logoUrl));

        JSON_ADD_ARRAY_ADDR(marshallCompanyIndustry, obj, companyIndustries_Key, company->industries);
    }

    return obj;
}

bool unmarshallCompany(struct Company* company, const struct json_object* obj)
{
    freeCompany(company);
    initCompany(company);

    bool b1 = jsonGetInt32(obj, id_Key, &company->id);
    bool b2 = jsonGetInt32(obj, parentId_Key, &company->parentId);

    bool b3 = false;
    JSON_GET_INT32_ARRAY(obj, industries_Key, company->parentIndustryIds, b3);

    bool b4 = jsonGetString(obj, stage_Key, &company->stage);
    bool b5 = jsonGetInt32(obj, numEmployees_Key, &company->numEmployees);
    bool b6 = jsonGetString(obj, name_Key, &company->name);
    bool b7 = jsonGetString(obj, domain_Key, &company->domain);
    bool b8 = jsonGetString(obj, headquartersCity_Key, &company->headquartersCity);
    bool b9 = jsonGetString(obj, headquartersState_Key, &company->headquartersState);
    bool b10 = jsonGetString(obj, headquartersZipcode_Key, &company->headquartersZipcode);
    bool b11 = jsonGetString(obj, url_Key, &company->url);
    bool b12 = jsonGetString(obj, description_Key, &company->description);
    bool b13 = jsonGetString(obj, crunchbaseUrl_Key, &company->crunchbaseUrl);
    bool b14 = jsonGetString(obj, crunchbaseHeadquarters_Key, &company->crunchbaseHeadquarters);
    bool b15 = jsonGetString(obj, headquartersCountry_Key, &company->headquartersCountry);
    bool b16 = jsonGetString(obj, facebookUrl_Key, &company->facebookUrl);
    bool b17 = jsonGetString(obj, twitterUrl_Key, &company->twitterUrl);
    bool b18 = jsonGetString(obj, linkedinUrl_Key, &company->linkedinUrl);
    bool b19 = jsonGetString(obj, linkedinUser_Key, &company->linkedinUser);
    bool b20 = jsonGetTime(obj, lastCachedAt_Key, &company->lastCachedAt);
    bool b21 = jsonGetString(obj, logoUrl_Key, &company->logoUrl);

    bool b22 = false;
    JSON_GET_COMPANY_INDUSTRY_ARRAY(obj, companyIndustries_Key, company->industries, b22);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12
        && b13 && b14 && b15 && b16 && b17 && b18 && b19 && b20 && b21 && b22))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* companyToBinary(uint8_t* byteStream, const struct Company* company)
{
    uint8_t* offset = byteStream;

    offset = int32ToBinary(offset, company->id);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, company->parentId);
    if(offset == NULL) { return NULL; }

    INT32_ARRAY_TO_BINARY(company->parentIndustryIds, offset, offset);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->stage);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, company->numEmployees);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->name);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->domain);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->headquartersCity);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->headquartersState);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->headquartersZipcode);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->url);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->description);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->crunchbaseUrl);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->crunchbaseHeadquarters);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->headquartersCountry);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->facebookUrl);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->twitterUrl);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->linkedinUrl);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->linkedinUser);
    if(offset == NULL) { return NULL; }

    offset = timeToBinary(offset, company->lastCachedAt);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, company->logoUrl);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(company->industries, companyIndustryToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    return offset;
}

static const uint8_t* binaryToCompany_active(const uint8_t* byteStream,
    struct Company* company)
{
    const uint8_t* offset = byteStream;

    offset = binaryToInt32(offset, &company->id);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &company->parentId);
    if (offset == NULL) { return NULL; }

    BINARY_TO_INT32_ARRAY(company->parentIndustryIds, offset, offset);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->stage);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &company->numEmployees);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->name);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->domain);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersCity);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersState);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersZipcode);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->url);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->description);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->crunchbaseUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->crunchbaseHeadquarters);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersCountry);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->facebookUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->twitterUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->linkedinUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->linkedinUser);
    if (offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &company->lastCachedAt);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->logoUrl);
    if (offset == NULL) { return NULL; }

    BINARY_TO_COMPANY_INDUSTRY_ARRAY(company->industries, offset, offset);
    if (offset == NULL) { return NULL; }

    return offset;
}

const uint8_t* binaryToCompany(const uint8_t* byteStream,
    struct Company* company, int fileVersion)
{
    freeCompany(company);
    initCompany(company);

    switch (fileVersion)
    {
        case 7: return binaryToCompany_V7(byteStream, company); break;
        case COMPANY_DAOS_VERSION: return binaryToCompany_active(byteStream, company); break;
        default: return NULL; // any value that signals that we do not handle this version
    }
    return NULL;
}

uint32_t companyBinarySize(const struct Company* company)
{
    uint32_t binarySize = 3 * INT32_BINARY_SIZE
        + stringBinarySize(company->stage) + stringBinarySize(company->name) + stringBinarySize(company->domain)
        + stringBinarySize(company->headquartersCity) + stringBinarySize(company->headquartersState)
        + stringBinarySize(company->headquartersZipcode) + stringBinarySize(company->url)
        + stringBinarySize(company->description) + stringBinarySize(company->crunchbaseUrl)
        + stringBinarySize(company->crunchbaseHeadquarters)
        + stringBinarySize(company->headquartersCountry) + stringBinarySize(company->facebookUrl)
        + stringBinarySize(company->twitterUrl) + stringBinarySize(company->linkedinUrl)
        + stringBinarySize(company->linkedinUser) + TIME_BINARY_SIZE
        + stringBinarySize(company->logoUrl);

    binarySize += FIXED_ARRAY_BINARY_SIZE(company->parentIndustryIds, INT32_BINARY_SIZE);

    uint32_t arrayBinarySize = 0;

    VARIABLE_ARRAY_BINARY_SIZE(company->industries, companyIndustryBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    return binarySize;
}
