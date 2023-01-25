#ifndef COMPANY_TEST_DATA_H
#define COMPANY_TEST_DATA_H

#include <company.h>
#include <company_cached.h>
#include <macro_utils.h>

//
// Company1
//

void initTestData_Company1(struct Company* company);

void initTestData_CompanyCached1(struct CompanyCached* company);

#define COMPANY_1_ID 16

#define COMPANY_1_JSON_STRING \
    "{ \"id\": " MACRO_TO_STRING(COMPANY_1_ID) \
    ", \"parent_id\": 8," \
    " \"industries\": [ 3, 6 ]," \
    " \"stage\": \"Operational\"," \
    " \"number_employees\": 25," \
    " \"name\": \"ABC XYZ\"," \
    " \"domain\": \"Computers\"," \
    " \"headquarters_city\": \"New York\"," \
    " \"headquarters_state\": \"NY\"," \
    " \"headquarters_zipcode\": \"10001\"," \
    " \"url\": \"http:\\/\\/www.abcxyz.com\"," \
    " \"description\": \"DESCRIPTION_ABC\"," \
    " \"crunchbase_url\": \"http:\\/\\/www.crunchbase.com\\/abcxyz\"," \
    " \"crunchbase_headquarters\": \"New York\"," \
    " \"headquarters_country\": \"USA\"," \
    " \"facebook_url\": \"http:\\/\\/www.facebook.com\\/abcxyz\"," \
    " \"twitter_url\": \"http:\\/\\/www.twitter.com\\/abcxyz\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\\/abcxyz\"," \
    " \"linkedin_user\": \"abcxyz\"," \
    " \"last_cached_at\": \"2000-01-01T01:01:01Z\"," \
    " \"logo_url\": \"http:\\/\\/www.abcxyz.com\\/logo.jpg\"," \
    " \"company_industries\": [ { \"id\": 9, \"industry\": \"INDUSTRY_ABC\" } ] }"

#define COMPANY_1_CACHED_PORTION_BINARY_SIZE \
    ( 4 + 4 + 10 + 14 + 4 )

#define COMPANY_1_BINARY_SIZE \
    ( COMPANY_1_CACHED_PORTION_BINARY_SIZE \
      + 10 + 12 + 11 + 5 + 8 + 24 + 18 + 35 + 11 + 6 \
      + 33 + 32 + 33 + 9 + 8 + 33 + 21 )

#define COMPANY_1_BINARY \
    /* Little endian */ \
    /* id */ \
    0x10, 0x00, 0x00, 0x00, \
    /* parentId */ \
    0x08, 0x00, 0x00, 0x00, \
    /* parent_industries */ \
    0x02, 0x00, \
    0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, \
    /* stage */ \
    0x0c, 0x00, 'O', 'p', 'e', 'r', 'a', 't', 'i', 'o', 'n', 'a', 'l', '\0', \
    /* numEmployees */ \
    0x19, 0x00, 0x00, 0x00, \
    /* name */ \
    0x08, 0x00, 'A', 'B', 'C', ' ', 'X', 'Y', 'Z', '\0', \
    /* domain */ \
    0x0a, 0x00, 'C', 'o', 'm', 'p', 'u', 't', 'e', 'r', 's', '\0', \
    /* headquartersCity */ \
    0x09, 0x00, 'N', 'e', 'w', ' ', 'Y', 'o', 'r', 'k', '\0', \
    /* headquartersState */ \
    0x03, 0x00, 'N', 'Y', '\0', \
    /* headquartersZipcode */ \
    0x06, 0x00, '1', '0', '0', '0', '1', '\0', \
    /* url */ \
    0x16, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'a', 'b', \
          'c', 'x', 'y', 'z', '.', 'c', 'o', 'm', '\0', \
    /* description */ \
    0x10, 0x00, 'D', 'E', 'S', 'C', 'R', 'I', 'P', 'T', 'I', 'O', 'N', \
          '_', 'A', 'B', 'C', '\0', \
    /* crunchbaseUrl */ \
    0x21, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'c', 'r', \
          'u', 'n', 'c', 'h', 'b', 'a', 's', 'e', '.', 'c', 'o', 'm', '/', 'a', \
          'b', 'c', 'x', 'y', 'z', '\0', \
    /* crunchbaseHeadquarters */ \
    0x09, 0x00, 'N', 'e', 'w', ' ', 'Y', 'o', 'r', 'k', '\0', \
    /* headquartersCountry */ \
    0x04, 0x00, 'U', 'S', 'A', '\0', \
    /* facebookUrl */ \
    0x1f, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'f', 'a', \
          'c', 'e', 'b', 'o', 'o', 'k', '.', 'c', 'o', 'm', '/', 'a', 'b', 'c', \
          'x', 'y', 'z', '\0', \
    /* twitterUrl */ \
    0x1e, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 't', 'w', \
          'i', 't', 't', 'e', 'r', '.', 'c', 'o', 'm', '/', 'a', 'b', 'c', 'x', \
          'y', 'z', '\0', \
    /* linkedinUrl */ \
    0x1f, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
          'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', 'a', 'b', 'c', \
          'x', 'y', 'z', '\0', \
    /* linkedinUser */ \
    0x07, 0x00, 'a', 'b', 'c', 'x', 'y', 'z', '\0', \
    /* lastCachedAt */ \
    0xcd, 0x51, 0x6d, 0x38, 0x00, 0x00, 0x00, 0x00, \
    /* logoUrl */ \
    0x1f, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'a', 'b', \
          'c', 'x', 'y', 'z', '.', 'c', 'o', 'm', '/', 'l', 'o', 'g', 'o', '.', \
          'j', 'p', 'g', '\0', \
    /* industries */ \
    0x01, 0x0, \
    0x09, 0x00, 0x00, 0x00, \
    0x0d, 0x00, 'I', 'N', 'D', 'U', 'S', 'T', 'R', 'Y', '_', 'A', 'B', 'C', '\0',

//
// Company2
//

void initTestData_Company2(struct Company* company);

void initTestData_CompanyCached2(struct CompanyCached* company);

#define COMPANY_2_ID 20

#define COMPANY_2_JSON_STRING \
    "{ \"id\": " MACRO_TO_STRING(COMPANY_2_ID) \
    ", \"parent_id\": 4," \
    " \"industries\": [ ]," \
    " \"stage\": \"Startup\"," \
    " \"number_employees\": 75," \
    " \"name\": \"DEFGH\"," \
    " \"domain\": \"Computers\"," \
    " \"headquarters_city\": \"San Jose\"," \
    " \"headquarters_state\": \"CA\"," \
    " \"headquarters_zipcode\": \"95113\"," \
    " \"url\": \"http:\\/\\/www.defgh.com\"," \
    " \"description\": \"DESCRIPTION_DEF\"," \
    " \"crunchbase_url\": \"http:\\/\\/www.crunchbase.com\\/defgh\"," \
    " \"crunchbase_headquarters\": \"San Jose\"," \
    " \"headquarters_country\": \"USA\"," \
    " \"facebook_url\": \"http:\\/\\/www.facebook.com\\/defgh\"," \
    " \"twitter_url\": \"http:\\/\\/www.twitter.com\\/defgh\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\\/defgh\"," \
    " \"linkedin_user\": \"defgh\"," \
    " \"last_cached_at\": \"2010-02-02T02:02:02Z\"," \
    " \"logo_url\": \"http:\\/\\/www.defgh.com\\/logo.jpg\"," \
    " \"company_industries\": [ ] }"

#define COMPANY_2_CACHED_PORTION_BINARY_SIZE \
    ( 4 + 4 + 2 + 10 + 4 )

#define COMPANY_2_BINARY_SIZE \
    ( COMPANY_2_CACHED_PORTION_BINARY_SIZE \
      + 8 + 12 + 11 + 5 + 8 + 23 + 18 + 34 + 11 + 6 \
      + 32 + 31 + 32 + 8 + 8 + 32 + 2 )

#define COMPANY_2_BINARY \
    /* Little endian */ \
    /* id */ \
    0x14, 0x00, 0x00, 0x00, \
    /* parentId */ \
    0x04, 0x00, 0x00, 0x00, \
    /* parent_industries */ \
    0x00, 0x00, \
    /* stage */ \
    0x08, 0x00, 'S', 't', 'a', 'r', 't', 'u', 'p', '\0', \
    /* numEmployees */ \
    0x4b, 0x00, 0x00, 0x00, \
    /* name */ \
    0x06, 0x00, 'D', 'E', 'F', 'G', 'H', '\0', \
    /* domain */ \
    0x0a, 0x00, 'C', 'o', 'm', 'p', 'u', 't', 'e', 'r', 's', '\0', \
    /* headquartersCity */ \
    0x09, 0x00, 'S', 'a', 'n', ' ', 'J', 'o', 's', 'e', '\0', \
    /* headquartersState */ \
    0x03, 0x00, 'C', 'A', '\0', \
    /* headquartersZipcode */ \
    0x06, 0x00, '9', '5', '1', '1', '3', '\0', \
    /* url */ \
    0x15, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'd', 'e', \
          'f', 'g', 'h', '.', 'c', 'o', 'm', '\0', \
    /* description */ \
    0x10, 0x00, 'D', 'E', 'S', 'C', 'R', 'I', 'P', 'T', 'I', 'O', 'N', \
          '_', 'D', 'E', 'F', '\0', \
    /* crunchbaseUrl */ \
    0x20, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'c', 'r', \
          'u', 'n', 'c', 'h', 'b', 'a', 's', 'e', '.', 'c', 'o', 'm', '/', 'd', \
          'e', 'f', 'g', 'h', '\0', \
    /* crunchbaseHeadquarters */ \
    0x09, 0x00, 'S', 'a', 'n', ' ', 'J', 'o', 's', 'e', '\0', \
    /* headquartersCountry */ \
    0x04, 0x00, 'U', 'S', 'A', '\0', \
    /* facebookUrl */ \
    0x1e, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'f', 'a', \
          'c', 'e', 'b', 'o', 'o', 'k', '.', 'c', 'o', 'm', '/', 'd', 'e', 'f', \
          'g', 'h', '\0', \
    /* twitterUrl */ \
    0x1d, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 't', 'w', \
          'i', 't', 't', 'e', 'r', '.', 'c', 'o', 'm', '/', 'd', 'e', 'f', 'g', \
          'h', '\0', \
    /* linkedinUrl */ \
    0x1e, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
          'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', 'd', 'e', 'f', \
          'g', 'h', '\0', \
    /* linkedinUser */ \
    0x06, 0x00, 'd', 'e', 'f', 'g', 'h', '\0', \
    /* lastCachedAt */ \
    0x9a, 0x87, 0x67, 0x4b, 0x00, 0x00, 0x00, 0x00, \
    /* logoUrl */ \
    0x1e, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'd', 'e', \
          'f', 'g', 'h', '.', 'c', 'o', 'm', '/', 'l', 'o', 'g', 'o', '.', 'j', \
          'p', 'g', '\0', \
    /* industries */ \
    0x00, 0x0,

#endif // COMPANY_TEST_DATA_H
