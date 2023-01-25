#ifndef VECTOR_COMPARISON_H
#define VECTOR_COMPARISON_H

#include <kvec.h>
#include <stdbool.h>

#define VECTOR_ARG_VALUE(v) (v)
#define VECTOR_ARG_ADDR(v) &(v)

#define ASSERT_VECTOR_EQUAL(ASSERT_EQUAL, VECTOR_ARG, VEC1, VEC2) do { \
        assert_int_equal(kv_size(VEC1), kv_size(VEC2)); \
        size_t i = 0; \
        for(; i < kv_size(VEC1); ++i) { \
            ASSERT_EQUAL(VECTOR_ARG(kv_A((VEC1), i)), VECTOR_ARG(kv_A((VEC2), i))); \
        } \
    } while(false)

#define ASSERT_VECTOR_VALUE_EQUAL(ASSERT_EQUAL, VEC1, VEC2) \
    ASSERT_VECTOR_EQUAL(ASSERT_EQUAL, VECTOR_ARG_VALUE, VEC1, VEC2)

#define ASSERT_VECTOR_ADDR_EQUAL(ASSERT_EQUAL, VEC1, VEC2) \
    ASSERT_VECTOR_EQUAL(ASSERT_EQUAL, VECTOR_ARG_ADDR, VEC1, VEC2)

#endif // VECTOR_COMPARISON_H
