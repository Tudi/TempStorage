#include <profile_phone_number_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_ProfilePhoneNumber_equal(const struct ProfilePhoneNumber* op1,
    const struct ProfilePhoneNumber* op2)
{
    assert_string_equal(op1->uuid, op2->uuid);
    assert_string_equal(op1->countryCode, op2->countryCode);
    assert_string_equal(op1->countryName, op2->countryName);
    assert_string_equal(op1->callingCode, op2->callingCode);
    assert_string_equal(op1->internationalNumber, op2->internationalNumber);
    assert_string_equal(op1->localNumber, op2->localNumber);
}
