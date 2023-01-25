#include <binary_utils.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <stdlib.h>

//
// bool
//

#define TEST_BOOLEAN_ARRAY_SIZE 2
#define TEST_BINARY_BOOLEAN_ARRAY_SIZE (TEST_BOOLEAN_ARRAY_SIZE * sizeof(bool))

void test_binary_ToFrom_Boolean_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_BOOLEAN_ARRAY_SIZE] = { 0x1, 0x0 };
    uint8_t outputBinArray[TEST_BINARY_BOOLEAN_ARRAY_SIZE * 2] = { 0xff, 0xff };

    bool expectedBool[TEST_BOOLEAN_ARRAY_SIZE] = { true, false };

    uint8_t* result = booleanToBinary(outputBinArray, expectedBool[0]);
    assert_ptr_equal(result, outputBinArray + 1);
    result = booleanToBinary(outputBinArray + 1, expectedBool[1]);
    assert_ptr_equal(result, outputBinArray + 2);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    bool resultBool[TEST_BOOLEAN_ARRAY_SIZE] = { false, true };

    const uint8_t* constResult = binaryToBoolean(outputBinArray, &resultBool[0]);
    assert_ptr_equal(constResult, outputBinArray + 1);
    constResult = binaryToBoolean(outputBinArray + 1, &resultBool[1]);
    assert_ptr_equal(constResult, outputBinArray + 2);
    assert_int_equal(expectedBool[0], resultBool[0]);
    assert_int_equal(expectedBool[1], resultBool[1]);
}

//
// int32_t
//

#define TEST_INT32_ARRAY_SIZE 3
#define TEST_BINARY_INT32_ARRAY_SIZE (TEST_INT32_ARRAY_SIZE * sizeof(int32_t))

void test_binary_ToFrom_Int32_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_INT32_ARRAY_SIZE] = { 0x0, 0x0, 0x0, 0x0,
        0x78, 0x56, 0x34, 0x12, 0xff, 0xff, 0xff, 0xff }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_INT32_ARRAY_SIZE * 2] = { 0xaa, 0xaa, 0xaa, 0xaa };

    int32_t expectedInt32[TEST_INT32_ARRAY_SIZE] = { 0x0, 0x12345678, 0xffffffff };

    uint8_t* offset = outputBinArray;
    uint8_t* result = int32ToBinary(offset, expectedInt32[0]);
    offset += sizeof(int32_t);
    assert_ptr_equal(result, offset);
    result = int32ToBinary(offset, expectedInt32[1]);
    offset += sizeof(int32_t);
    assert_ptr_equal(result, offset);
    result = int32ToBinary(offset, expectedInt32[2]);
    offset += sizeof(int32_t);
    assert_ptr_equal(result, offset);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    int32_t resultInt32[TEST_INT32_ARRAY_SIZE] = { 0xaaaaaaaa };

    const uint8_t* constOffset = outputBinArray;
    const uint8_t* constResult = binaryToInt32(constOffset, &resultInt32[0]);
    constOffset += sizeof(int32_t);
    assert_ptr_equal(constResult, constOffset);
    constResult = binaryToInt32(constOffset, &resultInt32[1]);
    constOffset += sizeof(int32_t);
    assert_ptr_equal(constResult, constOffset);
    constResult = binaryToInt32(constOffset, &resultInt32[2]);
    constOffset += sizeof(int32_t);
    assert_ptr_equal(constResult, constOffset);
    assert_int_equal(expectedInt32[0], resultInt32[0]);
    assert_int_equal(expectedInt32[1], resultInt32[1]);
    assert_int_equal(expectedInt32[2], resultInt32[2]);
}

//
// int64_t
//

#define TEST_INT64_ARRAY_SIZE 3
#define TEST_BINARY_INT64_ARRAY_SIZE (TEST_INT64_ARRAY_SIZE * sizeof(int64_t))

void test_binary_ToFrom_Int64_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_INT64_ARRAY_SIZE] = {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_INT64_ARRAY_SIZE * 2] = {
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa };

    int64_t expectedInt64[TEST_INT64_ARRAY_SIZE] = { 0x0, 0x123456789abcdef0, 0xffffffffffffffff };

    uint8_t* offset = outputBinArray;
    uint8_t* result = int64ToBinary(offset, expectedInt64[0]);
    offset += sizeof(int64_t);
    assert_ptr_equal(result, offset);
    result = int64ToBinary(offset, expectedInt64[1]);
    offset += sizeof(int64_t);
    assert_ptr_equal(result, offset);
    result = int64ToBinary(offset, expectedInt64[2]);
    offset += sizeof(int64_t);
    assert_ptr_equal(result, offset);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    int64_t resultInt64[TEST_INT64_ARRAY_SIZE] = { 0xaaaaaaaaaaaaaaaa };

    const uint8_t* constOffset = outputBinArray;
    const uint8_t* constResult = binaryToInt64(constOffset, &resultInt64[0]);
    constOffset += sizeof(int64_t);
    assert_ptr_equal(constResult, constOffset);
    constResult = binaryToInt64(constOffset, &resultInt64[1]);
    constOffset += sizeof(int64_t);
    assert_ptr_equal(constResult, constOffset);
    constResult = binaryToInt64(constOffset, &resultInt64[2]);
    constOffset += sizeof(int64_t);
    assert_ptr_equal(constResult, constOffset);
    assert_int_equal(expectedInt64[0], resultInt64[0]);
    assert_int_equal(expectedInt64[1], resultInt64[1]);
    assert_int_equal(expectedInt64[2], resultInt64[2]);
}

//
// char*
//

#define TEST_STRING_ARRAY_SIZE 2
#define TEST_BINARY_STRING1_SIZE (3 * sizeof(char))
#define TEST_BINARY_STRING2_SIZE (18 * sizeof(char))
#define TEST_BINARY_STRING_ARRAY_SIZE (TEST_BINARY_STRING1_SIZE + TEST_BINARY_STRING2_SIZE) 

char* resultString[TEST_STRING_ARRAY_SIZE] = { NULL, NULL };

int test_binary_ToFrom_String_setUp(void** state)
{
    resultString[0] = NULL;
    resultString[1] = NULL;
    return 0;
}

int test_binary_ToFrom_String_tearDown(void** state)
{
    free(resultString[0]);
    free(resultString[1]);
    return 0;
}

void test_binary_ToFrom_String_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_STRING_ARRAY_SIZE] = { 0x1, 0x0, 0x0,
      0x10, 0x0, 'S', 't', 'r', 'i', 'n', 'g', ' ' , '/', ' ', 'b', 'i', 'n', 'a', 'r', 'y', '\0' };
    uint8_t outputBinArray[TEST_BINARY_STRING_ARRAY_SIZE * 2] = { 0x0 };

    const char* expectedString[TEST_STRING_ARRAY_SIZE] = { "", "String / binary" };

    uint8_t* offset = outputBinArray;
    uint8_t* result = stringToBinary(offset, expectedString[0]);
    offset += TEST_BINARY_STRING1_SIZE;
    assert_ptr_equal(result, offset);
    result = stringToBinary(offset, expectedString[1]);
    offset += TEST_BINARY_STRING2_SIZE;
    assert_ptr_equal(result, offset);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constOffset = outputBinArray;
    const uint8_t* constResult = binaryToString(constOffset, &resultString[0]);
    constOffset += TEST_BINARY_STRING1_SIZE;
    assert_ptr_equal(constResult, constOffset);
    constResult = binaryToString(constOffset, &resultString[1]);
    constOffset += TEST_BINARY_STRING2_SIZE;
    assert_ptr_equal(constResult, constOffset);
    assert_string_equal(expectedString[0], resultString[0]);
    assert_string_equal(expectedString[1], resultString[1]);
}
