#include <id_value.h>
#include <id_value_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

//
// struct Id_Int32Value tests
//

static struct Id_Int32Value expectedIdInt32Val, idInt32Val;
static struct json_object* idInt32ValObj = NULL;

int test_Id_Int32Value_setUp(void** state)
{
    initId_Int32Value(&expectedIdInt32Val);
    initId_Int32Value(&idInt32Val);
    idInt32ValObj = NULL;

    return 0;
}

int test_Id_Int32Value_tearDown(void** state)
{
    json_object_put(idInt32ValObj);
    freeId_Int32Value(&idInt32Val);
    freeId_Int32Value(&expectedIdInt32Val);

    return 0;
}

void test_marshall_unmarshall_Id_Int32Value_succeeds(void** state)
{
    const char* expectedStr = "{ \"id\": 1234, \"value\": 5678 }";

    expectedIdInt32Val.id    = 1234;
    expectedIdInt32Val.value = 5678;

    idInt32ValObj = marshallId_Int32Value(&expectedIdInt32Val);

    assert_non_null(idInt32ValObj);

    const char* str = json_object_to_json_string_ext(idInt32ValObj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallId_Int32Value(&idInt32Val, idInt32ValObj));
    assert_Id_Int32Value_equal(&expectedIdInt32Val, &idInt32Val);
}

#define TEST_BINARY_IDINT32VALUE_SIZE (sizeof(int32_t) * 2)

void test_binary_ToFrom_Id_Int32Value_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_IDINT32VALUE_SIZE]
        = { 0x78, 0x56, 0x34, 0x12, 0xf0, 0xde, 0xbc, 0x9a }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_IDINT32VALUE_SIZE * 2] = { 0x0 };

    expectedIdInt32Val.id    = 0x12345678;
    expectedIdInt32Val.value = 0x9abcdef0;

    assert_int_equal(TEST_BINARY_IDINT32VALUE_SIZE, ID_INT32VALUE_BINARY_SIZE);

    uint8_t* result = id_Int32ValueToBinary(outputBinArray, &expectedIdInt32Val);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_IDINT32VALUE_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToId_Int32Value(outputBinArray, &idInt32Val);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_IDINT32VALUE_SIZE);
    assert_Id_Int32Value_equal(&expectedIdInt32Val, &idInt32Val);
}

//
// struct Id_StringValue tests
//

static struct Id_StringValue expectedIdStrVal, idStrVal;
static struct json_object* idStrValObj = NULL;

int test_Id_StringValue_setUp(void** state)
{
    initId_StringValue(&expectedIdStrVal);
    initId_StringValue(&idStrVal);
    idStrValObj = NULL;

    return 0;
}

int test_Id_StringValue_tearDown(void** state)
{
    json_object_put(idStrValObj);
    freeId_StringValue(&idStrVal);
    freeId_StringValue(&expectedIdStrVal);

    return 0;
}

void test_marshall_unmarshall_Id_StringValue_succeeds(void** state)
{
    const char* expectedStr = "{ \"id\": 1234, \"value\": \"Some Text\" }";

    expectedIdStrVal.id    = 1234;
    expectedIdStrVal.value = strdup("Some Text");

    idStrValObj = marshallId_StringValue(&expectedIdStrVal);

    assert_non_null(idStrValObj);

    const char* str = json_object_to_json_string_ext(idStrValObj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallId_StringValue(&idStrVal, idStrValObj));
    assert_Id_StringValue_equal(&expectedIdStrVal, &idStrVal);
}

#define TEST_BINARY_IDSTRINGVALUE_SIZE (sizeof(int32_t) + 12 * sizeof(char))

void test_binary_ToFrom_Id_StringValue_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_IDSTRINGVALUE_SIZE] = { 0x78, 0x56, 0x34, 0x12,
        0xa, 0x0, 'S', 'o', 'm', 'e', ' ', 'T', 'e', 'x', 't', '\0' }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_IDSTRINGVALUE_SIZE * 2] = { 0x0 };

    expectedIdStrVal.id    = 0x12345678;
    expectedIdStrVal.value = strdup("Some Text");

    assert_int_equal(TEST_BINARY_IDSTRINGVALUE_SIZE, id_StringValueBinarySize(&expectedIdStrVal));

    uint8_t* result = id_StringValueToBinary(outputBinArray, &expectedIdStrVal);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_IDSTRINGVALUE_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToId_StringValue(outputBinArray, &idStrVal);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_IDSTRINGVALUE_SIZE);
    assert_Id_StringValue_equal(&expectedIdStrVal, &idStrVal);
}

//
// struct Id_TimeValue tests
//

static struct Id_TimeValue expectedIdTimeVal, idTimeVal;
static struct json_object* idTimeValObj = NULL;

static struct tm valueTm = { .tm_sec = 30, .tm_min = 20, .tm_hour = 10,
    .tm_mday = 5, .tm_mon = 9, .tm_year = 100, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "05-Oct-2000 10:20:30"

int test_Id_TimeValue_setUp(void** state)
{
    initId_TimeValue(&expectedIdTimeVal);
    initId_TimeValue(&idTimeVal);
    idTimeValObj = NULL;

    return 0;
}

int test_Id_TimeValue_tearDown(void** state)
{
    json_object_put(idTimeValObj);
    freeId_TimeValue(&idTimeVal);
    freeId_TimeValue(&expectedIdTimeVal);

    return 0;
}

void test_marshall_unmarshall_Id_TimeValue_succeeds(void** state)
{
    const char* expectedStr = "{ \"id\": 1234, \"value\": \"2000-10-05T10:20:30Z\" }";

    expectedIdTimeVal.id    = 1234;
    expectedIdTimeVal.value = mktime(&valueTm);

    idTimeValObj = marshallId_TimeValue(&expectedIdTimeVal);

    assert_non_null(idTimeValObj);

    const char* str = json_object_to_json_string_ext(idTimeValObj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallId_TimeValue(&idTimeVal, idTimeValObj));
    assert_Id_TimeValue_equal(&expectedIdTimeVal, &idTimeVal);
}

#define TEST_BINARY_IDTIMEVALUE_SIZE (sizeof(int32_t) + sizeof(time_t))

void test_binary_ToFrom_Id_TimeValue_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_IDTIMEVALUE_SIZE] = { 0x78, 0x56, 0x34, 0x12,
        0xee, 0x55, 0xdc, 0x39, 0x00, 0x00, 0x00, 0x00 }; // Little endian

    uint8_t outputBinArray[TEST_BINARY_IDTIMEVALUE_SIZE * 2] = { 0x0 };

    expectedIdTimeVal.id    = 0x12345678;
    expectedIdTimeVal.value = mktime(&valueTm);

    assert_int_equal(TEST_BINARY_IDTIMEVALUE_SIZE, ID_TIMEVALUE_BINARY_SIZE);

    uint8_t* result = id_TimeValueToBinary(outputBinArray, &expectedIdTimeVal);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_IDTIMEVALUE_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToId_TimeValue(outputBinArray, &idTimeVal);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_IDTIMEVALUE_SIZE);
    assert_Id_TimeValue_equal(&expectedIdTimeVal, &idTimeVal);
}
