#include <position_cached.h>
#include <position_cached_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static struct tm startDate = { .tm_sec = 31, .tm_min = 21, .tm_hour = 11,
    .tm_mday = 1, .tm_mon = 2, .tm_year = 103, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "01-Mar-2003 11:21:31"

static struct tm endDate = { .tm_sec = 33, .tm_min = 23, .tm_hour = 13,
    .tm_mday = 2, .tm_mon = 3, .tm_year = 104, .tm_wday = 0, .tm_yday = 0,
    .tm_isdst = 0 }; // "02-Apr-2004 13:23:33"

static struct PositionPersistent positionPersistent;
static struct PositionCached expectedPositionCached, positionCached;

static void initTestData_PositionPersistent(struct PositionPersistent* position);
static void initTestData_PositionCached(struct PositionCached* position);

int test_PositionCached_setUp(void** state)
{
    initPositionPersistent(&positionPersistent);
    initPositionCached(&positionCached);

    initPositionCached(&expectedPositionCached);
    initTestData_PositionCached(&expectedPositionCached);

    return 0;
}

int test_PositionCached_tearDown(void** state)
{
    freePositionCached(&expectedPositionCached);
    freePositionCached(&positionCached);
    freePositionPersistent(&positionPersistent);

    return 0;
}

void test_positionPersistentToPositionCached_succeeds(void** state)
{
    initTestData_PositionPersistent(&positionPersistent);

    assert_true(positionPersistentToPositionCached(&positionCached, &positionPersistent));

    assert_non_null(positionCached.companyName);
    assert_non_null(positionCached.title);

    assert_PositionCached_equal(&expectedPositionCached, &positionCached);
}

static void initTestData_PositionPersistent(struct PositionPersistent* position)
{
    position->uuid            = strdup("1234567890");
    position->title           = strdup("TITLE_ABC");
    position->titleCorrected  = strdup("TITLE_DEF");
    position->source          = strdup("SRC_GHI");
    position->locality        = strdup("LOCAL_JKL");
    position->companyId       = 123;
    position->parentCompanyId = 456;
    position->companyName     = strdup("COMPANY_MNO");
    position->startDate       = mktime(&startDate);
    position->endDate         = mktime(&endDate);
    position->description     = strdup("DESC_PQR");
    position->seniority       = strdup("SENIOR_STU");
    position->modifiedInLoad  = true;
    position->titleId         = 78;
    position->parentTitleId   = 79;
}

static void initTestData_PositionCached(struct PositionCached* position)
{
    position->companyName   = strdup("company_mno");
    position->title         = strdup("title_def");
    position->startDate     = mktime(&startDate);
    position->endDate       = mktime(&endDate);
    position->companyId     = 123;
    position->parentTitletId = 79;
}
