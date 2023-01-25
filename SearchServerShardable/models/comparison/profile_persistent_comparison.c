#include <profile_persistent_comparison.h>
#include <education_comparison.h>
#include <id_value_comparison.h>
#include <position_persistent_comparison.h>
#include <profile_email_comparison.h>
#include <profile_phone_number_comparison.h>
#include <profile_social_url_comparison.h>
#include <profile_tag_comparison.h>
#include <vector_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_ProfilePersistent_equal(const struct ProfilePersistent* op1,
    const struct ProfilePersistent* op2)
{
    assert_int_equal(op1->logicalDelete, op2->logicalDelete);
    assert_int_equal(op1->id, op2->id);
    assert_string_equal(op1->firstName, op2->firstName);
    assert_string_equal(op1->lastName, op2->lastName);
    assert_int_equal(op1->localityId, op2->localityId);
    assert_int_equal(op1->countryId, op2->countryId);
    assert_int_equal(op1->stateId, op2->stateId);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->lastMessaged, op2->lastMessaged);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->lastReplied, op2->lastReplied);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->lastPositiveReply,
        op2->lastPositiveReply);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_Int32Value_equal, op1->groups, op2->groups);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_Int32Value_equal, op1->projects, op2->projects);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_Int32Value_equal, op1->campaigns, op2->campaigns);
    ASSERT_VECTOR_VALUE_EQUAL(assert_int_equal, op1->actioned, op2->actioned);
    ASSERT_VECTOR_ADDR_EQUAL(assert_PositionPersistent_equal, op1->positions, op2->positions);
    ASSERT_VECTOR_VALUE_EQUAL(assert_string_equal, op1->skills, op2->skills);
    assert_string_equal(op1->headline, op2->headline);
    assert_string_equal(op1->summary, op2->summary);
    assert_string_equal(op1->archivedPrimaryEmail, op2->archivedPrimaryEmail);
    assert_string_equal(op1->linkedinUser, op2->linkedinUser);
    assert_string_equal(op1->profileCode, op2->profileCode);
    assert_string_equal(op1->profileKey, op2->profileKey);
    assert_string_equal(op1->imageUrl, op2->imageUrl);
    assert_string_equal(op1->industry, op2->industry);
    assert_string_equal(op1->linkedinUrl, op2->linkedinUrl);
    assert_int_equal(op1->numConnections, op2->numConnections);
    assert_string_equal(op1->source, op2->source);
    assert_int_equal(op1->batch, op2->batch);
    assert_int_equal(op1->covid19, op2->covid19);
    assert_string_equal(op1->linkedinCanonicalUrl, op2->linkedinCanonicalUrl);
    assert_string_equal(op1->linkedinCanonicalUser, op2->linkedinCanonicalUser);
    assert_string_equal(op1->linkedinId, op2->linkedinId);
    assert_string_equal(op1->sourceKey, op2->sourceKey);
    assert_string_equal(op1->location, op2->location);
    assert_string_equal(op1->country, op2->country);
    assert_string_equal(op1->gender, op2->gender);
    assert_string_equal(op1->ethnicity, op2->ethnicity);
    assert_int_equal(op1->lastCachedAt, op2->lastCachedAt);

    ASSERT_VECTOR_VALUE_EQUAL(assert_int_equal, op1->atsCompanies, op2->atsCompanies);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_StringValue_equal, op1->atsStatus, op2->atsStatus);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Id_TimeValue_equal, op1->atsLastactivity, op2->atsLastactivity);
    ASSERT_VECTOR_ADDR_EQUAL(assert_Education_equal, op1->educations, op2->educations);
    ASSERT_VECTOR_ADDR_EQUAL(assert_ProfileEmail_equal, op1->profileEmails, op2->profileEmails);
    ASSERT_VECTOR_ADDR_EQUAL(assert_ProfileSocialUrl_equal, op1->profileSocialUrls,
        op2->profileSocialUrls);
    ASSERT_VECTOR_ADDR_EQUAL(assert_ProfilePhoneNumber_equal, op1->profilePhoneNumbers,
        op2->profilePhoneNumbers);
    ASSERT_VECTOR_ADDR_EQUAL(assert_ProfileTag_equal, op1->profileTags, op2->profileTags);
}
