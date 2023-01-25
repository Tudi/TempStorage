#include <json_utils.h>
#include <date_time.h>
#include <logger.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

bool jsonGetObject(const struct json_object* parentObj, const char* key, struct json_object** obj)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_object)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not an object.", key);
        return false;
    }

    *obj = value;

    return true;
}

bool jsonGetBoolean(const struct json_object* parentObj, const char* key, bool* b)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_boolean)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not a boolean.", key);
        return false;
    }

    *b = json_object_get_boolean(value);

    return true;
}

bool jsonGetInt16(const struct json_object* parentObj, const char* key, int16_t* i)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_int)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not an integer.", key);
        return false;
    }

    errno = 0;
    int32_t i32 = json_object_get_int(value);
    if((i32 == 0) && (errno == EINVAL))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid value for key \"%s\".",key);
        return false;
    }

    if(i32 < INT16_MIN)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: value too small (%d) for int16_t"
            " (minimum = %hd) for key \"%s\".", i32, INT16_MIN, key);
        return false;
    }

    if(i32 > INT16_MAX)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: value too large (%d) for int16_t"
            " (maximum = %hd) for key \"%s\".", i32, INT16_MAX, key);
        return false;
    }

    *i = (int16_t) i32;

    return true;
}

bool jsonGetUint16(const struct json_object* parentObj, const char* key, uint16_t* i)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_int)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not an integer.", key);
        return false;
    }

    errno = 0;
    int32_t i32 = json_object_get_int(value);
    if((i32 == 0) && (errno == EINVAL))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid value for key \"%s\".",key);
        return false;
    }

    if(i32 < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: value too small (%d) for uint16_t"
            " (minimum = 0) for key \"%s\".", i32, key);
        return false;
    }

    if(i32 > UINT16_MAX)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: value too large (%d) for uint16_t"
            " (maximum = %hd) for key \"%s\".", i32, UINT16_MAX, key);
        return false;
    }

    *i = (uint16_t) i32;

    return true;
}

bool jsonGetInt32(const struct json_object* parentObj, const char* key, int32_t* i)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_int)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not an integer.", key);
        return false;
    }

    errno = 0;
    *i = json_object_get_int(value);
    if((*i == 0) && (errno == EINVAL))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid value for key \"%s\".",key);
        return false;
    }

    return true;
}

bool jsonGetUint32(const struct json_object* parentObj, const char* key, uint32_t* i)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_int)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not an integer.", key);
        return false;
    }

    errno = 0;
    int64_t i64 = json_object_get_int64(value);
    if((i64 == 0) && (errno == EINVAL))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid value for key \"%s\".",key);
        return false;
    }

    if(i64 < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: value too small (%lld) for uint32_t"
            " (minimum = 0) for key \"%s\".", i64, key);
        return false;
    }

    if(i64 > UINT32_MAX)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: value too large (%lld) for uint32_t"
            " (maximum = %u) for key \"%s\".", i64, UINT32_MAX, key);
        return false;
    }

    *i = (uint32_t) i64;

    return true;
}

bool jsonGetInt64(const struct json_object* parentObj, const char* key, int64_t* i)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_int)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not an integer.", key);
        return false;
    }

    errno = 0;
    *i = json_object_get_int64(value);
    if((*i == 0) && (errno == EINVAL))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid value for key \"%s\".",key);
        return false;
    }

    return true;
}

bool jsonGetDouble(const struct json_object* parentObj, const char* key, double* d)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_double)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not a double.", key);
        return false;
    }

    errno = 0;
    *d = json_object_get_double(value);
    if((*d == 0.0) && (errno == EINVAL))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid value for key \"%s\".", key);
        return false;
    }

    if(isnan(*d))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid double value (NaN) for key \"%s\".", key);
        return false;
    }

    if(isinf(*d))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid double value (infinite) for key \"%s\".", key);
        return false;
    }

    if(errno == ERANGE)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: out of range double value for key \"%s\".", key);
        return false;
    }

    return true;
}

bool jsonGetString(const struct json_object* parentObj, const char* key, char** str)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(json_object_get_type(value) != json_type_string)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\", value is not a string.", key);
        return false;
    }

    int strLength = json_object_get_string_len(value) + 1;

    *str = malloc(strLength * sizeof(char));
    if(*str == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.", strLength * sizeof(char));
        return false;
    }

    strcpy(*str, json_object_get_string(value));

    return true;
}

bool jsonGetTime(const struct json_object* parentObj, const char* key, time_t* t)
{
    struct json_object* value = NULL;

    if(!json_object_object_get_ex(parentObj, key, &value))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: key \"%s\" is not present.", key);
        return false;
    }

    if(unmarshallTime(t, value) == false)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallTime(key = \"%s\") failed.", key);
        return false;
    }

    return true;
}
