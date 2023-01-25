#include <position_persistent.h>
#include <position_persistent_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct PositionPersistent expectedPosition, position;
static struct json_object* obj = NULL;

struct tm startDate = { .tm_sec = 31, .tm_min = 21, .tm_hour = 11,
    .tm_mday = 1, .tm_mon = 2, .tm_year = 103, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "01-Mar-2003 11:21:31"

struct tm endDate = { .tm_sec = 33, .tm_min = 23, .tm_hour = 13,
    .tm_mday = 2, .tm_mon = 3, .tm_year = 104, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "02-Apr-2004 13:23:33"

int test_PositionPersistent_setUp(void** state)
{
    initPositionPersistent(&expectedPosition);
    initPositionPersistent(&position);
    obj = NULL;

    return 0;
}

int test_PositionPersistent_tearDown(void** state)
{
    json_object_put(obj);
    freePositionPersistent(&position);
    freePositionPersistent(&expectedPosition);

    return 0;
}

static void initTestData_PositionPersistent(struct PositionPersistent* position)
{
    position->companyName     = strdup("COMPANY_MNO");
    position->title           = strdup("TITLE_ABC");
    position->startDate       = mktime(&startDate);
    position->endDate         = mktime(&endDate);
    position->companyId       = 123;
    position->titleId         = 78;

    position->uuid            = strdup("1234567890");
    position->titleCorrected  = strdup("TITLE_DEF");
    position->source          = strdup("SRC_GHI");
    position->locality        = strdup("LOCAL_JKL");
    position->parentCompanyId = 456;
    position->description     = strdup("DESC_PQR");
    position->seniority       = strdup("SENIOR_STU");
    position->modifiedInLoad  = true;
    position->parentTitleId   = 79;
}

void test_marshall_unmarshall_PositionPersistent_succeeds(void** state)
{
    const char* expectedStr = "{ \"company_name\": \"COMPANY_MNO\", \"title\": \"TITLE_ABC\","
        " \"start_date\": \"2003-03-01T11:21:31Z\", \"end_date\": \"2004-04-02T13:23:33Z\","
        " \"company_id\": 123, \"title_id\": 78,"
        " \"uuid\": \"1234567890\", \"title_corrected\": \"TITLE_DEF\", \"source\": \"SRC_GHI\","
        " \"locality\": \"LOCAL_JKL\", \"parent_company_id\": 456, \"description\": \"DESC_PQR\","
        " \"seniority\": \"SENIOR_STU\", \"modified_in_load\": true, \"title_parent_id\": 79 }";
    
    initTestData_PositionPersistent(&expectedPosition);

    obj = marshallPositionPersistent(&expectedPosition);

    assert_non_null(obj);

    const char* str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_SPACED);

    assert_non_null(str);
    assert_string_equal(expectedStr, str);

    assert_true(unmarshallPositionPersistent(&position, obj));
    assert_PositionPersistent_equal(&expectedPosition, &position);
}

#define TEST_BINARY_POSITIONPERSISTENT_SIZE \
    (14 + 12 + 8 + 8 + 4 + 4 + 13 + 12 + 10 + 12 + 4 + 11 + 13 + 1 + 4)

void test_binary_ToFrom_PositionPersistent_succeeds(void** state)
{
    uint8_t expectedBinArray[TEST_BINARY_POSITIONPERSISTENT_SIZE]
        = { 0xc, 0x0, 'C', 'O', 'M', 'P', 'A', 'N', 'Y', '_', 'M', 'N', 'O', '\0',
            0xa, 0x0, 'T', 'I', 'T', 'L', 'E', '_', 'A', 'B', 'C', '\0',
            0xbb, 0x97, 0x60, 0x3e, 0x0, 0x0, 0x0, 0x0,
            0x55, 0x69, 0x6d, 0x40, 0x0, 0x0, 0x0, 0x0,
            0x7b, 0x0, 0x0, 0x0,
            0x4e, 0x0, 0x0, 0x0,
            0xb, 0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0',
            0xa, 0x0, 'T', 'I', 'T', 'L', 'E', '_', 'D', 'E', 'F', '\0',
            0x8, 0x0, 'S', 'R', 'C', '_', 'G', 'H', 'I', '\0',
            0xa, 0x0, 'L', 'O', 'C', 'A', 'L', '_', 'J', 'K', 'L', '\0',
            0xc8, 0x1, 0x0, 0x0,
            0x9, 0x0, 'D', 'E', 'S', 'C', '_', 'P', 'Q', 'R', '\0',
            0xb, 0x0, 'S', 'E', 'N', 'I', 'O', 'R', '_', 'S', 'T', 'U', '\0',
            0x1,
            0x4f, 0x0, 0x0, 0x0}; // Little endian

    uint8_t outputBinArray[TEST_BINARY_POSITIONPERSISTENT_SIZE * 2] = { 0x0 };

    initTestData_PositionPersistent(&expectedPosition);

    assert_int_equal(TEST_BINARY_POSITIONPERSISTENT_SIZE,
        positionPersistentBinarySize(&expectedPosition));

    uint8_t* result = positionPersistentToBinary(outputBinArray, &expectedPosition);
    assert_ptr_equal(result, outputBinArray + TEST_BINARY_POSITIONPERSISTENT_SIZE);
    assert_memory_equal(expectedBinArray, outputBinArray, sizeof(expectedBinArray));

    const uint8_t* constResult = binaryToPositionPersistent(outputBinArray, &position);
    assert_ptr_equal(constResult, outputBinArray + TEST_BINARY_POSITIONPERSISTENT_SIZE);
    assert_PositionPersistent_equal(&expectedPosition, &position);
}
