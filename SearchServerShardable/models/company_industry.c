#include <company_industry.h>
#include <binary_utils.h>
#include <logger.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* id_Key       = "id";
static const char* industry_Key = "industry";

void initCompanyIndustry(struct CompanyIndustry* companyIndustry)
{
    companyIndustry->id       = 0;
    companyIndustry->name = NULL;
}

void freeCompanyIndustry(struct CompanyIndustry* companyIndustry)
{
    free(companyIndustry->name);
    companyIndustry->name = NULL;
}

struct json_object* marshallCompanyIndustry(const struct CompanyIndustry* companyIndustry)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, id_Key, json_object_new_int(companyIndustry->id));
        json_object_object_add(obj, industry_Key,
            json_object_new_string(companyIndustry->name));
    }

    return obj;
}

bool unmarshallCompanyIndustry(struct CompanyIndustry* companyIndustry,
    const struct json_object* obj)
{
    freeCompanyIndustry(companyIndustry);
    initCompanyIndustry(companyIndustry);

    bool b1 = jsonGetInt32(obj, id_Key, &companyIndustry->id);
    bool b2 = jsonGetString(obj, industry_Key, &companyIndustry->name);

    if(!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* companyIndustryToBinary(uint8_t* byteStream, const struct CompanyIndustry* companyIndustry)
{
    uint8_t* offset = byteStream;

    offset = int32ToBinary(offset, companyIndustry->id);
    if(offset == NULL) { return NULL; }

    return offset = stringToBinary(offset, companyIndustry->name);
}

const uint8_t* binaryToCompanyIndustry(const uint8_t* byteStream,
    struct CompanyIndustry* companyIndustry)
{
    freeCompanyIndustry(companyIndustry);
    initCompanyIndustry(companyIndustry);

    const uint8_t* offset = byteStream;

    offset = binaryToInt32(offset, &companyIndustry->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &companyIndustry->name);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t companyIndustryBinarySize(const struct CompanyIndustry* companyIndustry)
{
    return INT32_BINARY_SIZE + stringBinarySize(companyIndustry->name);
}
