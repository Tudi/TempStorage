#include <company_cached_comparison.h>
#include <vector_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_CompanyCached_equal(const struct CompanyCached* op1, const struct CompanyCached* op2)
{
    assert_int_equal(op1->id, op2->id);
    assert_int_equal(op1->parentId, op2->parentId);

    ASSERT_VECTOR_VALUE_EQUAL(assert_int_equal, op1->parentIndustryIds, op2->parentIndustryIds);

    assert_string_equal(op1->stage, op2->stage);
    assert_int_equal(op1->numEmployees, op2->numEmployees);
}
