#ifndef PROFILE_CACHED_H
#define PROFILE_CACHED_H

#include <profile_persistent.h>
#include <position_cached.h>
#include <binary_array.h>
#include <id_value.h>
#include <kvec.h>

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// any number that is not part of a string
#define PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR 1 

struct ProfileCached
{
    // Commented out members will be reconsidered in the future.

    int32_t id;                                     // int `json:"i"`
    char* fullName;                                 // string `json:"fn"`
    int32_t localityId;                             // int `json:"lc"`
    int32_t countryId;
    int32_t stateId;
    // bool unavailable;                            // bool `json:"un"`
    // bool hidden;                                 // bool `json:"hd"`
    // char* headline;                              // string `json:"h"`
    // char* summary;                               // string `json:"sm"`
    kvec_t(struct PositionCached) positions;        // []Position `json:"p"`
    int16_t totalExperienceMonths;                  // float32 `json:"e"`
    // float avgTenure;                             // float32 `json:"t"`
    // bool women;                                  // bool `json:"wo"`
    // bool minority;                               // bool `json:"mi"`
    // bool veteran;                                // bool `json:"vt"`
    kvec_t(struct Id_TimeValue) lastMessaged;       // map[int]int64 `json:"lm,omitempty"`
    kvec_t(struct Id_TimeValue) lastReplied;        // map[int]int64 `json:"lr,omitempty"`
    kvec_t(struct Id_TimeValue) lastPositiveReply;  // map[int]int64 `json:"lpr,omitempty"`
    kvec_t(struct Id_Int32Value) groups;            // map[int]int `json:"gs,omitempty"`
    kvec_t(struct Id_Int32Value) projects;          // map[int]int `json:"ps,omitempty"`
    kvec_t(struct Id_Int32Value) campaigns;         // map[int]int `json:"ps,omitempty"`
    // kvec_t(int32_t) atsCompanies;                // []int `json:"ats,omitempty"`
    // kvec_t(struct Id_StringValue) atsStatus;     // map[int]string `json:"atss,omitempty"`
    // kvec_t(struct Id_TimeValue) atsLastactivity; // map[int]int64 `json:"atsla,omitempty"`
    kvec_t(int32_t) actioned;                       // []int `json:"act,omitempty"`
    // kvec_t(char*) skills;                        // []string `json:"tgs,omitempty"`
    char* skillsHeadlineSummaryPosDescription;
    kvec_t(int32_t) talentPools;                    // []int `json:"talent_pools"`
};

void initProfileCached(struct ProfileCached* profile);
void freeProfileCached(struct ProfileCached* profile);

bool profilePersistentToProfileCached(struct ProfileCached* profileCached,
    const struct ProfilePersistent* profilePersistent);
const uint8_t* binaryToProfileCached(const uint8_t* byteStream, struct ProfileCached* profile);

#endif // PROFILE_CACHED_H
