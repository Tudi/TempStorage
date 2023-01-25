#ifndef JSON_ARRAY_H
#define JSON_ARRAY_H

#include <macro_utils.h>
#include <logger.h>
#include <json_object.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <kvec.h>

//
// ADD_ARRAY
//

#define JSON_ADD_ARRAY(FUNC, FUNC_ARG, OBJ, KEY, ARRAY) \
do { \
    struct json_object* arrObj = json_object_new_array_ext(kv_size(ARRAY)); \
\
    size_t i = 0; \
    for(; i < kv_size((ARRAY)); ++i) { \
        json_object_array_add(arrObj, (FUNC)(FUNC_ARG(kv_A((ARRAY), i)))); \
    } \
\
    json_object_object_add((OBJ), (KEY), arrObj); \
} while(false)

#define JSON_ADD_ARRAY_VALUE(FUNC, OBJ, KEY, ARRAY) \
    JSON_ADD_ARRAY(FUNC, FUNC_ARG_VALUE, OBJ, KEY, ARRAY)

#define JSON_ADD_ARRAY_ADDR(FUNC, OBJ, KEY, ARRAY) \
    JSON_ADD_ARRAY(FUNC, FUNC_ARG_ADDR, OBJ, KEY, ARRAY)

//
// GET_ARRAY
//

#define JSON_GET_ARRAY(TYPE, PARSE_FUNC, INIT_FUNC, FREE_FUNC, OBJ, KEY, ARRAY, RET) \
do { \
    struct json_object* jsonArray = NULL; \
\
    if(json_object_object_get_ex((OBJ), (KEY), &jsonArray)) \
    { \
        if(json_object_get_type(jsonArray) == json_type_array) \
        { \
            size_t numElems = json_object_array_length(jsonArray); \
\
            for(size_t i = 0; i < kv_size((ARRAY)); ++i) \
            { \
                FREE_FUNC(&kv_A((ARRAY), i)); \
                INIT_FUNC(&kv_A((ARRAY), i)); \
            } \
\
            kv_resize(TYPE, (ARRAY), numElems); \
\
            size_t i = 0; \
            for(; i < numElems; ++i) \
            { \
                struct json_object* valueObj = json_object_array_get_idx(jsonArray, i); \
                if(valueObj == NULL) \
                { \
                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: JSON_GET_ARRAY() - nonexistent value." \
                        " Array (key = \"%s\"), element (index = %zu).", KEY, i); \
                    (RET) = false; \
                    break; \
                } \
\
                (void) kv_a(TYPE, (ARRAY), i); /* To prevent compiler warning. */ \
                INIT_FUNC(&kv_A((ARRAY), i)); \
                PARSE_FUNC(valueObj, kv_A((ARRAY), i), (RET)); \
                if((RET) == false) \
                {  \
                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: JSON_GET_ARRAY() - parse function" \
                        " failed. Array (key = \"%s\"), element (index = %zu).", KEY, i); \
                    break; \
                } \
            } \
\
            if(i == numElems) { (RET) = true; } \
        } \
        else \
        { \
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: JSON_GET_ARRAY() - key (\"%s\"), value is" \
                " not an array.",KEY); \
            (RET) = false; \
        } \
    } \
    else \
    { \
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: JSON_GET_ARRAY() - key \"%s\" is not" \
            " present.", KEY); \
        (RET) = false; \
    } \
} while(false)

#define PARSE_TYPE(FUNC, OBJ, VAR, RET) \
do { \
    (VAR) = (FUNC)(OBJ); \
    (RET) = true; \
} while(false)

#define UNMARSHALL_TYPE(FUNC, OBJ, VAR, RET) \
do { \
    (RET) = (FUNC)(&(VAR), (OBJ)); \
} while(false)

#endif // JSON_ARRAY_H
