#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

int test_DateTime_setUp(void** state);
int test_DateTime_tearDown(void** state);
void test_marshall_unmarshall_DateTime_succeeds(void** state);
void test_binary_ToFrom_DateTime_succeeds(void** state);

int test_Int32BinaryHeap_setUp(void** state);
int test_Int32BinaryHeap_tearDown(void** state);
void test_Int32AscendingBinaryHeap_functions_succeeds(void** state);
void test_Int32DescendingBinaryHeap_functions_succeeds(void** state);

int test_Int32List_setUp(void** state);
int test_Int32List_tearDown(void** state);
void test_Int32List_functions_succeeds(void** state);

int test_MtQueue_setUp(void** state);
int test_MtQueue_tearDown(void** state);
void test_MtQueue_push_pop_singleThread_succeeds(void** state);

int test_BitField_setUp(void** state);
int test_BitField_tearDown(void** state);
void test_BitField_succeeds(void** state);

int test_boolean_parser_setUp(void** state);
int test_boolean_parser_tearDown(void** state);
void test_boolean_parser_succeeds(void** state);

int main(int argc, char* argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_DateTime_succeeds,
            test_DateTime_setUp, test_DateTime_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_DateTime_succeeds,
            test_DateTime_setUp, test_DateTime_tearDown),
        cmocka_unit_test_setup_teardown(test_Int32AscendingBinaryHeap_functions_succeeds,
            test_Int32BinaryHeap_setUp, test_Int32BinaryHeap_tearDown),
        cmocka_unit_test_setup_teardown(test_Int32DescendingBinaryHeap_functions_succeeds,
            test_Int32BinaryHeap_setUp, test_Int32BinaryHeap_tearDown),
        cmocka_unit_test_setup_teardown(test_Int32List_functions_succeeds,
            test_Int32List_setUp, test_Int32List_tearDown),
        cmocka_unit_test_setup_teardown(test_MtQueue_push_pop_singleThread_succeeds,
            test_MtQueue_setUp, test_MtQueue_tearDown),
        cmocka_unit_test_setup_teardown(test_BitField_succeeds,
            test_BitField_setUp, test_BitField_tearDown),
        cmocka_unit_test_setup_teardown(test_boolean_parser_succeeds,
            test_boolean_parser_setUp, test_boolean_parser_tearDown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
