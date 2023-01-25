#include <id_value.h>
#include <binary_utils.h>
#include <utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

static const char* id_Key    = "id";
static const char* value_Key = "value";

//
// struct Id_Int32Value
//

void initId_Int32Value(struct Id_Int32Value* idVal)
{
    idVal->id = 0;
    idVal->value = 0;
}

void freeId_Int32Value(struct Id_Int32Value* idVal) { }

struct json_object* marshallId_Int32Value(const struct Id_Int32Value* idVal)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, id_Key, json_object_new_int(idVal->id));
        json_object_object_add(obj, value_Key, json_object_new_int(idVal->value));
    }

    return obj;
}

bool unmarshallId_Int32Value(struct Id_Int32Value* idVal, const struct json_object* obj)
{
    bool b1 = jsonGetInt32(obj, id_Key, &idVal->id);
    bool b2 = jsonGetInt32(obj, value_Key, &idVal->value);

    if(!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}

uint8_t* id_Int32ValueToBinary(uint8_t* byteStream, const struct Id_Int32Value* idVal)
{
    uint8_t* offset = int32ToBinary(byteStream, idVal->id);
    if(offset == NULL) { return NULL; }

    return int32ToBinary(offset, idVal->value);
}

const uint8_t* binaryToId_Int32Value(const uint8_t* byteStream, struct Id_Int32Value* idVal)
{
    const uint8_t* offset = binaryToInt32(byteStream, &idVal->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &idVal->value);
    if(offset == NULL) { return NULL; }

    return offset;
}

//
// struct Id_StringValue
//

void initId_StringValue(struct Id_StringValue* idVal)
{
    idVal->id = 0;
    idVal->value = NULL;
}

void freeId_StringValue(struct Id_StringValue* idVal)
{
    free(idVal->value);
    idVal->value = NULL;
}

bool copyId_StringValue(struct Id_StringValue* dest, const struct Id_StringValue* src)
{
    freeId_StringValue(dest);
    initId_StringValue(dest);

    dest->id = src->id;
    dest->value = strdup(src->value);

    return dest->value != NULL;
}

struct json_object* marshallId_StringValue(const struct Id_StringValue* idVal)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, id_Key, json_object_new_int(idVal->id));
        json_object_object_add(obj, value_Key, json_object_new_string(idVal->value));
    }

    return obj;
}

bool unmarshallId_StringValue(struct Id_StringValue* idVal, const struct json_object* obj)
{
    freeId_StringValue(idVal);
    initId_StringValue(idVal);

    bool b1 = jsonGetInt32(obj, id_Key, &idVal->id);
    bool b2 = jsonGetString(obj, value_Key, &idVal->value);

    if(!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallId_StringValue() failed.");
        return false;
    }

    return true;
}

uint8_t* id_StringValueToBinary(uint8_t* byteStream, const struct Id_StringValue* idVal)
{
    uint8_t* offset = int32ToBinary(byteStream, idVal->id);
    if(offset == NULL) { return NULL; }

    return stringToBinary(offset, idVal->value);
}

const uint8_t* binaryToId_StringValue(const uint8_t* byteStream, struct Id_StringValue* idVal)
{
    freeId_StringValue(idVal);
    initId_StringValue(idVal);

    const uint8_t* offset = binaryToInt32(byteStream, &idVal->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToString(offset, &idVal->value);
    if(offset == NULL) { return NULL; }

    return offset;
}

uint32_t id_StringValueBinarySize(const struct Id_StringValue* idVal)
{
    return sizeof(int32_t) + stringBinarySize(idVal->value);
}

//
// struct Id_TimeValue
//

void initId_TimeValue(struct Id_TimeValue* idVal)
{
    idVal->id = 0;
    idVal->value = getDate1_time_t();
}

void freeId_TimeValue(struct Id_TimeValue* idVal) { }

struct json_object* marshallId_TimeValue(const struct Id_TimeValue* idVal)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, id_Key, json_object_new_int(idVal->id));
        json_object_object_add(obj, value_Key, marshallTime(idVal->value));
    }

    return obj;
}

bool unmarshallId_TimeValue(struct Id_TimeValue* idVal, const struct json_object* obj)
{
    bool b1 = jsonGetInt32(obj, id_Key, &idVal->id);
    bool b2 = jsonGetTime(obj, value_Key, &idVal->value);

    if(!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallId_TimeValue() failed.");
        return false;
    }

    return true;
}

uint8_t* id_TimeValueToBinary(uint8_t* byteStream, const struct Id_TimeValue* idVal)
{
    uint8_t* offset = int32ToBinary(byteStream, idVal->id);
    if(offset == NULL) { return NULL; }

    return timeToBinary(offset, idVal->value);
}

const uint8_t* binaryToId_TimeValue(const uint8_t* byteStream, struct Id_TimeValue* idVal)
{
    const uint8_t* offset = binaryToInt32(byteStream, &idVal->id);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &idVal->value);
    if(offset == NULL) { return NULL; }

    return offset;
}
