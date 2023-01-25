#ifndef JSON_SPECIALIZED_ARRAY_H
#define JSON_SPECIALIZED_ARRAY_H

#include <json_array.h>
#include <json_object.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//
// ADD_ARRAY
//

#define JSON_ADD_INT32_ARRAY(OBJ, KEY, ARRAY) \
    JSON_ADD_ARRAY_VALUE(json_object_new_int, OBJ, KEY, ARRAY)

#define JSON_ADD_STRING_ARRAY(OBJ, KEY, ARRAY) \
    JSON_ADD_ARRAY_VALUE(json_object_new_string, OBJ, KEY, ARRAY)

//
// GET_ARRAY
//

//
// int32_t
//

#define PARSE_INT32(OBJ, VAR, RET) \
    PARSE_TYPE(json_object_get_int, OBJ, VAR, RET)

#define JSON_GET_INT32_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(int32_t, PARSE_INT32, DUMMY_INIT, DUMMY_FREE, OBJ, KEY, ARRAY, RET)

//
// char*
//

#define PARSE_STRING(OBJ, VAR, RET) \
do { \
    int strLength = json_object_get_string_len(OBJ) + 1; \
\
    char *auxStr = malloc(strLength * sizeof(char)); \
    if(auxStr != NULL) \
    { \
        strcpy(auxStr, json_object_get_string(OBJ)); \
        (VAR) = auxStr; \
        RET = true; \
    } \
    else \
    { \
        RET = false; \
    } \
} while(false)

#define INIT_STRING(STR) do { *STR = NULL; } while(false)

#define FREE_STRING(STR) do { free(*STR); *STR = NULL; } while(false)

#define JSON_GET_STRING_ARRAY(OBJ, KEY, ARRAY, RET) \
    JSON_GET_ARRAY(char*, PARSE_STRING, INIT_STRING, FREE_STRING, OBJ, KEY, ARRAY, RET)

#endif // JSON_SPECIALIZED_ARRAY_H
