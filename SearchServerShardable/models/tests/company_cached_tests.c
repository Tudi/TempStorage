#include <company.h>
#include <company_cached_comparison.h>
#include <company_test_data.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

static struct Company company1, company2;
static struct CompanyCached companyCached1, companyCached2,
    expectedCompanyCached1, expectedCompanyCached2;

int test_CompanyCached_setUp(void** state)
{
    initCompany(&company1);
    initCompany(&company2);

    initCompanyCached(&companyCached1);
    initCompanyCached(&companyCached2);

    initCompanyCached(&expectedCompanyCached1);
    initCompanyCached(&expectedCompanyCached2);

    return 0;
}

int test_CompanyCached_tearDown(void** state)
{
    freeCompanyCached(&expectedCompanyCached2);
    freeCompanyCached(&expectedCompanyCached1);

    freeCompanyCached(&companyCached2);
    freeCompanyCached(&companyCached1);

    freeCompany(&company2);
    freeCompany(&company1);

    return 0;
}

void test_companyToCompanyCached_succeeds(void** state)
{
    initTestData_Company1(&company1);
    initTestData_Company2(&company2);

    initTestData_CompanyCached1(&expectedCompanyCached1);
    initTestData_CompanyCached2(&expectedCompanyCached2);

    assert_true(companyToCompanyCached(&companyCached1, &company1));

    assert_non_null(companyCached1.stage);

    assert_CompanyCached_equal(&expectedCompanyCached1, &companyCached1);

    assert_true(companyToCompanyCached(&companyCached2, &company2));

    assert_non_null(companyCached2.stage);

    assert_CompanyCached_equal(&expectedCompanyCached2, &companyCached2);
}

void test_binaryToCompanyCached_succeeds(void** state)
{
    uint8_t binaryCompany1[COMPANY_1_BINARY_SIZE] = { COMPANY_1_BINARY };
    uint8_t binaryCompany2[COMPANY_2_BINARY_SIZE] = { COMPANY_2_BINARY };

    initTestData_CompanyCached1(&expectedCompanyCached1);
    initTestData_CompanyCached2(&expectedCompanyCached2);

    const uint8_t* result = binaryToCompanyCached(binaryCompany1, &companyCached1);
    assert_ptr_equal(binaryCompany1 + COMPANY_1_CACHED_PORTION_BINARY_SIZE, result);

    assert_CompanyCached_equal(&expectedCompanyCached1, &companyCached1);

    result = binaryToCompanyCached(binaryCompany2, &companyCached2);
    assert_ptr_equal(binaryCompany2 + COMPANY_2_CACHED_PORTION_BINARY_SIZE, result);

    assert_CompanyCached_equal(&expectedCompanyCached2, &companyCached2);
}
