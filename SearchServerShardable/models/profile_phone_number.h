#ifndef PROFILE_PHONE_NUMBER_H
#define PROFILE_PHONE_NUMBER_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>

struct ProfilePhoneNumber
{
    char* uuid;                // string `json:"uuid"`
    char* countryCode;         // string `json:"country_code" validate:"required"`
    char* countryName;         // string `json:"country_name"`
    char* callingCode;         // string `json:"calling_code"`
    char* internationalNumber; // string `json:"international_number"`
    char* localNumber;         // string `json:"local_number" validate:"required"`
};

void initProfilePhoneNumber(struct ProfilePhoneNumber* profilePhoneNumber);
void freeProfilePhoneNumber(struct ProfilePhoneNumber* profilePhoneNumber);

struct json_object* marshallProfilePhoneNumber(const struct ProfilePhoneNumber* profilePhoneNumber);
bool unmarshallProfilePhoneNumber(struct ProfilePhoneNumber* profilePhoneNumber,
    const struct json_object* obj);
uint8_t* profilePhoneNumberToBinary(uint8_t* byteStream,
    const struct ProfilePhoneNumber* profilePhoneNumber);
const uint8_t* binaryToProfilePhoneNumber(const uint8_t* byteStream,
    struct ProfilePhoneNumber* profilePhoneNumber);
uint32_t profilePhoneNumberBinarySize(const struct ProfilePhoneNumber* profilePhoneNumber);

#define UNMARSHALL_PROFILEPHONENUMBER(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallProfilePhoneNumber, OBJ, VAR, RET)

#define JSON_GET_PROFILEPHONENUMBER_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(struct ProfilePhoneNumber, UNMARSHALL_PROFILEPHONENUMBER, \
        initProfilePhoneNumber, freeProfilePhoneNumber, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_PROFILEPHONENUMBER_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct ProfilePhoneNumber, ARRAY, binaryToProfilePhoneNumber, \
        initProfilePhoneNumber, freeProfilePhoneNumber, BYTESTREAM, OFFSET)

#endif // PROFILE_PHONE_NUMBER_H
