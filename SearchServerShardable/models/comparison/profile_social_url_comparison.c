#include <profile_social_url_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_ProfileSocialUrl_equal(const struct ProfileSocialUrl* op1,
    const struct ProfileSocialUrl* op2)
{
    assert_string_equal(op1->uuid, op2->uuid);
    assert_string_equal(op1->source, op2->source);
    assert_string_equal(op1->url, op2->url);
    assert_string_equal(op1->username, op2->username);
}
