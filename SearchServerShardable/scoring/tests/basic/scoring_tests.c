#include <scoring/composite_score.h>
#include <profile_cached.h>
#include <filters.h>
#include <companyrolefilter.h>
#include <search_criteria.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

void TestScoringInitDeinit()
{
    ScoringCompositeScore score;
    struct SearchCriteria companyRole;

    initScoringCompositeScore(&score);
    
    initSearchCriteria(&companyRole);

    kv_pushp(struct SearchFilter, companyRole.filters);
    struct SearchFilter* filter = &kv_A(companyRole.filters, kv_size(companyRole.filters) - 1); // get the last filter( the one we just added )
    initSearchFilter(filter);
    filter->name = strdup("current_previous_title_include");
    filter->textValue = strdup("developer");

    // this is not the real role structure. temp init with temp values
//    score.companyRole->companyID = 1;
    companyRole.role = 1;
    kv_push(int32_t, companyRole.localities, 1);
    convertLocalitiesToBitfield(&companyRole);

    setScoringCompositeScoreSearchCriteria(&score, &companyRole, 0);

    struct ProfileCached profile;
    initProfileCached(&profile);
    profile.localityId = 1;

    // add a position to the profile
    kv_pushp(struct PositionCached, profile.positions);
    struct PositionCached* pos = &(kv_A(profile.positions, 0));
    initPositionCached(pos);
    pos->title = strdup("developer");
    pos->companyId = 1;
    score.profile = &profile;

    int res = runCompare(&score);
//    printf("runCompare result is : %d\n", res);
    assert_true(res == 0);

    freeScoringCompositeScore(&score);
    freeSearchCriteria(&companyRole);
    freeProfileCached(&profile);
}

int main(int argc, char* argv[])
{
    const struct CMUnitTest tests[] = { {"test_ScoringInitDeinit", TestScoringInitDeinit, NULL, NULL, NULL} };
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    return ret;
}