#ifndef _FILTER_SETUPS_H_
#define _FILTER_SETUPS_H_

int test_FilterName_1_setUp(void** state);
int test_FilterName_2_setUp(void** state);
int test_FilterNameInclude_3(void** state);
int test_FilterName_3_setUp(void** state);
int test_FilterName_4_setUp(void** state);
int test_FilterName_5_setUp(void** state);
int test_FilterName_tearDown(void** state);
void test_FilterAndScore_succeeds(void** state);

int test_FilterTitle_setUp(void** state);
int test_FilterTitleCapsMulti_setUp(void** state);
int test_FilterTitleCorrected_setUp(void** state);

int test_FilterCompany1_setUp(void** state);
int test_FilterCompany2_setUp(void** state);
int test_FilterCompany3_setUp(void** state);
int test_FilterCompany4_setUp(void** state);
int test_FilterCompany5_setUp(void** state);
int test_FilterCompany6_setUp(void** state);
int test_FilterCompany7_setUp(void** state);

int test_FilterKeywords_setUp(void** state);
int test_FilterKeywords2_setUp(void** state);
int test_FilterKeywordsTitle_setUp(void** state);
int test_FilterKeywordsCompany_setUp(void** state);
int test_FilterKeywordsSkills_setUp(void** state);
int test_FilterKeywordsHeadline_setUp(void** state);
int test_FilterKeywordsSummary_setUp(void** state);
int test_FilterKeywordsDescription_setUp(void** state);

int test_FilterExperience_setUp(void** state);

int test_FilterTenure1_setUp(void** state);
int test_FilterTenure2_setUp(void** state);
int test_FilterTenure3_setUp(void** state);

int test_FilterRelevantExperience_setUp(void** state);

int test_FilterTotalExperience1_setUp(void** state);

int test_FilterProject1_setUp(void** state);
int test_FilterProject2_setUp(void** state);

int test_FilterGroup1_setUp(void** state);
int test_FilterGroup2_setUp(void** state);

int test_FilterReply1_setUp(void** state);
int test_FilterReply2_setUp(void** state);

int test_FilterMulti_setUp(void** state);

int test_FilterCompanyScore_setUp(void** state);

int test_FilterIndustry_setUp(void** state);

int test_FilterLocality_setUp(void** state);
int test_FilterLocalityLargeSet_setUp(void** state);

int test_FilterCompanyFunctionSet_setUp(void** state);

int test_FilterCompanyNRE_setUp(void** state);

int test_FilterBoolean1_setUp(void** state);
int test_FilterBoolean2_setUp(void** state);
int test_FilterBoolean3_setUp(void** state);

int test_CountryIdInclude_setUp(void** state);
int test_CountryIdExclude_setUp(void** state);

int test_SkillScore1_setUp(void** state);
int test_SkillScore2_setUp(void** state);

int test_ProfileIdInclude_setUp(void** state);
int test_ProfileIdExclude_setUp(void** state);

int test_StateIdInclude_setUp(void** state);
int test_StateIdExclude_setUp(void** state);

int test_CampaignInclude_setUp(void** state);
int test_CampaignExclude_setUp(void** state);

int test_TalentPoolInclude_setUp(void** state);
int test_TalentPoolExclude_setUp(void** state);

#endif