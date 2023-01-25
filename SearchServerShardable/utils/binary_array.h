#ifndef BINARY_ARRAY_H
#define BINARY_ARRAY_H

#include <binary_utils.h>
#include <macro_utils.h>
#include <kvec.h>
#include <stdbool.h>

//
// To binary
//

#define ARRAY_TO_BINARY(ARRAY, TO_BINARY_FUNC, FUNC_ARG, BYTESTREAM, OFFSET) \
do { \
    if((kv_size(ARRAY)) <= UINT16_MAX) \
    {\
        uint16_t numElems = (uint16_t) kv_size(ARRAY); \
        OFFSET = uint16ToBinary(BYTESTREAM, numElems); \
\
        size_t i = 0; \
        for(; (i < numElems) && (OFFSET != NULL); ++i) { \
            OFFSET = TO_BINARY_FUNC(OFFSET, (FUNC_ARG(kv_A(ARRAY, i)))); \
        } \
    } \
} while(false)

#define VALUE_ARRAY_TO_BINARY(ARRAY, TO_BINARY_FUNC, BYTESTREAM, OFFSET) \
    ARRAY_TO_BINARY(ARRAY, TO_BINARY_FUNC, FUNC_ARG_VALUE, BYTESTREAM, OFFSET)

#define ADDR_ARRAY_TO_BINARY(ARRAY, TO_BINARY_FUNC, BYTESTREAM, OFFSET) \
    ARRAY_TO_BINARY(ARRAY, TO_BINARY_FUNC, FUNC_ARG_ADDR, BYTESTREAM, OFFSET)

//
// From binary
//

#define BINARY_TO_ARRAY(TYPE, ARRAY, FROM_BINARY_FUNC, INIT_FUNC, FREE_FUNC, BYTESTREAM, OFFSET) \
do { \
    uint16_t i = 0; \
    for(; i < kv_size((ARRAY)); ++i) \
    { \
        FREE_FUNC(&kv_A((ARRAY), i)); \
        INIT_FUNC(&kv_A((ARRAY), i)); \
    } \
\
    uint16_t numElems = 0; \
    OFFSET = binaryToUint16(BYTESTREAM, &numElems); \
    if(OFFSET != NULL) \
    { \
        kv_resize(TYPE, ARRAY, numElems); \
\
        for(; (i < numElems) && (OFFSET != NULL); ++i) \
        { \
            (void) kv_a(TYPE, (ARRAY), i); /* To prevent compiler warning. */ \
            INIT_FUNC(&kv_A((ARRAY), i)); \
            OFFSET = FROM_BINARY_FUNC(OFFSET, &(kv_A(ARRAY, i))); \
        } \
    } \
} while(false)

#define SKIP_BINARY_ARRAY(SKIP_BINARY_FUNC, BYTESTREAM, OFFSET) \
do { \
    uint16_t numElems = 0; \
    OFFSET = binaryToUint16(BYTESTREAM, &numElems); \
    if(OFFSET != NULL) \
    { \
        for(uint16_t i = 0;(i < numElems) && (OFFSET != NULL); ++i) { \
            OFFSET = SKIP_BINARY_FUNC(OFFSET); \
        } \
    } \
} while(false)

//
// Binary size
//

#define VARIABLE_ARRAY_BINARY_SIZE(ARRAY, BINARY_SIZE_FUNC, TOTAL_SIZE) \
do { \
    TOTAL_SIZE = sizeof(uint16_t);\
    size_t i = 0; \
    for(; i < kv_size((ARRAY)); ++i) { \
        TOTAL_SIZE += BINARY_SIZE_FUNC(&kv_A((ARRAY), i)); \
    } \
} while(false)

#define FIXED_ARRAY_BINARY_SIZE(ARRAY, BINARY_SIZE_FUNC) \
    (sizeof(uint16_t) + kv_size(ARRAY) * BINARY_SIZE_FUNC)

#endif // BINARY_ARRAY_H
