#include <search_criteria_comparison.h>
#include <search_filter_comparison.h>
#include <vector_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_SearchCriteria_equal(const struct SearchCriteria* op1, const struct SearchCriteria* op2)
{
    assert_int_equal(op1->role, op2->role);
    ASSERT_VECTOR_VALUE_EQUAL(assert_int_equal, op1->localities, op2->localities);
    ASSERT_VECTOR_ADDR_EQUAL(assert_SearchFilter_equal, op1->filters, op2->filters);
    assert_int_equal(op1->organizationID, op2->organizationID);
}
