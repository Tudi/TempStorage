#include <profile_cached.h>
#include <strings_ext.h>
#include <scoring_definitions.h>
#include <logger.h>
#include <binary_specialized_array.h>
#include <k_utils.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//
// Macros
//

#define ADD_STRING_MERGE_REQ_MEM(dst, str) if(str[0]!=0) dst += strlen(str) + 1;  

//
// Local prototypes
//

static char* generateFullName(const char* firstName, const char* lastName);
static int addStringToMergedStrings(char* dst, size_t* writeIndex, const char* src);
static void* allocMemoryForMergedStrings(char** str, size_t memReq);
static bool reallocStringRemoveNonUsedSpace(char** str);
static char* mergeSkillsHeadlineSummaryDescription(const PositionPersistentKvec_t* positions,
    const StringKvec_t* skills, const char* headline, const char* summary);
static int16_t TotalWorkExperience(struct ProfileCached* profileCached);

//
// External interface
//

void initProfileCached(struct ProfileCached* profile)
{
    profile->id            = 0;
    profile->fullName      = NULL;
    profile->localityId    = 0;
    profile->countryId     = 0;
    kv_init(profile->positions);
    kv_init(profile->lastMessaged);
    kv_init(profile->lastReplied);
    kv_init(profile->lastPositiveReply);
    kv_init(profile->groups);
    kv_init(profile->projects);
    kv_init(profile->campaigns);
    kv_init(profile->actioned);
    profile->totalExperienceMonths = 0;
    profile->skillsHeadlineSummaryPosDescription = NULL;
    kv_init(profile->talentPools);
}

void freeProfileCached(struct ProfileCached* profile)
{
    free(profile->fullName);
    profile->fullName = NULL;

    FREE_KVEC_ADDR(profile->positions, freePositionCached);

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

    free(profile->skillsHeadlineSummaryPosDescription);
    profile->skillsHeadlineSummaryPosDescription = NULL;

    kv_destroy(profile->talentPools);
    kv_init(profile->talentPools);
}

bool profilePersistentToProfileCached(struct ProfileCached* profileCached,
    const struct ProfilePersistent* profilePersistent)
{
    freeProfileCached(profileCached);
    initProfileCached(profileCached);

    if(profilePersistent->firstName == NULL) { return false; }
    if(profilePersistent->lastName == NULL) { return false; }

    profileCached->fullName = generateFullName(profilePersistent->firstName, profilePersistent->lastName);
    if(profileCached->fullName == NULL) { return false; }

    profileCached->id = profilePersistent->id;

    profileCached->localityId = profilePersistent->localityId;
    profileCached->countryId = profilePersistent->countryId;
    profileCached->stateId = profilePersistent->stateId;

    kv_destroy(profileCached->positions);
    kv_init(profileCached->positions);
    kv_resize(struct PositionCached, profileCached->positions, kv_size(profilePersistent->positions));

    for(size_t i = 0; i < kv_max(profileCached->positions); ++i)
    {
        (void) kv_a(struct PositionCached, profileCached->positions, i);

        if(positionPersistentToPositionCached(&(kv_A(profileCached->positions, i)),
            &(kv_A(profilePersistent->positions, i))) == false)
        {
            for(size_t j = 0; j < i; ++j) {
                freePositionCached(&(kv_A(profileCached->positions, i)));
            }

            kv_destroy(profileCached->positions);
            kv_init(profileCached->positions);

            freeProfileCached(profileCached);
            return false;
        }
    }

    if (kv_size(profileCached->positions) > MaxNumPositionsProcessedBySearch)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Profile (id = %lu) has %lu positions, but scoring will only use %lu.",
            profileCached->id, kv_size(profileCached->positions), MaxNumPositionsProcessedBySearch);
    }

    if(kv_size(profilePersistent->lastMessaged) > 0) {
        kv_copy(struct Id_TimeValue, profileCached->lastMessaged, profilePersistent->lastMessaged);
    }

    if(kv_size(profilePersistent->lastReplied) > 0) {
        kv_copy(struct Id_TimeValue, profileCached->lastReplied, profilePersistent->lastReplied);
    }

    if(kv_size(profilePersistent->lastPositiveReply) > 0) {
        kv_copy(struct Id_TimeValue, profileCached->lastPositiveReply, profilePersistent->lastPositiveReply);
    }

    if(kv_size(profilePersistent->groups) > 0) {
        kv_copy(struct Id_Int32Value, profileCached->groups, profilePersistent->groups);
    }

    if(kv_size(profilePersistent->projects) > 0) {
        kv_copy(struct Id_Int32Value, profileCached->projects, profilePersistent->projects);
    }

    if(kv_size(profilePersistent->campaigns) > 0) {
        kv_copy(struct Id_Int32Value, profileCached->campaigns, profilePersistent->campaigns);
    }

    if(kv_size(profilePersistent->actioned) > 0) {
        kv_copy(int32_t, profileCached->actioned, profilePersistent->actioned);
    }

    profileCached->skillsHeadlineSummaryPosDescription = mergeSkillsHeadlineSummaryDescription(
        &profilePersistent->positions, &profilePersistent->skills, profilePersistent->headline,
        profilePersistent->summary);
    if (profileCached->skillsHeadlineSummaryPosDescription == NULL)
    {
        freeProfileCached(profileCached);
        return false;
    }

    kv_resize(int32_t, profileCached->talentPools, kv_size(profilePersistent->talentPools));
    for (size_t i = 0; i < kv_size(profilePersistent->talentPools); i++)
    {
        const struct Id_TimeValue* entry = &kv_A(profilePersistent->talentPools, i);
        kv_push(int32_t, profileCached->talentPools, entry->id);
    }

    // standardize strings. Searched terms are standardized that requires DB strings to be standardized
    // also to make sure they match
    profileCached->fullName = StrStandardizeScoringClient(profileCached->fullName);
    profileCached->skillsHeadlineSummaryPosDescription
        = StrStandardizeScoringClient(profileCached->skillsHeadlineSummaryPosDescription);
    for (size_t i = 0; i < kv_size(profileCached->positions); i++)
    {
        struct PositionCached* pos = &kv_A(profileCached->positions, i);
        pos->companyName = StrStandardizeScoringClient(pos->companyName);
        pos->title = StrStandardizeScoringClient(pos->title);
    }

    profileCached->totalExperienceMonths = TotalWorkExperience(profileCached);

    return true;
}

const uint8_t* binaryToProfileCached(const uint8_t* byteStream, struct ProfileCached* profile)
{
    char* firstName = NULL;
    char* lastName  = NULL;
    char* headline  = NULL;
    char* summary   = NULL;

    PositionPersistentKvec_t positionsPersistent;
    StringKvec_t skills;

    kv_init(positionsPersistent);
    kv_init(skills);

    freeProfileCached(profile);
    initProfileCached(profile);

    const uint8_t* offset = byteStream;

    offset = SKIP_BINARY_BOOLEAN(offset); // logicalDelete

    offset = binaryToInt32(offset, &profile->id);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    // Generate fullName

    offset = binaryToString(offset, &firstName);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    offset = binaryToString(offset, &lastName);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    profile->fullName = generateFullName(firstName, lastName);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    // Other fields

    offset = binaryToInt32(offset, &profile->localityId);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    offset = binaryToInt32(offset, &profile->countryId);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    offset = binaryToInt32(offset, &profile->stateId);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->lastMessaged, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->lastReplied, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_ID_TIMEVALUE_ARRAY(profile->lastPositiveReply, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_ID_INT32VALUE_ARRAY(profile->groups, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_ID_INT32VALUE_ARRAY(profile->projects, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_ID_INT32VALUE_ARRAY(profile->campaigns, offset, offset);
    if (offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_INT32_ARRAY(profile->actioned, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_POSITIONPERSISTENT_ARRAY(positionsPersistent, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    BINARY_TO_STRING_ARRAY(skills, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    offset = binaryToString(offset, &headline);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    offset = binaryToString(offset, &summary);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    kvec_t(struct Id_TimeValue) talentPools;
    kv_init(talentPools);
    BINARY_TO_ID_TIMEVALUE_ARRAY(talentPools, offset, offset);
    if(offset == NULL) { goto binaryToProfileCached_cleanup; }

    // only need the IDs from talentPools. The date is for persistent profile
    kv_resize(int32_t, profile->talentPools, kv_size(talentPools));
    for (size_t i = 0; i < kv_size(talentPools); i++)
    {
        const struct Id_TimeValue* entry = &kv_A(talentPools, i);
        kv_push(int32_t, profile->talentPools, entry->id);
    }
    kv_destroy(talentPools);

    // PositionPersistent to PositionCached

    kv_resize(struct PositionCached, profile->positions, kv_size(positionsPersistent));

    for(size_t i = 0; i < kv_max(profile->positions); ++i)
    {
        (void) kv_a(struct PositionCached, profile->positions, i);

        if(positionPersistentToPositionCached(&(kv_A(profile->positions, i)),
            &(kv_A(positionsPersistent, i))) == false)
        {
            kv_resize(struct PositionCached, profile->positions, i);
            offset = NULL;
            goto binaryToProfileCached_cleanup;
        }
    }

    profile->skillsHeadlineSummaryPosDescription
        = mergeSkillsHeadlineSummaryDescription(&positionsPersistent, &skills, headline, summary);
    if(profile->skillsHeadlineSummaryPosDescription == NULL)
    {
        offset = NULL;
        goto binaryToProfileCached_cleanup;
    }

    // Standardize strings since searched terms are standardized.
    profile->fullName = StrStandardizeScoringClient(profile->fullName);
    profile->skillsHeadlineSummaryPosDescription
        = StrStandardizeScoringClient(profile->skillsHeadlineSummaryPosDescription);
    for (size_t i = 0; i < kv_size(profile->positions); i++)
    {
        struct PositionCached* pos = &kv_A(profile->positions, i);
        pos->companyName = StrStandardizeScoringClient(pos->companyName);
        pos->title = StrStandardizeScoringClient(pos->title);
    }

    profile->totalExperienceMonths = TotalWorkExperience(profile);

binaryToProfileCached_cleanup:
    free(firstName);
    free(lastName);
    free(headline);
    free(summary);

    FREE_KVEC_ADDR(positionsPersistent, freePositionPersistent);
    FREE_KVEC_VALUE(skills, free);

    if(offset == NULL) { freeProfileCached(profile); }

    return offset;
}

//
// Local functions
//

static char* generateFullName(const char* firstName, const char* lastName)
{
    char* fullName = malloc(strlen(firstName) + strlen(lastName) + 2);
    if(fullName == NULL) { return NULL; }

    sprintf(fullName, "%s %s", firstName, lastName);
    StrToLower(fullName);

    return fullName;
}

/// <summary>
/// Append a string to the array of strings
/// </summary>
/// <param name="dst"></param>
/// <param name="writeIndex"></param>
/// <param name="src"></param>
static int addStringToMergedStrings(char* dst, size_t* writeIndex, const char* src)
{
    size_t srcLen = strlen(src);
    if (srcLen == 0)
    {
        return 0;
    }

    // Xuan said it's worth the loading speed sacrifice
    // use this comparison in case at some point keyword filter tests full word matches
    // if (strstrMerged(dst, src, srcLen))
    if (strstr(dst, src) != NULL) // right now there is no feature that tests for full match. There is no reason to store substrings
    {
        return 1;
    }

    // append the string to the array of strings
    memcpy(&dst[*writeIndex], src, srcLen);
    dst[*writeIndex + srcLen] = PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR; // some separator to avoid word merge
    dst[*writeIndex + srcLen + 1] = 0; // Sanity. Always add null terminator even if we rewrite it next
    *writeIndex = *writeIndex + srcLen + 1;

    return 0;
}

/// <summary>
/// Unified error message for allocations. Extracted function to make other functions smaller
/// </summary>
/// <param name="str"></param>
/// <param name="memReq"></param>
/// <returns></returns>
static void* allocMemoryForMergedStrings(char** str, size_t memReq)
{
    *str = malloc(memReq);
    if (*str == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Failed to allocate %d bytes of memory", memReq);
        return str;
    }
    (*str)[0] = PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR;
    (*str)[1] = 0; // Sanity. Not required
    return str;
}

/// <summary>
/// Functions used only on strings that ended up smaller than initial estimated size. 
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
static bool reallocStringRemoveNonUsedSpace(char** str)
{
    size_t newLen = strlen(*str) + 1;
    char* newAlloc = realloc(*str, newLen);
    if (newAlloc == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Failed to allocate %d bytes of memory", newLen);
        return false;
    }
    *str = newAlloc;
    return true;
}

/// <summary>
/// Merge Skills Headline Summary Description fields into a single field
/// Repeating strings will be removed from the merged string. Result string might end up smaller than input
/// Strings are enclosed by a separator byte to be able to search for full words in case it will be required in the future
/// </summary>
/// <param name="positions"></param>
/// <param name="">skills</param>
/// <param name="">headline</param>
/// <param name="">summary</param>
/// <returns></returns>
static char* mergeSkillsHeadlineSummaryDescription(const PositionPersistentKvec_t* positions,
    const StringKvec_t* skills, const char* headline, const char* summary)
{
    char* result = NULL;

    // Count how many bytes we need to allocate
    size_t totalMemReqForMergedStrings = 0;
    for (size_t i = 0; i < kv_max(*positions); ++i)
    {
        struct PositionPersistent* posPers = &(kv_A(*positions, i));
        ADD_STRING_MERGE_REQ_MEM(totalMemReqForMergedStrings, posPers->description);
    }

    for (size_t i = 0; i < kv_size(*skills); i++) {
        ADD_STRING_MERGE_REQ_MEM(totalMemReqForMergedStrings, kv_A(*skills, i));
    }

    ADD_STRING_MERGE_REQ_MEM(totalMemReqForMergedStrings, headline);
    ADD_STRING_MERGE_REQ_MEM(totalMemReqForMergedStrings, summary);

    if (totalMemReqForMergedStrings != 0)
    {
        // Place a separator at the beginning + end so we may search for values and separator
        // in case a full match is required
        totalMemReqForMergedStrings += 2;
        if (allocMemoryForMergedStrings(&result, totalMemReqForMergedStrings) == NULL) {
            return NULL;
        }

        size_t stringsSkipped = 0;
        size_t bytesWritten = 1;

        // add headline 
        stringsSkipped += addStringToMergedStrings(result, &bytesWritten, headline);

        // add summary 
        stringsSkipped += addStringToMergedStrings(result, &bytesWritten, summary);

        // add skill strings
        for (size_t i = 0; i < kv_size(*skills); i++) {
            stringsSkipped += addStringToMergedStrings(result, &bytesWritten, kv_A(*skills, i));
        }

        // add position description
        for (size_t i = 0; i < kv_size(*positions); i++) {
            stringsSkipped += addStringToMergedStrings(result, &bytesWritten, kv_A(*positions, i).description);
        }

        StrToLower(result); // every string is lowercase

        // Xuan said it's worth the processing time
        if (stringsSkipped > 0) {
            reallocStringRemoveNonUsedSpace(&result);
        }
    }

    // very low chance to happen
    if (result == NULL) {
        result = strdup("");
    }

    return result;
}

static int16_t TotalWorkExperience(struct ProfileCached* profileCached)
{
    if (kv_size(profileCached->positions) == 0)
    {
        return 0;
    }

    #define MAX_RELEVANT_POSITIONS 500
    struct PositionCached *relevantPositions[MAX_RELEVANT_POSITIONS];
    size_t maxUsablePosition = 0;
    size_t relevantPositionCount = 0;

    if (kv_size(profileCached->positions) <= MAX_RELEVANT_POSITIONS)
    {
        maxUsablePosition = kv_size(profileCached->positions);
    }
    else
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Profile (id = %lu) has more positions (%lu) than "
            "relevantPositions capacity (%lu).", kv_size(profileCached->positions),
            MAX_RELEVANT_POSITIONS);
        maxUsablePosition = MAX_RELEVANT_POSITIONS;
    }

    time_t timeStamp = time(NULL);
    int32_t totalWorkExperience = 0;
    for (size_t indexPosition = 0; indexPosition < maxUsablePosition; indexPosition++)
    {
        struct PositionCached* pos = &kv_A(profileCached->positions, indexPosition);
        // incomplete data
        if (pos->startDate == getDate1_time_t())
        {
            continue;
        }
        // if this position is overlapped by previous position, no need to take it into account
        if (relevantPositionCount > 0 && pos->startDate >= relevantPositions[relevantPositionCount - 1]->startDate)
        {
            continue;
        }
        // calculate total experience for this position
        relevantPositions[relevantPositionCount] = pos;
        relevantPositionCount++;
    }

    if (relevantPositionCount > 0)
    {
        totalWorkExperience += GetPositionCachedDuration(relevantPositions[0], timeStamp);
    }
    else
    {
        return 0;
    }
  
    for (size_t indexPosition = 0; indexPosition < relevantPositionCount - 1; indexPosition++)
    {
        time_t curStartDate = relevantPositions[indexPosition + 1]->startDate;
        time_t curEndDate = relevantPositions[indexPosition + 1]->endDate;
        time_t prevStartDate = relevantPositions[indexPosition + 0]->startDate;

        if (curEndDate == getDate1_time_t())
        {
            curEndDate = timeStamp;
        }

        if (curEndDate > prevStartDate)
        {
            totalWorkExperience += ((prevStartDate - curStartDate) / NUMBER_SECONDS_IN_DAY);
        }
        else
        {
            totalWorkExperience
                += GetPositionCachedDuration(relevantPositions[indexPosition + 1], timeStamp);
        }
    }

    return (int16_t)(CONVERT_DAYS_XP_TO_MONTHS(int32_t, totalWorkExperience));
}
