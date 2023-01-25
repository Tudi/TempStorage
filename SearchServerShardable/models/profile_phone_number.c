#include <profile_phone_number.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* uuid_Key                = "uuid";
static const char* countryCode_Key         = "country_code";
static const char* countryName_Key         = "country_name";
static const char* callingCode_Key         = "calling_code";
static const char* internationalNumber_Key = "international_number";
static const char* localNumber_Key         = "local_number";

void initProfilePhoneNumber(struct ProfilePhoneNumber* profilePhoneNumber)
{
    profilePhoneNumber->uuid                = NULL;
    profilePhoneNumber->countryCode         = NULL;
    profilePhoneNumber->countryName         = NULL;
    profilePhoneNumber->callingCode         = NULL;
    profilePhoneNumber->internationalNumber = NULL;
    profilePhoneNumber->localNumber         = NULL;
}

void freeProfilePhoneNumber(struct ProfilePhoneNumber* profilePhoneNumber)
{
    free(profilePhoneNumber->uuid);
    profilePhoneNumber->uuid = NULL;
    free(profilePhoneNumber->countryCode);
    profilePhoneNumber->countryCode = NULL;
    free(profilePhoneNumber->countryName);
    profilePhoneNumber->countryName = NULL;
    free(profilePhoneNumber->callingCode);
    profilePhoneNumber->callingCode = NULL;
    free(profilePhoneNumber->internationalNumber);
    profilePhoneNumber->internationalNumber = NULL;
    free(profilePhoneNumber->localNumber);
    profilePhoneNumber->localNumber = NULL;
}

struct json_object* marshallProfilePhoneNumber(const struct ProfilePhoneNumber* profilePhoneNumber)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, uuid_Key, json_object_new_string(profilePhoneNumber->uuid));
        json_object_object_add(obj, countryCode_Key,
            json_object_new_string(profilePhoneNumber->countryCode));
        json_object_object_add(obj, countryName_Key,
            json_object_new_string(profilePhoneNumber->countryName));
        json_object_object_add(obj, callingCode_Key,
            json_object_new_string(profilePhoneNumber->callingCode));
        json_object_object_add(obj, internationalNumber_Key,
            json_object_new_string(profilePhoneNumber->internationalNumber));
        json_object_object_add(obj, localNumber_Key,
            json_object_new_string(profilePhoneNumber->localNumber));
    }

    return obj;
}

bool unmarshallProfilePhoneNumber(struct ProfilePhoneNumber* profilePhoneNumber, const struct json_object* obj)
{
    freeProfilePhoneNumber(profilePhoneNumber);
    initProfilePhoneNumber(profilePhoneNumber);

    bool b1 = jsonGetString(obj, uuid_Key, &profilePhoneNumber->uuid);
    bool b2 = jsonGetString(obj, countryCode_Key, &profilePhoneNumber->countryCode);
    bool b3 = jsonGetString(obj, countryName_Key, &profilePhoneNumber->countryName);
    bool b4 = jsonGetString(obj, callingCode_Key, &profilePhoneNumber->callingCode);
    bool b5 = jsonGetString(obj, internationalNumber_Key, &profilePhoneNumber->internationalNumber);
    bool b6 = jsonGetString(obj, localNumber_Key, &profilePhoneNumber->localNumber);

    if(!(b1 && b2 && b3 && b4 && b5 && b6))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* profilePhoneNumberToBinary(uint8_t* byteStream,
    const struct ProfilePhoneNumber* profilePhoneNumber)
{
    uint8_t* offset = byteStream;

    offset = stringToBinary(offset, profilePhoneNumber->uuid);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profilePhoneNumber->countryCode);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profilePhoneNumber->countryName);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profilePhoneNumber->callingCode);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profilePhoneNumber->internationalNumber);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profilePhoneNumber->localNumber);
    if(offset == NULL) { return NULL; }

    return offset;
}

const uint8_t* binaryToProfilePhoneNumber(const uint8_t* byteStream,
    struct ProfilePhoneNumber* profilePhoneNumber)
{
    freeProfilePhoneNumber(profilePhoneNumber);
    initProfilePhoneNumber(profilePhoneNumber);

    const uint8_t* offset = byteStream;

    offset = binaryToString(offset, &profilePhoneNumber->uuid);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profilePhoneNumber->countryCode);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profilePhoneNumber->countryName);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profilePhoneNumber->callingCode);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profilePhoneNumber->internationalNumber);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profilePhoneNumber->localNumber);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t profilePhoneNumberBinarySize(const struct ProfilePhoneNumber* profilePhoneNumber)
{
    return stringBinarySize(profilePhoneNumber->uuid)
        + stringBinarySize(profilePhoneNumber->countryCode)
        + stringBinarySize(profilePhoneNumber->countryName)
        + stringBinarySize(profilePhoneNumber->callingCode)
        + stringBinarySize(profilePhoneNumber->internationalNumber)
        + stringBinarySize(profilePhoneNumber->localNumber);
}
