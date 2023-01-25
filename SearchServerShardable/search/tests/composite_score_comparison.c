#include <composite_score_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_CompositeScore_equal(const struct CompositeScore* op1, const struct CompositeScore* op2)
{
    assert_int_equal(op1->role, op2->role);
    assert_int_equal(op1->profile, op2->profile);
    assert_int_equal(op1->total, op2->total);
    assert_int_equal(op1->heuristicScore, op2->heuristicScore);
    assert_int_equal(op1->companyScore, op2->companyScore);
    assert_int_equal(op1->experienceScore, op2->experienceScore);
    assert_int_equal(op1->skillsScore, op2->skillsScore);
    assert_int_equal(op1->jobTitleScore, op2->jobTitleScore);
    assert_int_equal(op1->relevantExperience, op2->relevantExperience);
}
