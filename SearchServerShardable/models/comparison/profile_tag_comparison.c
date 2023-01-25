#include <profile_tag_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

void assert_ProfileTag_equal(const struct ProfileTag* op1, const struct ProfileTag* op2)
{
    assert_string_equal(op1->uuid, op2->uuid);
    assert_int_equal(op1->tagId, op2->tagId);
    assert_string_equal(op1->source, op2->source);
    assert_string_equal(op1->tagType, op2->tagType);
    assert_string_equal(op1->text, op2->text);
}
