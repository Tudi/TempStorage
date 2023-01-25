#ifndef PROFILE_TEST_DATA_H
#define PROFILE_TEST_DATA_H

#include <profile_cached.h>
#include <profile_persistent.h>
#include <macro_utils.h>

//
// ProfilePersistent1
//

void initTestData_ProfilePersistent1(struct ProfilePersistent* profile);

void initTestData_ProfileCached1(struct ProfileCached* profile);

#define PROFILE_PERSISTENT_1_ID 16

#define PROFILE_PERSISTENT_1_JSON_STRING \
    "{ \"logical_delete\": false, \"id\": " MACRO_TO_STRING(PROFILE_PERSISTENT_1_ID) \
    ", \"first_name\": \"First\", \"last_name\": \"Last\"," \
    " \"locality_id\": 90210, \"country_id\": 1, \"state_id\": 2," \
    " \"lm\": [ { \"id\": 1, \"value\": \"1989-10-09T09:09:09Z\" }," \
    " { \"id\": 3, \"value\": \"1990-11-10T10:10:10Z\" } ]," \
    " \"lr\": [ { \"id\": 11, \"value\": \"1991-12-11T11:11:11Z\" }," \
    " { \"id\": 13, \"value\": \"1992-01-12T12:12:12Z\" } ]," \
    " \"lpr\": [ { \"id\": 21, \"value\": \"1993-02-13T13:13:13Z\" }," \
    " { \"id\": 23, \"value\": \"1994-03-14T14:14:14Z\" } ]," \
    " \"gs\": [ { \"id\": 31, \"value\": 2 }, { \"id\": 33, \"value\": 4 } ]," \
    " \"ps\": [ { \"id\": 41, \"value\": 12 }, { \"id\": 43, \"value\": 14 } ]," \
    " \"cs\": [ { \"id\": 51, \"value\": 22 }, { \"id\": 53, \"value\": 24 } ]," \
    " \"act\": [ 81, 83 ]," \
    " \"positions\": [ { \"company_name\": \"COMPANY_IJ\", \"title\": \"TITLE_AB\"," \
    " \"start_date\": \"1999-08-19T19:19:19Z\", \"end_date\": \"2000-09-20T20:20:20Z\"," \
    " \"company_id\": 12, \"title_id\": 78," \
    " \"uuid\": \"1234\", \"title_corrected\": \"TITLE_CD\", \"source\": \"SRC_EF\"," \
    " \"locality\": \"LOCAL_GH\", \"parent_company_id\": 34," \
    " \"description\": \"DESC_KL\", \"seniority\": \"SENIOR_MN\"," \
    " \"modified_in_load\": true, \"title_parent_id\": 79 } ]," \
    " \"skills\": [ \"SKILL_ABC\", \"SKILL_DEF\" ]," \
    " \"headline\": \"Engineer\", \"summary\": \"Nothing relevant.\"," \
    " \"talent_pools\": [ { \"id\": 81, \"value\": \"2004-01-24T00:24:24Z\" }," \
    " { \"id\": 83, \"value\": \"2005-02-25T01:25:25Z\" } ]," \
    " \"archived_primary_email\": \"f.last@email.com\", \"linkedin_user\": \"first.name\"," \
    " \"profile_code\": \"firstname123\", \"profile_key\": \"123\"," \
    " \"image_url\": \"http:\\/\\/www.someimage.com\\/image.jpeg\", \"industry\": \"IT\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\", \"num_connections\": 12," \
    " \"source\": \"SRC_456\", \"batch\": 34, \"covid19\": false," \
    " \"linkedin_canonical_url\": \"http:\\/\\/www.linkedin.com\\/firstname123\"," \
    " \"linkedin_canonical_user\": \"firstname1234\", \"linkedin_id\": \"1234\"," \
    " \"source_key\": \"SRC_KEY_789\", \"location\": \"Beverly Hills, CA, United States\"," \
    " \"country\": \"United States\", \"gender\": \"male\", \"ethnicity\": \"white\"," \
    " \"last_cached_at\": \"1987-08-07T07:07:07Z\"," \
    " \"ats\": [ 51, 53 ]," \
    " \"atss\": [ { \"id\": 61, \"value\": \"OK\" }, { \"id\": 63, \"value\": \"NOT OK\" } ]," \
    " \"atsla\": [ { \"id\": 71, \"value\": \"1995-04-15T15:15:15Z\" }," \
    " { \"id\": 73, \"value\": \"1996-05-16T16:16:16Z\" } ]," \
    " \"educations\": [ { \"uuid\": \"2468\", \"name\": \"Some College\", \"degree\": \"BSCS\"," \
    " \"subject\": \"Data Structures\", \"university_id\": 123," \
    " \"start_date\": \"2002-11-22T22:22:22Z\", \"end_date\": \"2003-12-23T23:23:23Z\" } ]," \
    " \"profile_emails\": [ { \"uuid\": \"1357\", \"email\": \"john.smith@somemail.com\"," \
    " \"toxic\": true, \"domain\": \"somemail.com\", \"gender\": \"male\", \"manual\": false," \
    " \"status\": \"OK\", \"bounced\": true, \"primary\": false, \"lastname\": \"Smith\"," \
    " \"firstname\": \"John\", \"mx_found\": true, \"personal\": false, \"mx_record\": \"mx\"," \
    " \"disposable\": true, \"free_email\": false, \"sub_status\": \"working\"," \
    " \"smtp_provider\": \"smtp.somemail.com\", \"domain_age_days\": 10 } ]," \
    " \"profile_social_urls\": [ { \"uuid\": \"4680\", \"source\": \"SRC_ABCD\"," \
    " \"url\": \"http:\\/\\/www.facebook.com\", \"username\": \"john.smith\" } ]," \
    " \"profile_phone_numbers\": [ { \"uuid\": \"3579\", \"country_code\": \"1\"," \
    " \"country_name\": \"United States\", \"calling_code\": \"00\"," \
    " \"international_number\": \"1\", \"local_number\": \"987-6543\" } ]," \
    " \"profile_tags\": [ { \"uuid\": \"5678\", \"tag_id\": 34, \"source\": \"SRC_EFGH\"," \
    " \"tag_type\": \"TAG_IJ\", \"text\": \"SOME_TEXT_KL\" } ] }"

#define PROFILE_PERSISTENT_1_CACHED_PORTION_BINARY_SIZE \
    ( 1 + 4 + 8 + 7 + 4 + 4 + 4 + 26 + 26 + 26 \
      + 18 + 18 + 18 + 10 + 119 + 26 + 11 + 20 + 26 )

#define PROFILE_PERSISTENT_1_BINARY_SIZE \
    ( PROFILE_PERSISTENT_1_CACHED_PORTION_BINARY_SIZE \
      + 19 + 13 + 15 + 6 + 38 + 5 + 26 + 4 + 10 + 4 \
      + 1 + 39 + 16 + 7 + 14 + 35 + 16 + 7 + 8 + 8 \
      + 10 + 24 + 26 + 73 + 124 + 59 + 49 + 52)

#define PROFILE_PERSISTENT_1_BINARY \
        /* Little endian */ \
        /* logicalDelete */ \
        0x00, \
        /* id */ \
        0x10, 0x00, 0x00, 0x00, \
        /* firstName */ \
        0x06, 0x00, 'F', 'i', 'r', 's', 't', 0x00, \
        /* lastName */ \
        0x05, 0x00, 'L', 'a', 's', 't', 0x00, \
        /* localityId */ \
        0x62, 0x60, 0x01, 0x00, \
        /* country id */ \
        0x01, 0x00, 0x00, 0x00, \
        /* state id */ \
        0x02, 0x00, 0x00, 0x00, \
        /* lastMessaged */ \
        0x02, 0x00, \
        0x01, 0x00, 0x00, 0x00, 0x35, 0x60, 0x30, 0x25, 0x00, 0x00, 0x00, 0x00, \
        0x03, 0x00, 0x00, 0x00, 0x02, 0xD2, 0x3B, 0x27, 0x00, 0x00, 0x00, 0x00, \
        /* lastReplied */ \
        0x02, 0x00, \
        0x0B, 0x00, 0x00, 0x00, 0x4F, 0xF2, 0x45, 0x29, 0x00, 0x00, 0x00, 0x00, \
        0x0D, 0x00, 0x00, 0x00, 0x9C, '0', 'p',   0x29, 0x00, 0x00, 0x00, 0x00, \
        /* lastPositiveReply */ \
        0x02, 0x00, \
        0x15, 0x00, 0x00, 0x00, 0xE9, 0xF3, 0x7C, 0x2B, 0x00, 0x00, 0x00, 0x00, \
        0x17, 0x00, 0x00, 0x00, '6', 'q',   0x84, 0x2D, 0x00, 0x00, 0x00, 0x00, \
        /* groups */ \
        0x02, 0x00, \
        0x1F, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, \
        0x21, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, \
        /* projects */ \
        0x02, 0x00, \
        0x29, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, \
        0x2B, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, \
        /* campaigns */ \
        0x02, 0x00, \
        0x33, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, \
        0x35, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, \
        /* actioned */ \
        0x02, 0x00, \
        0x51, 0x00, 0x00, 0x00, 0x53, 0x00, 0x00, 0x00, \
        /* positions */ \
        0x01, 0x00, \
        0x0B, 0x00, 'C', 'O', 'M', 'P', 'A', 'N', 'Y', '_', 'I', 'J', 0x00, \
        0x09, 0x00, 'T', 'I', 'T', 'L', 'E', '_', 'A', 'B', 0x00, \
        0xB7, 0x58, 0xBC, 0x37, 0x00, 0x00, 0x00, 0x00, \
        0x04, 0x1C, 0xC9, 0x39, 0x00, 0x00, 0x00, 0x00, \
        0x0C, 0x00, 0x00, 0x00, \
        0x4E, 0x0, 0x0, 0x0, \
        0x05, 0x00, '1', '2', '3', '4', 0x00, \
        0x09, 0x00, 'T', 'I', 'T', 'L', 'E', '_', 'C', 'D', 0x00, \
        0x07, 0x00, 'S', 'R', 'C', '_', 'E', 'F', 0x00, \
        0x09, 0x00, 'L', 'O', 'C', 'A', 'L', '_', 'G', 'H', 0x00, \
        0x22, 0x00, 0x00, 0x00, \
        0x08, 0x00, 'D', 'E', 'S', 'C', '_', 'K', 'L', 0x00, \
        0x0A, 0x00, 'S', 'E', 'N', 'I', 'O', 'R', '_', 'M', 'N', 0x00, \
        0x01, \
        0x4F, 0x0, 0x0, 0x0, \
        /* skills */ \
        0x02, 0x00, \
        0x0A, 0x00, 'S', 'K', 'I', 'L', 'L', '_', 'A', 'B', 'C', 0x00, \
        0x0A, 0x00, 'S', 'K', 'I', 'L', 'L', '_', 'D', 'E', 'F', 0x00, \
        /* headline */ \
        0x09, 0x00, 'E', 'n', 'g', 'i', 'n', 'e', 'e', 'r', 0x00, \
        /* summary */ \
        0x12, 0x00, 'N', 'o', 't', 'h', 'i', 'n', 'g', ' ', 'r', 'e', 'l', 'e', 'v', 'a', \
        'n', 't', '.', 0x00, \
        /* talent_pools */ \
        0x02, 0x00, \
        0x51, 0x00, 0x00, 0x00, 0x38, 0xbb, 0x11, 0x40, 0x00, 0x00, 0x00, 0x00, \
        0x53, 0x00, 0x00, 0x00, 0x85, 0x7e, 0x1e, 0x42, 0x00, 0x00, 0x00, 0x00, \
        /* archivedPrimaryEmail */ \
        0x11, 0x00, 'f', '.', 'l', 'a', 's', 't', '@', 'e', 'm', 'a', 'i', 'l', '.', 'c', \
        'o', 'm', 0x00, \
        /* linkedinUser */ \
        0x0B, 0x00, 'f', 'i', 'r', 's', 't', '.', 'n', 'a', 'm', 'e', 0x00, \
        /* profileCode */ \
        0x0D, 0x00, 'f', 'i', 'r', 's', 't', 'n', 'a', 'm', 'e', '1', '2', '3', 0x00, \
        /* profileKey */ \
        0x04, 0x00, '1', '2', '3', 0x00, \
        /* imageUrl */ \
        0x24, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 's', 'o', 'm', \
        'e', 'i', 'm', 'a', 'g', 'e', '.', 'c', 'o', 'm', '/', 'i', 'm', 'a', 'g', 'e', \
        '.', 'j', 'p', 'e', 'g', 0x00, \
        /* industry */ \
        0x03, 0x00, 'I', 'T', 0x00, \
        /* linkedinUrl */ \
        0x18, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', 'n', \
        'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', 0x00, \
        /* numConnections */ \
        0x0C, 0x00, 0x00, 0x00, \
        /* source */ \
        0x08, 0x00, 'S', 'R', 'C', '_', '4', '5', '6', 0x00, \
        /* batch */ \
        0x22, 0x00, 0x00, 0x00, \
        /* covid19 */ \
        0x00, \
        /* linkedinCanonicalUrl */ \
        0x25, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', \
        'w', 'w', '.', 'l', 'i', 'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', \
        'f', 'i', 'r', 's', 't', 'n', 'a', 'm', 'e', '1', '2', '3', 0x00, \
        /* linkedinCanonicalUser */ \
        0x0E, 0x00, 'f', 'i', 'r', 's', 't', 'n', 'a', 'm', 'e', '1', '2', '3', '4', 0x00, \
        /* linkedinId */ \
        0x05, 0x00, '1', '2', '3', '4', 0x00, \
        /* sourceKey */ \
        0x0C, 0x00, 'S', 'R', 'C', '_', 'K', 'E', 'Y', '_', '7', '8', '9', 0x00, \
        /* location */ \
        0x21, 0x00, 'B', 'e', 'v', 'e', 'r', 'l', 'y', ' ', 'H', 'i', 'l', 'l', 's', ',', \
        ' ', 'C', 'A', ',', ' ', 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', \
        'e', 's', 0x00, \
        /* country */ \
        0x0E, 0x00, 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', 's', 0x00, \
        /* gender */ \
        0x05, 0x00, 'm', 'a', 'l', 'e', 0x00, \
        /* ethnicity */ \
        0x06, 0x00, 'w', 'h', 'i', 't', 'e', 0x00, \
        /* lastCachedAt */ \
        0x9B, 0x7C, 0x19, 0x21, 0x00, 0x00, 0x00, 0x00, \
        /* atsCompanies */ \
        0x02, 0x00, \
        0x33, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, \
        /* atsStatus */ \
        0x02, 0x00, \
        0x3D, 0x00, 0x00, 0x00, \
        0x03, 0x00, 'O', 'K', 0x00, \
        0x3F, 0x00, 0x00, 0x00, \
        0x07, 0x00, 'N', 'O', 'T', ' ', 'O', 'K', 0x00, \
        /* atsLastactivity */ \
        0x02, 0x00, \
        0x47, 0x00, 0x00, 0x00, 0x03, 0xE3, 0x8F, 0x2F, 0x00, 0x00, 0x00, 0x00, \
        0x49, 0x00, 0x00, 0x00, 0xD0, 0x54, 0x9B, 0x31, 0x00, 0x00, 0x00, 0x00, \
        /* educations */ \
        0x01, 0x00, \
        0x05, 0x00, '2', '4', '6', '8', 0x00, \
        0x0D, 0x00, 'S', 'o', 'm', 'e', ' ', 'C', 'o', 'l', 'l', 'e', 'g', 'e', 0x00, \
        0x05, 0x00, 'B', 'S', 'C', 'S', 0x00, \
        0x10, 0x00, 'D', 'a', 't', 'a', ' ', 'S', 't', 'r', 'u', 'c', 't', 'u', 'r', 'e', \
        's', 0x00, \
        0x7B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0xAE, 0xDE, 0x3D, 0x00, 0x00, \
        0x00, 0x00, 0x6B, 0xCE, 0xE8, 0x3F, 0x00, 0x00, 0x00, 0x00, \
        /* profileEmails */ \
        0x01, 0x00, \
        0x05, 0x00, '1', '3', '5', '7', 0x00, \
        0x18, 0x00, 'j', 'o', 'h', 'n', '.', 's', 'm', 'i', 't', 'h', '@', 's', 'o', 'm', \
        'e', 'm', 'a', 'i', 'l', '.', 'c', 'o', 'm', 0x00, \
        0x01, \
        0x0D, 0x00, 's', 'o', 'm', 'e', 'm', 'a', 'i', 'l', '.', 'c', 'o', 'm', 0x00, \
        0x05, 0x00, 'm', 'a', 'l', 'e', 0x00, \
        0x00, \
        0x03, 0x00, 'O', 'K', 0x00, \
        0x01, \
        0x00, \
        0x06, 0x00, 'S', 'm', 'i', 't', 'h', 0x00, \
        0x05, 0x00, 'J', 'o', 'h', 'n', 0x00, \
        0x01, \
        0x00, \
        0x03, 0x00, 'm', 'x', 0x00, \
        0x01, \
        0x00, \
        0x08, 0x00, 'w', 'o', 'r', 'k', 'i', 'n', 'g', 0x00, \
        0x12, 0x00, 's', 'm', 't', 'p', '.', 's', 'o', 'm', 'e', 'm', 'a', 'i', 'l', '.', \
        'c', 'o', 'm', 0x00, \
        0x0A, 0x00, 0x00, 0x00, \
        /* profileSocialUrls */ \
        0x01, 0x00, \
        0x05, 0x00, '4', '6', '8', '0', 0x00, \
        0x09, 0x00, 'S', 'R', 'C', '_', 'A', 'B', 'C', 'D', 0x00, \
        0x18, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'f', 'a', 'c', \
        'e', 'b', 'o', 'o', 'k', '.', 'c', 'o', 'm', 0x00, \
        0x0B, 0x00, 'j', 'o', 'h', 'n', '.', 's', 'm', 'i', 't', 'h', 0x00, \
        /* profilePhoneNumbers */ \
        0x01, 0x00, \
        0x05, 0x00, '3', '5', '7', '9', 0x00, \
        0x02, 0x00, 0x31, 0x00, \
        0x0E, 0x00, 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', 's', 0x00, \
        0x03, 0x00, '0', '0', 0x00, \
        0x02, 0x00, 0x31, 0x00, 0x09, 0x00, '9', '8', '7', '-', '6', '5', '4', '3', 0x00, \
        /* profileTags */ \
        0x01, 0x00, \
        0x05, 0x00, '5', '6', '7', '8', 0x00, \
        0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x09, 0x00, 'S', 'R', 'C', '_', 'E', 'F', 'G', 'H', 0x00, \
        0x07, 0x00, 'T', 'A', 'G', '_', 'I', 'J', 0x00, \
        0x0D, 0x00, 'S', 'O', 'M', 'E', '_', 'T', 'E', 'X', 'T', '_', 'K', 'L', 0x00,

//
// ProfilePersistent2
//

void initTestData_ProfilePersistent2(struct ProfilePersistent* profile);

void initTestData_ProfileCached2(struct ProfileCached* profile);

#define PROFILE_PERSISTENT_2_ID 20

#define PROFILE_PERSISTENT_2_JSON_STRING \
    "{ \"logical_delete\": false, \"id\": " MACRO_TO_STRING(PROFILE_PERSISTENT_2_ID) \
    ", \"first_name\": \"Second\", \"last_name\": \"Last\","\
    " \"locality_id\": 90210, \"country_id\": 1, \"state_id\": 2," \
    " \"lm\": [ ], \"lr\": [ ], \"lpr\": [ ], \"gs\": [ ], \"ps\": [ ], \"cs\": [ ], \"act\": [ ]," \
    " \"positions\": [ ], \"skills\": [ ], \"headline\": \"Engineer\"," \
    " \"summary\": \"Nothing relevant.\", \"talent_pools\": [ ]," \
    " \"archived_primary_email\": \"s.last@email.com\", \"linkedin_user\": \"second.name\"," \
    " \"profile_code\": \"secondname456\", \"profile_key\": \"456\"," \
    " \"image_url\": \"http:\\/\\/www.someimage.com\\/image2.jpeg\", \"industry\": \"IT\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\", \"num_connections\": 34," \
    " \"source\": \"SRC_789\", \"batch\": 56, \"covid19\": false," \
    " \"linkedin_canonical_url\": \"http:\\/\\/www.linkedin.com\\/secondname456\"," \
    " \"linkedin_canonical_user\": \"secondname7890\", \"linkedin_id\": \"7890\"," \
    " \"source_key\": \"SRC_KEY_015\", \"location\": \"Beverly Hills, CA, United States\"," \
    " \"country\": \"United States\", \"gender\": \"female\", \"ethnicity\": \"white\"," \
    " \"last_cached_at\": \"1997-08-07T07:07:07Z\", \"ats\": [ ], \"atss\": [ ], \"atsla\": [ ]," \
    " \"educations\": [ ], \"profile_emails\": [ ], \"profile_social_urls\": [ ]," \
    " \"profile_phone_numbers\": [ ], \"profile_tags\": [ ] }"

#define PROFILE_PERSISTENT_2_CACHED_PORTION_BINARY_SIZE \
    ( 1 + 4 + 9 + 7 + 4 + 4 + 4 + 2 + 2 + 2 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 11 + 20 + 2 )

#define PROFILE_PERSISTENT_2_BINARY_SIZE \
    ( PROFILE_PERSISTENT_2_CACHED_PORTION_BINARY_SIZE \
      + 19 + 14 + 16 + 6 + 39 + 5 + 26 + 4 + 10 + 4 \
      + 1 + 40 + 17 + 7 + 14 + 35 + 16 + 9 + 8 + 8 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2)

#define PROFILE_PERSISTENT_2_BINARY \
        /* Little endian */ \
        /* logicalDelete */ \
        0x00, \
        /* id */ \
        0x14, 0x00, 0x00, 0x00, \
        /* firstName */ \
        0x07, 0x00, 'S', 'e', 'c', 'o', 'n', 'd', 0x00, \
        /* lastName */ \
        0x05, 0x00, 'L', 'a', 's', 't', 0x00, \
        /* localityId */ \
        0x62, 0x60, 0x01, 0x00, \
        /* countryId */ \
        0x01, 0x00, 0x00, 0x00, \
        /* stateId */ \
        0x02, 0x00, 0x00, 0x00, \
        /* lastMessaged */ \
        0x00, 0x00, \
        /* lastReplied */ \
        0x00, 0x00, \
        /* lastPositiveReply */ \
        0x00, 0x00, \
        /* groups */ \
        0x00, 0x00, \
        /* projects */ \
        0x00, 0x00, \
        /* campaigns */ \
        0x00, 0x00, \
        /* actioned */ \
        0x00, 0x00, \
        /* positions */ \
        0x00, 0x00, \
        /* skills */ \
        0x00, 0x00, \
        /* headline */ \
        0x09, 0x00, 'E', 'n', 'g', 'i', 'n', 'e', 'e', 'r', 0x00, \
        /* summary */ \
        0x12, 0x00, 'N', 'o', 't', 'h', 'i', 'n', 'g', ' ', 'r', 'e', 'l', 'e', 'v', 'a', \
        'n', 't', '.', 0x00, \
        /* talent_pools */ \
        0x00, 0x00, \
        /* archivedPrimaryEmail */ \
        0x11, 0x00, 's', '.', 'l', 'a', 's', 't', '@', 'e', 'm', 'a', 'i', 'l', '.', 'c', \
        'o', 'm', 0x00, \
        /* linkedinUser */ \
        0x0C, 0x00, 's', 'e', 'c', 'o', 'n', 'd', '.', 'n', 'a', 'm', 'e', 0x00, \
        /* profileCode */ \
        0x0E, 0x00, 's', 'e', 'c', 'o', 'n', 'd', 'n', 'a', 'm', 'e', '4', '5', '6', 0x00, \
        /* profileKey */ \
        0x04, 0x00, '4', '5', '6', 0x00, \
        /* imageUrl */ \
        0x25, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 's', 'o', 'm', \
        'e', 'i', 'm', 'a', 'g', 'e', '.', 'c', 'o', 'm', '/', 'i', 'm', 'a', 'g', 'e', \
        '2', '.', 'j', 'p', 'e', 'g', 0x00, \
        /* industry */ \
        0x03, 0x00, 'I', 'T', 0x00, \
        /* linkedinUrl */ \
        0x18, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', 'n', \
        'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', 0x00, \
        /* numConnections */ \
        0x22, 0x00, 0x00, 0x00, \
        /* source */ \
        0x08, 0x00, 'S', 'R', 'C', '_', '7', '8', '9', 0x00, \
        /* batch */ \
        0x38, 0x00, 0x00, 0x00, \
        /* covid19 */ \
        0x00, \
        /* linkedinCanonicalUrl */ \
        0x26, 0x00, 'h', 't', 't', 'p', ':', '/', '/', 'w', \
        'w', 'w', '.', 'l', 'i', 'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', \
        's', 'e', 'c', 'o', 'n', 'd', 'n', 'a', 'm', 'e', '4', '5', '6', 0x00, \
        /* linkedinCanonicalUser */ \
        0x0F, 0x00, 's', 'e', 'c', 'o', 'n', 'd', 'n', 'a', 'm', 'e', '7', '8', '9', '0', 0x00, \
        /* linkedinId */ \
        0x05, 0x00, '7', '8', '9', '0', 0x00, \
        /* sourceKey */ \
        0x0C, 0x00, 'S', 'R', 'C', '_', 'K', 'E', 'Y', '_', '0', '1', '5', 0x00, \
        /* location */ \
        0x21, 0x00, 'B', 'e', 'v', 'e', 'r', 'l', 'y', ' ', 'H', 'i', 'l', 'l', 's', ',', \
        ' ', 'C', 'A', ',', ' ', 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', \
        'e', 's', 0x00, \
        /* country */ \
        0x0E, 0x00, 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', 's', 0x00, \
        /* gender */ \
        0x07, 0x00, 'f', 'e', 'm', 'a', 'l', 'e', 0x00, \
        /* ethnicity */ \
        0x06, 0x00, 'w', 'h', 'i', 't', 'e', 0x00, \
        /* lastCachedAt */ \
        0x1B, 0x74, 0xE9, 0x33, 0x00, 0x00, 0x00, 0x00, \
        /* atsCompanies */ \
        0x00, 0x00, \
        /* atsStatus */ \
        0x00, 0x00, \
        /* atsLastactivity */ \
        0x00, 0x00, \
        /* educations */ \
        0x00, 0x00, \
        /* profileEmails */ \
        0x00, 0x00, \
        /* profileSocialUrls */ \
        0x00, 0x00, \
        /* profilePhoneNumbers */ \
        0x00, 0x00, \
        /* profileTags */ \
        0x00, 0x00,

//
// ProfilePersistent3
//

void initTestData_ProfilePersistent3(struct ProfilePersistent* profile);

void initTestData_ProfileCached3(struct ProfileCached* profile);

#define PROFILE_PERSISTENT_3_ID 22

#define PROFILE_PERSISTENT_3_JSON_STRING \
    "{ \"logical_delete\": false, \"id\": " MACRO_TO_STRING(PROFILE_PERSISTENT_3_ID) \
    ", \"first_name\": \"Third\", \"last_name\": \"Last\"," \
    " \"locality_id\": 90210, \"country_id\": 1, \"state_id\": 3," \
    " \"lm\": [ ], \"lr\": [ ], \"lpr\": [ ], \"gs\": [ ], \"ps\": [ ], \"cs\": [ ], \"act\": [ ]," \
    " \"positions\": [ ], \"skills\": [ ], \"headline\": \"Tester\"," \
    " \"summary\": \"Nothing relevant.\", \"talent_pools\": [ ]," \
    " \"archived_primary_email\": \"t.last@email.com\", \"linkedin_user\": \"third.name\"," \
    " \"profile_code\": \"thirdname789\", \"profile_key\": \"789\"," \
    " \"image_url\": \"http:\\/\\/www.someimage.com\\/image3.jpeg\", \"industry\": \"IT\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\", \"num_connections\": 56," \
    " \"source\": \"SRC_012\", \"batch\": 78, \"covid19\": false," \
    " \"linkedin_canonical_url\": \"http:\\/\\/www.linkedin.com\\/thirdname789\"," \
    " \"linkedin_canonical_user\": \"thirdname1234\"," \
    " \"linkedin_id\": \"1234\", \"source_key\": \"SRC_KEY_020\"," \
    " \"location\": \"Beverly Hills, CA, United States\", \"country\": \"United States\"," \
    " \"gender\": \"male\", \"ethnicity\": \"black\"," \
    " \"last_cached_at\": \"1998-09-08T08:08:08Z\"," \
    " \"ats\": [ ], \"atss\": [ ], \"atsla\": [ ], \"educations\": [ ], \"profile_emails\": [ ]," \
    " \"profile_social_urls\": [ ], \"profile_phone_numbers\": [ ], \"profile_tags\": [ ] }"

#define PROFILE_PERSISTENT_3_CACHED_PORTION_BINARY_SIZE \
    ( 1 + 4 + 8 + 7 + 4 + 4 + 4 + 2 + 2 + 2 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 9 + 20 + 2 )

#define PROFILE_PERSISTENT_3_BINARY_SIZE \
    ( PROFILE_PERSISTENT_3_CACHED_PORTION_BINARY_SIZE \
      + 19 + 13 + 15 + 6 + 39 + 5 + 26 + 4 + 10 + 4 \
      + 1 + 39 + 16 + 7 + 14 + 35 + 16 + 7 + 8 + 8 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2)

#define PROFILE_PERSISTENT_3_BINARY \
        /* Little endian */ \
        /* logicalDelete */ \
        0x0, \
        /* id */ \
        0x16, 0x0, 0x0, 0x0, \
        /* firstName */ \
        0x6, 0x0, 'T', 'h', 'i', 'r', 'd', '\0', \
        /* lastName */ \
        0x5, 0x0, 'L', 'a', 's', 't', '\0', \
        /* localityId */ \
        0x62, 0x60, 0x1, 0x0, \
        /* countryId */ \
        0x1, 0x0, 0x0, 0x0, \
        /* stateId */ \
        0x3, 0x0, 0x0, 0x0, \
        /* lastMessaged */ \
        0x00, 0x00, \
        /* lastReplied */ \
        0x00, 0x00, \
        /* lastPositiveReply */ \
        0x00, 0x00, \
        /* groups */ \
        0x00, 0x00, \
        /* projects */ \
        0x00, 0x00, \
        /* campaigns */ \
        0x00, 0x00, \
        /* actioned */ \
        0x00, 0x00, \
        /* positions */ \
        0x00, 0x00, \
        /* skills */ \
        0x00, 0x00, \
        /* headline */ \
        0x7, 0x0, 'T', 'e', 's', 't', 'e', 'r', '\0', \
        /* summary */ \
        0x12, 0x0, 'N', 'o', 't', 'h', 'i', 'n', 'g', ' ', 'r', 'e', 'l', 'e', 'v', \
              'a', 'n', 't', '.', '\0', \
        /* talent_pools */ \
        0x00, 0x00, \
        /* archivedPrimaryEmail */ \
        0x11, 0x0, 't', '.', 'l', 'a', 's', 't', '@', 'e', 'm', 'a', 'i', 'l', '.', \
              'c', 'o', 'm', '\0', \
        /* linkedinUser */ \
        0xb, 0x0, 't', 'h', 'i', 'r', 'd', '.', 'n', 'a', 'm', 'e', '\0', \
        /* profileCode */ \
        0xd, 0x0, 't', 'h', 'i', 'r', 'd', 'n', 'a', 'm', 'e', '7', '8', '9', '\0', \
        /* profileKey */ \
        0x4, 0x0, '7', '8', '9', '\0', \
        /* imageUrl */ \
        0x25, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 's', 'o', \
              'm', 'e', 'i', 'm', 'a', 'g', 'e', '.', 'c', 'o', 'm', '/', 'i', 'm', \
              'a', 'g', 'e', '3', '.', 'j', 'p', 'e', 'g', '\0', \
        /* industry */ \
        0x3, 0x0, 'I', 'T', '\0', \
        /* linkedinUrl */ \
        0x18, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
              'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '\0', \
        /* numConnections */ \
        0x38, 0x0, 0x0, 0x0, \
        /* source */ \
        0x8, 0x0, 'S', 'R', 'C', '_', '0', '1', '2', '\0', \
        /* batch */ \
        0x4e, 0x0, 0x0, 0x0, \
        /* covid19 */ \
        0x0, \
        /* linkedinCanonicalUrl */ \
        0x25, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
              'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', 't', 'h', 'i', \
              'r', 'd', 'n', 'a', 'm', 'e', '7', '8', '9', '\0', \
        /* linkedinCanonicalUser */ \
        0xe, 0x0, 't', 'h', 'i', 'r', 'd', 'n', 'a', 'm', 'e', '1', '2', '3', '4', '\0', \
        /* linkedinId */ \
        0x5, 0x0, '1', '2', '3', '4', '\0', \
        /* sourceKey */ \
        0xc, 0x0, 'S', 'R', 'C', '_', 'K', 'E', 'Y', '_', '0', '2', '0', '\0', \
        /* location */ \
        0x21, 0x0, 'B', 'e', 'v', 'e', 'r', 'l', 'y', ' ', 'H', 'i', 'l', 'l', 's', ',', ' ', \
              'C', 'A', ',', ' ', 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', \
              's', '\0', \
        /* country */ \
        0xe, 0x0, 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', 's', '\0', \
        /* gender */ \
        0x5, 0x0, 'm', 'a', 'l', 'e', '\0', \
        /* ethnicity */ \
        0x6, 0x0, 'b', 'l', 'a', 'c', 'k', '\0', \
        /* lastCachedAt */ \
        0xe8, 0xe5, 0xf4, 0x35, 0x0, 0x0, 0x0, 0x0, \
        /* atsCompanies */ \
        0x00, 0x00, \
        /* atsStatus */ \
        0x00, 0x00, \
        /* atsLastactivity */ \
        0x00, 0x00, \
        /* educations */ \
        0x00, 0x00, \
        /* profileEmails */ \
        0x00, 0x00, \
        /* profileSocialUrls */ \
        0x00, 0x00, \
        /* profilePhoneNumbers */ \
        0x00, 0x00, \
        /* profileTags */ \
        0x00, 0x00,

//
// ProfilePersistent4
//

void initTestData_ProfilePersistent4(struct ProfilePersistent* profile);

void initTestData_ProfileCached4(struct ProfileCached* profile);

#define PROFILE_PERSISTENT_4_ID 24

#define PROFILE_PERSISTENT_4_JSON_STRING \
    "{ \"logical_delete\": false, \"id\": " MACRO_TO_STRING(PROFILE_PERSISTENT_4_ID) \
    ", \"first_name\": \"Fourth\", \"last_name\": \"Last\"," \
    " \"locality_id\": 90210, \"country_id\": 1, \"state_id\": 4," \
    " \"lm\": [ ], \"lr\": [ ], \"lpr\": [ ], \"gs\": [ ], \"ps\": [ ], \"cs\": [ ], \"act\": [ ]," \
    " \"positions\": [ ], \"skills\": [ ], \"headline\": \"Developer\"," \
    " \"summary\": \"Nothing relevant.\", \"talent_pools\": [ ]," \
    " \"archived_primary_email\": \"f.last@email.com\", \"linkedin_user\": \"fourth.name\"," \
    " \"profile_code\": \"fourthname012\", \"profile_key\": \"012\"," \
    " \"image_url\": \"http:\\/\\/www.someimage.com\\/image4.jpeg\", \"industry\": \"IT\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\", \"num_connections\": 78," \
    " \"source\": \"SRC_345\", \"batch\": 90, \"covid19\": false," \
    " \"linkedin_canonical_url\": \"http:\\/\\/www.linkedin.com\\/fourthname345\"," \
    " \"linkedin_canonical_user\": \"fourthname5678\"," \
    " \"linkedin_id\": \"5678\", \"source_key\": \"SRC_KEY_025\"," \
    " \"location\": \"Beverly Hills, CA, United States\", \"country\": \"United States\"," \
    " \"gender\": \"male\", \"ethnicity\": \"black\"," \
    " \"last_cached_at\": \"1999-10-09T09:09:09Z\"," \
    " \"ats\": [ ], \"atss\": [ ], \"atsla\": [ ], \"educations\": [ ], \"profile_emails\": [ ]," \
    " \"profile_social_urls\": [ ], \"profile_phone_numbers\": [ ], \"profile_tags\": [ ] }"

#define PROFILE_PERSISTENT_4_CACHED_PORTION_BINARY_SIZE \
    ( 1 + 4 + 9 + 7 + 4 + 4 + 4 + 2 + 2 + 2 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 12 + 20 + 2 )

#define PROFILE_PERSISTENT_4_BINARY_SIZE \
    ( PROFILE_PERSISTENT_4_CACHED_PORTION_BINARY_SIZE \
      + 19 + 14 + 16 + 6 + 39 + 5 + 26 + 4 + 10 + 4 \
      + 1 + 40 + 17 + 7 + 14 + 35 + 16 + 7 + 8 + 8 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2)

#define PROFILE_PERSISTENT_4_BINARY \
        /* Little endian */ \
        /* logicalDelete */ \
        0x0, \
        /* id */ \
        0x18, 0x0, 0x0, 0x0, \
        /* firstName */ \
        0x7, 0x0, 'F', 'o', 'u', 'r', 't', 'h', '\0', \
        /* lastName */ \
        0x5, 0x0, 'L', 'a', 's', 't', '\0', \
        /* localityId */ \
        0x62, 0x60, 0x1, 0x0, \
        /* countryId */ \
        0x1, 0x0, 0x0, 0x0, \
        /* stateId */ \
        0x4, 0x0, 0x0, 0x0, \
        /* lastMessaged */ \
        0x00, 0x00, \
        /* lastReplied */ \
        0x00, 0x00, \
        /* lastPositiveReply */ \
        0x00, 0x00, \
        /* groups */ \
        0x00, 0x00, \
        /* projects */ \
        0x00, 0x00, \
        /* campaigns */ \
        0x00, 0x00, \
        /* actioned */ \
        0x00, 0x00, \
        /* positions */ \
        0x00, 0x00, \
        /* skills */ \
        0x00, 0x00, \
        /* headline */ \
        0xa, 0x0, 'D', 'e', 'v', 'e', 'l', 'o', 'p', 'e', 'r', '\0', \
        /* summary */ \
        0x12, 0x0, 'N', 'o', 't', 'h', 'i', 'n', 'g', ' ', 'r', 'e', 'l', 'e', 'v', \
              'a', 'n', 't', '.', '\0', \
        /* talent_pools */ \
        0x00, 0x00, \
        /* archivedPrimaryEmail */ \
        0x11, 0x0, 'f', '.', 'l', 'a', 's', 't', '@', 'e', 'm', 'a', 'i', 'l', '.', \
              'c', 'o', 'm', '\0', \
        /* linkedinUser */ \
        0xc, 0x0, 'f', 'o', 'u', 'r', 't', 'h', '.', 'n', 'a', 'm', 'e', '\0', \
        /* profileCode */ \
        0xe, 0x0, 'f', 'o', 'u', 'r', 't', 'h', 'n', 'a', 'm', 'e', '0', '1', '2', '\0', \
        /* profileKey */ \
        0x4, 0x0, '0', '1', '2', '\0', \
        /* imageUrl */ \
        0x25, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 's', 'o', \
              'm', 'e', 'i', 'm', 'a', 'g', 'e', '.', 'c', 'o', 'm', '/', 'i', 'm', \
              'a', 'g', 'e', '4', '.', 'j', 'p', 'e', 'g', '\0', \
        /* industry */ \
        0x3, 0x0, 'I', 'T', '\0', \
        /* linkedinUrl */ \
        0x18, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
              'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '\0', \
        /* numConnections */ \
        0x4e, 0x0, 0x0, 0x0, \
        /* source */ \
        0x8, 0x0, 'S', 'R', 'C', '_', '3', '4', '5', '\0', \
        /* batch */ \
        0x5a, 0x0, 0x0, 0x0, \
        /* covid19 */ \
        0x0, \
        /* linkedinCanonicalUrl */ \
        0x26, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
              'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', 'f', 'o', 'u', \
              'r', 't', 'h', 'n', 'a', 'm', 'e', '3', '4', '5', '\0', \
        /* linkedinCanonicalUser */ \
        0xf, 0x0, 'f', 'o', 'u', 'r', 't', 'h', 'n', 'a', 'm', 'e', '5', '6', '7', '8', '\0', \
        /* linkedinId */ \
        0x5, 0x0, '5', '6', '7', '8', '\0', \
        /* sourceKey */ \
        0xc, 0x0, 'S', 'R', 'C', '_', 'K', 'E', 'Y', '_', '0', '2', '5', '\0', \
        /* location */ \
        0x21, 0x0, 'B', 'e', 'v', 'e', 'r', 'l', 'y', ' ', 'H', 'i', 'l', 'l', 's', ',', ' ', \
              'C', 'A', ',', ' ', 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', \
              's', '\0', \
        /* country */ \
        0xe, 0x0, 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', 's', '\0', \
        /* gender */ \
        0x5, 0x0, 'm', 'a', 'l', 'e', '\0', \
        /* ethnicity */ \
        0x6, 0x0, 'b', 'l', 'a', 'c', 'k', '\0', \
        /* lastCachedAt */ \
        0x35, 0x6, 0xff, 0x37, 0x0, 0x0, 0x0, 0x0, \
        /* atsCompanies */ \
        0x00, 0x00, \
        /* atsStatus */ \
        0x00, 0x00, \
        /* atsLastactivity */ \
        0x00, 0x00, \
        /* educations */ \
        0x00, 0x00, \
        /* profileEmails */ \
        0x00, 0x00, \
        /* profileSocialUrls */ \
        0x00, 0x00, \
        /* profilePhoneNumbers */ \
        0x00, 0x00, \
        /* profileTags */ \
        0x00, 0x00,

//
// ProfilePersistent5
//

void initTestData_ProfilePersistent5(struct ProfilePersistent* profile);

void initTestData_ProfileCached5(struct ProfileCached* profile);

#define PROFILE_PERSISTENT_5_ID 26

#define PROFILE_PERSISTENT_5_JSON_STRING \
    "{ \"logical_delete\": false, \"id\": " MACRO_TO_STRING(PROFILE_PERSISTENT_5_ID) \
    ", \"first_name\": \"Fifth\", \"last_name\": \"Last\"," \
    " \"locality_id\": 90210, \"country_id\": 1, \"state_id\": 5," \
    " \"lm\": [ ], \"lr\": [ ], \"lpr\": [ ], \"gs\": [ ], \"ps\": [ ], \"cs\": [ ], \"act\": [ ]," \
    " \"positions\": [ ], \"skills\": [ ], \"headline\": \"Tester\"," \
    " \"summary\": \"Nothing relevant.\", \"talent_pools\": [ ]," \
    " \"archived_primary_email\": \"f.last@email.com\", \"linkedin_user\": \"fifth.name\"," \
    " \"profile_code\": \"fifthname345\", \"profile_key\": \"345\"," \
    " \"image_url\": \"http:\\/\\/www.someimage.com\\/image5.jpeg\", \"industry\": \"IT\"," \
    " \"linkedin_url\": \"http:\\/\\/www.linkedin.com\", \"num_connections\": 90," \
    " \"source\": \"SRC_678\", \"batch\": 12, \"covid19\": false," \
    " \"linkedin_canonical_url\": \"http:\\/\\/www.linkedin.com\\/fifthname678\"," \
    " \"linkedin_canonical_user\": \"fifthname9012\"," \
    " \"linkedin_id\": \"9012\", \"source_key\": \"SRC_KEY_030\"," \
    " \"location\": \"Beverly Hills, CA, United States\", \"country\": \"United States\"," \
    " \"gender\": \"male\", \"ethnicity\": \"white\"," \
    " \"last_cached_at\": \"2000-11-10T10:10:10Z\"," \
    " \"ats\": [ ], \"atss\": [ ], \"atsla\": [ ], \"educations\": [ ], \"profile_emails\": [ ]," \
    " \"profile_social_urls\": [ ], \"profile_phone_numbers\": [ ], \"profile_tags\": [ ] }"

#define PROFILE_PERSISTENT_5_CACHED_PORTION_BINARY_SIZE \
    ( 1 + 4 + 8 + 7 + 4 + 4 + 4 + 2 + 2 + 2 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 9 + 20 + 2 )

#define PROFILE_PERSISTENT_5_BINARY_SIZE \
    ( PROFILE_PERSISTENT_5_CACHED_PORTION_BINARY_SIZE \
      + 19 + 13 + 15 + 6 + 39 + 5 + 26 + 4 + 10 + 4 \
      + 1 + 39 + 16 + 7 + 14 + 35 + 16 + 7 + 8 + 8 \
      + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2)

#define PROFILE_PERSISTENT_5_BINARY \
        /* Little endian */ \
        /* logicalDelete */ \
        0x0, \
        /* id */ \
        0x1a, 0x0, 0x0, 0x0, \
        /* firstName */ \
        0x6, 0x0, 'F', 'i', 'f', 't', 'h', '\0', \
        /* lastName */ \
        0x5, 0x0, 'L', 'a', 's', 't', '\0', \
        /* localityId */ \
        0x62, 0x60, 0x1, 0x0, \
        /* countryId */ \
        0x1, 0x0, 0x0, 0x0, \
        /* stateId */ \
        0x5, 0x0, 0x0, 0x0, \
        /* lastMessaged */ \
        0x00, 0x00, \
        /* lastReplied */ \
        0x00, 0x00, \
        /* lastPositiveReply */ \
        0x00, 0x00, \
        /* groups */ \
        0x00, 0x00, \
        /* projects */ \
        0x00, 0x00, \
        /* campaigns */ \
        0x00, 0x00, \
        /* actioned */ \
        0x00, 0x00, \
        /* positions */ \
        0x00, 0x00, \
        /* skills */ \
        0x00, 0x00, \
        /* headline */ \
        0x7, 0x0, 'T', 'e', 's', 't', 'e', 'r', '\0', \
        /* summary */ \
        0x12, 0x0, 'N', 'o', 't', 'h', 'i', 'n', 'g', ' ', 'r', 'e', 'l', 'e', 'v', \
              'a', 'n', 't', '.', '\0', \
        /* talent_pools */ \
        0x00, 0x00, \
        /* archivedPrimaryEmail */ \
        0x11, 0x0, 'f', '.', 'l', 'a', 's', 't', '@', 'e', 'm', 'a', 'i', 'l', '.', \
              'c', 'o', 'm', '\0', \
        /* linkedinUser */ \
        0xb, 0x0, 'f', 'i', 'f', 't', 'h', '.', 'n', 'a', 'm', 'e', '\0', \
        /* profileCode */ \
        0xd, 0x0, 'f', 'i', 'f', 't', 'h', 'n', 'a', 'm', 'e', '3', '4', '5', '\0', \
        /* profileKey */ \
        0x4, 0x0, '3', '4', '5', '\0', \
        /* imageUrl */ \
        0x25, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 's', 'o', \
              'm', 'e', 'i', 'm', 'a', 'g', 'e', '.', 'c', 'o', 'm', '/', 'i', 'm', \
              'a', 'g', 'e', '5', '.', 'j', 'p', 'e', 'g', '\0', \
        /* industry */ \
        0x3, 0x0, 'I', 'T', '\0', \
        /* linkedinUrl */ \
        0x18, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
              'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '\0', \
        /* numConnections */ \
        0x5a, 0x0, 0x0, 0x0, \
        /* source */ \
        0x8, 0x0, 'S', 'R', 'C', '_', '6', '7', '8', '\0', \
        /* batch */ \
        0x0c, 0x0, 0x0, 0x0, \
        /* covid19 */ \
        0x0, \
        /* linkedinCanonicalUrl */ \
        0x25, 0x0, 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'l', 'i', \
              'n', 'k', 'e', 'd', 'i', 'n', '.', 'c', 'o', 'm', '/', 'f', 'i', 'f', \
              't', 'h', 'n', 'a', 'm', 'e', '6', '7', '8', '\0', \
        /* linkedinCanonicalUser */ \
        0xe, 0x0, 'f', 'i', 'f', 't', 'h', 'n', 'a', 'm', 'e', '9', '0', '1', '2', '\0', \
        /* linkedinId */ \
        0x5, 0x0, '9', '0', '1', '2', '\0', \
        /* sourceKey */ \
        0xc, 0x0, 'S', 'R', 'C', '_', 'K', 'E', 'Y', '_', '0', '3', '0', '\0', \
        /* location */ \
        0x21, 0x0, 'B', 'e', 'v', 'e', 'r', 'l', 'y', ' ', 'H', 'i', 'l', 'l', 's', ',', ' ', \
              'C', 'A', ',', ' ', 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', \
              's', '\0', \
        /* country */ \
        0xe, 0x0, 'U', 'n', 'i', 't', 'e', 'd', ' ', 'S', 't', 'a', 't', 'e', 's', '\0', \
        /* gender */ \
        0x5, 0x0, 'm', 'a', 'l', 'e', '\0', \
        /* ethnicity */ \
        0x6, 0x0, 'w', 'h', 'i', 't', 'e', '\0', \
        /* lastCachedAt */ \
        0x82, 0xc9, 0xb, 0x3a, 0x0, 0x0, 0x0, 0x0, \
        /* atsCompanies */ \
        0x00, 0x00, \
        /* atsStatus */ \
        0x00, 0x00, \
        /* atsLastactivity */ \
        0x00, 0x00, \
        /* educations */ \
        0x00, 0x00, \
        /* profileEmails */ \
        0x00, 0x00, \
        /* profileSocialUrls */ \
        0x00, 0x00, \
        /* profilePhoneNumbers */ \
        0x00, 0x00, \
        /* profileTags */ \
        0x00, 0x00,

#endif // PROFILE_TEST_DATA_H
