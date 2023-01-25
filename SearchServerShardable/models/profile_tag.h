#ifndef PROFILETAG_H
#define PROFILETAG_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct ProfileTag
{
    char* uuid;          // string `json:"uuid"`
    int64_t tagId;       // int64 `json:"tag_id" validate:"required"`
    char* source;        // string `json:"source" validate:"required"`
    char* tagType;       // string `json:"tag_type" validate:"required"`
    char* text;          // string `json:"text" validate:"required"`
};

void initProfileTag(struct ProfileTag* profileTag);
void freeProfileTag(struct ProfileTag* profileTag);

struct json_object* marshallProfileTag(const struct ProfileTag* profileTag);
bool unmarshallProfileTag(struct ProfileTag* profileTag, const struct json_object* obj);
uint8_t* profileTagToBinary(uint8_t* byteStream, const struct ProfileTag* profileTag);
const uint8_t* binaryToProfileTag(const uint8_t* byteStream, struct ProfileTag* profileTag);
uint32_t profileTagBinarySize(const struct ProfileTag* profileTag);

#define UNMARSHALL_PROFILETAG(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallProfileTag, OBJ, VAR, RET)

#define JSON_GET_PROFILETAG_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct ProfileTag, \
    UNMARSHALL_PROFILETAG, initProfileTag, freeProfileTag, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_PROFILETAG_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct ProfileTag, ARRAY, binaryToProfileTag, \
        initProfileTag, freeProfileTag, BYTESTREAM, OFFSET)

#endif // PROFILETAG_H
