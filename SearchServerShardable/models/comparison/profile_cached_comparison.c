#include <profile_cached_comparison.h>
#include <id_value_comparison.h>
#include <position_cached_comparison.h>
#include <vector_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_ProfileCached_equal(const struct ProfileCached* op1, const struct ProfileCached* op2)
{
    assert_int_equal(op1->id, op2->id);
    assert_string_equal(op1->fullName, op2->fullName);
    assert_int_equal(op1->localityId, op2->localityId);
    assert_int_equal(op1->countryId, op2->countryId);
    assert_int_equal(op1->stateId, op2->stateId);
    assert_string_equal(op1->skillsHeadlineSummaryPosDescription, op2->skillsHeadlineSummaryPosDescription);
    ASSERT_VECTOR_ADDR_EQUAL(assert_PositionCached_equal, op1->positions, op2->positions);
    assert_int_equal(op1->totalExperienceMonths, op2->totalExperienceMonths);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->lastMessaged, op2->lastMessaged);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->lastReplied, op2->lastReplied);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->lastPositiveReply,
        op2->lastPositiveReply);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_Int32Value_equal, op1->groups, op2->groups);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_Int32Value_equal, op1->projects, op2->projects);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_Int32Value_equal, op1->campaigns, op2->campaigns);
    ASSERT_VECTOR_VALUE_EQUAL(assert_int_equal, op1->actioned, op2->actioned);
}
