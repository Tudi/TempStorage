#include <company_cached.h>
#include <binary_utils.h>
#include <binary_specialized_array.h>
#include <utils.h>
#include <date_time.h>
#include <string.h>
#include <stdlib.h>

//
// External interface
//

void initCompanyCached(struct CompanyCached* company)
{
    company->id           = 0;
    company->parentId     = 0;
    kv_init(company->parentIndustryIds);
    company->stage        = NULL;
    company->numEmployees = 0;
}

void freeCompanyCached(struct CompanyCached* company)
{
    kv_destroy(company->parentIndustryIds);
    kv_init(company->parentIndustryIds);
    free(company->stage);
    company->stage = NULL;
}

const uint8_t* binaryToCompanyCached(const uint8_t* byteStream, struct CompanyCached* company)
{
    freeCompanyCached(company);
    initCompanyCached(company);

    const uint8_t* offset = byteStream;

    offset = binaryToInt32(offset, &company->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &company->parentId);
    if(offset == NULL) { return NULL; }

    BINARY_TO_INT32_ARRAY(company->parentIndustryIds, offset, offset);
    if(offset == NULL) { return NULL; }

    offset = binaryToLowerString(offset, &company->stage);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &company->numEmployees);
    if(offset == NULL) { return NULL; }

    return offset;
}

bool companyToCompanyCached(struct CompanyCached* companyCached, const struct Company* company)
{
    freeCompanyCached(companyCached);
    initCompanyCached(companyCached);

    companyCached->id       = company->id;
    companyCached->parentId = company->parentId;

    // This happens due to non yet processed data by GO manager. We are not expecting company ID 0
    // By setting parentId to same ID, one less sanity check is removed in scoring and filtering
    if (companyCached->parentId == 0) {
        companyCached->parentId = companyCached->id;
    }

    if(kv_size(company->industries) > 0) {
        kv_copy(int32_t, companyCached->parentIndustryIds, company->parentIndustryIds);
    }

    companyCached->stage = strdupLower(company->stage);
    if(companyCached->stage == NULL) { return false; }

    companyCached->numEmployees = company->numEmployees;

    return true;
}
