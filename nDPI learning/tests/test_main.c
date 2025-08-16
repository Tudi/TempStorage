#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void run_bufferpool_tests();
void run_structpool_tests();

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(run_bufferpool_tests),
        cmocka_unit_test(run_structpool_tests),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
