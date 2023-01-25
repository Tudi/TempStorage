#include <profile_tag.h>
#include <profile_tag_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct ProfileTag expectedTag, tag;
static struct json_object* obj = NULL;

int test_ProfileTag_setUp(void** state)
{
    initProfileTag(&expectedTag);
    initProfileTag(&tag);
    obj = NULL;

    return 0;
}

int test_ProfileTag_tearDown(void** state)
{
    json_object_put(obj);
    freeProfileTag(&tag);
    freeProfileTag(&expectedTag);

    return 0;
}

static void initTestData_ProfileTag(struct ProfileTag* tag)
{
    tag->uuid      = strdup("1234567890");
    tag->tagId     = 2345;
    tag->source    = strdup("SRC_ABC");
    tag->tagType   = strdup("TAG_XYZ");
    tag->text      = strdup("SOME_TXT");
}

void test_marshall_unmarshall_ProfileTag_succeeds(void** state)
{
    const char* expectedStr = "{ \"uuid\": \"1234567890\", \"tag_id\": 2345,"
        " \"source\": \"SRC_ABC\", \"tag_type\": \"TAG_XYZ\", \"text\": \"SOME_TXT\" }";

    initTestData_ProfileTag(&expectedTag);

    obj = marshallProfileTag(&expectedTag);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallProfileTag(&tag, obj));
    assert_ProfileTag_equal(&expectedTag, &tag);
}

#define TEST_BINARY_PROFILETAG_SIZE (13 + 8 + 10 + 10 + 11)

void test_binary_ToFrom_ProfileTag_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_PROFILETAG_SIZE]
        = { 0xb, 0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0',
            0x29, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x8, 0x0, 'S', 'R', 'C', '_', 'A', 'B', 'C', '\0',
            0x8, 0x0, 'T', 'A', 'G', '_', 'X', 'Y', 'Z', '\0',
            0x9, 0x0, 'S', 'O', 'M', 'E', '_', 'T', 'X', 'T', '\0' }; // Little endian
    uint8_t outputBinArray[TEST_BINARY_PROFILETAG_SIZE * 2] = { 0x0 };

    initTestData_ProfileTag(&expectedTag);

    assert_int_equal(TEST_BINARY_PROFILETAG_SIZE, profileTagBinarySize(&expectedTag));

    uint8_t* result = profileTagToBinary(outputBinArray, &expectedTag);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_PROFILETAG_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToProfileTag(outputBinArray, &tag);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_PROFILETAG_SIZE);
    assert_ProfileTag_equal(&expectedTag, &tag);
}
