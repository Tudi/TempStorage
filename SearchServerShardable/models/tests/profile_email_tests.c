#include <profile_email.h>
#include <profile_email_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct ProfileEmail expectedProfileEmail, profileEmail;
static struct json_object* obj = NULL;

int test_ProfileEmail_setUp(void** state)
{
    initProfileEmail(&expectedProfileEmail);
    initProfileEmail(&profileEmail);
    obj = NULL;

    return 0;
}

int test_ProfileEmail_tearDown(void** state)
{
    json_object_put(obj);
    freeProfileEmail(&profileEmail);
    freeProfileEmail(&expectedProfileEmail);

    return 0;
}

static void initTestData_ProfileEmail(struct ProfileEmail* profileEmail)
{
    profileEmail->uuid          = strdup("1234567890");
    profileEmail->email         = strdup("f.last@email.com");
    profileEmail->toxic         = true;
    profileEmail->domain        = strdup("email.com");
    profileEmail->gender        = strdup("male");
    profileEmail->manual        = false;
    profileEmail->status        = strdup("OK");
    profileEmail->bounced       = true;
    profileEmail->primary       = false;
    profileEmail->lastName      = strdup("last");
    profileEmail->firstName     = strdup("first");
    profileEmail->mxFound       = true;
    profileEmail->personal      = false;
    profileEmail->mxRecord      = strdup("24680");
    profileEmail->disposable    = true;
    profileEmail->freeEmail     = false;
    profileEmail->subStatus     = strdup("NOERROR");
    profileEmail->smtpProvider  = strdup("smtp.email.com");
    profileEmail->domainAgeDays = 99;
}

void test_marshall_unmarshall_ProfileEmail_succeeds(void** state)
{
    const char* expectedStr = "{ \"uuid\": \"1234567890\", \"email\": \"f.last@email.com\","
        " \"toxic\": true, \"domain\": \"email.com\", \"gender\": \"male\", \"manual\": false,"
        " \"status\": \"OK\", \"bounced\": true, \"primary\": false, \"lastname\": \"last\","
        " \"firstname\": \"first\", \"mx_found\": true, \"personal\": false,"
        " \"mx_record\": \"24680\", \"disposable\": true, \"free_email\": false,"
        " \"sub_status\": \"NOERROR\", \"smtp_provider\": \"smtp.email.com\","
        " \"domain_age_days\": 99 }";

    initTestData_ProfileEmail(&expectedProfileEmail);

    obj = marshallProfileEmail(&expectedProfileEmail);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfileEmail(&profileEmail, obj));
    assert_ProfileEmail_equal(&expectedProfileEmail, &profileEmail);
}

#define TEST_BINARY_PROFILEEMAIL_SIZE \
    (13 + 19 + 1 + 12 + 7 + 1 + 5 + 1 + 1 + 7 + 8 + 1 + 1 + 8 + 1 + 1 + 10 + 17 + 4)

void test_binary_ToFrom_ProfileEmail_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_PROFILEEMAIL_SIZE]
        = { 0xb, 0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0',
            0x11, 0x0, 'f', '.', 'l', 'a', 's', 't', '@', 'e', 'm', 'a', 'i', 'l', '.', 'c', 'o',
                  'm', '\0',
            0x1,
            0xa, 0x0, 'e', 'm', 'a', 'i', 'l', '.', 'c', 'o', 'm', '\0',
            0x5, 0x0, 'm', 'a', 'l', 'e', '\0',
            0x0,
            0x3, 0x0, 'O', 'K', '\0',
            0x1,
            0x0,
            0x5, 0x0, 'l', 'a', 's', 't', '\0',
            0x6, 0x0, 'f', 'i', 'r', 's', 't', '\0',
            0x1,
            0x0,
            0x6, 0x0, '2', '4', '6', '8', '0', '\0',
            0x1,
            0x0,
            0x8, 0x0, 'N', 'O', 'E', 'R', 'R', 'O', 'R', '\0',
            0xf, 0x0, 's', 'm', 't', 'p', '.', 'e', 'm', 'a', 'i', 'l', '.', 'c', 'o', 'm', '\0',
            0x63, 0x0, 0x0, 0x0 }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_PROFILEEMAIL_SIZE * 2] = { 0x0 };

    initTestData_ProfileEmail(&expectedProfileEmail);

    assert_int_equal(TEST_BINARY_PROFILEEMAIL_SIZE, profileEmailBinarySize(&expectedProfileEmail));

    uint8_t* result = profileEmailToBinary(outputBinArray, &expectedProfileEmail);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_PROFILEEMAIL_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfileEmail(outputBinArray, &profileEmail);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_PROFILEEMAIL_SIZE);
    assert_ProfileEmail_equal(&expectedProfileEmail, &profileEmail);
}
