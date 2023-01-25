#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>

int test_MtQueue_setUp(void** state);
int test_MtQueue_tearDown(void** state);
void test_MtQueue_push_pop_singleThread_succeeds(void** state);

int main(int argc, char* argv[])
{
	test_MtQueue_setUp(NULL);
	test_MtQueue_push_pop_singleThread_succeeds(NULL);
	test_MtQueue_tearDown(NULL);

    return 0;
}
