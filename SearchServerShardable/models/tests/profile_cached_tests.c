#include <profile_persistent.h>
#include <profile_cached.h>
#include <profile_cached_comparison.h>
#include <profile_test_data.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

static struct ProfilePersistent profilePersistent1, profilePersistent2;
static struct ProfileCached profileCached1, profileCached2;
static struct ProfileCached expectedProfileCached1, expectedProfileCached2;

int test_ProfileCached_setUp(void** state)
{
    initProfilePersistent(&profilePersistent1);
    initProfilePersistent(&profilePersistent2);

    initProfileCached(&profileCached1);
    initProfileCached(&expectedProfileCached1);

    initProfileCached(&profileCached2);
    initProfileCached(&expectedProfileCached2);

    return 0;
}

int test_ProfileCached_tearDown(void** state)
{
    freeProfileCached(&expectedProfileCached2);
    freeProfileCached(&expectedProfileCached1);

    freeProfileCached(&profileCached2);
    freeProfileCached(&profileCached1);

    freeProfilePersistent(&profilePersistent2);
    freeProfilePersistent(&profilePersistent1);

    return 0;
}

void test_profilePersistentToProfileCached_succeeds(void** state)
{
    initTestData_ProfilePersistent1(&profilePersistent1);
    initTestData_ProfilePersistent2(&profilePersistent2);

    initTestData_ProfileCached1(&expectedProfileCached1);
    initTestData_ProfileCached2(&expectedProfileCached2);

    assert_true(profilePersistentToProfileCached(&profileCached1, &profilePersistent1));

    assert_non_null(profileCached1.fullName);

    assert_ProfileCached_equal(&expectedProfileCached1, &profileCached1);

    assert_true(profilePersistentToProfileCached(&profileCached2, &profilePersistent2));

    assert_non_null(profileCached2.fullName);

    assert_ProfileCached_equal(&expectedProfileCached2, &profileCached2);
}

void test_binaryToProfileCached_succeeds(void** state)
{
    uint8_t binaryProfilePersistent1[PROFILE_PERSISTENT_1_BINARY_SIZE] = { PROFILE_PERSISTENT_1_BINARY };
    uint8_t binaryProfilePersistent2[PROFILE_PERSISTENT_2_BINARY_SIZE] = { PROFILE_PERSISTENT_2_BINARY };

    initTestData_ProfileCached1(&expectedProfileCached1);
    initTestData_ProfileCached2(&expectedProfileCached2);

    const uint8_t* result = binaryToProfileCached(binaryProfilePersistent1, &profileCached1);
    assert_ptr_equal(binaryProfilePersistent1 + PROFILE_PERSISTENT_1_CACHED_PORTION_BINARY_SIZE, result);

    assert_ProfileCached_equal(&expectedProfileCached1, &profileCached1);

    result = binaryToProfileCached(binaryProfilePersistent2, &profileCached2);
    assert_ptr_equal(binaryProfilePersistent2 + PROFILE_PERSISTENT_2_CACHED_PORTION_BINARY_SIZE, result);

    assert_ProfileCached_equal(&expectedProfileCached2, &profileCached2);
}
