#include <company_industry.h>
#include <company_industry_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct CompanyIndustry expectedCompanyIndustry, companyIndustry;
static struct json_object* obj = NULL;

int test_CompanyIndustry_setUp(void** state)
{
    initCompanyIndustry(&expectedCompanyIndustry);
    initCompanyIndustry(&companyIndustry);
    obj = NULL;

    return 0;
}

int test_CompanyIndustry_tearDown(void** state)
{
    json_object_put(obj);
    freeCompanyIndustry(&companyIndustry);
    freeCompanyIndustry(&expectedCompanyIndustry);

    return 0;
}

static void initTestData_CompanyIndustry(struct CompanyIndustry* companyIndustry)
{
    companyIndustry->id       = 1234;
    companyIndustry->name     = strdup("INDUSTRY_DEF");
}

void test_marshall_unmarshall_CompanyIndustry_succeeds(void** state)
{
    const char* expectedStr = "{ \"id\": 1234, \"industry\": \"INDUSTRY_DEF\" }";

    initTestData_CompanyIndustry(&expectedCompanyIndustry);

    obj = marshallCompanyIndustry(&expectedCompanyIndustry);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallCompanyIndustry(&companyIndustry, obj));
    assert_CompanyIndustry_equal(&expectedCompanyIndustry, &companyIndustry);
}

#define TEST_BINARY_COMPANY_INDUSTRY_ARRAY_SIZE (4 + 15)

void test_binary_ToFrom_CompanyIndustry_succeeds(void** state)
{
    // Little endian
    uint8_t expectedBinArray[TEST_BINARY_COMPANY_INDUSTRY_ARRAY_SIZE]
        = { 0xd2, 0x4, 0x0, 0x0,
            0xd, 0x0, 'I', 'N', 'D', 'U', 'S', 'T', 'R', 'Y', '_', 'D', 'E', 'F', '\0', };

    uint8_t outputBinArray[TEST_BINARY_COMPANY_INDUSTRY_ARRAY_SIZE * 2] = { 0x0 };

    initTestData_CompanyIndustry(&expectedCompanyIndustry);

    assert_int_equal(TEST_BINARY_COMPANY_INDUSTRY_ARRAY_SIZE,
        companyIndustryBinarySize(&expectedCompanyIndustry));

    uint8_t* result = companyIndustryToBinary(outputBinArray, &expectedCompanyIndustry);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_COMPANY_INDUSTRY_ARRAY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToCompanyIndustry(outputBinArray, &companyIndustry);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_COMPANY_INDUSTRY_ARRAY_SIZE);
    assert_CompanyIndustry_equal(&expectedCompanyIndustry, &companyIndustry);
}
