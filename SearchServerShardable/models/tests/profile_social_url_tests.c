#include <profile_social_url.h>
#include <profile_social_url_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct ProfileSocialUrl expectedSocialUrl, socialUrl;
static struct json_object* obj = NULL;

int test_ProfileSocialUrl_setUp(void** state)
{
    initProfileSocialUrl(&expectedSocialUrl);
    initProfileSocialUrl(&socialUrl);
    obj = NULL;

    return 0;
}

int test_ProfileSocialUrl_tearDown(void** state)
{
    json_object_put(obj);
    freeProfileSocialUrl(&socialUrl);
    freeProfileSocialUrl(&expectedSocialUrl);

    return 0;
}

static void initTestData_ProfileSocialUrl(struct ProfileSocialUrl* socialUrl)
{
    expectedSocialUrl.uuid          = strdup("1234567890");
    expectedSocialUrl.source        = strdup("SRC_ABC");
    expectedSocialUrl.url           = strdup("URL_DEF");
    expectedSocialUrl.username      = strdup("USER_GHI");
}

void test_marshall_unmarshall_ProfileSocialUrl_succeeds(void** state)
{
    const char* expectedStr = "{ \"uuid\": \"1234567890\", \"source\": \"SRC_ABC\","
        " \"url\": \"URL_DEF\", \"username\": \"USER_GHI\" }";

    initTestData_ProfileSocialUrl(&expectedSocialUrl);

    obj = marshallProfileSocialUrl(&expectedSocialUrl);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfileSocialUrl(&socialUrl, obj));
    assert_ProfileSocialUrl_equal(&expectedSocialUrl, &socialUrl);
}

#define TEST_BINARY_PROFILESOCIALURL_SIZE (13 + 10 + 10 + 11)

void test_binary_ToFrom_ProfileSocialUrl_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_PROFILESOCIALURL_SIZE]
        = { 0xb, 0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0',
            0x8, 0x0, 'S', 'R', 'C', '_', 'A', 'B', 'C', '\0',
            0x8, 0x0, 'U', 'R', 'L', '_', 'D', 'E', 'F', '\0',
            0x9, 0x0, 'U', 'S', 'E', 'R', '_', 'G', 'H', 'I', '\0' }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_PROFILESOCIALURL_SIZE * 2] = { 0x0 };

    initTestData_ProfileSocialUrl(&expectedSocialUrl);

    assert_int_equal(TEST_BINARY_PROFILESOCIALURL_SIZE,
        profileSocialUrlBinarySize(&expectedSocialUrl));

    uint8_t* result = profileSocialUrlToBinary(outputBinArray, &expectedSocialUrl);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_PROFILESOCIALURL_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfileSocialUrl(outputBinArray, &socialUrl);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_PROFILESOCIALURL_SIZE);
    assert_ProfileSocialUrl_equal(&expectedSocialUrl, &socialUrl);
}
