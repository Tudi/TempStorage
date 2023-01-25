#include <profile_loader.h>
#include <company_loader.h>
#include <profile_persistent.h>
#include <profile_cached.h>
#include <companyrolefilter.h>
#include <scoring/composite_score.h>
#include <strings_ext.h>
#include <filter_setups.h>
#include <profile_info.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

int main(int argc, char* argv[])
{
    // repeatadly I wondered why filter and scoring messages do not show up. Because default log level became info
    logger_setLogLevel(DEBUG_LOG_MSG);

    LoadCompaniesFromFiles();
    LoadProfilesFromFiles();

    const struct CMUnitTest tests[] = {
        {"test_FilterName_Include", test_FilterAndScore_succeeds, test_FilterName_1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterName_Include", test_FilterAndScore_succeeds, test_FilterName_2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterName_Include3", test_FilterAndScore_succeeds, test_FilterNameInclude_3, test_FilterName_tearDown, NULL},
        {"test_FilterName_Exclude", test_FilterAndScore_succeeds, test_FilterName_3_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterName_IncludeMulti", test_FilterAndScore_succeeds, test_FilterName_4_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterName_ExcludeMulti", test_FilterAndScore_succeeds, test_FilterName_5_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTitle", test_FilterAndScore_succeeds, test_FilterTitle_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTitleCaps", test_FilterAndScore_succeeds, test_FilterTitleCapsMulti_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTitleCorrected", test_FilterAndScore_succeeds, test_FilterTitleCorrected_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyInclude", test_FilterAndScore_succeeds, test_FilterCompany1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyExclude", test_FilterAndScore_succeeds, test_FilterCompany2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyExcludeAll", test_FilterAndScore_succeeds, test_FilterCompany3_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyIncludePrev", test_FilterAndScore_succeeds, test_FilterCompany4_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyExcludePrev", test_FilterAndScore_succeeds, test_FilterCompany5_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyMultiInclude", test_FilterAndScore_succeeds, test_FilterCompany6_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyMultiExclude", test_FilterAndScore_succeeds, test_FilterCompany7_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywords", test_FilterAndScore_succeeds, test_FilterKeywords_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywordsMulti", test_FilterAndScore_succeeds, test_FilterKeywords2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywordsTitle", test_FilterAndScore_succeeds, test_FilterKeywordsTitle_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywordsCompany", test_FilterAndScore_succeeds, test_FilterKeywordsCompany_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywordsHeadline", test_FilterAndScore_succeeds, test_FilterKeywordsHeadline_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywordsSummary", test_FilterAndScore_succeeds, test_FilterKeywordsSummary_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterKeywordsDescription", test_FilterAndScore_succeeds, test_FilterKeywordsDescription_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterExperience", test_FilterAndScore_succeeds, test_FilterExperience_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTenureBetween", test_FilterAndScore_succeeds, test_FilterTenure1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTenureSmaller", test_FilterAndScore_succeeds, test_FilterTenure2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTenureLarger", test_FilterAndScore_succeeds, test_FilterTenure3_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterRelevantExperience", test_FilterAndScore_succeeds, test_FilterRelevantExperience_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterTotalExperience", test_FilterAndScore_succeeds, test_FilterTotalExperience1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterProjectInclude", test_FilterAndScore_succeeds, test_FilterProject1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterProjectExclude", test_FilterAndScore_succeeds, test_FilterProject2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterGroupInclude", test_FilterAndScore_succeeds, test_FilterGroup1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterGroupExclude", test_FilterAndScore_succeeds, test_FilterGroup2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterReplyMissing", test_FilterAndScore_succeeds, test_FilterReply1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterReplyPresent", test_FilterAndScore_succeeds, test_FilterReply2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterMultiple", test_FilterAndScore_succeeds, test_FilterMulti_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyScore", test_FilterAndScore_succeeds, test_FilterCompanyScore_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterIndustry", test_FilterAndScore_succeeds, test_FilterIndustry_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterLocality", test_FilterAndScore_succeeds, test_FilterLocality_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterLocalityLarge", test_FilterAndScore_succeeds, test_FilterLocalityLargeSet_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyFunction", test_FilterAndScore_succeeds, test_FilterCompanyFunctionSet_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterCompanyNRE", test_FilterAndScore_succeeds, test_FilterCompanyNRE_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterBoolean1", test_FilterAndScore_succeeds, test_FilterBoolean1_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterBoolean2", test_FilterAndScore_succeeds, test_FilterBoolean2_setUp, test_FilterName_tearDown, NULL},
        {"test_FilterBoolean3", test_FilterAndScore_succeeds, test_FilterBoolean3_setUp, test_FilterName_tearDown, NULL},
        {"test_CountryInclude", test_FilterAndScore_succeeds, test_CountryIdInclude_setUp, test_FilterName_tearDown, NULL},
        {"test_CountryExclude", test_FilterAndScore_succeeds, test_CountryIdExclude_setUp, test_FilterName_tearDown, NULL},
        {"test_SkillScore1", test_FilterAndScore_succeeds, test_SkillScore1_setUp, test_FilterName_tearDown, NULL},
        {"test_SkillScore2", test_FilterAndScore_succeeds, test_SkillScore2_setUp, test_FilterName_tearDown, NULL},
        {"test_ProfileIdInclude", test_FilterAndScore_succeeds, test_ProfileIdInclude_setUp, test_FilterName_tearDown, NULL},
        {"test_ProfileIdExclude", test_FilterAndScore_succeeds, test_ProfileIdExclude_setUp, test_FilterName_tearDown, NULL},
        {"test_StateIdInclude", test_FilterAndScore_succeeds, test_StateIdInclude_setUp, test_FilterName_tearDown, NULL},
        {"test_StateIdExclude", test_FilterAndScore_succeeds, test_StateIdExclude_setUp, test_FilterName_tearDown, NULL},
        {"test_CampaignInclude", test_FilterAndScore_succeeds, test_CampaignInclude_setUp, test_FilterName_tearDown, NULL},
        {"test_CampaignExclude", test_FilterAndScore_succeeds, test_CampaignExclude_setUp, test_FilterName_tearDown, NULL},
        {"test_TalentPoolInclude", test_FilterAndScore_succeeds, test_TalentPoolInclude_setUp, test_FilterName_tearDown, NULL},
        {"test_TalentPoolExclude", test_FilterAndScore_succeeds, test_TalentPoolExclude_setUp, test_FilterName_tearDown, NULL},
    };
   
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    
    freeLoadedCompanies();
    freeLoadedProfiles();

    return ret;
}
