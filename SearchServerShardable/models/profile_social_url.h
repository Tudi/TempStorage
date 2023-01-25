#ifndef PROFILESOCIALURL_H
#define PROFILESOCIALURL_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct ProfileSocialUrl
{
    char* uuid;              // string `json:"uuid"`
    char* source;            // string `json:"source" validate:"required"`
    char* url;               // string `json:"url" validate:"required"`
    char* username;          // string `json:"username"`
};

void initProfileSocialUrl(struct ProfileSocialUrl* profileSocialUrl);
void freeProfileSocialUrl(struct ProfileSocialUrl* profileSocialUrl);

struct json_object* marshallProfileSocialUrl(const struct ProfileSocialUrl* profileSocialUrl);
bool unmarshallProfileSocialUrl(struct ProfileSocialUrl* profileSocialUrl,
    const struct json_object* obj);
uint8_t* profileSocialUrlToBinary(uint8_t* byteStream,
    const struct ProfileSocialUrl* profileSocialUrl);
const uint8_t* binaryToProfileSocialUrl(const uint8_t* byteStream,
    struct ProfileSocialUrl* profileSocialUrl);
uint32_t profileSocialUrlBinarySize(const struct ProfileSocialUrl* profileSocialUrl);

#define UNMARSHALL_PROFILESOCIALURL(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallProfileSocialUrl, OBJ, VAR, RET)

#define JSON_GET_PROFILESOCIALURL_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(struct ProfileSocialUrl, UNMARSHALL_PROFILESOCIALURL, \
        initProfileSocialUrl, freeProfileSocialUrl, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_PROFILESOCIALURL_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct ProfileSocialUrl, ARRAY, binaryToProfileSocialUrl, \
        initProfileSocialUrl, freeProfileSocialUrl, BYTESTREAM, OFFSET)

#endif // PROFILESOCIALURL_H
