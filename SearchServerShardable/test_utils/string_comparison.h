#ifndef STRING_COMPARISON_H
#define STRING_COMPARISON_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>

bool assert_string_equal_impl(const char* str1, const char* str2);
bool assert_string_not_equal_impl(const char* str1, const char* str2);

#define ASSERT_STRING_EQUAL(TM1, TM2) \
    do { if(!assert_string_equal_impl(TM1, TM2)) { fail(); } } while(0)

#define ASSERT_STRING_NOT_EQUAL(TM1, TM2) \
    do { if(!assert_string_not_equal_impl(TM1, TM2)) { fail(); } } while(0)

#endif // STRING_COMPARISON_H
