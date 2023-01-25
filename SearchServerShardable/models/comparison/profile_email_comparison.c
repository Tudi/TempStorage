#include <profile_email_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_ProfileEmail_equal(const struct ProfileEmail* op1, const struct ProfileEmail* op2)
{
    assert_string_equal(op1->uuid, op2->uuid);
    assert_string_equal(op1->email, op2->email);
    assert_int_equal(op1->toxic, op2->toxic);
    assert_string_equal(op1->domain, op2->domain);
    assert_string_equal(op1->gender, op2->gender);
    assert_int_equal(op1->manual, op2->manual);
    assert_string_equal(op1->status, op2->status);
    assert_int_equal(op1->bounced, op2->bounced);
    assert_int_equal(op1->primary, op2->primary);
    assert_string_equal(op1->firstName, op2->firstName);
    assert_string_equal(op1->lastName, op2->lastName);
    assert_int_equal(op1->mxFound, op2->mxFound);
    assert_int_equal(op1->personal, op2->personal);
    assert_string_equal(op1->mxRecord, op2->mxRecord);
    assert_int_equal(op1->disposable, op2->disposable);
    assert_int_equal(op1->freeEmail, op2->freeEmail);
    assert_string_equal(op1->subStatus, op2->subStatus);
    assert_string_equal(op1->smtpProvider, op2->smtpProvider);
    assert_int_equal(op1->domainAgeDays, op2->domainAgeDays);
}
