#include <system_info_data.h>
#include <system_info_data_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct SystemInfoData expectedSysInfoData, sysInfoData;
static struct json_object* obj = NULL;

int test_SystemInfoData_setUp(void** state)
{
    initSystemInfoData(&expectedSysInfoData);
    initSystemInfoData(&sysInfoData);
    obj = NULL;

    return 0;
}

int test_SystemInfoData_tearDown(void** state)
{
    json_object_put(obj);
    freeSystemInfoData(&sysInfoData);
    freeSystemInfoData(&expectedSysInfoData);

    return 0;
}

static void initTestData_SystemInfoData(struct SystemInfoData* sysInfoData)
{
    sysInfoData->config.numConnections = 1;
    sysInfoData->persistentItems.profiles.minId = 2;
    sysInfoData->persistentItems.profiles.maxId = 3;
    sysInfoData->persistentItems.profiles.total = 4;
    sysInfoData->persistentItems.companies.minId = 5;
    sysInfoData->persistentItems.companies.maxId = 6;
    sysInfoData->persistentItems.companies.total = 7;
}

void test_marshall_unmarshall_SystemInfoData_succeeds(void** state)
{
    const char* expectedStr = "{ \"config\": { \"client_connections\": 1 }, "
        "\"persistent_items\": { \"profiles\": { \"min\": 2, \"max\": 3, \"total\": 4 }, "
        "\"companies\": { \"min\": 5, \"max\": 6, \"total\": 7 } } }";

    initTestData_SystemInfoData(&expectedSysInfoData);

    obj = marshallSystemInfoData(&expectedSysInfoData);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallSystemInfoData(&sysInfoData, obj));
    assert_SystemInfoData_equal(&expectedSysInfoData, &sysInfoData);
}
