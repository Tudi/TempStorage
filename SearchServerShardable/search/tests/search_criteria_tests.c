#include <search_criteria.h>
#include <search_criteria_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct SearchCriteria expectedSearchCriteria, searchCriteria;
static struct json_object* obj = NULL;

int test_SearchCriteria_setUp(void** state)
{
    initSearchCriteria(&expectedSearchCriteria);
    initSearchCriteria(&searchCriteria);
    obj = NULL;

    return 0;
}

int test_SearchCriteria_tearDown(void** state)
{
    json_object_put(obj);
    freeSearchCriteria(&searchCriteria);
    freeSearchCriteria(&expectedSearchCriteria);

    return 0;
}

static bool initTestData_SearchCriteria(struct SearchCriteria* searchCriteria)
{
    searchCriteria->role = 56;

    kv_resize(int32_t, searchCriteria->localities, 2);
    kv_push(int32_t, searchCriteria->localities, 71);
    kv_push(int32_t, searchCriteria->localities, 72);

    kv_resize(struct SearchFilter, searchCriteria->filters, 1);
    kv_push(struct SearchFilter, searchCriteria->filters, ((struct SearchFilter) {
        .name = strdup("NAME_ABC"), .modifier = strdup("LESS_THAN"),
        .textValue = strdup("TEXT_DEF"), .codeValue = strdup("CODE_GHI"), 
        .rangeLow = 12, .rangeHigh= 34 }));

    searchCriteria->organizationID = 17;

    addScoreToMap(&searchCriteria->similarityScores.companyScores, 1, 5000);
    addScoreToMap(&searchCriteria->similarityScores.industryScores, 2, 6000);

    return true;
}

void test_marshall_unmarshall_SearchCriteria_succeeds(void** state)
{
    const char* expectedStr = "{ \"role_code\": 56, \"localities\": [ 71, 72 ], "
        "\"filters\": [ { \"filter\": \"NAME_ABC\", \"modifier\": \"LESS_THAN\", "
        "\"text_value\": \"TEXT_DEF\", \"code_value\": \"CODE_GHI\", "
        "\"range_low\": 12, \"range_high\": 34 } ], "
        "\"similar_companies\": [ { \"id\": 1, \"value\": 5000 } ], "
        "\"similar_industries\": [ { \"id\": 2, \"value\": 6000 } ], "
        "\"organization_id\": 17 "
        "}";

    bool initResult = initTestData_SearchCriteria(&expectedSearchCriteria);
    assert_true(initResult);

    obj = marshallSearchCriteria(&expectedSearchCriteria);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallSearchCriteria(&searchCriteria, obj, NULL, 0));
    assert_SearchCriteria_equal(&expectedSearchCriteria, &searchCriteria);
}
