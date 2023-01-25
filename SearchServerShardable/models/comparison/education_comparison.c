#include <education_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_Education_equal(const struct Education* op1, const struct Education* op2)
{
    assert_string_equal(op1->uuid, op2->uuid);
    assert_string_equal(op1->name, op2->name);
    assert_string_equal(op1->degree, op2->degree);
    assert_string_equal(op1->subject, op2->subject);
    assert_int_equal(op1->universityId, op2->universityId);
    assert_int_equal(op1->startDate, op2->startDate);
    assert_int_equal(op1->endDate, op2->endDate);
}
