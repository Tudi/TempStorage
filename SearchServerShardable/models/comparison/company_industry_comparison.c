#include <company_industry_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_CompanyIndustry_equal(const struct CompanyIndustry* op1,
    const struct CompanyIndustry* op2)
{
    assert_int_equal(op1->id, op2->id);
    assert_string_equal(op1->name, op2->name);
}
