#ifndef ID_VALUE_H
#define ID_VALUE_H

#include <json_array.h>
#include <binary_array.h>
#include <json_object.h>
#include <date_time.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

//
// struct Id_Int32Value
//

struct Id_Int32Value
{
    int32_t id;
    int32_t value;
};

void initId_Int32Value(struct Id_Int32Value* idVal);
void freeId_Int32Value(struct Id_Int32Value* idVal);

struct json_object* marshallId_Int32Value(const struct Id_Int32Value* idVal);
bool unmarshallId_Int32Value(struct Id_Int32Value* idVal, const struct json_object* obj);
uint8_t* id_Int32ValueToBinary(uint8_t* byteStream, const struct Id_Int32Value* idVal);
const uint8_t* binaryToId_Int32Value(const uint8_t* byteStream, struct Id_Int32Value* idVal);

#define ID_INT32VALUE_BINARY_SIZE (sizeof(int32_t) + sizeof(int32_t))

#define UNMARSHALL_ID_INT32VALUE(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallId_Int32Value, OBJ, VAR, RET)

#define JSON_GET_ID_INT32VALUE_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct Id_Int32Value, \
    UNMARSHALL_ID_INT32VALUE, initId_Int32Value, freeId_Int32Value, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_ID_INT32VALUE_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct Id_Int32Value, ARRAY, binaryToId_Int32Value, \
        initId_Int32Value, freeId_Int32Value, BYTESTREAM, OFFSET)

//
// struct Id_StringValue
//

struct Id_StringValue
{
    int32_t id;
    char* value;
};

void initId_StringValue(struct Id_StringValue* idVal);
void freeId_StringValue(struct Id_StringValue* idVal);

bool copyId_StringValue(struct Id_StringValue* dest, const struct Id_StringValue* src);

struct json_object* marshallId_StringValue(const struct Id_StringValue* idVal);
bool unmarshallId_StringValue(struct Id_StringValue* idVal, const struct json_object* obj);
uint8_t* id_StringValueToBinary(uint8_t* byteStream, const struct Id_StringValue* idVal);
const uint8_t* binaryToId_StringValue(const uint8_t* byteStream, struct Id_StringValue* idVal);
uint32_t id_StringValueBinarySize(const struct Id_StringValue* idVal);

#define UNMARSHALL_ID_STRINGVALUE(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallId_StringValue, OBJ, VAR, RET)

#define JSON_GET_ID_STRINGVALUE_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct Id_StringValue, \
    UNMARSHALL_ID_STRINGVALUE, initId_StringValue, freeId_StringValue, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_ID_STRINGVALUE_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct Id_StringValue, ARRAY, binaryToId_StringValue, \
        initId_StringValue, freeId_StringValue, BYTESTREAM, OFFSET)

//
// struct Id_TimeValue
//

struct Id_TimeValue
{
    int32_t id;
    time_t value;
};

void initId_TimeValue(struct Id_TimeValue* idVal);
void freeId_TimeValue(struct Id_TimeValue* idVal);

struct json_object* marshallId_TimeValue(const struct Id_TimeValue* idVal);
bool unmarshallId_TimeValue(struct Id_TimeValue* idVal, const struct json_object* obj);
uint8_t* id_TimeValueToBinary(uint8_t* byteStream, const struct Id_TimeValue* idVal);
const uint8_t* binaryToId_TimeValue(const uint8_t* byteStream, struct Id_TimeValue* idVal);

#define ID_TIMEVALUE_BINARY_SIZE (sizeof(int32_t) + TIME_BINARY_SIZE)

#define UNMARSHALL_ID_TIMEVALUE(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallId_TimeValue, OBJ, VAR, RET)

#define JSON_GET_ID_TIMEVALUE_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct Id_TimeValue, \
    UNMARSHALL_ID_TIMEVALUE, initId_TimeValue, freeId_TimeValue, OBJ, KEY, ARRAY, RET)

#define BINARY_TO_ID_TIMEVALUE_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct Id_TimeValue, ARRAY, binaryToId_TimeValue, \
        initId_TimeValue, freeId_TimeValue, BYTESTREAM, OFFSET)

#endif // ID_VALUE_H
