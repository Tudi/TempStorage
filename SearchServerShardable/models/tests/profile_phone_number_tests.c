#include <profile_phone_number.h>
#include <profile_phone_number_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct ProfilePhoneNumber expectedPhoneNumber, phoneNumber;
static struct json_object* obj = NULL;

int test_ProfilePhoneNumber_setUp(void** state)
{
    initProfilePhoneNumber(&expectedPhoneNumber);
    initProfilePhoneNumber(&phoneNumber);
    obj = NULL;

    return 0;
}

int test_ProfilePhoneNumber_tearDown(void** state)
{
    json_object_put(obj);
    freeProfilePhoneNumber(&phoneNumber);
    freeProfilePhoneNumber(&expectedPhoneNumber);

    return 0;
}

static void initTestData_ProfilePhoneNumber(struct ProfilePhoneNumber* phoneNumber)
{
    phoneNumber->uuid                = strdup("1234567890");
    phoneNumber->countryCode         = strdup("1");
    phoneNumber->countryName         = strdup("USA");
    phoneNumber->callingCode         = strdup("555");
    phoneNumber->internationalNumber = strdup("011");
    phoneNumber->localNumber         = strdup("234-5678");
}

void test_marshall_unmarshall_ProfilePhoneNumber_succeeds(void** state)
{
    const char* expectedStr = "{ \"uuid\": \"1234567890\", \"country_code\": \"1\","
        " \"country_name\": \"USA\", \"calling_code\": \"555\","
	" \"international_number\": \"011\", \"local_number\": \"234-5678\" }";

    initTestData_ProfilePhoneNumber(&expectedPhoneNumber);

    obj = marshallProfilePhoneNumber(&expectedPhoneNumber);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfilePhoneNumber(&phoneNumber, obj));
    assert_ProfilePhoneNumber_equal(&expectedPhoneNumber, &phoneNumber);
}

#define TEST_BINARY_PROFILEPHONENUMBER_SIZE (13 + 4 + 6 + 6 + 6 + 11)

void test_binary_ToFrom_ProfilePhoneNumber_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_PROFILEPHONENUMBER_SIZE]
        = { 0xb, 0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0',
            0x2, 0x0, '1', '\0',
            0x4, 0x0, 'U', 'S', 'A', '\0',
            0x4, 0x0, '5', '5', '5', '\0',
            0x4, 0x0, '0', '1', '1', '\0',
            0x9, 0x0, '2', '3', '4', '-', '5', '6', '7', '8', '\0' }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_PROFILEPHONENUMBER_SIZE * 2] = { 0x0 };

    initTestData_ProfilePhoneNumber(&expectedPhoneNumber);

    assert_int_equal(TEST_BINARY_PROFILEPHONENUMBER_SIZE,
        profilePhoneNumberBinarySize(&expectedPhoneNumber));

    uint8_t* result = profilePhoneNumberToBinary(outputBinArray, &expectedPhoneNumber);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_PROFILEPHONENUMBER_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfilePhoneNumber(outputBinArray, &phoneNumber);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_PROFILEPHONENUMBER_SIZE);
    assert_ProfilePhoneNumber_equal(&expectedPhoneNumber, &phoneNumber);
}
