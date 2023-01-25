#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <logger.h>

int test_CompositeScore_setUp(void** state);
int test_CompositeScore_tearDown(void** state);
void test_marshall_unmarshall_CompositeScore_succeeds(void** state);
void test_binary_ToFrom_CompositeScore_succeeds(void** state);

int test_SearchCriteria_setUp(void** state);
int test_SearchCriteria_tearDown(void** state);
void test_marshall_unmarshall_SearchCriteria_succeeds(void** state);
void test_binary_ToFrom_SearchCriteria_succeeds(void** state);

int test_SearchFilter_setUp(void** state);
int test_SearchFilter_tearDown(void** state);
void test_marshall_unmarshall_SearchFilter_succeeds(void** state);
void test_binary_ToFrom_SearchFilter_succeeds(void** state);

int test_SimilarityScore_setUp(void** state);
int test_SimilarityScore_tearDown(void** state);
void test_marshall_unmarshallBinary_SimilarityScore_succeeds(void** state);

int main(int argc, char* argv[])
{
    logger_setLogLevel(DEBUG_LOG_MSG);

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_CompositeScore_succeeds,
            test_CompositeScore_setUp, test_CompositeScore_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_SearchCriteria_succeeds,
            test_SearchCriteria_setUp, test_SearchCriteria_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_SearchFilter_succeeds,
            test_SearchFilter_setUp, test_SearchFilter_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshallBinary_SimilarityScore_succeeds,
            test_SimilarityScore_setUp, test_SimilarityScore_tearDown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
