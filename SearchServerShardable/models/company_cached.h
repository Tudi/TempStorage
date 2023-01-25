#ifndef COMPANY_CACHED_H
#define COMPANY_CACHED_H

#include <company.h>
#include <kvec.h>
#include <stdbool.h>
#include <stdint.h>

struct CompanyCached
{
    int32_t id;
    int32_t parentId;
    kvec_t(int32_t) parentIndustryIds;
    char* stage;
    int32_t numEmployees;
};

void initCompanyCached(struct CompanyCached* company);
void freeCompanyCached(struct CompanyCached* company);

const uint8_t* binaryToCompanyCached(const uint8_t* byteStream, struct CompanyCached* company);
bool companyToCompanyCached(struct CompanyCached* companyCached, const struct Company* company);

#endif // COMPANY_CACHED_H
