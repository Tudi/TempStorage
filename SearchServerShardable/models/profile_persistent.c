#include <profile_persistent.h>
#include <profile_definitions.h>
#include <daos_versioned.h>
#include <binary_utils.h>
#include <binary_specialized_array.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <json_specialized_array.h>
#include <stdlib.h>

static const char* logicalDelete_Key         = "logical_delete";
static const char* id_Key                    = "id";
static const char* firstName_Key             = "first_name";
static const char* lastName_Key              = "last_name";
static const char* localityId_Key            = "locality_id";
static const char* countryId_Key             = "country_id";
static const char* stateId_Key               = "state_id";
static const char* lastMessaged_Key          = "lm";
static const char* lastReplied_Key           = "lr";
static const char* lastPositiveReply_Key     = "lpr";
static const char* groups_Key                = "gs";
static const char* projects_Key              = "ps";
static const char* campaigns_Key             = "cs";
static const char* actioned_Key              = "act";
static const char* positions_Key             = "positions";
static const char* skills_Key                = "skills";
static const char* headline_Key              = "headline";
static const char* summary_Key               = "summary";
static const char* talentPools_Key           = "talent_pools";
static const char* archivedPrimaryEmail_Key  = "archived_primary_email";
static const char* linkedinUser_Key          = "linkedin_user";
static const char* profileCode_Key           = "profile_code";
static const char* profileKey_Key            = "profile_key";
static const char* imageUrl_Key              = "image_url";
static const char* industry_Key              = "industry";
static const char* linkedinUrl_Key           = "linkedin_url";
static const char* numConnections_Key        = "num_connections";
static const char* source_Key                = "source";
static const char* batch_Key                 = "batch";
static const char* covid19_Key               = "covid19";
static const char* linkedinCanonicalUrl_Key  = "linkedin_canonical_url";
static const char* linkedinCanonicalUser_Key = "linkedin_canonical_user";
static const char* linkedinId_Key            = "linkedin_id";
static const char* sourceKey_Key             = "source_key";
static const char* location_Key              = "location";
static const char* country_Key               = "country";
static const char* gender_Key                = "gender";
static const char* ethnicity_Key             = "ethnicity";
static const char* lastCachedAt_Key          = "last_cached_at";
static const char* atsCompanies_Key          = "ats";
static const char* atsStatus_Key             = "atss";
static const char* atsLastactivity_Key       = "atsla";
static const char* educations_Key            = "educations";
static const char* profileEmails_Key         = "profile_emails";
static const char* profileSocialUrls_Key     = "profile_social_urls";
static const char* profilePhoneNumbers_Key   = "profile_phone_numbers";
static const char* profileTags_Key           = "profile_tags";

void initProfilePersistent(struct ProfilePersistent* profile)
{
    profile->logicalDelete = false;
    profile->id = 0;
    profile->firstName = NULL;
    profile->lastName = NULL;
    profile->localityId = 0;
    profile->countryId = 0;
    profile->stateId = 0;
    kv_init(profile->lastMessaged);
    kv_init(profile->lastReplied);
    kv_init(profile->lastPositiveReply);
    kv_init(profile->groups);
    kv_init(profile->projects);
    kv_init(profile->campaigns);
    kv_init(profile->actioned);
    kv_init(profile->positions);
    kv_init(profile->skills);
    profile->headline = NULL;
    profile->summary = NULL;
    kv_init(profile->talentPools);

    profile->archivedPrimaryEmail = NULL;
    profile->linkedinUser = NULL;
    profile->profileCode = NULL;
    profile->profileKey = NULL;
    profile->imageUrl = NULL;
    profile->industry = NULL;
    profile->linkedinUrl = NULL;
    profile->numConnections = 0;
    profile->source = NULL;
    profile->batch = 0;
    profile->covid19 = false;
    profile->linkedinCanonicalUrl = NULL;
    profile->linkedinCanonicalUser = NULL;
    profile->linkedinId = NULL;
    profile->sourceKey = NULL;
    profile->location = NULL;
    profile->country = NULL;
    profile->gender = NULL;
    profile->ethnicity = NULL;
    profile->lastCachedAt = getDate1_time_t();

    kv_init(profile->atsCompanies);
    kv_init(profile->atsStatus);
    kv_init(profile->atsLastactivity);
    kv_init(profile->educations);
    kv_init(profile->profileEmails);
    kv_init(profile->profileSocialUrls);
    kv_init(profile->profilePhoneNumbers);
    kv_init(profile->profileTags);
}

void freeProfilePersistent(struct ProfilePersistent* profile)
{
    free(profile->firstName);
    profile->firstName = NULL;
    free(profile->lastName);
    profile->lastName = NULL;

    kv_destroy(profile->lastMessaged);
    kv_init(profile->lastMessaged);

    kv_destroy(profile->lastReplied);
    kv_init(profile->lastReplied);

    kv_destroy(profile->lastPositiveReply);
    kv_init(profile->lastPositiveReply);

    kv_destroy(profile->groups);
    kv_init(profile->groups);

    kv_destroy(profile->projects);
    kv_init(profile->projects);

    kv_destroy(profile->campaigns);
    kv_init(profile->campaigns);

    kv_destroy(profile->actioned);
    kv_init(profile->actioned);

    for (size_t i = 0; i < kv_size(profile->positions); ++i) {
        freePositionPersistent(&kv_A(profile->positions, i));
    }
    kv_destroy(profile->positions);
    kv_init(profile->positions);

    for (size_t i = 0; i < kv_size(profile->skills); ++i) {
        free(kv_A(profile->skills, i));
    }
    kv_destroy(profile->skills);
    kv_init(profile->skills);

    free(profile->headline);
    profile->headline = NULL;
    free(profile->summary);
    profile->summary = NULL;

    kv_destroy(profile->talentPools);
    kv_init(profile->talentPools);

    free(profile->archivedPrimaryEmail);
    profile->archivedPrimaryEmail = NULL;
    free(profile->linkedinUser);
    profile->linkedinUser = NULL;
    free(profile->profileCode);
    profile->profileCode = NULL;
    free(profile->profileKey);
    profile->profileKey = NULL;
    free(profile->imageUrl);
    profile->imageUrl = NULL;
    free(profile->industry);
    profile->industry = NULL;
    free(profile->linkedinUrl);
    profile->linkedinUrl = NULL;
    free(profile->source);
    profile->source = NULL;
    free(profile->linkedinCanonicalUrl);
    profile->linkedinCanonicalUrl = NULL;
    free(profile->linkedinCanonicalUser);
    profile->linkedinCanonicalUser = NULL;
    free(profile->linkedinId);
    profile->linkedinId = NULL;
    free(profile->sourceKey);
    profile->sourceKey = NULL;
    free(profile->location);
    profile->location = NULL;
    free(profile->country);
    profile->country = NULL;
    free(profile->gender);
    profile->gender = NULL;
    free(profile->ethnicity);
    profile->ethnicity = NULL;

    kv_destroy(profile->atsCompanies);
    kv_init(profile->atsCompanies);

    for(size_t i = 0; i < kv_size(profile->atsStatus); ++i) {
        free((kv_A(profile->atsStatus, i)).value);
    }
    kv_destroy(profile->atsStatus);
    kv_init(profile->atsStatus);

    kv_destroy(profile->atsLastactivity);
    kv_init(profile->atsLastactivity);

    for(size_t i = 0; i < kv_size(profile->educations); ++i) {
        freeEducation(&kv_A(profile->educations, i));
    }
    kv_destroy(profile->educations);
    kv_init(profile->educations);

    for(size_t i = 0; i < kv_size(profile->profileEmails); ++i) {
        freeProfileEmail(&kv_A(profile->profileEmails, i));
    }
    kv_destroy(profile->profileEmails);
    kv_init(profile->profileEmails);

    for(size_t i = 0; i < kv_size(profile->profileSocialUrls); ++i) {
        freeProfileSocialUrl(&kv_A(profile->profileSocialUrls, i));
    }
    kv_destroy(profile->profileSocialUrls);
    kv_init(profile->profileSocialUrls);

    for(size_t i = 0; i < kv_size(profile->profilePhoneNumbers); ++i) {
        freeProfilePhoneNumber(&kv_A(profile->profilePhoneNumbers, i));
    }
    kv_destroy(profile->profilePhoneNumbers);
    kv_init(profile->profilePhoneNumbers);

    for(size_t i = 0; i < kv_size(profile->profileTags); ++i) {
        freeProfileTag(&kv_A(profile->profileTags, i));
    }
    kv_destroy(profile->profileTags);
    kv_init(profile->profileTags);
}

struct json_object* marshallProfilePersistent(const struct ProfilePersistent* profile)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, logicalDelete_Key, json_object_new_boolean(profile->logicalDelete));
        json_object_object_add(obj, id_Key, json_object_new_int(profile->id));
        json_object_object_add(obj, firstName_Key, json_object_new_string(profile->firstName));
        json_object_object_add(obj, lastName_Key, json_object_new_string(profile->lastName));
        json_object_object_add(obj, localityId_Key, json_object_new_int(profile->localityId));
        json_object_object_add(obj, countryId_Key, json_object_new_int(profile->countryId));
        json_object_object_add(obj, stateId_Key, json_object_new_int(profile->stateId));
        JSON_ADD_ARRAY_ADDR(marshallId_TimeValue, obj, lastMessaged_Key, profile->lastMessaged);
        JSON_ADD_ARRAY_ADDR(marshallId_TimeValue, obj, lastReplied_Key, profile->lastReplied);
        JSON_ADD_ARRAY_ADDR(marshallId_TimeValue, obj, lastPositiveReply_Key, profile->lastPositiveReply);
        JSON_ADD_ARRAY_ADDR(marshallId_Int32Value, obj, groups_Key, profile->groups);
        JSON_ADD_ARRAY_ADDR(marshallId_Int32Value, obj, projects_Key, profile->projects);
        JSON_ADD_ARRAY_ADDR(marshallId_Int32Value, obj, campaigns_Key, profile->campaigns);
        JSON_ADD_INT32_ARRAY(obj, actioned_Key, profile->actioned);
        JSON_ADD_ARRAY_ADDR(marshallPositionPersistent, obj, positions_Key, profile->positions);
        JSON_ADD_STRING_ARRAY(obj, skills_Key, profile->skills);
        json_object_object_add(obj, headline_Key, json_object_new_string(profile->headline));
        json_object_object_add(obj, summary_Key, json_object_new_string(profile->summary));
        JSON_ADD_ARRAY_ADDR(marshallId_TimeValue, obj, talentPools_Key, profile->talentPools);

        json_object_object_add(obj, archivedPrimaryEmail_Key, json_object_new_string(profile->archivedPrimaryEmail));
        json_object_object_add(obj, linkedinUser_Key, json_object_new_string(profile->linkedinUser));
        json_object_object_add(obj, profileCode_Key, json_object_new_string(profile->profileCode));
        json_object_object_add(obj, profileKey_Key, json_object_new_string(profile->profileKey));
        json_object_object_add(obj, imageUrl_Key, json_object_new_string(profile->imageUrl));
        json_object_object_add(obj, industry_Key, json_object_new_string(profile->industry));
        json_object_object_add(obj, linkedinUrl_Key, json_object_new_string(profile->linkedinUrl));
        json_object_object_add(obj, numConnections_Key, json_object_new_int(profile->numConnections));
        json_object_object_add(obj, source_Key, json_object_new_string(profile->source));
        json_object_object_add(obj, batch_Key, json_object_new_int(profile->batch));
        json_object_object_add(obj, covid19_Key, json_object_new_boolean(profile->covid19));
        json_object_object_add(obj, linkedinCanonicalUrl_Key, json_object_new_string(profile->linkedinCanonicalUrl));
        json_object_object_add(obj, linkedinCanonicalUser_Key, json_object_new_string(profile->linkedinCanonicalUser));
        json_object_object_add(obj, linkedinId_Key, json_object_new_string(profile->linkedinId));
        json_object_object_add(obj, sourceKey_Key, json_object_new_string(profile->sourceKey));
        json_object_object_add(obj, location_Key, json_object_new_string(profile->location));
        json_object_object_add(obj, country_Key, json_object_new_string(profile->country));
        json_object_object_add(obj, gender_Key, json_object_new_string(profile->gender));
        json_object_object_add(obj, ethnicity_Key, json_object_new_string(profile->ethnicity));
        json_object_object_add(obj, lastCachedAt_Key, marshallTime(profile->lastCachedAt));

        JSON_ADD_INT32_ARRAY(obj, atsCompanies_Key, profile->atsCompanies);
        JSON_ADD_ARRAY_ADDR(marshallId_StringValue, obj, atsStatus_Key, profile->atsStatus);
        JSON_ADD_ARRAY_ADDR(marshallId_TimeValue, obj, atsLastactivity_Key, profile->atsLastactivity);
        JSON_ADD_ARRAY_ADDR(marshallEducation, obj, educations_Key, profile->educations);
        JSON_ADD_ARRAY_ADDR(marshallProfileEmail, obj, profileEmails_Key, profile->profileEmails);
        JSON_ADD_ARRAY_ADDR(marshallProfileSocialUrl, obj, profileSocialUrls_Key, profile->profileSocialUrls);
        JSON_ADD_ARRAY_ADDR(marshallProfilePhoneNumber, obj, profilePhoneNumbers_Key, profile->profilePhoneNumbers);
        JSON_ADD_ARRAY_ADDR(marshallProfileTag, obj, profileTags_Key, profile->profileTags);
    }

    return obj;
}

bool unmarshallProfilePersistent(struct ProfilePersistent* profile, const struct json_object* obj)
{
    freeProfilePersistent(profile);
    initProfilePersistent(profile);

    bool b1 = jsonGetBoolean(obj, logicalDelete_Key, &profile->logicalDelete);
    bool b2 = jsonGetInt32(obj, id_Key, &profile->id);
    bool b3 = jsonGetString(obj, firstName_Key, &profile->firstName);
    bool b4 = jsonGetString(obj, lastName_Key, &profile->lastName);
    bool b5 = jsonGetInt32(obj, localityId_Key, &profile->localityId);
    bool b6 = jsonGetInt32(obj, countryId_Key, &profile->countryId);
    bool b7 = jsonGetInt32(obj, stateId_Key, &profile->stateId);

    bool b8 = false;
    JSON_GET_ID_TIMEVALUE_ARRAY(obj, lastMessaged_Key, profile->lastMessaged, b8);

    bool b9 = false;
    JSON_GET_ID_TIMEVALUE_ARRAY(obj, lastReplied_Key, profile->lastReplied, b9);

    bool b10 = false;
    JSON_GET_ID_TIMEVALUE_ARRAY(obj, lastPositiveReply_Key, profile->lastPositiveReply, b10);

    bool b11 = false;
    JSON_GET_ID_INT32VALUE_ARRAY(obj, groups_Key, profile->groups, b11);

    bool b12 = false;
    JSON_GET_ID_INT32VALUE_ARRAY(obj, projects_Key, profile->projects, b12);

    bool b13 = false;
    JSON_GET_ID_INT32VALUE_ARRAY(obj, campaigns_Key, profile->campaigns, b13);

    bool b14 = false;
    JSON_GET_INT32_ARRAY(obj, actioned_Key, profile->actioned, b14);

    bool b15 = false;
    JSON_GET_POSITIONPERSISTENT_ARRAY(obj, positions_Key, profile->positions, b15);

    bool b16 = false;
    JSON_GET_STRING_ARRAY(obj, skills_Key, profile->skills, b16);

    bool b17 = jsonGetString(obj, headline_Key, &profile->headline);
    bool b18 = jsonGetString(obj, summary_Key, &profile->summary);

    bool b19 = false;
    JSON_GET_ID_TIMEVALUE_ARRAY(obj, talentPools_Key, profile->talentPools, b19);

    bool b20 = jsonGetString(obj, archivedPrimaryEmail_Key, &profile->archivedPrimaryEmail);
    bool b21 = jsonGetString(obj, linkedinUser_Key, &profile->linkedinUser);
    bool b22 = jsonGetString(obj, profileCode_Key, &profile->profileCode);
    bool b23 = jsonGetString(obj, profileKey_Key, &profile->profileKey);
    bool b24 = jsonGetString(obj, imageUrl_Key, &profile->imageUrl);
    bool b25 = jsonGetString(obj, industry_Key, &profile->industry);
    bool b26 = jsonGetString(obj, linkedinUrl_Key, &profile->linkedinUrl);
    bool b27 = jsonGetInt32(obj, numConnections_Key, &profile->numConnections);
    bool b28 = jsonGetString(obj, source_Key, &profile->source);
    bool b29 = jsonGetInt32(obj, batch_Key, &profile->batch);
    bool b30 = jsonGetBoolean(obj, covid19_Key, &profile->covid19);
    bool b31 = jsonGetString(obj, linkedinCanonicalUrl_Key, &profile->linkedinCanonicalUrl);
    bool b32 = jsonGetString(obj, linkedinCanonicalUser_Key, &profile->linkedinCanonicalUser);
    bool b33 = jsonGetString(obj, linkedinId_Key, &profile->linkedinId);
    bool b34 = jsonGetString(obj, sourceKey_Key, &profile->sourceKey);
    bool b35 = jsonGetString(obj, location_Key, &profile->location);
    bool b36 = jsonGetString(obj, country_Key, &profile->country);
    bool b37 = jsonGetString(obj, gender_Key, &profile->gender);
    bool b38 = jsonGetString(obj, ethnicity_Key, &profile->ethnicity);
    bool b39 = jsonGetTime(obj, lastCachedAt_Key, &profile->lastCachedAt);

    bool b40 = false;
    JSON_GET_INT32_ARRAY(obj, atsCompanies_Key, profile->atsCompanies, b40);

    bool b41 = false;
    JSON_GET_ID_STRINGVALUE_ARRAY(obj, atsStatus_Key, profile->atsStatus, b41);

    bool b42 = false;
    JSON_GET_ID_TIMEVALUE_ARRAY(obj, atsLastactivity_Key, profile->atsLastactivity, b42);

    bool b43 = false;
    JSON_GET_EDUCATION_ARRAY(obj, educations_Key, profile->educations, b43);

    bool b44 = false;
    JSON_GET_PROFILEEMAIL_ARRAY(obj, profileEmails_Key, profile->profileEmails, b44);

    bool b45 = false;
    JSON_GET_PROFILESOCIALURL_ARRAY(obj, profileSocialUrls_Key, profile->profileSocialUrls, b45);

    bool b46 = false;
    JSON_GET_PROFILEPHONENUMBER_ARRAY(obj, profilePhoneNumbers_Key, profile->profilePhoneNumbers, b46);

    bool b47 = false;
    JSON_GET_PROFILETAG_ARRAY(obj, profileTags_Key, profile->profileTags, b47);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10
        && b11 && b12 && b13 && b14 && b15 && b16 && b17 && b18 && b19 && b20
        && b21 && b22 && b23 && b24 && b25 && b26 && b27 && b28 && b29 && b30
        && b31 && b32 && b33 && b34 && b35 && b36 && b37 && b38 && b39 && b40
        && b41 && b42 && b43 && b44 && b45 && b46 && b47))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* profilePersistentToBinary(uint8_t* byteStream, const struct ProfilePersistent* profile)
{
    uint8_t* offset = byteStream;

    offset = booleanToBinary(offset, profile->logicalDelete);
    if (offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, profile->id);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->firstName);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->lastName);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, profile->localityId);
    if (offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, profile->countryId);
    if (offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, profile->stateId);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->lastMessaged, id_TimeValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->lastReplied, id_TimeValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->lastPositiveReply, id_TimeValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->groups, id_Int32ValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->projects, id_Int32ValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->campaigns, id_Int32ValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    INT32_ARRAY_TO_BINARY(profile->actioned, offset, offset);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->positions, positionPersistentToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    STRING_ARRAY_TO_BINARY(profile->skills, offset, offset);
    if (offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->headline);
    if (offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->summary);
    if (offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->talentPools, id_TimeValueToBinary, offset, offset);
    if (offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->archivedPrimaryEmail);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->linkedinUser);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->profileCode);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->profileKey);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->imageUrl);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->industry);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->linkedinUrl);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, profile->numConnections);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->source);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, profile->batch);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profile->covid19);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->linkedinCanonicalUrl);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->linkedinCanonicalUser);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->linkedinId);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->sourceKey);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->location);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->country);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->gender);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profile->ethnicity);
    if(offset == NULL) { return NULL; }

    offset = timeToBinary(offset, profile->lastCachedAt);
    if(offset == NULL) { return NULL; }

    INT32_ARRAY_TO_BINARY(profile->atsCompanies, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->atsStatus, id_StringValueToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->atsLastactivity, id_TimeValueToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->educations, educationToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->profileEmails, profileEmailToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->profileSocialUrls, profileSocialUrlToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->profilePhoneNumbers, profilePhoneNumberToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    ADDR_ARRAY_TO_BINARY(profile->profileTags, profileTagToBinary, offset, offset);
    if(offset == NULL) { return NULL; }

    return offset;
}

static const uint8_t* binaryToProfilePersistent_active(const uint8_t* byteStream,
    struct ProfilePersistent* profile)
{
    const uint8_t* offset = byteStream;

    offset = binaryToBoolean(offset, &profile->logicalDelete);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->id);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->firstName);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->lastName);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->localityId);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->countryId);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->stateId);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->lastMessaged, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->lastReplied, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->lastPositiveReply, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_INT32VALUE_ARRAY(profile->groups, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_INT32VALUE_ARRAY(profile->projects, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_INT32VALUE_ARRAY(profile->campaigns, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_INT32_ARRAY(profile->actioned, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_POSITIONPERSISTENT_ARRAY(profile->positions, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_STRING_ARRAY(profile->skills, offset, offset);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->headline);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->summary);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->talentPools, offset, offset);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->archivedPrimaryEmail);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinUser);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->profileCode);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->profileKey);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->imageUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->industry);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->numConnections);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->source);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->batch);
    if (offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profile->covid19);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinCanonicalUrl);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinCanonicalUser);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinId);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->sourceKey);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->location);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->country);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->gender);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->ethnicity);
    if (offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &profile->lastCachedAt);
    if (offset == NULL) { return NULL; }

    BINARY_TO_INT32_ARRAY(profile->atsCompanies, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_STRINGVALUE_ARRAY(profile->atsStatus, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->atsLastactivity, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_EDUCATION_ARRAY(profile->educations, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_PROFILEEMAIL_ARRAY(profile->profileEmails, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_PROFILESOCIALURL_ARRAY(profile->profileSocialUrls, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_PROFILEPHONENUMBER_ARRAY(profile->profilePhoneNumbers, offset, offset);
    if (offset == NULL) { return NULL; }

    BINARY_TO_PROFILETAG_ARRAY(profile->profileTags, offset, offset);
    if (offset == NULL) { return NULL; }

    return offset;
}

const uint8_t* binaryToProfilePersistent(const uint8_t* byteStream,
    struct ProfilePersistent* profile, int fileVersion)
{
    freeProfilePersistent(profile);
    initProfilePersistent(profile);

    switch (fileVersion)
    {
        case 7: return binaryToProfilePersistent_V7(byteStream, profile); break;
        case PROFILE_DAOS_VERSION: return binaryToProfilePersistent_active(byteStream, profile); break;
        default: return NULL; // any value that signals that we do not handle this version
    }
    return NULL;
}

uint32_t profilePersistentBinarySize(const struct ProfilePersistent* profile)
{
    uint32_t binarySize = 2 * BOOLEAN_BINARY_SIZE + 6 * INT32_BINARY_SIZE
        + stringBinarySize(profile->firstName) + stringBinarySize(profile->lastName)
        + stringBinarySize(profile->headline) + stringBinarySize(profile->summary)
        + stringBinarySize(profile->archivedPrimaryEmail) + stringBinarySize(profile->linkedinUser)
        + stringBinarySize(profile->profileCode) + stringBinarySize(profile->profileKey)
        + stringBinarySize(profile->imageUrl) + stringBinarySize(profile->industry)
        + stringBinarySize(profile->linkedinUrl) + stringBinarySize(profile->source)
        + stringBinarySize(profile->linkedinCanonicalUrl) + stringBinarySize(profile->linkedinCanonicalUser)
        + stringBinarySize(profile->linkedinId) + stringBinarySize(profile->sourceKey)
        + stringBinarySize(profile->location) + stringBinarySize(profile->country)
        + stringBinarySize(profile->gender) + stringBinarySize(profile->ethnicity)
        + TIME_BINARY_SIZE;

    uint32_t arrayBinarySize = 0;

    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->lastMessaged, ID_TIMEVALUE_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->lastReplied, ID_TIMEVALUE_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->lastPositiveReply, ID_TIMEVALUE_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->groups, ID_INT32VALUE_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->projects, ID_INT32VALUE_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->campaigns, ID_INT32VALUE_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->actioned, INT32_BINARY_SIZE);
    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->talentPools, ID_TIMEVALUE_BINARY_SIZE);

    VARIABLE_ARRAY_BINARY_SIZE(profile->positions, positionPersistentBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    STRING_ARRAY_BINARY_SIZE(profile->skills, arrayBinarySize);
    binarySize += arrayBinarySize;

    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->atsCompanies, INT32_BINARY_SIZE);

    VARIABLE_ARRAY_BINARY_SIZE(profile->atsStatus, id_StringValueBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    binarySize += FIXED_ARRAY_BINARY_SIZE(profile->atsLastactivity, ID_TIMEVALUE_BINARY_SIZE);

    VARIABLE_ARRAY_BINARY_SIZE(profile->educations, educationBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    VARIABLE_ARRAY_BINARY_SIZE(profile->profileEmails, profileEmailBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    VARIABLE_ARRAY_BINARY_SIZE(profile->profileSocialUrls, profileSocialUrlBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    VARIABLE_ARRAY_BINARY_SIZE(profile->profilePhoneNumbers, profilePhoneNumberBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    VARIABLE_ARRAY_BINARY_SIZE(profile->profileTags, profileTagBinarySize, arrayBinarySize);
    binarySize += arrayBinarySize;

    return binarySize;
}
