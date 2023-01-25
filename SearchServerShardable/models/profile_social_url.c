#include <profile_social_url.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* uuid_Key          = "uuid";
static const char* source_Key        = "source";
static const char* url_Key           = "url";
static const char* username_Key      = "username";

void initProfileSocialUrl(struct ProfileSocialUrl* profileSocialUrl)
{
    profileSocialUrl->uuid          = NULL;
    profileSocialUrl->source        = NULL;
    profileSocialUrl->url           = NULL;
    profileSocialUrl->username      = NULL;
}

void freeProfileSocialUrl(struct ProfileSocialUrl* profileSocialUrl)
{
    free(profileSocialUrl->uuid);
    profileSocialUrl->uuid = NULL;
    free(profileSocialUrl->source);
    profileSocialUrl->source = NULL;
    free(profileSocialUrl->url);
    profileSocialUrl->url = NULL;
    free(profileSocialUrl->username);
    profileSocialUrl->username = NULL;
}

struct json_object* marshallProfileSocialUrl(const struct ProfileSocialUrl* profileSocialUrl)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, uuid_Key, json_object_new_string(profileSocialUrl->uuid));
        json_object_object_add(obj, source_Key, json_object_new_string(profileSocialUrl->source));
        json_object_object_add(obj, url_Key, json_object_new_string(profileSocialUrl->url));
        json_object_object_add(obj, username_Key,
       	    json_object_new_string(profileSocialUrl->username));
    }

    return obj;
}

bool unmarshallProfileSocialUrl(struct ProfileSocialUrl* profileSocialUrl,
    const struct json_object* obj)
{
    freeProfileSocialUrl(profileSocialUrl);
    initProfileSocialUrl(profileSocialUrl);

    bool b1 = jsonGetString(obj, uuid_Key, &profileSocialUrl->uuid);
    bool b2 = jsonGetString(obj, source_Key, &profileSocialUrl->source);
    bool b3 = jsonGetString(obj, url_Key, &profileSocialUrl->url);
    bool b4 = jsonGetString(obj, username_Key, &profileSocialUrl->username);

    if(!(b1 && b2 && b3 && b4))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* profileSocialUrlToBinary(uint8_t* byteStream,
    const struct ProfileSocialUrl* profileSocialUrl)
{
    uint8_t* offset = byteStream;

    offset = stringToBinary(offset, profileSocialUrl->uuid);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileSocialUrl->source);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileSocialUrl->url);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileSocialUrl->username);
    if(offset == NULL) { return NULL; }

    return offset;
}

const uint8_t* binaryToProfileSocialUrl(const uint8_t* byteStream,
    struct ProfileSocialUrl* profileSocialUrl)
{
    freeProfileSocialUrl(profileSocialUrl);
    initProfileSocialUrl(profileSocialUrl);

    const uint8_t* offset = byteStream;

    offset = binaryToString(offset, &profileSocialUrl->uuid);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileSocialUrl->source);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileSocialUrl->url);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileSocialUrl->username);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t profileSocialUrlBinarySize(const struct ProfileSocialUrl* profileSocialUrl)
{
    return stringBinarySize(profileSocialUrl->uuid) + stringBinarySize(profileSocialUrl->source)
        + stringBinarySize(profileSocialUrl->url) + stringBinarySize(profileSocialUrl->username);
}
