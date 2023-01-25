#include <profile_persistent.h>
#include <profile_persistent_comparison.h>
#include <profile_test_data.h>
#include <daos_definitions.h>
#include <profile_definitions.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct ProfilePersistent expectedProfile, profile;
static struct json_object* obj = NULL;

int test_ProfilePersistent_setUp(void** state)
{
    initProfilePersistent(&expectedProfile);
    initProfilePersistent(&profile);
    obj = NULL;

    return 0;
}

int test_ProfilePersistent_tearDown(void** state)
{
    json_object_put(obj);
    freeProfilePersistent(&profile);
    freeProfilePersistent(&expectedProfile);

    return 0;
}

void test_marshall_unmarshall_ProfilePersistent1_succeeds(void** state)
{
    const char* expectedStr = PROFILE_PERSISTENT_1_JSON_STRING;

    initTestData_ProfilePersistent1(&expectedProfile);

    obj = marshallProfilePersistent(&expectedProfile);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfilePersistent(&profile, obj));
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_marshall_unmarshall_ProfilePersistent2_succeeds(void** state)
{
    const char* expectedStr = PROFILE_PERSISTENT_2_JSON_STRING;

    initTestData_ProfilePersistent2(&expectedProfile);

    obj = marshallProfilePersistent(&expectedProfile);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfilePersistent(&profile, obj));
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_marshall_unmarshall_ProfilePersistent3_succeeds(void** state)
{
    const char* expectedStr = PROFILE_PERSISTENT_3_JSON_STRING;

    initTestData_ProfilePersistent3(&expectedProfile);

    obj = marshallProfilePersistent(&expectedProfile);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfilePersistent(&profile, obj));
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_marshall_unmarshall_ProfilePersistent4_succeeds(void** state)
{
    const char* expectedStr = PROFILE_PERSISTENT_4_JSON_STRING;

    initTestData_ProfilePersistent4(&expectedProfile);

    obj = marshallProfilePersistent(&expectedProfile);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfilePersistent(&profile, obj));
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_marshall_unmarshall_ProfilePersistent5_succeeds(void** state)
{
    const char* expectedStr = PROFILE_PERSISTENT_5_JSON_STRING;

    initTestData_ProfilePersistent5(&expectedProfile);

    obj = marshallProfilePersistent(&expectedProfile);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfilePersistent(&profile, obj));
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_binary_ToFrom_ProfilePersistent1_succeeds(void** state)
{
    uint8_t expectedBinArray[PROFILE_PERSISTENT_1_BINARY_SIZE] = { PROFILE_PERSISTENT_1_BINARY };
   
    uint8_t outputBinArray[PROFILE_PERSISTENT_1_BINARY_SIZE * 2] = { 0x0 };

    initTestData_ProfilePersistent1(&expectedProfile);

    assert_int_equal(PROFILE_PERSISTENT_1_BINARY_SIZE, profilePersistentBinarySize(&expectedProfile));

    uint8_t* result = profilePersistentToBinary(outputBinArray, &expectedProfile);
    assert_ptr_equal(result, outputBinArray + PROFILE_PERSISTENT_1_BINARY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfilePersistent(outputBinArray, &profile, PROFILE_DAOS_VERSION);
    assert_ptr_equal(constResult, outputBinArray + PROFILE_PERSISTENT_1_BINARY_SIZE);
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_binary_ToFrom_ProfilePersistent2_succeeds(void** state)
{
    uint8_t expectedBinArray[PROFILE_PERSISTENT_2_BINARY_SIZE] = { PROFILE_PERSISTENT_2_BINARY };
   
    uint8_t outputBinArray[PROFILE_PERSISTENT_2_BINARY_SIZE * 2] = { 0x0 };

    initTestData_ProfilePersistent2(&expectedProfile);

    assert_int_equal(PROFILE_PERSISTENT_2_BINARY_SIZE, profilePersistentBinarySize(&expectedProfile));

    uint8_t* result = profilePersistentToBinary(outputBinArray, &expectedProfile);
    assert_ptr_equal(result, outputBinArray + PROFILE_PERSISTENT_2_BINARY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfilePersistent(outputBinArray, &profile, PROFILE_DAOS_VERSION);
    assert_ptr_equal(constResult, outputBinArray + PROFILE_PERSISTENT_2_BINARY_SIZE);
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_binary_ToFrom_ProfilePersistent3_succeeds(void** state)
{
    uint8_t expectedBinArray[PROFILE_PERSISTENT_3_BINARY_SIZE] = { PROFILE_PERSISTENT_3_BINARY };
   
    uint8_t outputBinArray[PROFILE_PERSISTENT_3_BINARY_SIZE * 2] = { 0x0 };

    initTestData_ProfilePersistent3(&expectedProfile);

    assert_int_equal(PROFILE_PERSISTENT_3_BINARY_SIZE, profilePersistentBinarySize(&expectedProfile));

    uint8_t* result = profilePersistentToBinary(outputBinArray, &expectedProfile);
    assert_ptr_equal(result, outputBinArray + PROFILE_PERSISTENT_3_BINARY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfilePersistent(outputBinArray, &profile, PROFILE_DAOS_VERSION);
    assert_ptr_equal(constResult, outputBinArray + PROFILE_PERSISTENT_3_BINARY_SIZE);
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_binary_ToFrom_ProfilePersistent4_succeeds(void** state)
{
    uint8_t expectedBinArray[PROFILE_PERSISTENT_4_BINARY_SIZE] = { PROFILE_PERSISTENT_4_BINARY };
   
    uint8_t outputBinArray[PROFILE_PERSISTENT_4_BINARY_SIZE * 2] = { 0x0 };

    initTestData_ProfilePersistent4(&expectedProfile);

    assert_int_equal(PROFILE_PERSISTENT_4_BINARY_SIZE, profilePersistentBinarySize(&expectedProfile));

    uint8_t* result = profilePersistentToBinary(outputBinArray, &expectedProfile);
    assert_ptr_equal(result, outputBinArray + PROFILE_PERSISTENT_4_BINARY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfilePersistent(outputBinArray, &profile, PROFILE_DAOS_VERSION);
    assert_ptr_equal(constResult, outputBinArray + PROFILE_PERSISTENT_4_BINARY_SIZE);
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}

void test_binary_ToFrom_ProfilePersistent5_succeeds(void** state)
{
    uint8_t expectedBinArray[PROFILE_PERSISTENT_5_BINARY_SIZE] = { PROFILE_PERSISTENT_5_BINARY };
   
    uint8_t outputBinArray[PROFILE_PERSISTENT_5_BINARY_SIZE * 2] = { 0x0 };

    initTestData_ProfilePersistent5(&expectedProfile);

    assert_int_equal(PROFILE_PERSISTENT_5_BINARY_SIZE, profilePersistentBinarySize(&expectedProfile));

    uint8_t* result = profilePersistentToBinary(outputBinArray, &expectedProfile);
    assert_ptr_equal(result, outputBinArray + PROFILE_PERSISTENT_5_BINARY_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfilePersistent(outputBinArray, &profile, PROFILE_DAOS_VERSION);
    assert_ptr_equal(constResult, outputBinArray + PROFILE_PERSISTENT_5_BINARY_SIZE);
    assert_ProfilePersistent_equal(&expectedProfile, &profile);
}
