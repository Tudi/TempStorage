#include <system_info_data_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_SystemInfoData_equal(const struct SystemInfoData* op1, const struct SystemInfoData* op2)
{
    assert_int_equal(op1->config.numConnections, op2->config.numConnections);
    assert_int_equal(op1->persistentItems.profiles.minId, op2->persistentItems.profiles.minId);
    assert_int_equal(op1->persistentItems.profiles.maxId, op2->persistentItems.profiles.maxId);
    assert_int_equal(op1->persistentItems.profiles.total, op2->persistentItems.profiles.total);
    assert_int_equal(op1->persistentItems.companies.minId, op2->persistentItems.companies.minId);
    assert_int_equal(op1->persistentItems.companies.maxId, op2->persistentItems.companies.maxId);
    assert_int_equal(op1->persistentItems.companies.total, op2->persistentItems.companies.total);
}
