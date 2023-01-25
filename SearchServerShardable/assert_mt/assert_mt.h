#ifndef ASSERT_MT_H
#define ASSERT_MT_H

#include <logger.h>
#include <stdbool.h>
#include <pthread.h>

//
// Variables
//

extern bool programError;

//
// Assertion macros
//

#define ASSERT_MT_IMPL(TYPE, ARG1, ARG2, PRINT_FLAG, COMP_FUNC) \
    do { \
        TYPE arg1 = ARG1; \
        TYPE arg2 = ARG2; \
        if(!(COMP_FUNC(arg1, arg2))) \
        { \
            programError = true; \
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: ASSERT failed. File \"" __FILE__ "\"," \
                " line %d, " #COMP_FUNC "(" #ARG1 ", " #ARG2 "), arg1 = " PRINT_FLAG \
                ", arg2 = " PRINT_FLAG ".\n",  __LINE__, arg1, arg2); \
            pthread_exit(NULL); \
        } \
    } while(false)

#define MT_EQUAL_FUNC(ARG1, ARG2) ((ARG1) == (ARG2))
#define MT_NOT_EQUAL_FUNC(ARG1, ARG2) ((ARG1) != (ARG2))
#define MT_STR_EQUAL_FUNC(ARG1, ARG2) !strcmp((ARG1), (ARG2))
#define MT_STR_NOT_EQUAL_FUNC(ARG1, ARG2) strcmp((ARG1), (ARG2))

#define ASSERT_MT_INT_EQUAL(ARG1, ARG2) ASSERT_MT_IMPL(int64_t, ARG1, ARG2, "%jd", MT_EQUAL_FUNC)
#define ASSERT_MT_INT_NOT_EQUAL(ARG1, ARG2) \
    ASSERT_MT_IMPL(int64_t, ARG1, ARG2, "%jd", MT_NOT_EQUAL_FUNC)

#define ASSERT_MT_PTR_EQUAL(ARG1, ARG2) ASSERT_MT_IMPL(void*, ARG1, ARG2, "%p", MT_EQUAL_FUNC)
#define ASSERT_MT_PTR_NOT_EQUAL(ARG1, ARG2) \
    ASSERT_MT_IMPL(void*, ARG1, ARG2, "%p", MT_NOT_EQUAL_FUNC)

#define ASSERT_MT_STR_EQUAL(ARG1, ARG2) \
    ASSERT_MT_IMPL(const char*, ARG1, ARG2, "\"%s\"", MT_STR_EQUAL_FUNC)
#define ASSERT_MT_STR_NOT_EQUAL(ARG1, ARG2) \
    ASSERT_MT_IMPL(const char*, ARG1, ARG2, "\"%s\"", MT_STR_NOT_EQUAL_FUNC)

#define ASSERT_MT_PTR_EQUAL(ARG1, ARG2) ASSERT_MT_IMPL(void*, ARG1, ARG2, "%p", MT_EQUAL_FUNC)
#define ASSERT_MT_PTR_NOT_EQUAL(ARG1, ARG2) \
    ASSERT_MT_IMPL(void*, ARG1, ARG2, "%p", MT_NOT_EQUAL_FUNC)

#define ASSERT_MT_NULL(ARG) ASSERT_MT_IMPL(void*, NULL, ARG, "%p", MT_EQUAL_FUNC)
#define ASSERT_MT_NOT_NULL(ARG) ASSERT_MT_IMPL(void*, NULL, ARG, "%p", MT_NOT_EQUAL_FUNC)

#define ASSERT_MT_TRUE(ARG) ASSERT_MT_IMPL(bool, true, ARG, "%d", MT_EQUAL_FUNC)
#define ASSERT_MT_FALSE(ARG) ASSERT_MT_IMPL(bool, true, ARG, "%d", MT_NOT_EQUAL_FUNC)

#endif // ASSERT_MT_H
