#include <composite_score.h>
#include <composite_score_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct CompositeScore expectedCompositeScore, compositeScore;
static struct json_object* obj = NULL;

int test_CompositeScore_setUp(void** state)
{
    initCompositeScore(&expectedCompositeScore);
    initCompositeScore(&compositeScore);
    obj = NULL;

    return 0;
}

int test_CompositeScore_tearDown(void** state)
{
    json_object_put(obj);
    freeCompositeScore(&compositeScore);
    freeCompositeScore(&expectedCompositeScore);

    return 0;
}

static void initTestData_CompositeScore(struct CompositeScore* compositeScore)
{
    compositeScore->role = 10;
    compositeScore->profile = 20;
    compositeScore->total = 30;
    compositeScore->heuristicScore = 40;
    compositeScore->companyScore = 50;
    compositeScore->experienceScore = 60;
    compositeScore->skillsScore = 70;
    compositeScore->jobTitleScore = 80;
    compositeScore->relevantExperience = 90;
}

void test_marshall_unmarshall_CompositeScore_succeeds(void** state)
{
    const char* expectedStr = "{ \"r\": 10, \"p\": 20, \"t\": 30, \"hs\": 40,"
        " \"cs\": 50, \"e\": 60, \"s\": 70, \"j\": 80, \"rel\": 90, \"filters\": [ ] }";

    initTestData_CompositeScore(&expectedCompositeScore);

    obj = marshallCompositeScore(&expectedCompositeScore);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallCompositeScore(&compositeScore, obj));
    assert_CompositeScore_equal(&expectedCompositeScore, &compositeScore);
}
