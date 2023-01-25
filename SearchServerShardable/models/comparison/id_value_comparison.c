#include <id_value_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_Id_Int32Value_equal(const struct Id_Int32Value* op1, const struct Id_Int32Value* op2)
{
    assert_int_equal(op1->id, op2->id);
    assert_int_equal(op1->value, op2->value);
}

void assert_Id_StringValue_equal(const struct Id_StringValue* op1, const struct Id_StringValue* op2)
{
    assert_int_equal(op1->id, op2->id);
    assert_string_equal(op1->value, op2->value);
}

void assert_Id_TimeValue_equal(const struct Id_TimeValue* op1, const struct Id_TimeValue* op2)
{
    assert_int_equal(op1->id, op2->id);
    assert_int_equal(op1->value, op2->value);
}
