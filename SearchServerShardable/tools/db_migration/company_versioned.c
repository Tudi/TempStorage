#include <company.h>
#include <daos_definitions.h>
#include <binary_utils.h>
#include <binary_specialized_array.h>
#include <utils.h>
#include <date_time.h>
#include <stdlib.h>
#include <string.h>

const uint8_t* binaryToCompany_V7(const uint8_t* byteStream,
    struct Company* company)
{
    const uint8_t* offset = byteStream;

    offset = binaryToInt32(offset, &company->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &company->parentId);
    if(offset == NULL) { return NULL; }

    BINARY_TO_INT32_ARRAY(company->parentIndustryIds, offset, offset);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->stage);
    if(offset == NULL) { return NULL; }

	char *numEmployees = NULL;
    offset = binaryToString(offset, &numEmployees);
    if(offset == NULL) { return NULL; }

    char* numEmployeesPtr = NULL;
    const char* low = strtok_r(numEmployees, "-", &numEmployeesPtr);
    const char* high = strtok_r(NULL, "-", &numEmployeesPtr);

    if (low != NULL && high != NULL)
    {
        const int low_int = atoi(low);
        const int high_int = atoi(high);

        if (low_int != 0 && high_int != 0)
        {
            company->numEmployees = (high_int + low_int) / 2;
        }
    } else if (high != NULL) {
        const int high_int = atoi(high);
        if (high_int != 0) {
            company->numEmployees = high_int;
        }
    } else if (low != NULL) {
        const int low_int = atoi(low);
        if (low_int != 0) {
            company->numEmployees = low_int;
        }
    } else {
        company->numEmployees = -1;
    }

    free(numEmployees);

    offset = binaryToString(offset, &company->name);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->domain);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersCity);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersState);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersZipcode);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->url);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->description);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->crunchbaseUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->crunchbaseHeadquarters);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->headquartersCountry);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->facebookUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->twitterUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->linkedinUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->linkedinUser);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &company->lastCachedAt);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &company->logoUrl);
    if(offset == NULL) { return NULL; }

    BINARY_TO_COMPANY_INDUSTRY_ARRAY(company->industries, offset, offset);
    if(offset == NULL) { return NULL; }

    return offset;
}
