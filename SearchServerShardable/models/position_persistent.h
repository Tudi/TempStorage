#ifndef POSITIONPERSISTENT_H
#define POSITIONPERSISTENT_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct PositionPersistent
{
    char* companyName;       // string `json:"company_name"`
    char* title;             // string `json:"title" validate:"required"`
    time_t startDate;        // time.Time `json:"start_date"`
    time_t endDate;          // time.Time `json:"end_date"`
    int32_t companyId;       // int `json:"company_id" validate:"required"`
    int32_t titleId;         // int `json:"title_id"  

    // Fields below are only used by PositionPersistent and not by ProfileCached.
    char* uuid;              // string `json:"uuid"`
    char* titleCorrected;    // string `json:"title_corrected"`
    char* source;            // string `json:"source"`
    char* locality;          // string `json:"locality"`
    int32_t parentCompanyId; // int `json:"parent_company_id"`
    char* description;       // string `json:"description"`
    char* seniority;         // string `json:"seniority"`
    bool modifiedInLoad;     // bool `json:"modified_in_load"`
    int32_t parentTitleId;   // int `json:"title_parent_id"  
};

void initPositionPersistent(struct PositionPersistent* position);
void freePositionPersistent(struct PositionPersistent* position);

struct json_object* marshallPositionPersistent(const struct PositionPersistent* position);
bool unmarshallPositionPersistent(struct PositionPersistent* position,
    const struct json_object* obj);
uint8_t* positionPersistentToBinary(uint8_t* byteStream, const struct PositionPersistent* position);
const uint8_t* binaryToPositionPersistent(const uint8_t* byteStream,
    struct PositionPersistent* position);
uint32_t positionPersistentBinarySize(const struct PositionPersistent* position);

#define UNMARSHALL_POSITIONPERSISTENT(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallPositionPersistent, OBJ, VAR, RET)

#define JSON_GET_POSITIONPERSISTENT_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(struct PositionPersistent, UNMARSHALL_POSITIONPERSISTENT, \
        initPositionPersistent, freePositionPersistent, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_POSITIONPERSISTENT_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct PositionPersistent, ARRAY, binaryToPositionPersistent, \
        initPositionPersistent, freePositionPersistent, BYTESTREAM, OFFSET)

#endif // POSITIONPERSISTENT_H
