#ifndef PROFILEEMAIL_H
#define PROFILEEMAIL_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct ProfileEmail
{
    char* uuid;            // string `json:"uuid"`
    char* email;           // string `json:"email" validate:"required"`
    bool toxic;            // bool `json:"toxic"`
    char* domain;          // string `json:"domain"`
    char* gender;          // string `json:"gender"`
    bool manual;           // bool `json:"manual"`
    char* status;          // string `json:"status"`
    bool bounced;          // bool `json:"bounced"`
    bool primary;          // bool `json:"primary"`
    char* lastName;        // string `json:"lastname"`
    char* firstName;       // string `json:"firstname"`
    bool mxFound;          // bool `json:"mx_found"`
    bool personal;         // bool `json:"personal"`
    char* mxRecord;        // string `json:"mx_record"`
    bool disposable;       // bool `json:"disposable"`
    bool freeEmail;        // bool `json:"free_email"`
    char* subStatus;       // string `json:"sub_status"`
    char* smtpProvider;    // string `json:"smtp_provider"`
    int32_t domainAgeDays; // int `json:"domain_age_days"`
};

void initProfileEmail(struct ProfileEmail* profileEmail);
void freeProfileEmail(struct ProfileEmail* profileEmail);

struct json_object* marshallProfileEmail(const struct ProfileEmail* profileEmail);
bool unmarshallProfileEmail(struct ProfileEmail* profileEmail, const struct json_object* obj);
uint8_t* profileEmailToBinary(uint8_t* byteStream, const struct ProfileEmail* profileEmail);
const uint8_t* binaryToProfileEmail(const uint8_t* byteStream, struct ProfileEmail* profileEmail);
uint32_t profileEmailBinarySize(const struct ProfileEmail* profileEmail);

#define UNMARSHALL_PROFILEEMAIL(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallProfileEmail, OBJ, VAR, RET)

#define JSON_GET_PROFILEEMAIL_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct ProfileEmail, \
    UNMARSHALL_PROFILEEMAIL, initProfileEmail, freeProfileEmail, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_PROFILEEMAIL_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct ProfileEmail, ARRAY, binaryToProfileEmail, \
        initProfileEmail, freeProfileEmail, BYTESTREAM, OFFSET)

#endif // PROFILEEMAIL_H
