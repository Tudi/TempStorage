#include <search_filter_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_SearchFilter_equal(const struct SearchFilter* op1, const struct SearchFilter* op2)
{
    assert_string_equal(op1->name, op2->name);
    assert_string_equal(op1->modifier, op2->modifier);
    assert_string_equal(op1->textValue, op2->textValue);
    assert_string_equal(op1->codeValue, op2->codeValue);
    assert_int_equal(op1->rangeLow, op2->rangeLow);
    assert_int_equal(op1->rangeHigh, op2->rangeHigh);
}
