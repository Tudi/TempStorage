#include <profile_email.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* uuid_Key          = "uuid";
static const char* email_Key         = "email";
static const char* toxic_Key         = "toxic";
static const char* domain_Key        = "domain";
static const char* gender_Key        = "gender";
static const char* manual_Key        = "manual";
static const char* status_Key        = "status";
static const char* bounced_Key       = "bounced";
static const char* primary_Key       = "primary";
static const char* lastName_Key      = "lastname";
static const char* firstName_Key     = "firstname";
static const char* mxFound_Key       = "mx_found";
static const char* personal_Key      = "personal";
static const char* mxRecord_Key      = "mx_record";
static const char* disposable_Key    = "disposable";
static const char* freeEmail_Key     = "free_email";
static const char* subStatus_Key     = "sub_status";
static const char* smtpProvider_Key  = "smtp_provider";
static const char* domainAgeDays_Key = "domain_age_days";

void initProfileEmail(struct ProfileEmail* profileEmail)
{
    profileEmail->uuid          = NULL;
    profileEmail->email         = NULL;
    profileEmail->toxic         = false;
    profileEmail->domain        = NULL;
    profileEmail->gender        = NULL;
    profileEmail->manual        = false;
    profileEmail->status        = NULL;
    profileEmail->bounced       = false;
    profileEmail->primary       = false; 
    profileEmail->lastName      = NULL;
    profileEmail->firstName     = NULL;
    profileEmail->mxFound       = false;
    profileEmail->personal      = false;
    profileEmail->mxRecord      = NULL;
    profileEmail->disposable    = false;
    profileEmail->freeEmail     = false;
    profileEmail->subStatus     = NULL;
    profileEmail->smtpProvider  = NULL;
    profileEmail->domainAgeDays = 0;
}

void freeProfileEmail(struct ProfileEmail* profileEmail)
{
    free(profileEmail->uuid);
    profileEmail->uuid = NULL;
    free(profileEmail->email);
    profileEmail->email = NULL;
    free(profileEmail->domain);
    profileEmail->domain = NULL;
    free(profileEmail->gender);
    profileEmail->gender = NULL;
    free(profileEmail->status);
    profileEmail->status = NULL;
    free(profileEmail->lastName);
    profileEmail->lastName = NULL;
    free(profileEmail->firstName);
    profileEmail->firstName = NULL;
    free(profileEmail->mxRecord);
    profileEmail->mxRecord = NULL;
    free(profileEmail->subStatus);
    profileEmail->subStatus = NULL;
    free(profileEmail->smtpProvider);
    profileEmail->smtpProvider = NULL;
}

struct json_object* marshallProfileEmail(const struct ProfileEmail* profileEmail)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, uuid_Key, json_object_new_string(profileEmail->uuid));
        json_object_object_add(obj, email_Key, json_object_new_string(profileEmail->email));
        json_object_object_add(obj, toxic_Key, json_object_new_boolean(profileEmail->toxic));
        json_object_object_add(obj, domain_Key, json_object_new_string(profileEmail->domain));
        json_object_object_add(obj, gender_Key, json_object_new_string(profileEmail->gender));
        json_object_object_add(obj, manual_Key, json_object_new_boolean(profileEmail->manual));
        json_object_object_add(obj, status_Key, json_object_new_string(profileEmail->status));
        json_object_object_add(obj, bounced_Key, json_object_new_boolean(profileEmail->bounced));
        json_object_object_add(obj, primary_Key, json_object_new_boolean(profileEmail->primary));
        json_object_object_add(obj, lastName_Key, json_object_new_string(profileEmail->lastName));
        json_object_object_add(obj, firstName_Key, json_object_new_string(profileEmail->firstName));
        json_object_object_add(obj, mxFound_Key, json_object_new_boolean(profileEmail->mxFound));
        json_object_object_add(obj, personal_Key, json_object_new_boolean(profileEmail->personal));
        json_object_object_add(obj, mxRecord_Key, json_object_new_string(profileEmail->mxRecord));
        json_object_object_add(obj, disposable_Key,
            json_object_new_boolean(profileEmail->disposable));
        json_object_object_add(obj, freeEmail_Key,
            json_object_new_boolean(profileEmail->freeEmail));
        json_object_object_add(obj, subStatus_Key, json_object_new_string(profileEmail->subStatus));
        json_object_object_add(obj, smtpProvider_Key,
            json_object_new_string(profileEmail->smtpProvider));
        json_object_object_add(obj, domainAgeDays_Key,
            json_object_new_int(profileEmail->domainAgeDays));
    }

    return obj;
}

bool unmarshallProfileEmail(struct ProfileEmail* profileEmail, const struct json_object* obj)
{
    freeProfileEmail(profileEmail);
    initProfileEmail(profileEmail);

    bool b1 = jsonGetString(obj, uuid_Key, &profileEmail->uuid);
    bool b2 = jsonGetString(obj, email_Key, &profileEmail->email);
    bool b3 = jsonGetBoolean(obj, toxic_Key, &profileEmail->toxic);
    bool b4 = jsonGetString(obj, domain_Key, &profileEmail->domain);
    bool b5 = jsonGetString(obj, gender_Key, &profileEmail->gender);
    bool b6 = jsonGetBoolean(obj, manual_Key, &profileEmail->manual);
    bool b7 = jsonGetString(obj, status_Key, &profileEmail->status);
    bool b8 = jsonGetBoolean(obj, bounced_Key, &profileEmail->bounced);
    bool b9 = jsonGetBoolean(obj, primary_Key, &profileEmail->primary);
    bool b10 = jsonGetString(obj, lastName_Key, &profileEmail->lastName);
    bool b11 = jsonGetString(obj, firstName_Key, &profileEmail->firstName);
    bool b12 = jsonGetBoolean(obj, mxFound_Key, &profileEmail->mxFound);
    bool b13 = jsonGetBoolean(obj, personal_Key, &profileEmail->personal);
    bool b14 = jsonGetString(obj, mxRecord_Key, &profileEmail->mxRecord);
    bool b15 = jsonGetBoolean(obj, disposable_Key, &profileEmail->disposable);
    bool b16 = jsonGetBoolean(obj, freeEmail_Key, &profileEmail->freeEmail);
    bool b17 = jsonGetString(obj, subStatus_Key, &profileEmail->subStatus);
    bool b18 = jsonGetString(obj, smtpProvider_Key, &profileEmail->smtpProvider);
    bool b19 = jsonGetInt32(obj, domainAgeDays_Key, &profileEmail->domainAgeDays);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13
        && b14 && b15 && b16 && b17 && b18 && b19))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: parse failed.");
        return false;
    }

    return true;
}

uint8_t* profileEmailToBinary(uint8_t* byteStream, const struct ProfileEmail* profileEmail)
{
    uint8_t* offset = byteStream;

    offset = stringToBinary(offset, profileEmail->uuid);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->email);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->toxic);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->domain);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->gender);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->manual);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->status);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->bounced);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->primary);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->lastName);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->firstName);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->mxFound);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->personal);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->mxRecord);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->disposable);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, profileEmail->freeEmail);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->subStatus);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, profileEmail->smtpProvider);
    if(offset == NULL) { return NULL; }

    return int32ToBinary(offset, profileEmail->domainAgeDays);
}

const uint8_t* binaryToProfileEmail(const uint8_t* byteStream, struct ProfileEmail* profileEmail)
{
    freeProfileEmail(profileEmail);
    initProfileEmail(profileEmail);

    const uint8_t* offset = byteStream;

    offset = binaryToString(offset, &profileEmail->uuid);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->email);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->toxic);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->domain);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->gender);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->manual);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->status);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->bounced);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->primary);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->lastName);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->firstName);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->mxFound);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->personal);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->mxRecord);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->disposable);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &profileEmail->freeEmail);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->subStatus);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &profileEmail->smtpProvider);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &profileEmail->domainAgeDays);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t profileEmailBinarySize(const struct ProfileEmail* profileEmail)
{
    return stringBinarySize(profileEmail->uuid) + stringBinarySize(profileEmail->email)
        + 8 * BOOLEAN_BINARY_SIZE + stringBinarySize(profileEmail->domain)
        + stringBinarySize(profileEmail->gender) + stringBinarySize(profileEmail->status)
        + stringBinarySize(profileEmail->lastName) + stringBinarySize(profileEmail->firstName)
        + stringBinarySize(profileEmail->mxRecord) + stringBinarySize(profileEmail->subStatus)
        + stringBinarySize(profileEmail->smtpProvider) + INT32_BINARY_SIZE;
}
