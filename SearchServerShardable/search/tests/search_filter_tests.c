#include <search_filter.h>
#include <search_filter_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct SearchFilter expectedSearchFilter, searchFilter;
static struct json_object* obj = NULL;

int test_SearchFilter_setUp(void** state)
{
    initSearchFilter(&expectedSearchFilter);
    initSearchFilter(&searchFilter);
    obj = NULL;

    return 0;
}

int test_SearchFilter_tearDown(void** state)
{
    json_object_put(obj);
    freeSearchFilter(&searchFilter);
    freeSearchFilter(&expectedSearchFilter);

    return 0;
}

static void initTestData_SearchFilter(struct SearchFilter* searchFilter)
{
    searchFilter->name = strdup("NAME_ABC");
    searchFilter->textValue = strdup("TEXT_DEF");
    searchFilter->codeValue = strdup("CODE_GHI");
    searchFilter->modifier = strdup("LESS_THAN");
    searchFilter->rangeLow = 12;
    searchFilter->rangeHigh= 34;
}

void test_marshall_unmarshall_SearchFilter_succeeds(void** state)
{
    const char* expectedStr = "{ \"filter\": \"NAME_ABC\", \"modifier\": \"LESS_THAN\", "
        "\"text_value\": \"TEXT_DEF\", "
        "\"code_value\": \"CODE_GHI\", \"range_low\": 12, \"range_high\": 34 }";

    initTestData_SearchFilter(&expectedSearchFilter);

    obj = marshallSearchFilter(&expectedSearchFilter);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallSearchFilter(&searchFilter, obj));
    assert_SearchFilter_equal(&expectedSearchFilter, &searchFilter);
}
