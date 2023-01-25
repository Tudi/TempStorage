#include <profile_tag.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* uuid_Key      = "uuid";
static const char* tagId_Key     = "tag_id";
static const char* source_Key    = "source";
static const char* tagType_Key   = "tag_type";
static const char* text_Key      = "text";

void initProfileTag(struct ProfileTag* profileTag)
{
    profileTag->uuid      = NULL;
    profileTag->tagId     = 0;
    profileTag->source    = NULL;
    profileTag->tagType   = NULL;
    profileTag->text      = NULL;
}

void freeProfileTag(struct ProfileTag* profileTag)
{
    free(profileTag->uuid);
    profileTag->uuid = NULL;
    free(profileTag->source);
    profileTag->source = NULL;
    free(profileTag->tagType);
    profileTag->tagType = NULL;
    free(profileTag->text);
    profileTag->text = NULL;
}

struct json_object* marshallProfileTag(const struct ProfileTag* profileTag)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, uuid_Key, json_object_new_string(profileTag->uuid));
        json_object_object_add(obj, tagId_Key, json_object_new_int64(profileTag->tagId));
        json_object_object_add(obj, source_Key, json_object_new_string(profileTag->source));
        json_object_object_add(obj, tagType_Key, json_object_new_string(profileTag->tagType));
        json_object_object_add(obj, text_Key, json_object_new_string(profileTag->text));
    }

    return obj;
}

bool unmarshallProfileTag(struct ProfileTag* profileTag, const struct json_object* obj)
{
    freeProfileTag(profileTag);
    initProfileTag(profileTag);

    bool b1 = jsonGetString(obj, uuid_Key, &profileTag->uuid);
    bool b2 = jsonGetInt64(obj, tagId_Key, &profileTag->tagId);
    bool b3 = jsonGetString(obj, source_Key, &profileTag->source);
    bool b4 = jsonGetString(obj, tagType_Key, &profileTag->tagType);
    bool b5 = jsonGetString(obj, text_Key, &profileTag->text);

    if(!(b1 && b2 && b3 && b4 && b5))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* profileTagToBinary(uint8_t* byteStream, const struct ProfileTag* profileTag)
{
    uint8_t* offset = byteStream;

    offset = stringToBinary(offset, profileTag->uuid);
    if(offset == NULL) { return NULL; }

    offset = int64ToBinary(offset, profileTag->tagId);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileTag->source);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileTag->tagType);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileTag->text);
    if(offset == NULL) { return NULL; }

    return offset;
}

const uint8_t* binaryToProfileTag(const uint8_t* byteStream, struct ProfileTag* profileTag)
{
    freeProfileTag(profileTag);
    initProfileTag(profileTag);

    const uint8_t* offset = byteStream;

    offset = binaryToString(offset, &profileTag->uuid);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt64(offset, &profileTag->tagId);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileTag->source);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileTag->tagType);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileTag->text);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t profileTagBinarySize(const struct ProfileTag* profileTag)
{
    return stringBinarySize(profileTag->uuid) + INT64_BINARY_SIZE
        + stringBinarySize(profileTag->source) + stringBinarySize(profileTag->tagType)
        + stringBinarySize(profileTag->text);
}
