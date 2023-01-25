#include <mt_queue.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

static MtQueue_t queue = MtQueue_NULL;

static uint16_t numEntries = 4;

static void dummyFreeInt(void* item) { }

//
// Tests
//

int test_MtQueue_setUp(void** state)
{
    queue = MtQueue_NULL;

    return 0;
}

int test_MtQueue_tearDown(void** state)
{
    mtQueue_free(queue);

    return 0;
}

void test_MtQueue_push_pop_singleThread_succeeds(void** state)
{
    queue = mtQueue_init(numEntries, dummyFreeInt);
    assert_false(mtQueue_isNull(queue));

    mtQueue_push(queue, (void*) 1);
    mtQueue_push(queue, (void*) 2);

    int32_t item = (int32_t) (uint64_t) mtQueue_pop(queue);
    assert_int_equal(1, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    assert_int_equal(2, item);

    mtQueue_push(queue, (void*) 3);
    mtQueue_push(queue, (void*) 4);
    mtQueue_push(queue, (void*) 5);
    mtQueue_push(queue, (void*) 6);

    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    assert_int_equal(3, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    assert_int_equal(4, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    assert_int_equal(5, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    assert_int_equal(6, item);
}
