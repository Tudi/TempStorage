#include <binary_heap.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

static BinaryHeap_t heap = BinaryHeap_NULL;

#define ASSERT_ADD_INT32_TO_BINARY_HEAP(VALUE, HEAP) \
    do { \
        int32_t* int32Ptr = newInt32(VALUE); \
        if(binaryHeap_addItem(heap, int32Ptr) == false) \
        { \
            free(int32Ptr); \
            fail_msg("addItemToBinaryHeap(%d) failed.", *int32Ptr); \
        } \
    } while(0)

#define ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(VALUE, HEAP) \
    do { \
        int32_t* int32Ptr = binaryHeap_getFront(HEAP); \
        assert_non_null(int32Ptr); \
        assert_int_equal(VALUE, *int32Ptr); \
    } while(0)

static int compareInt32Ascending(void* arg1, void* arg2)
{
    int32_t* i1 = (int32_t*) arg1;
    int32_t* i2 = (int32_t*) arg2;

    return *i1 - *i2;
}

static int compareInt32Descending(void* arg1, void* arg2)
{
    int32_t* i1 = (int32_t*) arg1;
    int32_t* i2 = (int32_t*) arg2;

    return *i2 - *i1;
}

static void freeInt32Item(void* item)
{
    free(item);
}

int test_Int32BinaryHeap_setUp(void** state)
{
    return 0;
}

void* newInt32(int32_t value)
{
    int32_t* int32Ptr = malloc(sizeof(int32_t));
    *int32Ptr = value;

    return (void*) int32Ptr;
}

int test_Int32BinaryHeap_tearDown(void** state)
{
    binaryHeap_free(heap);

    return 0;
}

void test_Int32AscendingBinaryHeap_functions_succeeds(void** state)
{
    uint32_t capacity = 7;

    heap = binaryHeap_init(compareInt32Ascending, freeInt32Item, capacity);
    assert_false(binaryHeap_isNull(heap));

    assert_int_equal((int) capacity, (int) binaryHeap_capacity(heap));

    ASSERT_ADD_INT32_TO_BINARY_HEAP(7, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(5, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(3, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(1, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(6, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(4, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(2, heap);

    int32_t* int32Ptr = newInt32(8);
    assert_false(binaryHeap_addItem(heap, int32Ptr));
    free(int32Ptr);

    assert_int_equal(7, (int) binaryHeap_size(heap));

    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(1, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(2, heap);
    assert_true(binaryHeap_deleteFront(heap));

    assert_int_equal(5, (int) binaryHeap_size(heap));

    ASSERT_ADD_INT32_TO_BINARY_HEAP(9, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(10, heap);

    assert_int_equal(7, (int) binaryHeap_size(heap));

    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(3, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(4, heap);
    assert_true(binaryHeap_deleteFront(heap));

    assert_int_equal(5, (int) binaryHeap_size(heap));

    binaryHeap_pushItemAndDeleteFront(heap, newInt32(11));
    binaryHeap_pushItemAndDeleteFront(heap, newInt32(-1));

    assert_int_equal(7, (int) binaryHeap_size(heap));

    binaryHeap_pushItemAndDeleteFront(heap, newInt32(-2));

    assert_int_equal(7, (int) binaryHeap_size(heap));

    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(-1, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(5, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(6, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(7, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(9, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(10, heap);
    assert_non_null(int32Ptr = binaryHeap_popFront(heap));
    free(int32Ptr);
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(11, heap);
    assert_non_null(int32Ptr = binaryHeap_popFront(heap));
    free(int32Ptr);

    assert_null(binaryHeap_getFront(heap));
    assert_false(binaryHeap_deleteFront(heap));

    assert_int_equal(0, (int) binaryHeap_size(heap));

    // Adding items but not deleting them to test binaryHeap_free().

    ASSERT_ADD_INT32_TO_BINARY_HEAP(12, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(13, heap);

    assert_int_equal(2, (int) binaryHeap_size(heap));
}

void test_Int32DescendingBinaryHeap_functions_succeeds(void** state)
{
    uint32_t capacity = 5;

    heap = binaryHeap_init(compareInt32Descending, freeInt32Item, capacity);
    assert_false(binaryHeap_isNull(heap));

    assert_int_equal((int) capacity, (int) binaryHeap_capacity(heap));

    ASSERT_ADD_INT32_TO_BINARY_HEAP(5, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(3, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(1, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(4, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(2, heap);

    int32_t* int32Ptr = newInt32(6);
    assert_false(binaryHeap_addItem(heap, int32Ptr));
    free(int32Ptr);

    assert_int_equal(5, (int) binaryHeap_size(heap));

    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(5, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(4, heap);
    assert_true(binaryHeap_deleteFront(heap));

    assert_int_equal(3, (int) binaryHeap_size(heap));

    ASSERT_ADD_INT32_TO_BINARY_HEAP(7, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(8, heap);

    assert_int_equal(5, (int) binaryHeap_size(heap));

    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(8, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(7, heap);
    assert_true(binaryHeap_deleteFront(heap));

    assert_int_equal(3, (int) binaryHeap_size(heap));

    binaryHeap_pushItemAndDeleteFront(heap, newInt32(9));
    binaryHeap_pushItemAndDeleteFront(heap, newInt32(-1));

    assert_int_equal(5, (int) binaryHeap_size(heap));

    binaryHeap_pushItemAndDeleteFront(heap, newInt32(10));

    assert_int_equal(5, (int) binaryHeap_size(heap));

    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(9, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(3, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(2, heap);
    assert_true(binaryHeap_deleteFront(heap));
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(1, heap);
    assert_non_null(int32Ptr = binaryHeap_popFront(heap));
    free(int32Ptr);
    ASSERT_BINARY_HEAP_FRONT_EQUAL_TO_INT32(-1, heap);
    assert_non_null(int32Ptr = binaryHeap_popFront(heap));
    free(int32Ptr);

    assert_null(binaryHeap_getFront(heap));
    assert_false(binaryHeap_deleteFront(heap));

    assert_int_equal(0, (int) binaryHeap_size(heap));

    // Adding items but not deleting them to test binaryHeap_free().

    ASSERT_ADD_INT32_TO_BINARY_HEAP(10, heap);
    ASSERT_ADD_INT32_TO_BINARY_HEAP(11, heap);

    assert_int_equal(2, (int) binaryHeap_size(heap));
}
