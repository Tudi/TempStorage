#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

int test_Company_setUp(void** state);
int test_Company_tearDown(void** state);
void test_marshall_unmarshall_Company_succeeds(void** state);
void test_binary_ToFrom_Company_succeeds(void** state);

int test_CompanyCached_setUp(void** state);
int test_CompanyCached_tearDown(void** state);
void test_companyToCompanyCached_succeeds(void** state);
void test_binaryToCompanyCached_succeeds(void** state);

int test_CompanyIndustry_setUp(void** state);
int test_CompanyIndustry_tearDown(void** state);
void test_marshall_unmarshall_CompanyIndustry_succeeds(void** state);
void test_binary_ToFrom_CompanyIndustry_succeeds(void** state);

int test_Education_setUp(void** state);
int test_Education_tearDown(void** state);
void test_marshall_unmarshall_Education_succeeds(void** state);
void test_binary_ToFrom_Education_succeeds(void** state);

int test_Id_Int32Value_setUp(void** state);
int test_Id_Int32Value_tearDown(void** state);
void test_marshall_unmarshall_Id_Int32Value_succeeds(void** state);
void test_binary_ToFrom_Id_Int32Value_succeeds(void** state);

int test_Id_StringValue_setUp(void** state);
int test_Id_StringValue_tearDown(void** state);
void test_marshall_unmarshall_Id_StringValue_succeeds(void** state);
void test_binary_ToFrom_Id_StringValue_succeeds(void** state);

int test_Id_TimeValue_setUp(void** state);
int test_Id_TimeValue_tearDown(void** state);
void test_marshall_unmarshall_Id_TimeValue_succeeds(void** state);
void test_binary_ToFrom_Id_TimeValue_succeeds(void** state);

int test_PositionCached_setUp(void** state);
int test_PositionCached_tearDown(void** state);
void test_positionPersistentToPositionCached_succeeds(void** state);

int test_PositionPersistent_setUp(void** state);
int test_PositionPersistent_tearDown(void** state);
void test_marshall_unmarshall_PositionPersistent_succeeds(void** state);
void test_binary_ToFrom_PositionPersistent_succeeds(void** state);

int test_ProfileCached_setUp(void** state);
int test_ProfileCached_tearDown(void** state);
void test_profilePersistentToProfileCached_succeeds(void** state);
void test_binaryToProfileCached_succeeds(void** state);

int test_ProfileEmail_setUp(void** state);
int test_ProfileEmail_tearDown(void** state);
void test_marshall_unmarshall_ProfileEmail_succeeds(void** state);
void test_binary_ToFrom_ProfileEmail_succeeds(void** state);

int test_ProfilePersistent_setUp(void** state);
int test_ProfilePersistent_tearDown(void** state);
void test_marshall_unmarshall_ProfilePersistent1_succeeds(void** state);
void test_marshall_unmarshall_ProfilePersistent2_succeeds(void** state);
void test_marshall_unmarshall_ProfilePersistent3_succeeds(void** state);
void test_marshall_unmarshall_ProfilePersistent4_succeeds(void** state);
void test_marshall_unmarshall_ProfilePersistent5_succeeds(void** state);
void test_binary_ToFrom_ProfilePersistent1_succeeds(void** state);
void test_binary_ToFrom_ProfilePersistent2_succeeds(void** state);
void test_binary_ToFrom_ProfilePersistent3_succeeds(void** state);
void test_binary_ToFrom_ProfilePersistent4_succeeds(void** state);
void test_binary_ToFrom_ProfilePersistent5_succeeds(void** state);

int test_ProfilePhoneNumber_setUp(void** state);
int test_ProfilePhoneNumber_tearDown(void** state);
void test_marshall_unmarshall_ProfilePhoneNumber_succeeds(void** state);
void test_binary_ToFrom_ProfilePhoneNumber_succeeds(void** state);

int test_ProfileSocialUrl_setUp(void** state);
int test_ProfileSocialUrl_tearDown(void** state);
void test_marshall_unmarshall_ProfileSocialUrl_succeeds(void** state);
void test_binary_ToFrom_ProfileSocialUrl_succeeds(void** state);

int test_ProfileTag_setUp(void** state);
int test_ProfileTag_tearDown(void** state);
void test_marshall_unmarshall_ProfileTag_succeeds(void** state);
void test_binary_ToFrom_ProfileTag_succeeds(void** state);

void test_binary_ToFrom_Boolean_succeeds(void** state);
void test_binary_ToFrom_Int32_succeeds(void** state);
void test_binary_ToFrom_Int64_succeeds(void** state);

int test_binary_ToFrom_String_setUp(void** state);
int test_binary_ToFrom_String_tearDown(void** state);
void test_binary_ToFrom_String_succeeds(void** state);

int main(int argc, char* argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_companyToCompanyCached_succeeds,
            test_CompanyCached_setUp, test_CompanyCached_tearDown),
        cmocka_unit_test_setup_teardown(test_binaryToCompanyCached_succeeds,
            test_CompanyCached_setUp, test_CompanyCached_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_Company_succeeds,
            test_Company_setUp, test_Company_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_Company_succeeds,
            test_Company_setUp, test_Company_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_CompanyIndustry_succeeds,
            test_CompanyIndustry_setUp, test_CompanyIndustry_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_Education_succeeds,
            test_Education_setUp, test_Education_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_Education_succeeds,
            test_Education_setUp, test_Education_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_Id_Int32Value_succeeds,
            test_Id_Int32Value_setUp, test_Id_Int32Value_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_Id_Int32Value_succeeds,
            test_Id_Int32Value_setUp, test_Id_Int32Value_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_Id_StringValue_succeeds,
            test_Id_StringValue_setUp, test_Id_StringValue_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_Id_StringValue_succeeds,
            test_Id_StringValue_setUp, test_Id_StringValue_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_Id_TimeValue_succeeds,
            test_Id_TimeValue_setUp, test_Id_TimeValue_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_Id_TimeValue_succeeds,
            test_Id_TimeValue_setUp, test_Id_TimeValue_tearDown),
        cmocka_unit_test_setup_teardown(test_positionPersistentToPositionCached_succeeds,
            test_PositionCached_setUp, test_PositionCached_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_PositionPersistent_succeeds,
            test_PositionPersistent_setUp, test_PositionPersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_PositionPersistent_succeeds,
            test_PositionPersistent_setUp, test_PositionPersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_profilePersistentToProfileCached_succeeds,
            test_ProfileCached_setUp, test_ProfileCached_tearDown),
        cmocka_unit_test_setup_teardown(test_binaryToProfileCached_succeeds,
            test_ProfileCached_setUp, test_ProfileCached_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfileEmail_succeeds,
            test_ProfileEmail_setUp, test_ProfileEmail_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfileEmail_succeeds,
            test_ProfileEmail_setUp, test_ProfileEmail_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfilePersistent1_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfilePersistent2_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfilePersistent3_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfilePersistent4_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfilePersistent5_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfilePersistent1_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfilePersistent2_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfilePersistent3_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfilePersistent4_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfilePersistent5_succeeds,
            test_ProfilePersistent_setUp, test_ProfilePersistent_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfilePhoneNumber_succeeds,
            test_ProfilePhoneNumber_setUp, test_ProfilePhoneNumber_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfilePhoneNumber_succeeds,
            test_ProfilePhoneNumber_setUp, test_ProfilePhoneNumber_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfileSocialUrl_succeeds,
            test_ProfileSocialUrl_setUp, test_ProfileSocialUrl_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfileSocialUrl_succeeds,
            test_ProfileSocialUrl_setUp, test_ProfileSocialUrl_tearDown),
        cmocka_unit_test_setup_teardown(test_marshall_unmarshall_ProfileTag_succeeds,
            test_ProfileTag_setUp, test_ProfileTag_tearDown),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_ProfileTag_succeeds,
            test_ProfileTag_setUp, test_ProfileTag_tearDown),
        cmocka_unit_test(test_binary_ToFrom_Boolean_succeeds),
        cmocka_unit_test(test_binary_ToFrom_Int32_succeeds),
        cmocka_unit_test(test_binary_ToFrom_Int64_succeeds),
        cmocka_unit_test_setup_teardown(test_binary_ToFrom_String_succeeds,
            test_binary_ToFrom_String_setUp, test_binary_ToFrom_String_tearDown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
