#ifndef COMPANY_H
#define COMPANY_H

#include <company_industry.h>
#include <kvec.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct Company
{
	int32_t id;                                // int    `json:"id" redis:"c"`
	int32_t parentId;                          // int    `json:"parent_id,omitempty" redis:"p"`
	kvec_t(int32_t) parentIndustryIds;         // []int  `json:"industries,omitempty" redis:"i"`
	char* stage;                               // string `json:"stage,omitempty" redis:"s"`
	int32_t numEmployees;                      // int    `json:"number_employees,omitempty" redis:"e"`
	char* name;                                // string `json:"name" redis:"n"`
	char* domain;                              // string `json:"domain,omitempty" redis:"d" `
	char* headquartersCity;                    // string `json:"headquarters_city"`
	char* headquartersState;                   // string `json:"headquarters_state"`
	char* headquartersZipcode;                 // string `json:"headquarters_zipcode"`
	char* url;                                 // string `json:"url"`
	char* description;                         // string `json:"description"`
	char* crunchbaseUrl;                       // string `json:"crunchbase_url"`
	char* crunchbaseHeadquarters;              // string `json:"crunchbase_headquarters"`
	char* headquartersCountry;                 // string `json:"headquarters_country"`
	char* facebookUrl;                         // string `json:"facebook_url"`
	char* twitterUrl;                          // string `json:"twitter_url"`
	char* linkedinUrl;                         // string `json:"linkedin_url"`
	char* linkedinUser;                        // string `json:"linkedin_user"`
	time_t lastCachedAt;                       // time.Time `json:"last_cached_at"`
	char* logoUrl;                             // string `json:"logo_url"`
	kvec_t(struct CompanyIndustry) industries; // []CompanyIndustry `json:"company_industries"`
};

typedef kvec_t(struct Company) CompanyKvec_t;

void initCompany(struct Company* company);
void freeCompany(struct Company* company);

struct json_object* marshallCompany(const struct Company* company);
bool unmarshallCompany(struct Company* company, const struct json_object* obj);
uint8_t* companyToBinary(uint8_t* byteStream, const struct Company* company);
const uint8_t* binaryToCompany(const uint8_t* byteStream, struct Company* company, int fileVersion);
uint32_t companyBinarySize(const struct Company* company);

#define UNMARSHALL_COMPANY(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallCompany, OBJ, VAR, RET)

#define JSON_GET_COMPANY_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(struct Company, UNMARSHALL_COMPANY, \
        initCompany, freeCompany, OBJ, KEY, ARRAY, RET)

#endif // COMPANY_H
