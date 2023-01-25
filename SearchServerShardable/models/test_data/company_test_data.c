#include <company_test_data.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <kvec.h>

//
// Constants
//

//
// Company1
//

struct tm company1_lastCachedAt = { .tm_sec = 1, .tm_min = 1, .tm_hour = 1,
    .tm_mday = 1, .tm_mon = 0, .tm_year = 100, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0
}; // 2000-Jan-01 01:01:01

//
// Company2
//

struct tm company2_lastCachedAt = { .tm_sec = 2, .tm_min = 2, .tm_hour = 2,
    .tm_mday = 2, .tm_mon = 1, .tm_year = 110, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0
}; // 2010-Feb-02 02:02:02

//
// Company1
//

void initTestData_Company1(struct Company* company)
{
    company->id       = 16;
    company->parentId = 8;

    kv_resize(int32_t, company->parentIndustryIds, 2);
    kv_push(int32_t, company->parentIndustryIds, 3);
    kv_push(int32_t, company->parentIndustryIds, 6);

    company->stage                  = strdup("Operational");
    company->numEmployees           = 25;
    company->name                   = strdup("ABC XYZ");
    company->domain                 = strdup("Computers");
    company->headquartersCity       = strdup("New York");
    company->headquartersState      = strdup("NY");
    company->headquartersZipcode    = strdup("10001");
    company->url                    = strdup("http://www.abcxyz.com");
    company->description            = strdup("DESCRIPTION_ABC");
    company->crunchbaseUrl          = strdup("http://www.crunchbase.com/abcxyz");
    company->crunchbaseHeadquarters = strdup("New York");
    company->headquartersCountry    = strdup("USA");
    company->facebookUrl            = strdup("http://www.facebook.com/abcxyz");
    company->twitterUrl             = strdup("http://www.twitter.com/abcxyz");
    company->linkedinUrl            = strdup("http://www.linkedin.com/abcxyz");
    company->linkedinUser           = strdup("abcxyz");
    company->lastCachedAt           = mktime(&company1_lastCachedAt);
    company->logoUrl                = strdup("http://www.abcxyz.com/logo.jpg");

    kv_resize(struct CompanyIndustry, company->industries, 1);
    kv_push(struct CompanyIndustry, company->industries, ((struct CompanyIndustry) {
        .id = 9, .name = strdup("INDUSTRY_ABC") }));
}

void initTestData_CompanyCached1(struct CompanyCached* company)
{
    company->id       = 16;
    company->parentId = 8;

    kv_resize(int32_t, company->parentIndustryIds, 2);
    kv_push(int32_t, company->parentIndustryIds, 3);
    kv_push(int32_t, company->parentIndustryIds, 6);

    company->stage        = strdup("operational");
    company->numEmployees = 25;
}

//
// Company2
//

void initTestData_Company2(struct Company* company)
{
    company->id                     = 20;
    company->parentId               = 4;
    company->stage                  = strdup("Startup");
    company->numEmployees           = 75;
    company->name                   = strdup("DEFGH");
    company->domain                 = strdup("Computers");
    company->headquartersCity       = strdup("San Jose");
    company->headquartersState      = strdup("CA");
    company->headquartersZipcode    = strdup("95113");
    company->url                    = strdup("http://www.defgh.com");
    company->description            = strdup("DESCRIPTION_DEF");
    company->crunchbaseUrl          = strdup("http://www.crunchbase.com/defgh");
    company->crunchbaseHeadquarters = strdup("San Jose");
    company->headquartersCountry    = strdup("USA");
    company->facebookUrl            = strdup("http://www.facebook.com/defgh");
    company->twitterUrl             = strdup("http://www.twitter.com/defgh");
    company->linkedinUrl            = strdup("http://www.linkedin.com/defgh");
    company->linkedinUser           = strdup("defgh");
    company->lastCachedAt           = mktime(&company2_lastCachedAt);
    company->logoUrl                = strdup("http://www.defgh.com/logo.jpg");
}

void initTestData_CompanyCached2(struct CompanyCached* company)
{
    company->id           = 20;
    company->parentId     = 4;
    company->stage        = strdup("startup");
    company->numEmployees = 75;
}
