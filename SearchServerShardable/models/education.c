#include <education.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* uuid_Key         = "uuid";
static const char* name_Key         = "name";
static const char* degree_Key       = "degree";
static const char* subject_Key      = "subject";
static const char* universityId_Key = "university_id";
static const char* startDate_Key    = "start_date";
static const char* endDate_Key      = "end_date";

void initEducation(struct Education* education)
{
    education->uuid         = NULL;
    education->name         = NULL;
    education->degree       = NULL;
    education->subject      = NULL;
    education->universityId = 0;
    education->startDate    = getDate1_time_t();
    education->endDate      = getDate1_time_t();
}

void freeEducation(struct Education* education)
{
    free(education->uuid);
    education->uuid = NULL;
    free(education->name);
    education->name = NULL;
    free(education->degree);
    education->degree = NULL;
    free(education->subject);
    education->subject = NULL;
}

struct json_object* marshallEducation(const struct Education* education)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, uuid_Key, json_object_new_string(education->uuid));
        json_object_object_add(obj, name_Key, json_object_new_string(education->name));
        json_object_object_add(obj, degree_Key, json_object_new_string(education->degree));
        json_object_object_add(obj, subject_Key, json_object_new_string(education->subject));
        json_object_object_add(obj, universityId_Key,
       	    json_object_new_int64(education->universityId));
        json_object_object_add(obj, startDate_Key, marshallTime(education->startDate));
        json_object_object_add(obj, endDate_Key, marshallTime(education->endDate));
    }

    return obj;
}

bool unmarshallEducation(struct Education* education, const struct json_object* obj)
{
    freeEducation(education);
    initEducation(education);

    bool b1 = jsonGetString(obj, uuid_Key, &education->uuid);
    bool b2 = jsonGetString(obj, name_Key, &education->name);
    bool b3 = jsonGetString(obj, degree_Key, &education->degree);
    bool b4 = jsonGetString(obj, subject_Key, &education->subject);
    bool b5 = jsonGetInt64(obj, universityId_Key, &education->universityId);
    bool b6 = jsonGetTime(obj, startDate_Key, &education->startDate);
    bool b7 = jsonGetTime(obj, endDate_Key, &education->endDate);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* educationToBinary(uint8_t* byteStream, const struct Education* education)
{
    uint8_t* offset = byteStream;

    offset = stringToBinary(offset, education->uuid);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, education->name);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, education->degree);
    if(offset == NULL) { return NULL; }

    offset = stringToBinary(offset, education->subject);
    if(offset == NULL) { return NULL; }

    offset = int64ToBinary(offset, education->universityId);
    if(offset == NULL) { return NULL; }

    offset = timeToBinary(offset, education->startDate);
    if(offset == NULL) { return NULL; }

    return timeToBinary(offset, education->endDate);
}

const uint8_t* binaryToEducation(const uint8_t* byteStream, struct Education* education)
{
    freeEducation(education);
    initEducation(education);

    const uint8_t* offset = byteStream;

    offset = binaryToString(offset, &education->uuid);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &education->name);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &education->degree);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &education->subject);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt64(offset, &education->universityId);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &education->startDate);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &education->endDate);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t educationBinarySize(const struct Education* education)
{
    return stringBinarySize(education->uuid) + stringBinarySize(education->name)
        + stringBinarySize(education->degree) + stringBinarySize(education->subject)
        + INT64_BINARY_SIZE + 2 * TIME_BINARY_SIZE;
}
