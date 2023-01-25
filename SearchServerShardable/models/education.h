#ifndef EDUCATION_H
#define EDUCATION_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

struct Education
{
    char* uuid;           // string `json:"uuid"`
    char* name;           // string `json:"name" validate:"required"`
    char* degree;         // string `json:"degree"`
    char* subject;        // string `json:"subject"`
    int64_t universityId; // int64  `json:"university_id" validate:"required"`
    time_t startDate;     // time.Time `json:"start_date"`
    time_t endDate;       // time.Time `json:"end_date"`
};

void initEducation(struct Education* education);
void freeEducation(struct Education* education);

struct json_object* marshallEducation(const struct Education* education);
bool unmarshallEducation(struct Education* education, const struct json_object* obj);
uint8_t* educationToBinary(uint8_t* byteStream, const struct Education* education);
const uint8_t* binaryToEducation(const uint8_t* byteStream, struct Education* education);
uint32_t educationBinarySize(const struct Education* education);

#define UNMARSHALL_EDUCATION(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallEducation, OBJ, VAR, RET)

#define JSON_GET_EDUCATION_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct Education, \
    UNMARSHALL_EDUCATION, initEducation, freeEducation, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_EDUCATION_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct Education, ARRAY, binaryToEducation, initEducation, freeEducation, \
        BYTESTREAM, OFFSET)

#endif // EDUCATION_H
