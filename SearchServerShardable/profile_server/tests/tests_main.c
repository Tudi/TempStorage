#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

int test_SystemInfoData_setUp(void** state);
int test_SystemInfoData_tearDown(void** state);
void test_marshall_unmarshall_SystemInfoData_succeeds(void** state);

int main(int argc, char* argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_SystemInfoData_succeeds,
            test_SystemInfoData_setUp, test_SystemInfoData_tearDown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
