#include <date_time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

//
// Data
//

static struct json_object* obj = NULL;

static const time_t invalid_dateTime = INT64_MIN;

static struct tm dateTime1_tm = { .tm_sec = 0, .tm_min = 0, .tm_hour = 0,
    .tm_mday = 1, .tm_mon = 0, .tm_year = -1899, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

static struct tm dateTimeBefore1900_tm = { .tm_sec = 31, .tm_min = 21, .tm_hour = 11,
    .tm_mday = 10, .tm_mon = 2, .tm_year = -100, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

static struct tm dateTime1900_tm = { .tm_sec = 0, .tm_min = 0, .tm_hour = 0,
    .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

static struct tm dateTimeAfter1900_tm = { .tm_sec = 32, .tm_min = 22, .tm_hour = 12,
    .tm_mday = 20, .tm_mon = 0, .tm_year = 105, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

static struct tm dateTimeFirstDayOfYear_tm = { .tm_sec = 33, .tm_min = 23, .tm_hour = 0,
    .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 1 };

static struct tm dateTimeFirstDayOf2000_tm = { .tm_sec = 1, .tm_min = 1, .tm_hour = 1,
    .tm_mday = 1, .tm_mon = 0, .tm_year = 100, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

static const char* expected_dateTime1_string              = "\"0001-01-01T00:00:00Z\"";
static const char* expected_dateTimeBefore1900_string     = "\"1800-03-10T11:21:31Z\"";
static const char* expected_dateTime1900_string           = "\"1900-01-01T00:00:00Z\"";
static const char* expected_dateTimeAfter1900_string      = "\"2005-01-20T12:22:32Z\"";
static const char* expected_dateTimeFirstDayOfYear_string = "\"2020-01-01T00:23:33Z\"";
static const char* expected_dateTimeFirstDayOf2000_string = "\"2000-01-01T01:01:01Z\"";

//
// Functions
//

int test_DateTime_setUp(void** state)
{
    obj = NULL;
    return 0;
}

int test_DateTime_tearDown(void** state)
{
    json_object_put(obj);
    obj = NULL;
    return 0;
}

void test_marshall_unmarshall_DateTime_succeeds(void** state)
{
    const time_t expected_dateTime1              = mktime(&dateTime1_tm);
    const time_t expected_dateTimeBefore1900     = mktime(&dateTimeBefore1900_tm);
    const time_t expected_dateTime1900           = mktime(&dateTime1900_tm);
    const time_t expected_dateTimeAfter1900      = mktime(&dateTimeAfter1900_tm);
    const time_t expected_dateTimeFirstDayOfYear = mktime(&dateTimeFirstDayOfYear_tm);
    const time_t expected_dateTimeFirstDayOf2000 = mktime(&dateTimeFirstDayOf2000_tm);

    // expected_dateTime1

    obj = marshallTime(expected_dateTime1);
    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);
    assert_non_null(str);
    assert_string_equal(expected_dateTime1_string, str);

    time_t dateTime = invalid_dateTime;
    assert_true(unmarshallTime(&dateTime, obj));
    assert_int_equal(expected_dateTime1, dateTime);
    json_object_put(obj);
    obj = NULL;

    // expected_dateTimeBefore1900

    obj = marshallTime(expected_dateTimeBefore1900);
    assert_non_null(obj);

    str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);
    assert_non_null(str);
    assert_string_equal(expected_dateTimeBefore1900_string, str);

    dateTime = invalid_dateTime;
    assert_true(unmarshallTime(&dateTime, obj));
    assert_int_equal(expected_dateTimeBefore1900, dateTime);
    json_object_put(obj);
    obj = NULL;

    // expected_dateTime1900

    obj = marshallTime(expected_dateTime1900);
    assert_non_null(obj);

    str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);
    assert_non_null(str);
    assert_string_equal(expected_dateTime1900_string, str);

    dateTime = invalid_dateTime;
    assert_true(unmarshallTime(&dateTime, obj));
    assert_int_equal(expected_dateTime1900, dateTime);
    json_object_put(obj);
    obj = NULL;

    // expected_dateTimeAfter1900

    obj = marshallTime(expected_dateTimeAfter1900);
    assert_non_null(obj);

    str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);
    assert_non_null(str);
    assert_string_equal(expected_dateTimeAfter1900_string, str);

    dateTime = invalid_dateTime;
    assert_true(unmarshallTime(&dateTime, obj));
    assert_int_equal(expected_dateTimeAfter1900, dateTime);
    json_object_put(obj);
    obj = NULL;

    // expected_dateTimeFirstDayOfYear

    obj = marshallTime(expected_dateTimeFirstDayOfYear);
    assert_non_null(obj);

    str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);
    assert_non_null(str);
    assert_string_equal(expected_dateTimeFirstDayOfYear_string, str);

    dateTime = invalid_dateTime;
    assert_true(unmarshallTime(&dateTime, obj));
    assert_int_equal(expected_dateTimeFirstDayOfYear, dateTime);
    json_object_put(obj);
    obj = NULL;

    // expected_dateTimeFirstDayOf2000

    obj = marshallTime(expected_dateTimeFirstDayOf2000);
    assert_non_null(obj);

    str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);
    assert_non_null(str);
    assert_string_equal(expected_dateTimeFirstDayOf2000_string, str);

    dateTime = invalid_dateTime;
    assert_true(unmarshallTime(&dateTime, obj));
    assert_int_equal(expected_dateTimeFirstDayOf2000, dateTime);
    json_object_put(obj);
    obj = NULL;
}

void test_binary_ToFrom_DateTime_succeeds(void** state)
{
    const time_t expected_dateTime1              = mktime(&dateTime1_tm);
    const time_t expected_dateTimeBefore1900     = mktime(&dateTimeBefore1900_tm);
    const time_t expected_dateTime1900           = mktime(&dateTime1900_tm);
    const time_t expected_dateTimeAfter1900      = mktime(&dateTimeAfter1900_tm);
    const time_t expected_dateTimeFirstDayOfYear = mktime(&dateTimeFirstDayOfYear_tm);
    const time_t expected_dateTimeFirstDayOf2000 = mktime(&dateTimeFirstDayOf2000_tm);

    uint8_t outputBinArray[TIME_BINARY_SIZE * 2] = { 0x0 };

    // expected_dateTime1

    uint8_t* result = timeToBinary(outputBinArray, expected_dateTime1);
    assert_ptr_equal(result, outputBinArray + TIME_BINARY_SIZE);
    assert_memory_equal(&expected_dateTime1, outputBinArray, sizeof(expected_dateTime1));

    time_t dateTime = invalid_dateTime;
    const uint8_t* constResult = binaryToTime(outputBinArray, &dateTime);
    assert_ptr_equal(constResult, outputBinArray + TIME_BINARY_SIZE);
    assert_int_equal(expected_dateTime1, dateTime);

    // expected_dateTimeBefore1900

    result = timeToBinary(outputBinArray, expected_dateTimeBefore1900);
    assert_ptr_equal(result, outputBinArray + TIME_BINARY_SIZE);
    assert_memory_equal(&expected_dateTimeBefore1900, outputBinArray,
        sizeof(expected_dateTimeBefore1900));

    dateTime = invalid_dateTime;
    constResult = binaryToTime(outputBinArray, &dateTime);
    assert_ptr_equal(constResult, outputBinArray + TIME_BINARY_SIZE);
    assert_int_equal(expected_dateTimeBefore1900, dateTime);

    // expected_dateTime1900

    result = timeToBinary(outputBinArray, expected_dateTime1900);
    assert_ptr_equal(result, outputBinArray + TIME_BINARY_SIZE);
    assert_memory_equal(&expected_dateTime1900, outputBinArray, sizeof(expected_dateTime1900));

    dateTime = invalid_dateTime;
    constResult = binaryToTime(outputBinArray, &dateTime);
    assert_ptr_equal(constResult, outputBinArray + TIME_BINARY_SIZE);
    assert_int_equal(expected_dateTime1900, dateTime);

    // expected_dateTimeAfter1900

    result = timeToBinary(outputBinArray, expected_dateTimeAfter1900);
    assert_ptr_equal(result, outputBinArray + TIME_BINARY_SIZE);
    assert_memory_equal(&expected_dateTimeAfter1900, outputBinArray,
        sizeof(expected_dateTimeAfter1900));

    dateTime = invalid_dateTime;
    constResult = binaryToTime(outputBinArray, &dateTime);
    assert_ptr_equal(constResult, outputBinArray + TIME_BINARY_SIZE);
    assert_int_equal(expected_dateTimeAfter1900, dateTime);

    // expected_dateTimeFirstDayOfYear

    result = timeToBinary(outputBinArray, expected_dateTimeFirstDayOfYear);
    assert_ptr_equal(result, outputBinArray + TIME_BINARY_SIZE);
    assert_memory_equal(&expected_dateTimeFirstDayOfYear, outputBinArray,
        sizeof(expected_dateTimeFirstDayOfYear));

    dateTime = invalid_dateTime;
    constResult = binaryToTime(outputBinArray, &dateTime);
    assert_ptr_equal(constResult, outputBinArray + TIME_BINARY_SIZE);
    assert_int_equal(expected_dateTimeFirstDayOfYear, dateTime);

    // expected_dateTimeFirstDayOf2000

    result = timeToBinary(outputBinArray, expected_dateTimeFirstDayOf2000);
    assert_ptr_equal(result, outputBinArray + TIME_BINARY_SIZE);
    assert_memory_equal(&expected_dateTimeFirstDayOf2000, outputBinArray,
        sizeof(expected_dateTimeFirstDayOf2000));

    dateTime = invalid_dateTime;
    constResult = binaryToTime(outputBinArray, &dateTime);
    assert_ptr_equal(constResult, outputBinArray + TIME_BINARY_SIZE);
    assert_int_equal(expected_dateTimeFirstDayOf2000, dateTime);
}
