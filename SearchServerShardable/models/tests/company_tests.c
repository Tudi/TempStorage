#include <company.h>
#include <company_comparison.h>
#include <company_test_data.h>
#include <daos_definitions.h>
#include <company_definitions.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct Company expectedCompany1, expectedCompany2, company1, company2;
static struct json_object* obj1 = NULL;
static struct json_object* obj2 = NULL;

int test_Company_setUp(void** state)
{
    initCompany(&expectedCompany1);
    initCompany(&company1);
    obj1 = NULL;

    initCompany(&expectedCompany2);
    initCompany(&company2);
    obj2 = NULL;

    return 0;
}

int test_Company_tearDown(void** state)
{
    json_object_put(obj1);
    freeCompany(&company1);
    freeCompany(&expectedCompany1);

    json_object_put(obj2);
    freeCompany(&company2);
    freeCompany(&expectedCompany2);

    return 0;
}

void test_marshall_unmarshall_Company_succeeds(void** state)
{
    // Company1

    const char* expectedStr1 = COMPANY_1_JSON_STRING;

    initTestData_Company1(&expectedCompany1);

    obj1 = marshallCompany(&expectedCompany1);

    assert_non_null(obj1);

    const char* str1 = json_object_to_json_string_ext(obj1, JSON_C_TO_STRING_SPACED);

    assert_non_null(str1);
    assert_string_equal(expectedStr1, str1);

    assert_true(unmarshallCompany(&company1, obj1));
    assert_Company_equal(&expectedCompany1, &company1);

    // Company2

    const char* expectedStr2 = COMPANY_2_JSON_STRING;

    initTestData_Company2(&expectedCompany2);

    obj2 = marshallCompany(&expectedCompany2);

    assert_non_null(obj2);

    const char* str2 = json_object_to_json_string_ext(obj2, JSON_C_TO_STRING_SPACED);

    assert_non_null(str2);
    assert_string_equal(expectedStr2, str2);

    assert_true(unmarshallCompany(&company2, obj2));
    assert_Company_equal(&expectedCompany2, &company2);
}

void test_binary_ToFrom_Company_succeeds(void** state)
{
    // Company1

    uint8_t expectedBinArray1[COMPANY_1_BINARY_SIZE] = { COMPANY_1_BINARY };

    uint8_t outputBinArray1[COMPANY_1_BINARY_SIZE * 2] = { 0x0 };

    initTestData_Company1(&expectedCompany1);

    assert_int_equal(COMPANY_1_BINARY_SIZE, companyBinarySize(&expectedCompany1));

    uint8_t* result1 = companyToBinary(outputBinArray1, &expectedCompany1);
    assert_ptr_equal(result1, outputBinArray1 + COMPANY_1_BINARY_SIZE);
    assert_memory_equal(expectedBinArray1, outputBinArray1, sizeof(expectedBinArray1));

    const uint8_t* constResult1 = binaryToCompany(outputBinArray1, &company1, COMPANY_DAOS_VERSION);
    assert_ptr_equal(constResult1, outputBinArray1 + COMPANY_1_BINARY_SIZE);
    assert_Company_equal(&expectedCompany1, &company1);

    // Company2

    uint8_t expectedBinArray2[COMPANY_2_BINARY_SIZE] = { COMPANY_2_BINARY };

    uint8_t outputBinArray2[COMPANY_2_BINARY_SIZE * 2] = { 0x0 };

    initTestData_Company2(&expectedCompany2);

    assert_int_equal(COMPANY_2_BINARY_SIZE, companyBinarySize(&expectedCompany2));

    uint8_t* result2 = companyToBinary(outputBinArray2, &expectedCompany2);
    assert_ptr_equal(result2, outputBinArray2 + COMPANY_2_BINARY_SIZE);
    assert_memory_equal(expectedBinArray2, outputBinArray2, sizeof(expectedBinArray2));

    const uint8_t* constResult2 = binaryToCompany(outputBinArray2, &company2, COMPANY_DAOS_VERSION);
    assert_ptr_equal(constResult2, outputBinArray2 + COMPANY_2_BINARY_SIZE);
    assert_Company_equal(&expectedCompany2, &company2);
}
