#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

int test_company_daos_setUp(void** state);
int test_company_daos_tearDown(void** state);
void test_company_daos_functions_succeed(void** state);
void test_company_daos_load_fails(void** state);

int test_profile_daos_setUp(void** state);
int test_profile_daos_tearDown(void** state);
void test_profile_daos_functions_succeed(void** state);
void test_profile_daos_load_fails(void** state);

int test_daos_setUp(void** state);
int test_daos_tearDown(void** state);
void test_daos_save_get_ProfilePersistent_succeeds(void** state);

int test_daos_V7_setUp(void** state);
int test_daos_V7_tearDown(void** state);
void test_daos_V7_detect_convert_item_succeeds(void** state);

int main(int argc, char* argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_company_daos_functions_succeed,
            test_company_daos_setUp, test_company_daos_tearDown),
        cmocka_unit_test_setup_teardown(test_company_daos_load_fails,
            test_company_daos_setUp, test_company_daos_tearDown),
        cmocka_unit_test_setup_teardown(test_profile_daos_functions_succeed,
            test_profile_daos_setUp, test_profile_daos_tearDown),
        cmocka_unit_test_setup_teardown(test_profile_daos_load_fails,
            test_profile_daos_setUp, test_profile_daos_tearDown),
        cmocka_unit_test_setup_teardown(test_daos_V7_detect_convert_item_succeeds,
            test_daos_V7_setUp, test_daos_V7_tearDown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
