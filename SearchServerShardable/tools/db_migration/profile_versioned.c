#include <profile_persistent.h>
#include <daos_definitions.h>
#include <binary_utils.h>
#include <binary_specialized_array.h>
#include <utils.h>
#include <date_time.h>
#include <stdlib.h>

const uint8_t* binaryToProfilePersistent_V7(const uint8_t* byteStream,
    struct ProfilePersistent* profile)
{
    const uint8_t* offset = byteStream;

    offset = binaryToBoolean(offset, &profile->logicalDelete);
    if (offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->firstName);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->lastName);
    if(offset == NULL) { return NULL; }

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
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinUser);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->profileCode);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->profileKey);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->imageUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->industry);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->numConnections);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->source);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profile->batch);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profile->covid19);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinCanonicalUrl);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinCanonicalUser);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->linkedinId);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->sourceKey);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->location);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->country);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->gender);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profile->ethnicity);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &profile->lastCachedAt);
    if(offset == NULL) { return NULL; }

    BINARY_TO_INT32_ARRAY(profile->atsCompanies, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_ID_STRINGVALUE_ARRAY(profile->atsStatus, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->atsLastactivity, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_EDUCATION_ARRAY(profile->educations, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_PROFILEEMAIL_ARRAY(profile->profileEmails, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_PROFILESOCIALURL_ARRAY(profile->profileSocialUrls, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_PROFILEPHONENUMBER_ARRAY(profile->profilePhoneNumbers, offset, offset);
    if(offset == NULL) { return NULL; }

    BINARY_TO_PROFILETAG_ARRAY(profile->profileTags, offset, offset);
    if(offset == NULL) { return NULL; }

    return offset;
}
