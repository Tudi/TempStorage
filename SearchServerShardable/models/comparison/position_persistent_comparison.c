#include <position_persistent_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_PositionPersistent_equal(const struct PositionPersistent* op1,
    const struct PositionPersistent* op2)
{
    assert_string_equal(op1->uuid, op2->uuid);
    assert_string_equal(op1->title, op2->title);
    assert_string_equal(op1->titleCorrected, op2->titleCorrected);
    assert_string_equal(op1->source, op2->source);
    assert_string_equal(op1->locality, op2->locality);
    assert_int_equal(op1->companyId, op2->companyId);
    assert_int_equal(op1->parentCompanyId, op2->parentCompanyId);
    assert_int_equal(op1->startDate, op2->startDate);
    assert_int_equal(op1->endDate, op2->endDate);
    assert_string_equal(op1->description, op2->description);
    assert_string_equal(op1->seniority, op2->seniority);
    assert_int_equal(op1->modifiedInLoad, op2->modifiedInLoad);
    assert_int_equal(op1->titleId, op2->titleId);
    assert_int_equal(op1->parentTitleId, op2->parentTitleId);
}
