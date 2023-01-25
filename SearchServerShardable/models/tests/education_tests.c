#include <education.h>
#include <education_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct Education expectedEducation, education;
static struct json_object* obj = NULL;

static struct tm startDate = { .tm_sec = 31, .tm_min = 21, .tm_hour = 11,
    .tm_mday = 1, .tm_mon = 2, .tm_year = 103, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "01-Mar-2003 11:21:31"

static struct tm endDate = { .tm_sec = 33, .tm_min = 23, .tm_hour = 13,
    .tm_mday = 2, .tm_mon = 3, .tm_year = 104, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "02-Apr-2004 13:23:33"

int test_Education_setUp(void** state)
{
    initEducation(&expectedEducation);
    initEducation(&education);
    obj = NULL;

    return 0;
}

int test_Education_tearDown(void** state)
{
    json_object_put(obj);
    freeEducation(&education);
    freeEducation(&expectedEducation);

    return 0;
}

static void initTestData_Education(struct Education* education)
{
    education->uuid         = strdup("1234567890");
    education->name         = strdup("NAME_ABC");
    education->degree       = strdup("DEGREE_DEF");
    education->subject      = strdup("SUBJECT_GHI");
    education->universityId = 3456;
    education->startDate    = mktime(&startDate);
    education->endDate      = mktime(&endDate);
}

void test_marshall_unmarshall_Education_succeeds(void** state)
{
    const char* expectedStr = "{ \"uuid\": \"1234567890\", \"name\": \"NAME_ABC\","
        " \"degree\": \"DEGREE_DEF\", \"subject\": \"SUBJECT_GHI\", \"university_id\": 3456,"
        " \"start_date\": \"2003-03-01T11:21:31Z\", \"end_date\": \"2004-04-02T13:23:33Z\" }";

    initTestData_Education(&expectedEducation);

    obj = marshallEducation(&expectedEducation);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallEducation(&education, obj));
    assert_Education_equal(&expectedEducation, &education);
}

#define TEST_BINARY_EDUCATION_ARRAY_SIZE (13 + 11 + 13 + 14 + 8 + 8 + 8)

void test_binary_ToFrom_Education_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_EDUCATION_ARRAY_SIZE]
        = { 0x0b, 0x00, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0',
            0x09, 0x00, 'N', 'A', 'M', 'E', '_', 'A', 'B', 'C', '\0',
            0x0b, 0x00, 'D', 'E', 'G', 'R', 'E', 'E', '_', 'D', 'E', 'F', '\0',
            0x0c, 0x00, 'S', 'U', 'B', 'J', 'E', 'C', 'T', '_', 'G', 'H', 'I', '\0',
            0x80, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xbb, 0x97, 0x60, 0x3e, 0x00, 0x00, 0x00, 0x00,
            0x55, 0x69, 0x6d, 0x40, 0x00, 0x00, 0x00, 0x00 }; // Little endian

    uint8_t outputBinArray[TEST_BINARY_EDUCATION_ARRAY_SIZE * 2] = { 0x0 };

    initTestData_Education(&expectedEducation);

    assert_int_equal(TEST_BINARY_EDUCATION_ARRAY_SIZE, educationBinarySize(&expectedEducation));

    uint8_t* result = educationToBinary(outputBinArray, &expectedEducation);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_EDUCATION_ARRAY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToEducation(outputBinArray, &education);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_EDUCATION_ARRAY_SIZE);
    assert_Education_equal(&expectedEducation, &education);
}
