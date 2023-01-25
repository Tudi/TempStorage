#ifndef PROFILE_PERSISTENT_H
#define PROFILE_PERSISTENT_H

#include <education.h>
#include <position_persistent.h>
#include <profile_email.h>
#include <profile_phone_number.h>
#include <profile_social_url.h>
#include <profile_tag.h>
#include <id_value.h>
#include <types.h>

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <kvec.h>

typedef kvec_t(struct PositionPersistent) PositionPersistentKvec_t;

struct ProfilePersistent
{
    bool logicalDelete;                 // bool `json:"logical_delete"`
    int32_t id;                         // int `json:"id" validate:"required"`
    char* firstName;                    // string `json:"first_name"`
    char* lastName;                     // string `json:"last_name"`
    int32_t localityId;                 // int `json:"locality_id"`
    int32_t countryId;                  // int `json:"country_id" 
    int32_t stateId;                    // int `json:"state_id" 
    kvec_t(struct Id_TimeValue) lastMessaged;      // map[int]int64 `json:"lm"`
    kvec_t(struct Id_TimeValue) lastReplied;       // map[int]int64 `json:"lr"`
    kvec_t(struct Id_TimeValue) lastPositiveReply; // map[int]int64 `json:"lpr,omitempty"`
    kvec_t(struct Id_Int32Value) groups;           // map[int]int `json:"gs,omitempty"`
    kvec_t(struct Id_Int32Value) projects;         // map[int]int `json:"ps,omitempty"`
    kvec_t(struct Id_Int32Value) campaigns;        // map[int]int `json:"cs,omitempty"`
    kvec_t(int32_t) actioned;                      // []int `json:"act"`
    PositionPersistentKvec_t positions;      // []PositionPersistent `json:"positions"`
    StringKvec_t skills;                     // []string `json:"keywords"`
    char* headline;                          // string `json:"headline"`
    char* summary;                           // string `json:"summary"`
    kvec_t(struct Id_TimeValue) talentPools; // map[int]time.Time `json:"talent_pools"`

    // below fields are only used by profile persistent and not by profile cached
    char* archivedPrimaryEmail;         // string `json:"archived_primary_email"`
    char* linkedinUser;                 // string `json:"linkedin_user" validate:"required"`
    char* profileCode;                  // string `json:"profile_code"`
    char* profileKey;                   // string `json:"profile_key"`
    char* imageUrl;                     // string `json:"image_url"`
    char* industry;                     // string `json:"industry"`
    char* linkedinUrl;                  // string `json:"linkedin_url"`
    int32_t numConnections;             // int `json:"num_connections"`
    char* source;                       // string `json:"source"`
    int32_t batch;                      // int `json:"batch"`
    bool covid19;                       // bool `json:"covid19"`
    char* linkedinCanonicalUrl;         // string `json:"linkedin_canonical_url"`
    char* linkedinCanonicalUser;        // string `json:"linkedin_canonical_user"`
    char* linkedinId;                   // string `json:"linkedin_id"`
    char* sourceKey;                    // string `json:"source_key"`
    char* location;                     // string `json:"location"`
    char* country;                      // string `json:"country"`
    char* gender;                       // string `json:"gender"`
    char* ethnicity;                    // string `json:"ethnicity"`
    time_t lastCachedAt;                // time.Time `json:"last_cached_at"`
    kvec_t(int32_t) atsCompanies;                  // []int `json:"ats"`
    kvec_t(struct Id_StringValue) atsStatus;       // map[int]string `json:"atss"`
    kvec_t(struct Id_TimeValue) atsLastactivity;   // map[int]int64 `json:"atsla"`
    kvec_t(struct Education) educations;           // []Education `json:"educations"`
    kvec_t(struct ProfileEmail) profileEmails;     // []ProfileEmail `json:"profile_emails"`
    kvec_t(struct ProfileSocialUrl) profileSocialUrls;     // []ProfileSocialUrl `json:"profile_social_urls"`
    kvec_t(struct ProfilePhoneNumber) profilePhoneNumbers; // []ProfilePhoneNumber `json:"profile_phone_numbers"`
    kvec_t(struct ProfileTag) profileTags;         // []ProfileTag `json:"profile_tags"`
};

typedef kvec_t(struct ProfilePersistent) ProfilePersistentKvec_t;

void initProfilePersistent(struct ProfilePersistent* profile);
void freeProfilePersistent(struct ProfilePersistent* profile);

struct json_object* marshallProfilePersistent(const struct ProfilePersistent* profile);
bool unmarshallProfilePersistent(struct ProfilePersistent* profile, const struct json_object* obj);
uint8_t* profilePersistentToBinary(uint8_t* byteStream, const struct ProfilePersistent* profile);
const uint8_t* binaryToProfilePersistent(const uint8_t* byteStream,
    struct ProfilePersistent* profile, int fileVersion);
uint32_t profilePersistentBinarySize(const struct ProfilePersistent* profile);

#define UNMARSHALL_PROFILEPERSISTENT(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallProfilePersistent, OBJ, VAR, RET)

#define JSON_GET_PROFILEPERSISTENT_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(struct ProfilePersistent, UNMARSHALL_PROFILEPERSISTENT, \
        initProfilePersistent, freeProfilePersistent, OBJ, KEY, ARRAY, RET)

#endif // PROFILE_PERSISTENT_H
