#include <profile_test_data.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <kvec.h>

//
// Constants
//

//
// ProfilePersistent1
//

static struct tm profile1_lastCachedAt = { .tm_sec = 7, .tm_min = 7, .tm_hour = 7,
    .tm_mday = 7, .tm_mon = 7, .tm_year = 87, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1987-Aug-07 07:07:07

static struct tm profile1_lastMessaged1 = { .tm_sec = 9, .tm_min = 9, .tm_hour = 9,
    .tm_mday = 9, .tm_mon = 9, .tm_year = 89, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1989-Oct-09 09:09:09

static struct tm profile1_lastMessaged2 = { .tm_sec = 10, .tm_min = 10, .tm_hour = 10,
    .tm_mday = 10, .tm_mon = 10, .tm_year = 90, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1990-Nov-10 10:10:10

static struct tm profile1_lastReplied1 = { .tm_sec = 11, .tm_min = 11, .tm_hour = 11,
    .tm_mday = 11, .tm_mon = 11, .tm_year = 91, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1991-Dec-11 11:11:11

static struct tm profile1_lastReplied2 = { .tm_sec = 12, .tm_min = 12, .tm_hour = 12,
    .tm_mday = 12, .tm_mon = 0, .tm_year = 92, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1992-Jan-12 12:12:12

static struct tm profile1_lastPositiveReply1 = { .tm_sec = 13, .tm_min = 13, .tm_hour = 13,
    .tm_mday = 13, .tm_mon = 1, .tm_year = 93, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1993-Feb-13 13:13:13

static struct tm profile1_lastPositiveReply2 = { .tm_sec = 14, .tm_min = 14, .tm_hour = 14,
    .tm_mday = 14, .tm_mon = 2, .tm_year = 94, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1994-Mar-14 14:14:14

static struct tm profile1_atsLastactivity1 = { .tm_sec = 15, .tm_min = 15, .tm_hour = 15,
    .tm_mday = 15, .tm_mon = 3, .tm_year = 95, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1995-Apr-15 15:15:15

static struct tm profile1_atsLastactivity2 = { .tm_sec = 16, .tm_min = 16, .tm_hour = 16,
    .tm_mday = 16, .tm_mon = 4, .tm_year = 96, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1996-May-16 16:16:16

static struct tm profile1_positionStartDate = { .tm_sec = 19, .tm_min = 19, .tm_hour = 19,
    .tm_mday = 19, .tm_mon = 7, .tm_year = 99, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1999-Aug-19 19:19:19

static struct tm profile1_positionEndDate = { .tm_sec = 20, .tm_min = 20, .tm_hour = 20,
    .tm_mday = 20, .tm_mon = 8, .tm_year = 100, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 2000-Sep-20 20:20:20

static struct tm profile1_educationStartDate = { .tm_sec = 22, .tm_min = 22, .tm_hour = 22,
    .tm_mday = 22, .tm_mon = 10, .tm_year = 102, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 2002-Nov-22 22:22:22

static struct tm profile1_educationEndDate = { .tm_sec = 23, .tm_min = 23, .tm_hour = 23,
    .tm_mday = 23, .tm_mon = 11, .tm_year = 103, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 2003-Dec-23 23:23:23

static struct tm profile1_talentPools1 = { .tm_sec = 24, .tm_min = 24, .tm_hour = 0,
    .tm_mday = 24, .tm_mon = 0, .tm_year = 104, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 2004-Jan-24 00:24:24

static struct tm profile1_talentPools2 = { .tm_sec = 25, .tm_min = 25, .tm_hour = 1,
    .tm_mday = 25, .tm_mon = 1, .tm_year = 105, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 2005-Feb-25 01:25:25

//
// ProfilePersistent2
//

static struct tm profile2_lastCachedAt = { .tm_sec = 7, .tm_min = 7, .tm_hour = 7,
    .tm_mday = 7, .tm_mon = 7, .tm_year = 97, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1997-Aug-07 07:07:07

//
// ProfilePersistent3
//

static struct tm profile3_lastCachedAt = { .tm_sec = 8, .tm_min = 8, .tm_hour = 8,
    .tm_mday = 8, .tm_mon = 8, .tm_year = 98, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1998-Sep-08 08:08:08

//
// ProfilePersistent4
//

static struct tm profile4_lastCachedAt = { .tm_sec = 9, .tm_min = 9, .tm_hour = 9,
    .tm_mday = 9, .tm_mon = 9, .tm_year = 99, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 1999-Oct-09 09:09:09

//
// ProfilePersistent5
//

static struct tm profile5_lastCachedAt = { .tm_sec = 10, .tm_min = 10, .tm_hour = 10,
    .tm_mday = 10, .tm_mon = 10, .tm_year = 100, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // 2000-Nov-10 10:10:10

//
// External interface
//

//
// ProfilePersistent1
//

void initTestData_ProfilePersistent1(struct ProfilePersistent* profile)
{
    profile->logicalDelete = false;
    profile->id            = PROFILE_PERSISTENT_1_ID;
    profile->firstName     = strdup("First");
    profile->lastName      = strdup("Last");
    profile->localityId    = 90210;
    profile->countryId     = 1;
    profile->stateId       = 2;

    kv_resize(struct Id_TimeValue, profile->lastMessaged, 2);
    kv_push(struct Id_TimeValue, profile->lastMessaged,
        ((struct Id_TimeValue) { 1, mktime(&profile1_lastMessaged1) }));
    kv_push(struct Id_TimeValue, profile->lastMessaged,
        ((struct Id_TimeValue) { 3, mktime(&profile1_lastMessaged2) }));

    kv_resize(struct Id_TimeValue, profile->lastReplied, 2);
    kv_push(struct Id_TimeValue, profile->lastReplied,
        ((struct Id_TimeValue) { 11, mktime(&profile1_lastReplied1) }));
    kv_push(struct Id_TimeValue, profile->lastReplied,
        ((struct Id_TimeValue) { 13, mktime(&profile1_lastReplied2) }));

    kv_resize(struct Id_TimeValue, profile->lastPositiveReply, 2);
    kv_push(struct Id_TimeValue, profile->lastPositiveReply,
        ((struct Id_TimeValue) { 21, mktime(&profile1_lastPositiveReply1) }));
    kv_push(struct Id_TimeValue, profile->lastPositiveReply,
        ((struct Id_TimeValue) { 23, mktime(&profile1_lastPositiveReply2) }));

    kv_resize(struct Id_Int32Value, profile->groups, 2);
    kv_push(struct Id_Int32Value, profile->groups, ((struct Id_Int32Value) { 31, 2 }));
    kv_push(struct Id_Int32Value, profile->groups, ((struct Id_Int32Value) { 33, 4 }));

    kv_resize(struct Id_Int32Value, profile->projects, 2);
    kv_push(struct Id_Int32Value, profile->projects, ((struct Id_Int32Value) { 41, 12 }));
    kv_push(struct Id_Int32Value, profile->projects, ((struct Id_Int32Value) { 43, 14 }));

    kv_resize(struct Id_Int32Value, profile->campaigns, 2);
    kv_push(struct Id_Int32Value, profile->campaigns, ((struct Id_Int32Value) { 51, 22 }));
    kv_push(struct Id_Int32Value, profile->campaigns, ((struct Id_Int32Value) { 53, 24 }));

    kv_resize(int32_t, profile->actioned, 2);
    kv_push(int32_t, profile->actioned, 81);
    kv_push(int32_t, profile->actioned, 83);

    kv_resize(struct PositionPersistent, profile->positions, 1);
    kv_push(struct PositionPersistent, profile->positions, ((struct PositionPersistent) {
        .title = strdup("TITLE_AB"), .companyName = strdup("COMPANY_IJ"),
        .startDate = mktime(&profile1_positionStartDate),
        .endDate = mktime(&profile1_positionEndDate),
        .companyId = 12, .titleId = 78,
        .uuid = strdup("1234"), .titleCorrected = strdup("TITLE_CD"),
        .source = strdup("SRC_EF"), .locality = strdup("LOCAL_GH"), .parentCompanyId = 34,
        .description = strdup("DESC_KL"), .seniority = strdup("SENIOR_MN"),
        .modifiedInLoad = true, .parentTitleId = 79 }));

    kv_resize(char*, profile->skills, 2);
    kv_push(char*, profile->skills, strdup("SKILL_ABC"));
    kv_push(char*, profile->skills, strdup("SKILL_DEF"));

    profile->headline = strdup("Engineer");
    profile->summary  = strdup("Nothing relevant.");

    kv_resize(struct Id_TimeValue, profile->talentPools, 2);
    kv_push(struct Id_TimeValue, profile->talentPools,
        ((struct Id_TimeValue) { 81, mktime(&profile1_talentPools1) }));
    kv_push(struct Id_TimeValue, profile->talentPools,
        ((struct Id_TimeValue) { 83, mktime(&profile1_talentPools2) }));

    profile->archivedPrimaryEmail  = strdup("f.last@email.com");
    profile->linkedinUser          = strdup("first.name");
    profile->profileCode           = strdup("firstname123");
    profile->profileKey            = strdup("123");
    profile->imageUrl              = strdup("http://www.someimage.com/image.jpeg");
    profile->industry              = strdup("IT");
    profile->linkedinUrl           = strdup("http://www.linkedin.com");
    profile->numConnections        = 12;
    profile->source                = strdup("SRC_456");
    profile->batch                 = 34;
    profile->covid19               = false;
    profile->linkedinCanonicalUrl  = strdup("http://www.linkedin.com/firstname123");
    profile->linkedinCanonicalUser = strdup("firstname1234");
    profile->linkedinId            = strdup("1234");
    profile->sourceKey             = strdup("SRC_KEY_789");
    profile->location              = strdup("Beverly Hills, CA, United States");
    profile->country               = strdup("United States");
    profile->gender                = strdup("male");
    profile->ethnicity             = strdup("white");
    profile->lastCachedAt          = mktime(&profile1_lastCachedAt);

    kv_resize(int32_t, profile->atsCompanies, 2);
    kv_push(int32_t, profile->atsCompanies, 51);
    kv_push(int32_t, profile->atsCompanies, 53);

    kv_resize(struct Id_StringValue, profile->atsStatus, 2);
    kv_push(struct Id_StringValue, profile->atsStatus,
        ((struct Id_StringValue) { 61, strdup("OK") }));
    kv_push(struct Id_StringValue, profile->atsStatus,
        ((struct Id_StringValue) { 63, strdup("NOT OK") }));

    kv_resize(struct Id_TimeValue, profile->atsLastactivity, 2);
    kv_push(struct Id_TimeValue, profile->atsLastactivity,
        ((struct Id_TimeValue) { 71, mktime(&profile1_atsLastactivity1) }));
    kv_push(struct Id_TimeValue, profile->atsLastactivity,
        ((struct Id_TimeValue) { 73, mktime(&profile1_atsLastactivity2) }));

    kv_resize(struct Education, profile->educations, 1);
    kv_push(struct Education, profile->educations, ((struct Education) {
        .uuid = strdup("2468"), .name = strdup("Some College"), .degree = strdup("BSCS"),
        .subject = strdup("Data Structures"), .universityId = 123,
        .startDate = mktime(&profile1_educationStartDate),
        .endDate = mktime(&profile1_educationEndDate) }));

    kv_resize(struct ProfileEmail, profile->profileEmails, 1);
    kv_push(struct ProfileEmail, profile->profileEmails, ((struct ProfileEmail) {
        .uuid = strdup("1357"), .email = strdup("john.smith@somemail.com"), .toxic = true,
        .domain = strdup("somemail.com"), .gender = strdup("male"), .manual = false,
        .status = strdup("OK"), .bounced = true, .primary = false, .lastName = strdup("Smith"),
        .firstName = strdup("John"), .mxFound = true, .personal = false, .mxRecord = strdup("mx"),
        .disposable = true, .freeEmail = false, .subStatus = strdup("working"),
        .smtpProvider = strdup("smtp.somemail.com"), .domainAgeDays = 10 }));

    kv_resize(struct ProfileSocialUrl, profile->profileSocialUrls, 1);
    kv_push(struct ProfileSocialUrl, profile->profileSocialUrls, ((struct ProfileSocialUrl) {
        .uuid = strdup("4680"), .source = strdup("SRC_ABCD"),
        .url = strdup("http://www.facebook.com"), .username = strdup("john.smith") }));

    kv_resize(struct ProfilePhoneNumber, profile->profilePhoneNumbers, 1);
    kv_push(struct ProfilePhoneNumber, profile->profilePhoneNumbers,
        ((struct ProfilePhoneNumber) { .uuid = strdup("3579"), .countryCode = strdup("1"),
         .countryName = strdup("United States"), .callingCode = strdup("00"),
         .internationalNumber = strdup("1"), .localNumber = strdup("987-6543") }));

    kv_resize(struct ProfileTag, profile->profileTags, 1);
    kv_push(struct ProfileTag, profile->profileTags, ((struct ProfileTag) {
        .uuid = strdup("5678"), .tagId = 34, .source = strdup("SRC_EFGH"),
        .tagType = strdup("TAG_IJ"), .text = strdup("SOME_TEXT_KL") }));
}

void initTestData_ProfileCached1(struct ProfileCached* profile)
{
    profile->id         = PROFILE_PERSISTENT_1_ID;
    profile->fullName   = strdup("first last");
    profile->localityId = 90210;
    profile->countryId  = 1;
    profile->stateId    = 2;
    profile->skillsHeadlineSummaryPosDescription = strdup("\x01" "engineer" "\x01"
        "nothing relevant" "\x01" "skill_abc" "\x01" "skill_def" "\x01" "desc_kl" "\x01");

    kv_resize(struct PositionCached, profile->positions, 1);
    kv_push(struct PositionCached, profile->positions, ((struct PositionCached) {
        .companyName = strdup("company_ij"), .title = strdup("title_cd"),
        .startDate = mktime(&profile1_positionStartDate),
        .endDate = mktime(&profile1_positionEndDate), .companyId = 12,
        .parentTitletId = 79 }));

    profile->totalExperienceMonths = 13;

    kv_resize(struct Id_TimeValue, profile->lastMessaged, 2);
    kv_push(struct Id_TimeValue, profile->lastMessaged,
        ((struct Id_TimeValue) { 1, mktime(&profile1_lastMessaged1) }));
    kv_push(struct Id_TimeValue, profile->lastMessaged,
        ((struct Id_TimeValue) { 3, mktime(&profile1_lastMessaged2) }));

    kv_resize(struct Id_TimeValue, profile->lastReplied, 2);
    kv_push(struct Id_TimeValue, profile->lastReplied,
        ((struct Id_TimeValue) { 11, mktime(&profile1_lastReplied1) }));
    kv_push(struct Id_TimeValue, profile->lastReplied,
        ((struct Id_TimeValue) { 13, mktime(&profile1_lastReplied2) }));

    kv_resize(struct Id_TimeValue, profile->lastPositiveReply, 2);
    kv_push(struct Id_TimeValue, profile->lastPositiveReply,
        ((struct Id_TimeValue) { 21, mktime(&profile1_lastPositiveReply1) }));
    kv_push(struct Id_TimeValue, profile->lastPositiveReply,
        ((struct Id_TimeValue) { 23, mktime(&profile1_lastPositiveReply2) }));

    kv_resize(struct Id_Int32Value, profile->groups, 2);
    kv_push(struct Id_Int32Value, profile->groups, ((struct Id_Int32Value) { 31, 2 }));
    kv_push(struct Id_Int32Value, profile->groups, ((struct Id_Int32Value) { 33, 4 }));

    kv_resize(struct Id_Int32Value, profile->projects, 2);
    kv_push(struct Id_Int32Value, profile->projects, ((struct Id_Int32Value) { 41, 12 }));
    kv_push(struct Id_Int32Value, profile->projects, ((struct Id_Int32Value) { 43, 14 }));

    kv_resize(struct Id_Int32Value, profile->campaigns, 2);
    kv_push(struct Id_Int32Value, profile->campaigns, ((struct Id_Int32Value) { 51, 22 }));
    kv_push(struct Id_Int32Value, profile->campaigns, ((struct Id_Int32Value) { 53, 24 }));

    kv_resize(int32_t, profile->actioned, 2);
    kv_push(int32_t, profile->actioned, 81);
    kv_push(int32_t, profile->actioned, 83);

    kv_resize(int32_t, profile->talentPools, 2);
    kv_push(int32_t, profile->talentPools, 81);
    kv_push(int32_t, profile->talentPools, 83);
};

//
// ProfilePersistent2
//

void initTestData_ProfilePersistent2(struct ProfilePersistent* profile)
{
    profile->logicalDelete = false;
    profile->id            = PROFILE_PERSISTENT_2_ID;
    profile->firstName     = strdup("Second");
    profile->lastName      = strdup("Last");
    profile->localityId    = 90210;
    profile->countryId     = 1;
    profile->stateId       = 2;
    profile->headline      = strdup("Engineer");
    profile->summary       = strdup("Nothing relevant.");

    profile->archivedPrimaryEmail  = strdup("s.last@email.com");
    profile->linkedinUser          = strdup("second.name");
    profile->profileCode           = strdup("secondname456");
    profile->profileKey            = strdup("456");
    profile->imageUrl              = strdup("http://www.someimage.com/image2.jpeg");
    profile->industry              = strdup("IT");
    profile->linkedinUrl           = strdup("http://www.linkedin.com");
    profile->numConnections        = 34;
    profile->source                = strdup("SRC_789");
    profile->batch                 = 56;
    profile->covid19               = false;
    profile->linkedinCanonicalUrl  = strdup("http://www.linkedin.com/secondname456");
    profile->linkedinCanonicalUser = strdup("secondname7890");
    profile->linkedinId            = strdup("7890");
    profile->sourceKey             = strdup("SRC_KEY_015");
    profile->location              = strdup("Beverly Hills, CA, United States");
    profile->country               = strdup("United States");
    profile->gender                = strdup("female");
    profile->ethnicity             = strdup("white");
    profile->lastCachedAt          = mktime(&profile2_lastCachedAt);
}

void initTestData_ProfileCached2(struct ProfileCached* profile)
{
    profile->id                    = PROFILE_PERSISTENT_2_ID;
    profile->fullName              = strdup("second last");
    profile->localityId            = 90210;
    profile->countryId             = 1;
    profile->stateId               = 2;
    profile->totalExperienceMonths = 0;
    profile->skillsHeadlineSummaryPosDescription = strdup("\x01" "engineer" "\x01" "nothing relevant" "\x01");
}

//
// ProfilePersistent3
//

void initTestData_ProfilePersistent3(struct ProfilePersistent* profile)
{
    profile->logicalDelete = false;
    profile->id            = PROFILE_PERSISTENT_3_ID;
    profile->firstName     = strdup("Third");
    profile->lastName      = strdup("Last");
    profile->localityId    = 90210;
    profile->countryId     = 1;
    profile->stateId       = 3;
    profile->headline      = strdup("Tester");
    profile->summary       = strdup("Nothing relevant.");

    profile->archivedPrimaryEmail  = strdup("t.last@email.com");
    profile->linkedinUser          = strdup("third.name");
    profile->profileCode           = strdup("thirdname789");
    profile->profileKey            = strdup("789");
    profile->imageUrl              = strdup("http://www.someimage.com/image3.jpeg");
    profile->industry              = strdup("IT");
    profile->linkedinUrl           = strdup("http://www.linkedin.com");
    profile->numConnections        = 56;
    profile->source                = strdup("SRC_012");
    profile->batch                 = 78;
    profile->covid19               = false;
    profile->linkedinCanonicalUrl  = strdup("http://www.linkedin.com/thirdname789");
    profile->linkedinCanonicalUser = strdup("thirdname1234");
    profile->linkedinId            = strdup("1234");
    profile->sourceKey             = strdup("SRC_KEY_020");
    profile->location              = strdup("Beverly Hills, CA, United States");
    profile->country               = strdup("United States");
    profile->gender                = strdup("male");
    profile->ethnicity             = strdup("black");
    profile->lastCachedAt          = mktime(&profile3_lastCachedAt);
}

void initTestData_ProfileCached3(struct ProfileCached* profile)
{
    profile->id                    = PROFILE_PERSISTENT_3_ID;
    profile->fullName              = strdup("third last");
    profile->localityId            = 90210;
    profile->countryId             = 1;
    profile->stateId               = 3;
    profile->totalExperienceMonths = 0;
    profile->skillsHeadlineSummaryPosDescription = strdup("\x01" "tester" "\x01" "nothing relevant" "\x01");
}

//
// ProfilePersistent4
//

void initTestData_ProfilePersistent4(struct ProfilePersistent* profile)
{
    profile->logicalDelete = false;
    profile->id            = PROFILE_PERSISTENT_4_ID;
    profile->firstName     = strdup("Fourth");
    profile->lastName      = strdup("Last");
    profile->localityId    = 90210;
    profile->countryId     = 1;
    profile->stateId       = 4;
    profile->headline      = strdup("Developer");
    profile->summary       = strdup("Nothing relevant.");

    profile->archivedPrimaryEmail  = strdup("f.last@email.com");
    profile->linkedinUser          = strdup("fourth.name");
    profile->profileCode           = strdup("fourthname012");
    profile->profileKey            = strdup("012");
    profile->imageUrl              = strdup("http://www.someimage.com/image4.jpeg");
    profile->industry              = strdup("IT");
    profile->linkedinUrl           = strdup("http://www.linkedin.com");
    profile->numConnections        = 78;
    profile->source                = strdup("SRC_345");
    profile->batch                 = 90;
    profile->covid19               = false;
    profile->linkedinCanonicalUrl  = strdup("http://www.linkedin.com/fourthname345");
    profile->linkedinCanonicalUser = strdup("fourthname5678");
    profile->linkedinId            = strdup("5678");
    profile->sourceKey             = strdup("SRC_KEY_025");
    profile->location              = strdup("Beverly Hills, CA, United States");
    profile->country               = strdup("United States");
    profile->gender                = strdup("male");
    profile->ethnicity             = strdup("black");
    profile->lastCachedAt          = mktime(&profile4_lastCachedAt);
}

void initTestData_ProfileCached4(struct ProfileCached* profile)
{
    profile->id                    = PROFILE_PERSISTENT_4_ID;
    profile->fullName              = strdup("fourth last");
    profile->localityId            = 90210;
    profile->countryId             = 1;
    profile->stateId               = 4;
    profile->totalExperienceMonths = 0;
    profile->skillsHeadlineSummaryPosDescription = strdup("\x01" "developer" "\x01" "nothing relevant" "\x01");
}

//
// ProfilePersistent5
//

void initTestData_ProfilePersistent5(struct ProfilePersistent* profile)
{
    profile->logicalDelete = false;
    profile->id            = PROFILE_PERSISTENT_5_ID;
    profile->firstName     = strdup("Fifth");
    profile->lastName      = strdup("Last");
    profile->localityId    = 90210;
    profile->countryId     = 1;
    profile->stateId       = 5;
    profile->headline      = strdup("Tester");
    profile->summary       = strdup("Nothing relevant.");

    profile->archivedPrimaryEmail  = strdup("f.last@email.com");
    profile->linkedinUser          = strdup("fifth.name");
    profile->profileCode           = strdup("fifthname345");
    profile->profileKey            = strdup("345");
    profile->imageUrl              = strdup("http://www.someimage.com/image5.jpeg");
    profile->industry              = strdup("IT");
    profile->linkedinUrl           = strdup("http://www.linkedin.com");
    profile->numConnections        = 90;
    profile->source                = strdup("SRC_678");
    profile->batch                 = 12;
    profile->covid19               = false;
    profile->linkedinCanonicalUrl  = strdup("http://www.linkedin.com/fifthname678");
    profile->linkedinCanonicalUser = strdup("fifthname9012");
    profile->linkedinId            = strdup("9012");
    profile->sourceKey             = strdup("SRC_KEY_030");
    profile->location              = strdup("Beverly Hills, CA, United States");
    profile->country               = strdup("United States");
    profile->gender                = strdup("male");
    profile->ethnicity             = strdup("white");
    profile->lastCachedAt          = mktime(&profile5_lastCachedAt);
}

void initTestData_ProfileCached5(struct ProfileCached* profile)
{
    profile->id                    = PROFILE_PERSISTENT_5_ID;
    profile->fullName              = strdup("fifth last");
    profile->localityId            = 90210;
    profile->countryId             = 1;
    profile->stateId               = 5;
    profile->totalExperienceMonths = 0;
    profile->skillsHeadlineSummaryPosDescription = strdup("\x01" "tester" "\x01" "nothing relevant" "\x01");
}
