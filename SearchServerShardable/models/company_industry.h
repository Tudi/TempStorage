#ifndef COMPANY_INDUSTRY_H
#define COMPANY_INDUSTRY_H

#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct CompanyIndustry
{
	int32_t id;     // int    `json:"id"`
	char* name;     // string `json:"industry"`
};

void initCompanyIndustry(struct CompanyIndustry* company);
void freeCompanyIndustry(struct CompanyIndustry* company);

struct json_object* marshallCompanyIndustry(const struct CompanyIndustry* companyIndustry);
bool unmarshallCompanyIndustry(struct CompanyIndustry* companyIndustry,
    const struct json_object* obj);
uint8_t* companyIndustryToBinary(uint8_t* byteStream,
    const struct CompanyIndustry* companyIndustry);
const uint8_t* binaryToCompanyIndustry(const uint8_t* byteStream,
    struct CompanyIndustry* companyIndustry);
uint32_t companyIndustryBinarySize(const struct CompanyIndustry* companyIndustry);

#define UNMARSHALL_COMPANY_INDUSTRY(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallCompanyIndustry, OBJ, VAR, RET)

#define JSON_GET_COMPANY_INDUSTRY_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(struct CompanyIndustry, UNMARSHALL_COMPANY_INDUSTRY, \
    initCompanyIndustry, freeCompanyIndustry, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_COMPANY_INDUSTRY_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct CompanyIndustry, ARRAY, binaryToCompanyIndustry, \
    initCompanyIndustry, freeCompanyIndustry, BYTESTREAM, OFFSET)

#endif // COMPANY_INDUSTRY_H
