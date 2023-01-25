#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

bool jsonGetObject(const struct json_object* parentObj, const char* key, struct json_object** obj);
bool jsonGetBoolean(const struct json_object* parentObj, const char* key, bool* b);
bool jsonGetInt16(const struct json_object* parentObj, const char* key, int16_t* i);
bool jsonGetUint16(const struct json_object* parentObj, const char* key, uint16_t* i);
bool jsonGetInt32(const struct json_object* parentObj, const char* key, int32_t* i);
bool jsonGetUint32(const struct json_object* parentObj, const char* key, uint32_t* i);
bool jsonGetInt64(const struct json_object* parentObj, const char* key, int64_t* i);
bool jsonGetDouble(const struct json_object* parentObj, const char* key, double* d);
bool jsonGetString(const struct json_object* parentObj, const char* key, char** str);
bool jsonGetTime(const struct json_object* parentObj, const char* key, time_t* t);

#endif // JSON_UTILS_H
