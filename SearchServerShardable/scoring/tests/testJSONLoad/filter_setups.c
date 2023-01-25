#include <profile_persistent.h>
#include <profile_cached.h>
#include <id_value.h>
#include <companyrolefilter.h>
#include <filters.h>
#include <scoring/composite_score.h>
#include <strings_ext.h>
#include <filter_setups.h>
#include <profile_loader.h>
#include <company_loader.h>
#include <profile_info.h>
#include <filter_info.h>
#include <search_criteria.h>
#include <company_cached.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#define SKIP_CHECKING_THIS_VALUE    0
#define NUMBER_SECONDS_IN_DAY (24 * 60 * 60)
#define MAX_TEST_FILES  100

typedef struct FilterResultExpect
{
    const char* fileName;
    int filterResult;
    float relevantExperience;
    float companyScore;
    float titleScore;
    float skillScore;
    const char* explainStr;
    char testPerformed;
    int obtainedFilterResult;
    int testPassed;
}FilterResultExpect;

static int AnyErrorsReported = 0;
time_t StampWhenScoreValuesGotHardCoded = 1638884877; // since experience will increase as time passes by, we need to scale hardcoded values
ScoringCompositeScore score;
struct SearchCriteria companyRole;
FilterResultExpect results[MAX_TEST_FILES];

int GetExpectedIndex(const char* fileName, FilterResultExpect* results)
{
    for (size_t index = 0; results[index].fileName != NULL; index++)
    {
        if (strcmp(fileName, results[index].fileName) == 0)
        {
            return index;
        }
    }
    return -1;
}
/// <summary>
/// This is no longer required. "filterTitles" gets generated automatically
/// </summary>
/// <param name="score"></param>
/// <param name="titleExp"></param>
void SetupAddTitleExp(struct ScoringCompositeScore* score, const char* titleExp)
{
    if (titleExp == NULL || titleExp[0] == 0)
    {
        printf("Assert false : trying to add title value(for relevant experience) that is NULL or zero length\n");
        return;
    }
    // this is required for relevantExperience
    kv_pushp(SearchedString, score->filterTitles);
    SearchedString* ftitle = &kv_A(score->filterTitles, kv_size(score->filterTitles) - 1);
    initSearchedString(ftitle, (char*)titleExp);
}

void SetupAddFilter(struct ScoringCompositeScore* score, const char* filterType, const char *p_textValue)
{
    if (filterType == NULL)
    {
        return;
    }
    kv_pushp(struct SearchFilter, companyRole.filters);
    struct SearchFilter* filter = filter = &kv_A(companyRole.filters, kv_size(companyRole.filters) - 1); // get the last filter( the one we just added )
    initSearchFilter(filter);
    filter->name = strdup(filterType);
    if (p_textValue != NULL)
    {
        filter->textValue = strdup(p_textValue);
    }
    const char* p_codeValue = p_textValue; // late addition to testing. Could not find a reason why textValue and codeValue are not the one and the same
    if (p_codeValue != NULL)
    {
        filter->codeValue = strdup(p_codeValue);
    }
}

void SetupFilterGeneric(struct ScoringCompositeScore* score, const char *filterType, const char* title, const char* titleExp)
{
    initScoringCompositeScore(score);
    score->companyRole = &companyRole;
    initSearchCriteria(&companyRole);

    companyRole.role = 1;

    // At this point there is no proper similarity testing. Will need to add new test
    for (unsigned int i = 0; i < profilesLoadedCount && i < MAX_TEST_FILES; i++)
    {
        struct ProfileCached* prof = &cachedProfiles[i];
        for (size_t posIndex = 0; posIndex < kv_size(prof->positions); posIndex++)
        {
            struct PositionCached* pos = &kv_A(prof->positions, posIndex);
            if (pos->companyId >= MAX_COMPANY_ID_ALLOWED)
            {
                continue;
            }
            addScoreToMap(&companyRole.similarityScores.companyScores, pos->companyId, 5000);
            assert_float_equal(0.5, GetCompanySimilarityScore(pos->companyId, &companyRole.similarityScores) , 4);

            // add scores for parent company
            if (cachedCompanies[pos->companyId] != NULL)
            {
                struct CompanyCached *comp = cachedCompanies[pos->companyId];
                addScoreToMap(&companyRole.similarityScores.companyScores, comp->parentId, 5000);
                assert_float_equal(0.5, GetCompanySimilarityScore(comp->parentId, &companyRole.similarityScores), 4);
            }
        }
    }
    // also add scores for industries
    for (unsigned int i = 0; i < profilesLoadedCount && i < MAX_TEST_FILES; i++)
    {
        struct ProfileCached* prof = &cachedProfiles[i];
        for (size_t posIndex = 0; posIndex < kv_size(prof->positions); posIndex++)
        {
            struct PositionCached* pos = &kv_A(prof->positions, posIndex);
            if (pos->companyId >= MAX_COMPANY_ID_ALLOWED)
            {
                continue;
            }
            if (cachedCompanies[pos->companyId] != NULL)
            {
                struct CompanyCached* comp = cachedCompanies[pos->companyId];
                for (size_t j = 0; j < kv_size(comp->parentIndustryIds); j++)
                {
                    addScoreToMap(&companyRole.similarityScores.industryScores, kv_A(comp->parentIndustryIds, j), 6000);
                    assert_float_equal(0.6, GetIndustrySimilarityScore(kv_A(comp->parentIndustryIds, j), &companyRole.similarityScores), 4);
                }
                int32_t parentId = cachedCompanies[pos->companyId]->parentId;
                if (parentId < MAX_COMPANY_ID_ALLOWED && cachedCompanies[parentId] != NULL)
                {
                    struct CompanyCached* comp = cachedCompanies[parentId];
                    for (size_t j = 0; j < kv_size(comp->parentIndustryIds); j++)
                    {
                        addScoreToMap(&companyRole.similarityScores.industryScores, kv_A(comp->parentIndustryIds, j), 7000);
                        assert_float_equal(0.7, GetIndustrySimilarityScore(kv_A(comp->parentIndustryIds, j), &companyRole.similarityScores), 4);
                    }
                }
            }
        }
    }
    // also add scores for titles
    unsigned short nextTitleScore = 7600; // random whatever, but test results will be based on this
    for (unsigned int i = 0; i < profilesLoadedCount && i < MAX_TEST_FILES; i++)
    {
        struct ProfileCached* prof = &cachedProfiles[i];
        for (size_t posIndex = 0; posIndex < kv_size(prof->positions); posIndex++)
        {
            struct PositionCached* pos = &kv_A(prof->positions, posIndex);
            if (pos->parentTitletId == 0)
            {
                continue;
            }
            if (GetTitleSimilarityScore(prof->id, pos->parentTitletId, &companyRole.similarityScores) != 0)
            {
                continue;
            }
            addScoreToMap(&companyRole.similarityScores.titleScores, pos->parentTitletId, nextTitleScore);
            assert_float_equal((float)nextTitleScore / 10000, GetTitleSimilarityScore(prof->id, pos->parentTitletId, &companyRole.similarityScores), 4);
            nextTitleScore += 100; // avid same value over and over again
            nextTitleScore = nextTitleScore % 10000; // avoid overflow
        }
    }
    score->companyCachedList = (const struct CompanyCached**)cachedCompanies;
    score->companyCachedLargestId = MAX_COMPANY_ID_ALLOWED - 1;

    SetupAddFilter(score, filterType, title);

    if (titleExp != NULL)
    {
        SetupAddTitleExp(score, titleExp);
    }
}

void FinishFilterSetup(struct ScoringCompositeScore* score)
{
    // this exists because search criteria organizationID got added after the tests were created
    if (score->filter.filterCompanyID != 0 && companyRole.organizationID == 0)
    {
        companyRole.organizationID = score->filter.filterCompanyID;
    }
    int addProfileLocality = (kv_size(companyRole.localities) == 0);
    // because locality check has been added after scoring tests have been made
    if (addProfileLocality)
    {
        for (unsigned int i = 0; i < profilesLoadedCount && i < MAX_TEST_FILES; i++)
        {
            kv_push(int32_t, companyRole.localities, cachedProfiles[i].localityId);
        }
    }
    convertLocalitiesToBitfield(&companyRole);
    // this will initialize filters : convert strings to numeric values
    // to obtain same scoring, experience and filtering values over time, this need to be static in both scoring and filtering
    setScoringCompositeScoreSearchCriteria(score, &companyRole, StampWhenScoreValuesGotHardCoded);
}

void SetupFilterTitle(struct ScoringCompositeScore* score, const char* title, const char* titleExp)
{
    SetupFilterGeneric(score, "current_previous_title_include", title, titleExp);
}

static void RungenericTest(ScoringCompositeScore* score, FilterResultExpect* results)
{
    // this should have been called at the end of every setup, but since it's a late addition, it's ok to have it here
    FinishFilterSetup(score);

    for (unsigned int i = 0; results[i].fileName != NULL; i++)
    {
        results[i].testPerformed = 0;
        results[i].testPassed = 1; // untested files are presumed to pass the test
    }
    for (unsigned int i = 0; i < profilesLoadedCount && i < MAX_TEST_FILES; i++)
    {
        int expectedResultIndex = GetExpectedIndex(fileNames[i], results);
        if (expectedResultIndex < 0)
        {
            continue; // this file does not take part of the test
        }

        score->profile = &cachedProfiles[i];

        score->filterResult = 0; // make sure to reset value
        memset(&score->scores, 0, sizeof(score->scores));

        int scoringRes = runCompare(score);

        results[expectedResultIndex].testPerformed = 1;
        if (scoringRes != 0)
        {
            results[expectedResultIndex].obtainedFilterResult = 0;
        }
        else
        {
            results[expectedResultIndex].obtainedFilterResult = score->filterResult;
        }

        int printedSomething = 0;
        // we expect that scoring prepared the filter for us
        if (scoringRes == 0)
        {
            int explainResult = ProfileValid_explain(&score->filter);

            if (explainResult != results[expectedResultIndex].obtainedFilterResult)
            {
                printf("ASSERT FALSE : For func %s, file %s, fast filter and explain filter yielded different results\n",
                    __FUNCTION__, fileNames[i]);
                printedSomething = 1;
                AnyErrorsReported++;
            }
            if (results[expectedResultIndex].explainStr != NULL)
            {
                const char* jsonStr = NULL;
                struct json_object* filterExplainedArray = NULL;
                if (kv_size(score->filter.filterExplained) > 0)
                {
                    filterExplainedArray = json_object_new_array_ext(kv_size(score->filter.filterExplained));
                    for (size_t index = 0; index < kv_size(score->filter.filterExplained); index++)
                    {
                        json_object_array_add(filterExplainedArray, marshallSearchFilterExplain(&kv_A(score->filter.filterExplained, index)));
                    }

                    jsonStr = json_object_to_json_string_ext(filterExplainedArray, JSON_C_TO_STRING_SPACED);
                }
                if( jsonStr == NULL )
                {
                    printf("For func %s, file %s, we expected explain result \n'%s' and got \n' NULL \n", __FUNCTION__, fileNames[i], results[expectedResultIndex].explainStr);
                    printedSomething = 1;
                    AnyErrorsReported++;
                }
                else if (strcmp(results[expectedResultIndex].explainStr, jsonStr) != 0)
                {
                    printf("For func %s, file %s, we expected explain result \n'%s' and got \n'%s'\n", __FUNCTION__, fileNames[i], results[expectedResultIndex].explainStr, jsonStr);
                    printedSomething = 1;
                    AnyErrorsReported++;
                }
                if (filterExplainedArray != NULL)
                {
                    json_object_put(filterExplainedArray);
                }
            }
            for (size_t index = 0; index < kv_size(score->filter.filterExplained); index++)
            {
                freeSearchFilterExplain(&kv_A(score->filter.filterExplained, index));
            }
            kv_destroy(score->filter.filterExplained);
            kv_init(score->filter.filterExplained);
        }
        if (score->filterResult != results[expectedResultIndex].filterResult)
        {
            printf("For func %s, file %s, we expected filter result %d and got %d\n", __FUNCTION__, fileNames[i], results[expectedResultIndex].filterResult, score->filterResult);
            printedSomething = 1;
            AnyErrorsReported++;
        }
        // Since relevant experience will keep increasing from one day to another, no fixed value can be used to check if value is correct
        // relevant experience for current position is calculated based on time passed from the moment the test is executed
        if ((int)(score->scores.relevantExperience*100) != (int)(results[expectedResultIndex].relevantExperience*100))
        {
            printf("ASSERT FALSE : For func %s, file %s, we expected experience result %f and got %f\n", __FUNCTION__, fileNames[i], results[expectedResultIndex].relevantExperience, score->scores.relevantExperience);
            printedSomething = 1;
            AnyErrorsReported++;
        }
        if ((int)(score->scores.companyScore * 100) != (int)(results[expectedResultIndex].companyScore * 100))
        {
            printf("ASSERT FALSE : For func %s, file %s, we expected companyscore result %f and got %f\n", __FUNCTION__, fileNames[i], results[expectedResultIndex].companyScore, score->scores.companyScore);
            printedSomething = 1;
            AnyErrorsReported++;
        }
        if ((int)(score->scores.jobTitleScore * 100) != (int)(results[expectedResultIndex].titleScore * 100)
            && results[expectedResultIndex].titleScore > -1)
        {
            printf("ASSERT FALSE : For func %s, file %s, we expected title result %f and got %f\n", __FUNCTION__, fileNames[i], results[expectedResultIndex].titleScore, score->scores.jobTitleScore);
            printedSomething = 1;
            AnyErrorsReported++;
        }
        if ((int)(score->scores.skillsScore * 100) != (int)(results[expectedResultIndex].skillScore * 100)
            && results[expectedResultIndex].skillScore > -1)
        {
            printf("ASSERT FALSE : For func %s, file %s, we expected skill result %f and got %f\n", __FUNCTION__, fileNames[i], results[expectedResultIndex].skillScore, score->scores.skillsScore);
            printedSomething = 1;
            AnyErrorsReported++;
        }
        if (printedSomething != 0)
        {
            PrintFilter(&score->filter, FI_BASIC | FI_FILTER_TEXTS | FI_EXTENDED | FI_PROJECT_IDS | FI_SEPARATION_BAR);
//            PrintProfile(score->profile, PI_BASIC | PI_POSITIONS | PI_EXTENDED | PI_SEPARATION_BAR);
            PrintProfile(score->profile, PI_BASIC | PI_POSITIONS | PI_SEPARATION_BAR);
            printf("\n");
            results[expectedResultIndex].testPassed = 0;
        }
        else
        {
            results[expectedResultIndex].testPassed = 1;
        }
    }

    // make sure all the tests have been executed. Do not consider missing files as passed tests
    for (unsigned int i = 0; results[i].fileName != NULL; i++)
    {
        if (results[i].testPerformed == 0)
        {
            printf("ASSERT FALSE : file name '%s' is missing for testing\n", results[i].fileName);
        }
    }
}

int GetFileNameIndex(const char* fileName)
{
    for (unsigned int i = 0; i < profilesLoadedCount; i++)
    {
        if (strcmp(fileNames[i], fileName) == 0)
        {
            return i;
        }
    }
    return -1;
}

void RunFilterTitleTests()
{
    if (AnyErrorsReported == 0)
    {
        printf("No assertion errors were found while running the tests\n");
    }
}

static int resultWriteRow = 0;
static void SetExpectedResultValuesAtIndex(const char *fileName, int filterResult, float relevantExperience, float companyScore)
{
    if (resultWriteRow < 0 || resultWriteRow > MAX_TEST_FILES)
    {
        return;
    }
    results[resultWriteRow].fileName = fileName;
    results[resultWriteRow].filterResult = filterResult;
    results[resultWriteRow].relevantExperience = relevantExperience;
    results[resultWriteRow].companyScore = companyScore;
    resultWriteRow++;
}


static void SetExpectedResultValuesAtIndex3(const char* fileName, int filterResult, float relevantExperience, 
    float companyScore, float titleScore, float skillScore)
{
    if (resultWriteRow < 0 || resultWriteRow > MAX_TEST_FILES)
    {
        return;
    }
    results[resultWriteRow].titleScore = titleScore;
    results[resultWriteRow].skillScore = skillScore;
    SetExpectedResultValuesAtIndex(fileName, filterResult, relevantExperience, companyScore);
}

static void SetExpectedFilterExplain(const char* explainStr)
{
    if (resultWriteRow <= 0 || resultWriteRow > MAX_TEST_FILES)
    {
        return;
    }
    results[resultWriteRow - 1].explainStr = explainStr;
}

void ResetExpectedResultsForNewTest()
{
    memset(results, 0, sizeof(results));
    resultWriteRow = 0;
    // do not test title scores unless explicitly requested
    // because I do not have the time to update every old test
    for (size_t i = 0; i < MAX_TEST_FILES; i++)
    {
        results[i].titleScore = -2;
        results[i].skillScore = -2;
    }
}

int test_FilterName_1_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_include", "Bryan", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"name_include\", \"text_value\": "
        "\"bryan\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterName_2_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_include", "Fucetola", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"name_include\", \"text_value\": "
        "\"fucetola\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterNameInclude_3(void** state)
{
    SetupFilterGeneric(&score, "name_include", "\"uce\"#,#\"Tom rowinski\"", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.362117);
    return 0;
}

int test_FilterName_3_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_exclude", "Fucetola", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    return 0;
}

int test_FilterName_4_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_include", "Fucetola#,#Tom", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"name_include\", \"text_value\": "
        "\"fucetola\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    SetExpectedFilterExplain("[ { \"filter\": \"name_include\", \"text_value\": "
        "\"tom\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterName_5_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_exclude", "Fucetola#,#Tom", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 1, 0, 0.500000f);
    return 0;
}

int test_FilterName_tearDown(void** state)
{
    freeScoringCompositeScore(&score);
    freeSearchCriteria(&companyRole);
    return 0;
}

void test_FilterAndScore_succeeds(void** state)
{
    RungenericTest(&score, results);
    for (size_t index = 0; results[index].fileName != NULL; index++)
    {
        assert_true(results[index].testPassed == 1);
    }
}

int test_FilterTitle_setUp(void** state)
{
    SetupFilterTitle(&score, "Supply chain", "Supply chain");
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 27.879452f, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"supply#,#chain\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterTitleCapsMulti_setUp(void** state)
{
    SetupFilterTitle(&score, "account#,#chain", "account");
    SetupAddTitleExp(&score, "chain");
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 47.967125f, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"chain#,#account\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 118.224655f, 0.363077f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"account\" } ]");
    return 0;
}

int test_FilterTitleCorrected_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_previous_title_include", "\"director industry engagement\"#,#\"sales account executive\"#,#", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 57.238358f, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"director industry engagement#,#sales account executive\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 118.224655f, 0.362117f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"sales account executive\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 1, 248.383560f, 0.500000f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"sales account executive\" } ]");
    return 0;
}

int test_FilterCompany1_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_cmp_include", "GS1 US", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 1.000000f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_cmp_include\", \"text_value\": "
        "\"gs1#,#us\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterCompany2_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_cmp_exclude", "GS1 US", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    return 0;
}

int test_FilterCompany3_setUp(void** state)
{
    SetupFilterGeneric(&score, "previous_cmp_include", "GS1 US", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterCompany4_setUp(void** state)
{
    SetupFilterGeneric(&score, "previous_cmp_include", "Ferrero", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 1.000000f);
    SetExpectedFilterExplain("[ { \"filter\": \"previous_cmp_include\", \"text_value\": "
        "\"ferrero\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterCompany5_setUp(void** state)
{
    SetupFilterGeneric(&score, "previous_cmp_exclude", "Ferrero", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    return 0;
}

int test_FilterCompany6_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_previous_cmp_include", "GS1 US#,#Ferrero#,#Ace Part", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 1.000000f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_cmp_include\", \"text_value\": "
        "\"gs1#,#us#,#ferrero\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 1.000000f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_cmp_include\", \"text_value\": "
        "\"ace#,#part\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterCompany7_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_previous_cmp_exclude", "GS1 US#,#Ferrero#,#Ace Part", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 1, 0, 0.500000f);
    return 0;
}

int test_FilterKeywords_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "Engagement", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"engagement\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterKeywords2_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "Engagement#,#network#,#20+#,#Ups Freight", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"engagement#,#network\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 1, 0, 0.500000f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"network\" } ]");
    SetExpectedResultValuesAtIndex("7.json", 1, 0, 0.425406f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"20+#,#ups\" } ]");
    SetExpectedResultValuesAtIndex("8.json", 1, 0, 0.475946f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"ups#,#freight\" } ]");
    return 0;
}

int test_FilterKeywordsTitle_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "college1 financial1 representative1", NULL);
    SetupAddFilter(&score, "current_previous_title_include", "college1 financial1 representative1");
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 21.961643, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"college1#,#financial1#,#representative1\" }, { \"filter\": \"current_previous_title_include\", "
        "\"text_value\": \"college1#,#financial1#,#representative1\" } ]");
    return 0;
}

int test_FilterKeywordsCompany_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "Ferrero", NULL);
    SetupAddFilter(&score, "previous_cmp_include", "Ferrero");
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0.000000, 1.000000f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": \"ferrero\" }, "
        "{ \"filter\": \"previous_cmp_include\", \"text_value\": \"ferrero\" } ]");
    return 0;
}

int test_FilterKeywordsSkills_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "market1", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0.000000, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": \"market1\" } ]");
    return 0;
}

int test_FilterKeywordsHeadline_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "Director, Industry Engagement (Supply Chain)", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0.000000, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": \"director#,#industry#,#engagement#,#supply#,#chain\" } ]");
    return 0;
}

int test_FilterKeywordsSummary_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "advisory1", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0.000000, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": \"advisory1\" } ]");
    return 0;
}

int test_FilterKeywordsDescription_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "insurance1", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0.000000, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": \"insurance1\" } ]");
    return 0;
}

int test_FilterExperience_setUp(void** state)
{
    // not good : we are changing profile data. Right now these values are not calculated and they would be always 0
    for (unsigned int i = 0; i < profilesLoadedCount; i++)
    {
        if (strcmp(fileNames[i], "1.json") == 0)
        {
            cachedProfiles[i].totalExperienceMonths = 1 * 12;
        }
        if (strcmp(fileNames[i], "6.json") == 0)
        {
            cachedProfiles[i].totalExperienceMonths = 6 * 12;
        }
        if (strcmp(fileNames[i], "8.json") == 0)
        {
            cachedProfiles[i].totalExperienceMonths = 8 * 12;
        }
    }

    SetupFilterGeneric(&score, "experience", NULL, NULL);
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, 0);
    crf->rangeLow = 5 * 12;
    crf->rangeHigh = 7 * 12;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("6.json", 1, 0, 0.374898f);
    SetExpectedFilterExplain("[ { \"filter\": \"experience\", \"text_value\": "
        "\"72\" } ]");
    SetExpectedResultValuesAtIndex("8.json", 0, 0, 0);
    return 0;
}

int test_FilterTenure1_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_tenure", NULL, NULL);
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, 0);
    crf->rangeLow = 47; // value will depend on hadcoded clock value
    crf->rangeHigh = 55; // value will depend on hadcoded clock value
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_tenure\", \"text_value\": "
        "\"52\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterTenure2_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_tenure", NULL, NULL);
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, 0);
    crf->rangeLow = 145;
    crf->rangeHigh = 155;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterTenure3_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_tenure", NULL, NULL);
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, 0);
    crf->rangeLow = 1;
    crf->rangeHigh = 2;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterRelevantExperience_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_previous_title_include", NULL, "Director");

    SetupAddFilter(&score, "current_previous_title_include", "Customer");
    SetupAddFilter(&score, "current_previous_title_include", "sales");
    SetupAddFilter(&score, "current_previous_title_include", "account");

    SetupAddFilter(&score, "relevant_experience", NULL);
    struct SearchFilter* crf;
    for (size_t index = 0; index < kv_size(score.companyRole->filters); index++)
    {
        crf = &kv_A(score.companyRole->filters, index);
        crf->rangeLow = 10 * 12; // will be divided by 12
        crf->rangeHigh = 15 * 12;
    }

    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 136.241089f, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"customer\" }, "
        "{ \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"sales\" }, "
        "{ \"filter\": \"current_previous_title_include\", \"text_value\": "
        "\"account\" }, "
        "{ \"filter\": \"relevant_experience\", \"text_value\": "
        "\"136\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 118.224655f, 0);
    SetExpectedResultValuesAtIndex("3.json", 0, 248.383560f, 0);
    return 0;
}

int test_FilterTotalExperience1_setUp(void** state)
{
    SetupFilterGeneric(&score, "total_experience", NULL, NULL);
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, 0);
    crf->rangeLow = 5 * 12; 
    crf->rangeHigh = 11 * 12;

    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0); // exp = 1
    SetExpectedResultValuesAtIndex("6.json", 1, 0, 0.374898f); // exp = 6
    SetExpectedFilterExplain("[ { \"filter\": \"total_experience\", \"text_value\": "
        "\"72\" } ]");
    SetExpectedResultValuesAtIndex("9.json", 0, 0, 0); // exp = 12
    return 0;
}

int test_FilterProject1_setUp(void** state)
{
    SetupFilterGeneric(&score, "projects_include", "594", NULL);

    score.filter.filterCompanyID = 17;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"projects_include\", \"text_value\": "
        "\"594\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    SetExpectedFilterExplain("[ { \"filter\": \"projects_include\", \"text_value\": "
        "\"594\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterProject2_setUp(void** state)
{
    SetupFilterGeneric(&score, "projects_exclude", "594", NULL);

    score.filter.filterCompanyID = 17;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 1, 0, 0.500000f);
    return 0;
}

int test_FilterGroup1_setUp(void** state)
{
    SetupFilterGeneric(&score, "groups_include", "847", NULL);

    score.filter.filterCompanyID = 17;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"groups_include\", \"text_value\": "
        "\"847\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    SetExpectedFilterExplain("[ { \"filter\": \"groups_include\", \"text_value\": "
        "\"847\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterGroup2_setUp(void** state)
{
    SetupFilterGeneric(&score, "groups_exclude", "847", NULL);

    score.filter.filterCompanyID = 17;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 1, 0, 0.500000f);
    return 0;
}

int test_FilterReply1_setUp(void** state)
{

    // not good : we are changing profile data. Right now these values are not calculated and they would be always 0
    {
        int profileIndex = GetFileNameIndex("2.json");
        if (profileIndex < 0)
        {
            assert_false(0);
            return 1;
        }

        time_t dbStamp = StampWhenScoreValuesGotHardCoded - 5 * 24 * 60 * 60;
        struct Id_TimeValue* tv = NULL;

        kv_pushp(struct Id_TimeValue, cachedProfiles[profileIndex].lastReplied);
        kv_pushp(struct Id_TimeValue, cachedProfiles[profileIndex].lastPositiveReply);

        tv = &kv_A(cachedProfiles[profileIndex].lastReplied, 0);
        tv->id = 17; // company ID
        tv->value = dbStamp;

        tv = &kv_A(cachedProfiles[profileIndex].lastPositiveReply, 0);
        tv->id = 17; // company ID
        tv->value = dbStamp;

        kv_pushp(struct Id_TimeValue, cachedProfiles[profileIndex].lastReplied);
        kv_pushp(struct Id_TimeValue, cachedProfiles[profileIndex].lastPositiveReply);

        tv = &kv_A(cachedProfiles[profileIndex].lastReplied, 1);
        tv->id = 112345; // company ID
        tv->value = dbStamp;

        tv = &kv_A(cachedProfiles[profileIndex].lastPositiveReply, 1);
        tv->id = 112345; // company ID
        tv->value = dbStamp;
    }

    SetupFilterGeneric(&score, "reply_filter", "847", NULL); // needs a project ID
    SetupAddFilter(&score, "projects_include", "594");

    score.filter.replyInDays = 1;
    score.filter.filterCompanyID = 17;
    ResetExpectedResultsForNewTest();
    // these will not trigger. There is another test that should trigger
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterReply2_setUp(void** state)
{
    SetupFilterGeneric(&score, "reply_filter", "847", NULL); // needs a project ID
    SetupAddFilter(&score, "projects_include", "594");

    score.filter.replyInDays = 6;
    score.filter.filterCompanyID = 17;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.363077f);
    SetExpectedFilterExplain("[ { \"filter\": \"reply_filter\", \"text_value\": "
        "\"2021-12-02T13:47:57UTC\" }, "
        "{ \"filter\": \"projects_include\", \"text_value\": "
        "\"594\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_FilterMulti_setUp(void** state)
{
    SetupFilterTitle(&score, "account#,#chain", "account");
    SetupAddTitleExp(&score, "chain");
    SetupAddFilter(&score, "keywords", "Engagement#,#network#,#20+#,#Ups Freight");
    SetupAddFilter(&score, "name_include", "Bryan");

    SetupAddFilter(&score, "current_tenure", NULL);
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, kv_size(score.companyRole->filters) - 1);
    crf->rangeLow = 45;
    crf->rangeHigh = 55;

    SetupAddFilter(&score, "projects_include", "594");
    score.filter.filterCompanyID = 17;

    SetupAddFilter(&score, "groups_include", "847");
    SetupAddFilter(&score, "bad_filter_name", ""); // should get removed
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 47.9671250f, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_title_include\", "
        "\"text_value\": \"chain#,#account\" }, { \"filter\": \"keywords\", \"text_value\": "
        "\"engagement#,#network\" }, { \"filter\": \"name_include\", "
        "\"text_value\": \"bryan\" }, "
        "{ \"filter\": \"current_tenure\", \"text_value\": \"52\" }, { \"filter\": \"projects_include\", "
        "\"text_value\": \"594\" }, "
        "{ \"filter\": \"groups_include\", \"text_value\": \"847\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0.0f, 0);
    return 0;
}

int test_FilterCompanyScore_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "a", NULL);    // any filters just to force a result
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f); 
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"a\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.362117f);     
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"a\" } ]");
    return 0;
}

int test_FilterIndustry_setUp(void** state)
{
    SetupFilterGeneric(&score, "industry", "62#,#284,#,#92", NULL);    
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f); // list of industries are expected to match
    SetExpectedFilterExplain("[ { \"filter\": \"industry\", \"text_value\": "
        "\"62#,#284#,#92\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0); // industry list is different
    return 0;
}

int test_FilterLocality_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_exclude", "62781236021436", NULL); 
    kv_push(int32_t, companyRole.localities, 140352);

    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0); // has locality 124111
    return 0;
}


int test_FilterLocalityLargeSet_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_exclude", "62781236021436", NULL); 
    kv_push(int32_t, companyRole.localities, 140352);
    for (size_t i = 1; i < 10000; i++)
    {
        kv_push(int32_t, companyRole.localities, 140352 + i);
    }
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0); // has locality 124111
    return 0;
}

int test_FilterCompanyFunctionSet_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_previous_cmp_fn_include", "seed#,#financing#,#sEriEs b", NULL); 
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f); // 25075.json contains 'seed'
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_cmp_fn_include\", \"text_value\": "
        "\"seed\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0); 
    return 0;
}

int test_FilterCompanyNRE_setUp(void** state)
{
    SetupFilterGeneric(&score, "current_previous_cmp_nre_include", NULL, NULL); 
    struct SearchFilter* crf = &kv_A(score.companyRole->filters, 0);
    crf->rangeLow = 10;
    crf->rangeHigh = 20;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f); // 25075.json contains '1-10'
    SetExpectedFilterExplain("[ { \"filter\": \"current_previous_cmp_nre_include\", \"text_value\": "
        "\"10\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0); // 
    return 0;
}

int test_FilterBoolean1_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "EngaGemEnt", NULL);
    SetupAddFilter(&score, "keyword_boolean_filter", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f);
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"engagement\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    return 0;
}

int test_FilterBoolean2_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "EngaGemEnt or \"demOnstrated   history\" oR A+E   NetWorkS ", NULL);
    SetupAddFilter(&score, "keyword_boolean_filter", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f); // EngaGemEnt
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"engagement\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 1, 0, 0.362117f); // demOnstrated history
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"demonstrated history\" } ]");
    SetExpectedResultValuesAtIndex("3.json", 1, 0, 0.500000f); // A+E NetWorkS
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"a+e networks\" } ]");
    return 0;
}

int test_FilterBoolean3_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "EngaGemEnt anD (SuPPly Chain) aNd over 10  Years and ise membe", NULL);
    SetupAddFilter(&score, "keyword_boolean_filter", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex("1.json", 1, 0, 0.390376f); // 
    SetExpectedFilterExplain("[ { \"filter\": \"keywords\", \"text_value\": "
        "\"engagement#,#supply chain#,#over 10 years#,#ise membe\" } ]");
    SetExpectedResultValuesAtIndex("2.json", 0, 0, 0);
    SetExpectedResultValuesAtIndex("3.json", 0, 0, 0);
    return 0;
}

int test_CountryIdInclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "country_include", "1#,#2", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 0.700000f);
    SetExpectedFilterExplain("[ { \"filter\": \"country_include\", \"text_value\": "
        "\"1\" } ]");
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 0);
    SetExpectedFilterExplain("[ { \"filter\": \"country_include\", \"text_value\": "
        "\"1\" } ]");
    return 0;
}

int test_CountryIdExclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "country_exclude", "1#,#2", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 0, 0, 0, 0, 0);
    SetExpectedResultValuesAtIndex3("2.json", 0, 0, 0, 0, 0);
    return 0;
}

int test_SkillScore1_setUp(void** state)
{
    SetupFilterGeneric(&score, "name_exclude", "test_SkillScore1_setUp", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 0.700000f);
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 0);
    return 0;
}

int test_SkillScore2_setUp(void** state)
{
    SetupFilterGeneric(&score, "keywords", "a#,#b#,#c#,#d#,#test_SkillScore2_setUp", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 1);
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 1);
    return 0;
}

int test_ProfileIdInclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "linkedin_include", "1#,#2", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 0.700000f);
    SetExpectedFilterExplain("[ { \"filter\": \"linkedin_include\", \"text_value\": "
        "\"1\" } ]");
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 0);
    SetExpectedFilterExplain("[ { \"filter\": \"linkedin_include\", \"text_value\": "
        "\"2\" } ]");
    SetExpectedResultValuesAtIndex3("3.json", 0, 0, 0, 0, 0);
    return 0;
}

int test_ProfileIdExclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "linkedin_exclude", "1#,#2", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 0, 0, 0, 0, 0);
    SetExpectedResultValuesAtIndex3("2.json", 0, 0, 0, 0, 0);
    SetExpectedResultValuesAtIndex3("3.json", 1, 0, 0.500000f, 0.070000f, 0);
    return 0;
}

int test_StateIdInclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "state_include", "1#,#2", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 0.700000f);
    SetExpectedFilterExplain("[ { \"filter\": \"state_include\", \"text_value\": "
        "\"2\" } ]");
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 0);
    SetExpectedFilterExplain("[ { \"filter\": \"state_include\", \"text_value\": "
        "\"2\" } ]");
    return 0;
}

int test_StateIdExclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "state_exclude", "1#,#2", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 0, 0, 0, 0, 0);
    SetExpectedResultValuesAtIndex3("2.json", 0, 0, 0, 0, 0);
    return 0;
}

int test_CampaignInclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "campaigns_include", "1", NULL);
    score.filter.filterCompanyID = 1;

    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 0.700000f);
    SetExpectedFilterExplain("[ { \"filter\": \"campaigns_include\", \"text_value\": "
        "\"1\" } ]");
    SetExpectedResultValuesAtIndex3("2.json", 0, 0, 0, 0, 0);
    return 0;
}

int test_CampaignExclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "campaigns_exclude", "1", NULL);
    score.filter.filterCompanyID = 1;
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 0, 0, 0, 0, 0);
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 0);
    return 0;
}

int test_TalentPoolInclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "talentpools_include", "81", NULL);

    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 1, 0, 0.390376f, 0.243522f, 0.700000f);
    SetExpectedFilterExplain("[ { \"filter\": \"talentpools_include\", \"text_value\": "
        "\"81\" } ]");
    SetExpectedResultValuesAtIndex3("2.json", 0, 0, 0, 0, 0);
    return 0;
}

int test_TalentPoolExclude_setUp(void** state)
{
    SetupFilterGeneric(&score, "talentpools_exclude", "81", NULL);
    ResetExpectedResultsForNewTest();
    SetExpectedResultValuesAtIndex3("1.json", 0, 0, 0, 0, 0);
    SetExpectedResultValuesAtIndex3("2.json", 1, 0, 0.362117f, 0.059829f, 0);
    return 0;
}
