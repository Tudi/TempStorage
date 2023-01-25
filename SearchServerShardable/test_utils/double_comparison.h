#ifndef DOUBLE_COMPARISON_H
#define DOUBLE_COMPARISON_H

#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#define ASSERT_DOUBLE_EQUAL(ARG1, ARG2) \
    do { \
        if((ARG1) != (ARG2)) { \
            printf("Error: assert failed - %g == %g.", (ARG1), (ARG2)); fail(); \
        } \
    } while(0)

#define ASSERT_DOUBLE_NOT_EQUAL(ARG1, ARG2) \
    do { \
        if((ARG1) == (ARG2)) { \
            printf("Error: assert failed - %g != %g.", (ARG1), (ARG2)); fail(); \
        } \
    } while(0)

#endif // DOUBLE_COMPARISON_H
