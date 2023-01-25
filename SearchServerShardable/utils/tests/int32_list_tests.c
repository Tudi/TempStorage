#include <list.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

static List_t ls = List_NULL;

static int compareInt32Keys(void* key1, void* key2)
{
    int32_t k1 = (int32_t) (int64_t) key1;
    int32_t k2 = (int32_t) (int64_t) key2;
    return k1 - k2;
}

static void freeInt32Item(void* item) { }

static void* keyFromInt32Item(void* item)
{
    return item;
}

int test_Int32List_setUp(void** state)
{
    return 0;
}

int test_Int32List_tearDown(void** state)
{
    list_free(ls);

    return 0;
}

void test_Int32List_functions_succeeds(void** state)
{
    ls = list_init(compareInt32Keys, freeInt32Item, keyFromInt32Item);
    assert_false(list_isNull(ls));

    assert_true(list_addItem(ls, (void*) 2));
    assert_true(list_addItem(ls, (void*) 4));
    assert_true(list_addItem(ls, (void*) 4));
    assert_true(list_addItem(ls, (void*) -2));
    assert_true(list_addItem(ls, (void*) 8));
    assert_true(list_addItem(ls, (void*) 10));
    assert_true(list_addItem(ls, (void*) 6));

    ListIterator_t iter = list_search(ls, (void*) -2);
    assert_false(list_end(&iter));
    assert_int_equal(-2, (int32_t) (int64_t) list_iteratorValue(&iter));

    iter = list_search(ls, (void*) 8);
    assert_false(list_end(&iter));
    assert_int_equal(8, (int32_t) (int64_t) list_iteratorValue(&iter));

    iter = list_search(ls, (void*) -3);
    assert_true(list_end(&iter));
    iter = list_search(ls, (void*) 1);
    assert_true(list_end(&iter));
    iter = list_search(ls, (void*) 7);
    assert_true(list_end(&iter));

    iter = list_search(ls, (void*) -2);
    assert_false(list_end(&iter));
    list_deleteItem(&iter);
    assert_int_equal(2, (int32_t) (int64_t) list_iteratorValue(&iter));

    iter = list_search(ls, (void*) 4);
    assert_false(list_end(&iter));
    list_deleteItem(&iter);
    assert_int_equal(4, (int32_t) (int64_t) list_iteratorValue(&iter));

    iter = list_search(ls, (void*) 4);
    assert_false(list_end(&iter));
    list_deleteItem(&iter);
    assert_int_equal(6, (int32_t) (int64_t) list_iteratorValue(&iter));

    iter = list_search(ls, (void*) 4);
    assert_true(list_end(&iter));

    iter = list_search(ls, (void*) 10);
    assert_false(list_end(&iter));
    list_deleteItem(&iter);
    assert_true(list_end(&iter));

    iter = list_search(ls, (void*) 10);
    assert_true(list_end(&iter));

    iter = list_begin(ls);
    assert_false(list_end(&iter));
    assert_int_equal(2, (int32_t) (int64_t) list_iteratorValue(&iter));

    list_iteratorNext(&iter);
    assert_int_equal(6, (int32_t) (int64_t) list_iteratorPop(&iter));
    assert_false(list_end(&iter));

    assert_int_equal(8, (int32_t) (int64_t) list_iteratorValue(&iter));
    list_iteratorNext(&iter);
    assert_true(list_end(&iter));
}
