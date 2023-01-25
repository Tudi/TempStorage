#include <position_persistent.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* companyName_Key     = "company_name";
static const char* title_Key           = "title";
static const char* startDate_Key       = "start_date";
static const char* endDate_Key         = "end_date";
static const char* companyId_Key       = "company_id";
static const char* titleId_Key         = "title_id";
static const char* uuid_Key            = "uuid";
static const char* titleCorrected_Key  = "title_corrected";
static const char* source_Key          = "source";
static const char* locality_Key        = "locality";
static const char* parentCompanyId_Key = "parent_company_id";
static const char* description_Key     = "description";
static const char* seniority_Key       = "seniority";
static const char* modifiedInLoad_Key  = "modified_in_load";
static const char* parentTitleId_Key   = "title_parent_id";

void initPositionPersistent(struct PositionPersistent* position)
{
    position->companyName = NULL;
    position->title       = NULL;
    position->startDate   = getDate1_time_t();
    position->endDate     = getDate1_time_t();
    position->companyId   = 0;
    position->titleId     = 0;

    position->uuid            = NULL;
    position->titleCorrected  = NULL;
    position->source          = NULL;
    position->locality        = NULL;
    position->parentCompanyId = 0;
    position->description     = NULL;
    position->seniority       = NULL;
    position->modifiedInLoad  = false;
    position->parentTitleId   = 0;
}

void freePositionPersistent(struct PositionPersistent* position)
{
    free(position->companyName);
    position->companyName = NULL;
    free(position->title);
    position->title = NULL;

    free(position->uuid);
    position->uuid = NULL;
    free(position->titleCorrected);
    position->titleCorrected = NULL;
    free(position->source);
    position->source = NULL;
    free(position->locality);
    position->locality = NULL;
    free(position->description);
    position->description = NULL;
    free(position->seniority);
    position->seniority = NULL;
}

struct json_object* marshallPositionPersistent(const struct PositionPersistent* position)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, companyName_Key, json_object_new_string(position->companyName));
        json_object_object_add(obj, title_Key, json_object_new_string(position->title));
        json_object_object_add(obj, startDate_Key, marshallTime(position->startDate));
        json_object_object_add(obj, endDate_Key, marshallTime(position->endDate));
        json_object_object_add(obj, companyId_Key, json_object_new_int(position->companyId));
        json_object_object_add(obj, titleId_Key, json_object_new_int(position->titleId));

        json_object_object_add(obj, uuid_Key, json_object_new_string(position->uuid));
        json_object_object_add(obj, titleCorrected_Key,
            json_object_new_string(position->titleCorrected));
        json_object_object_add(obj, source_Key, json_object_new_string(position->source));
        json_object_object_add(obj, locality_Key, json_object_new_string(position->locality));
        json_object_object_add(obj, parentCompanyId_Key,
       	    json_object_new_int(position->parentCompanyId));
        json_object_object_add(obj, description_Key, json_object_new_string(position->description));
        json_object_object_add(obj, seniority_Key, json_object_new_string(position->seniority));
        json_object_object_add(obj, modifiedInLoad_Key,
            json_object_new_boolean(position->modifiedInLoad));
        json_object_object_add(obj, parentTitleId_Key, json_object_new_int(position->parentTitleId));
    }

    return obj;
}

bool unmarshallPositionPersistent(struct PositionPersistent* position,
    const struct json_object* obj)
{
    freePositionPersistent(position);
    initPositionPersistent(position);

    bool b1 = jsonGetString(obj, companyName_Key, &position->companyName);
    bool b2 = jsonGetString(obj, title_Key, &position->title);
    bool b3 = jsonGetTime(obj, startDate_Key, &position->startDate);
    bool b4 = jsonGetTime(obj, endDate_Key, &position->endDate);
    bool b5 = jsonGetInt32(obj, companyId_Key, &position->companyId);
    bool b6 = jsonGetInt32(obj, titleId_Key, &position->titleId);

    bool b7  = jsonGetString(obj, uuid_Key, &position->uuid);
    bool b8  = jsonGetString(obj, titleCorrected_Key, &position->titleCorrected);
    bool b9  = jsonGetString(obj, source_Key, &position->source);
    bool b10 = jsonGetString(obj, locality_Key, &position->locality);
    bool b11 = jsonGetInt32(obj, parentCompanyId_Key, &position->parentCompanyId);
    bool b12 = jsonGetString(obj, description_Key, &position->description);
    bool b13 = jsonGetString(obj, seniority_Key, &position->seniority);
    bool b14 = jsonGetBoolean(obj, modifiedInLoad_Key, &position->modifiedInLoad);
    bool b15 = jsonGetInt32(obj, parentTitleId_Key, &position->parentTitleId);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12 && b13 && b14 && b15))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* positionPersistentToBinary(uint8_t* byteStream, const struct PositionPersistent* position)
{
    uint8_t* offset = byteStream;

    offset = stringToBinary(offset, position->companyName);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->title);
    if(offset == NULL) { return NULL; }

    offset = timeToBinary(offset, position->startDate);
    if(offset == NULL) { return NULL; }

    offset = timeToBinary(offset, position->endDate);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, position->companyId);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, position->titleId);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->uuid);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->titleCorrected);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->source);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->locality);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, position->parentCompanyId);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->description);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, position->seniority);
    if(offset == NULL) { return NULL; }

    offset = booleanToBinary(offset, position->modifiedInLoad);
    if(offset == NULL) { return NULL; }

    offset = int32ToBinary(offset, position->parentTitleId);
    if(offset == NULL) { return NULL; }

    return offset;
}

const uint8_t* binaryToPositionPersistent(const uint8_t* byteStream,
    struct PositionPersistent* position)
{
    freePositionPersistent(position);
    initPositionPersistent(position);

    const uint8_t* offset = byteStream;

    offset = binaryToString(offset, &position->companyName);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->title);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &position->startDate);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &position->endDate);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &position->companyId);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &position->titleId);
    if (offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->uuid);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->titleCorrected);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->source);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->locality);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &position->parentCompanyId);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->description);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &position->seniority);
    if(offset == NULL) { return NULL; }

    offset = binaryToBoolean(offset, &position->modifiedInLoad);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &position->parentTitleId);
    if (offset == NULL) { return NULL; }

    return offset;
}

uint32_t positionPersistentBinarySize(const struct PositionPersistent* position)
{
    return stringBinarySize(position->title) + stringBinarySize(position->companyName)
        + 2 * TIME_BINARY_SIZE + 2 * INT32_BINARY_SIZE
        + stringBinarySize(position->uuid) + stringBinarySize(position->titleCorrected)
        + stringBinarySize(position->source) + BOOLEAN_BINARY_SIZE
        + stringBinarySize(position->locality) + 2 * INT32_BINARY_SIZE
        + stringBinarySize(position->description) + stringBinarySize(position->seniority);
}
