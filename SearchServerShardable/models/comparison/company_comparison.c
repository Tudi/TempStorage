#include <company_comparison.h>
#include <company_industry_comparison.h>
#include <vector_comparison.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void assert_Company_equal(const struct Company* op1, const struct Company* op2)
{
    assert_string_equal(op1->name, op2->name);
    assert_string_equal(op1->domain, op2->domain);
    assert_int_equal(op1->id, op2->id);
    assert_int_equal(op1->parentId, op2->parentId);

    ASSERT_VECTOR_VALUE_EQUAL(assert_int_equal, op1->parentIndustryIds, op2->parentIndustryIds);

    assert_int_equal(op1->numEmployees, op2->numEmployees);
    assert_string_equal(op1->stage, op2->stage);
    assert_string_equal(op1->headquartersCity, op2->headquartersCity);
    assert_string_equal(op1->headquartersState, op2->headquartersState);
    assert_string_equal(op1->headquartersZipcode, op2->headquartersZipcode);
    assert_string_equal(op1->url, op2->url);
    assert_string_equal(op1->description, op2->description);
    assert_string_equal(op1->crunchbaseUrl, op2->crunchbaseUrl);
    assert_string_equal(op1->crunchbaseHeadquarters, op2->crunchbaseHeadquarters);
    assert_string_equal(op1->headquartersCountry, op2->headquartersCountry);
    assert_string_equal(op1->facebookUrl, op2->facebookUrl);
    assert_string_equal(op1->twitterUrl, op2->twitterUrl);
    assert_string_equal(op1->linkedinUrl, op2->linkedinUrl);
    assert_string_equal(op1->linkedinUser, op2->linkedinUser);
    assert_int_equal(op1->lastCachedAt, op2->lastCachedAt);
    assert_string_equal(op1->logoUrl, op2->logoUrl);

    ASSERT_VECTOR_ADDR_EQUAL(assert_CompanyIndustry_equal, op1->industries, op2->industries);
}
