#include <position_cached_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_PositionCached_equal(const struct PositionCached* op1, const struct PositionCached* op2)
{
    assert_string_equal(op1->companyName, op2->companyName);
    assert_string_equal(op1->title, op2->title);
    assert_int_equal(op1->startDate, op2->startDate);
    assert_int_equal(op1->endDate, op2->endDate);
    assert_int_equal(op1->companyId, op2->companyId);
    assert_int_equal(op1->parentTitletId, op2->parentTitletId);
}
