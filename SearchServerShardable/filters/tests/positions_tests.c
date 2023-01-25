#include <filters.h>
#include <profile_cached.h>
#include <position_cached.h>
#include <company_cached.h>
#include <strings_ext.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

void TestStandardize()
{
    printf("==========================\n");
    printf("test StrStandardizeTitle \n");
    char* res;
    res = StrStandardizeScoringClient(strdup("avp xvpavp avpxvp"));
    if (res)
    {
        assert_string_equal(res, "associate vice president xvpavp avpxvp");
        printf("'%s'\n", res);
        free(res);
    }
    res = StrStandardizeScoringClient(strdup(" avp "));
    if (res)
    {
        assert_string_equal(res, "associate vice president");
        printf("'%s'\n", res);
        free(res);
    }
    res = StrStandardizeScoringClient(strdup(" avp"));
    if (res)
    {
        assert_string_equal(res, "associate vice president");
        printf("'%s'\n", res);
        free(res);
    }

    res = StrStandardizeScoringClient(strdup(" vice president"));
    if (res)
    {
        assert_string_equal(res, "vice president");
        printf("'%s'\n", res);
        free(res);
    }

    res = StrStandardizeScoringClient(strdup("vice   president "));
    if (res)
    {
        assert_string_equal(res, "vice president");
        printf("'%s'\n", res);
        free(res);
    }

    res = StrStandardizeScoringClient(strdup("vice presidente"));
    if (res)
    {
        assert_string_equal(res, "vice presidente");
        printf("'%s'\n", res);
        free(res);
    }

    res = StrStandardizeScoringClient(strdup("vice.president"));
    if (res)
    {
        assert_string_equal(res, "vicepresident");
        printf("'%s'\n", res);
        free(res);
    }

    res = StrStandardizeScoringClient(strdup(" Software DEVELOPER. Experience as a COO -- 5 years --"));
    if (res)
    {
        assert_string_equal(res, "software developer experience as a chief operating officer 5 years");
        printf("'%s'\n", res);
        free(res);
    }

    res = StrStandardizeScoringClient(strdup(".\t SW engineer Lead. Looking for MGR or R&d positions -- "));
    if (res)
    {
        printf("'%s'\n", res);
        assert_string_equal(res, "sw engineer lead looking for manager or research and development positions");
        free(res);
    }

    res = StrStandardizeScoringClient(strdup("CTO and DIR openings ONLY. Worked in LA, NYC & FLA -- Considering relocation . "));
    if (res)
    {
        printf("'%s'\n", res);
        assert_string_equal(res, "chief technology officer and director openings only worked in la nyc & fla considering relocation");
        free(res);
    }

    res = StrStandardizeScoringClient(strdup("  -- R&D CTO professional. NeW tO Word. Managed "
        "projects BUDGET:: $40,000,000.00. Asking for salary, 401K + benefits !"));
    if (res)
    {
        printf("'%s'\n", res);
        assert_string_equal(res, "research and development chief technology officer professional new "
            "to word managed projects budget $4000000000 asking for salary 401k + benefits !");
        free(res);
    }

    printf("==========================\n");
}

void TestFilterTitleIncludesValues(const char **titleNames, const size_t TitleNamesCount, const char* search, const int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);

    // Settings of the filter we are testing
    if (filterType == CRFT_CURRENT_TITLE_INCLUDE)
    {
        companyRoleFilters->filter = strdup("current_title_include");
    }
    else if (filterType == CRFT_CURRENT_TITLE_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("current_title_exclude");
    }
    else if (filterType == CRFT_PREVIOUS_TITLE_INCLUDE)
    {
        companyRoleFilters->filter = strdup("previous_title_include");
    }
    else if (filterType == CRFT_PREVIOUS_TITLE_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("previous_title_exclude");
    }
    else if (filterType == CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE)
    {
        companyRoleFilters->filter = strdup("current_previous_title_include");
    }
    else if (filterType == CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("current_previous_title_exclude");
    }

    if (search != NULL)
    {
        companyRoleFilters->textValue = strdup(search);
    }

    struct ProfileCached prof;
    initProfileCached(&prof);

    for (size_t i = 0; i < TitleNamesCount; i++)
    {
        // why would you even do this ?
        if (titleNames[i] == NULL)
        {
            continue;
        }
        
        kv_pushp(struct PositionCached, prof.positions);

        struct PositionCached* pos = &(kv_A(prof.positions, i));
        if (pos == NULL)
        {
            continue; // doubt it
        }
        initPositionCached(pos);
        pos->title = strdup(titleNames[i]);        
    }

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    if (companyRoleFilters->textValueAsArray != NULL)
    {
        int filterResult = ProfileValid(&performFilter);
        printf("filter result %d \n", filterResult);
        printf("title strings :\n");
        for (size_t i = 0; i < kv_size(prof.positions); i++)
        {
            printf("\t%s\n", kv_A(prof.positions, i).title);
        }
        printf("filter strings : \n");
        for (size_t i = 0; i < companyRoleFilters->textValueArraySize; i++)
        {
//            if (companyRoleFilters->textValueAsArray[i].isQuoteEnclosed)
            {
//                printf("\t\"%s\"\n", companyRoleFilters->textValueAsArray[i].strOriginal.str);
            }
//            else
            {
                printf("\t%s\n", companyRoleFilters->textValueAsArray[i].strOriginal.str);
            }
        }
        printf("\n");
    }

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestFilterTitleIncludes()
{
    printf("==========================\n");
    printf("Start ProfileValid->current_title_include test\n");
    const char* titleList[100];

    titleList[0] = "SeNioR";
    titleList[1] = "evp";
    titleList[2] = "associate vice president";
    TestFilterTitleIncludesValues(titleList, 1, "senior", CRFT_CURRENT_TITLE_INCLUDE);
    TestFilterTitleIncludesValues(titleList, 3, "president", CRFT_CURRENT_TITLE_INCLUDE);
    TestFilterTitleIncludesValues(titleList, 3, "executive", CRFT_CURRENT_TITLE_INCLUDE);

    // check for multi word search
    TestFilterTitleIncludesValues(titleList, 1, "seni#,#jun", CRFT_CURRENT_TITLE_INCLUDE);
    TestFilterTitleIncludesValues(titleList, 1, "jun#,#vice", CRFT_CURRENT_TITLE_INCLUDE);

    printf("==========================\n");
}

void TestCompanyFilterIncludesValues(const char* companyName, const char* search, const int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);

    // Settings of the filter we are testing
    if (filterType == CRFT_CURRENT_COMPANY_INCLUDE)
    {
        companyRoleFilters->filter = strdup("current_cmp_include");
    }
    else if (filterType == CRFT_CURRENT_COMPANY_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("current_cmp_exclude");
    }
    else if (filterType == CRFT_PREVIOUS_COMPANY_INCLUDE)
    {
        companyRoleFilters->filter = strdup("previous_cmp_include");
    }
    else if (filterType == CRFT_PREVIOUS_COMPANY_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("previous_cmp_exclude");
    }
    else if (filterType == CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE)
    {
        companyRoleFilters->filter = strdup("current_previous_cmp_include");
    }
    else if (filterType == CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("current_previous_cmp_exclude");
    }

    if (search != NULL)
    {
        companyRoleFilters->textValue = strdup(search);
    }

    struct ProfileCached prof;
    initProfileCached(&prof);

    for (size_t i = 0; i < 1; i++)
    {
        // why would you even do this ?
        if (companyName != NULL)
        {
            kv_pushp(struct PositionCached, prof.positions);

            struct PositionCached* pos = &(kv_A(prof.positions, i));
            if (pos == NULL)
            {
                continue; // doubt it
            }
            initPositionCached(pos);
            pos->companyName = strdup(companyName);
        }
    }

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    if (companyRoleFilters->textValueAsArray != NULL)
    {
        int filterResult = ProfileValid(&performFilter);
        printf("filter result %d \n", filterResult);
        printf("company name strings :\n");
        for (size_t i = 0; i < kv_size(prof.positions); i++)
        {
            printf("\t%s\n", kv_A(prof.positions, i).companyName);
        }
        printf("filter strings : \n");
        for (size_t i = 0; i < companyRoleFilters->textValueArraySize; i++)
        {
//            if (companyRoleFilters->textValueAsArray[i].isQuoteEnclosed)
            {
//                printf("\t\"%s\"\n", companyRoleFilters->textValueAsArray[i].strOriginal.str);
            }
//            else
            {
                printf("\t%s\n", companyRoleFilters->textValueAsArray[i].strOriginal.str);
            }
        }
        printf("\n");
    }

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestCompanyFilterIncludes()
{
    printf("==========================\n");
    printf("Start ProfileValid->current_cmp_include test\n");

    TestCompanyFilterIncludesValues("GOOGL", "google", CRFT_CURRENT_COMPANY_INCLUDE);
    TestCompanyFilterIncludesValues("goog", "google", CRFT_CURRENT_COMPANY_INCLUDE);
    TestCompanyFilterIncludesValues("google", "oog", CRFT_CURRENT_COMPANY_INCLUDE);

    // check for multi word search
    TestCompanyFilterIncludesValues("google", "og#,#ham", CRFT_CURRENT_COMPANY_INCLUDE);
    TestCompanyFilterIncludesValues("google", "\"google\"#,#vice", CRFT_CURRENT_COMPANY_INCLUDE);
    TestCompanyFilterIncludesValues("google", "\"ham\"#,#ster", CRFT_CURRENT_COMPANY_INCLUDE);

    printf("==========================\n");
}

void TestFilterKeywordIncludesValues(const char* headline, const char *summary, const char *skill1, const char *skill2, 
    const char *title, const char *description, const char* search, const int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);

    // Settings of the filter we are testing
    if (filterType == CRFT_KEYWORDS_INCLUDE)
    {
        companyRoleFilters->filter = strdup("keywords");
    }
    else if (filterType == CRFT_KEYWORDS_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("keywords_exclude");
    }

    if (search != NULL)
    {
        companyRoleFilters->textValue = strdup(search);
    }

    struct ProfileCached prof;
    initProfileCached(&prof);
    char tempStr[5000];
    snprintf(tempStr, sizeof(tempStr), "%c%s%c%s%c%s%c%s%c%s%c", PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR,
        headline, PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR, summary, PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR,
        skill1, PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR, skill2, PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR,
        description, PROFILE_STRING_FIELD_MERGE_TOKEN_CHAR );
    prof.skillsHeadlineSummaryPosDescription = strdup(tempStr);

    kv_pushp(struct PositionCached, prof.positions);

    struct PositionCached* pos = &(kv_A(prof.positions, 0));
    if (pos == NULL)
    {
        return; // doubt it
    }
    initPositionCached(pos);
    if (title)
    {
        pos->title = strdup(title);
    }
	pos->companyName = strdup("");

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    if (companyRoleFilters->textValueAsArray != NULL)
    {
        int filterResult = ProfileValid(&performFilter);
        printf("filter result %d \n", filterResult);
        printf("title - descriptionstrings :\n");
        printf("filter strings : \n");
        for (size_t i = 0; i < companyRoleFilters->textValueArraySize; i++)
        {
//            if (companyRoleFilters->textValueAsArray[i].isQuoteEnclosed)
            {
//                printf("\t\"%s\"\n", companyRoleFilters->textValueAsArray[i].strOriginal.str);
            }
//            else
            {
                printf("\t%s\n", companyRoleFilters->textValueAsArray[i].strOriginal.str);
            }
        }
        printf("\n");
    }

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestKeywordsInclude()
{
    printf("==========================\n");
    printf("Start ProfileValid->keywords test\n");

    //simple cases to test all fields are searched and strings are initialized properly
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "dline", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "mm", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "po", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "op", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "tl", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "esc", CRFT_KEYWORDS_INCLUDE);

    // keywords are like quoted strings, but they can be substrings
    TestFilterKeywordIncludesValues("Head Line\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "d line", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sum Marry.", "pope", "bishop", "-tiTle/", "\tdescription", "mm", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiT le/", "\tdescription", "tl", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tde scription", "esc", CRFT_KEYWORDS_INCLUDE);

    // check for multi word search
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "d line#,#mm", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "m m#,#abc", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "t l#,#none", CRFT_KEYWORDS_INCLUDE);
    TestFilterKeywordIncludesValues("HeadLine\n", "sumMarry.", "pope", "bishop", "-tiTle/", "\tdescription", "e sc#,#cript", CRFT_KEYWORDS_INCLUDE);

    printf("==========================\n");
}

void TestFilterTenureValues(time_t start, time_t end, int rangeLow, int rangeHigh, const int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);

    // Settings of the filter we are testing
    if (filterType == CRFT_CURRENT_TENURE)
    {
        companyRoleFilters->filter = strdup("current_tenure");
    }

    companyRoleFilters->rangeLow = rangeLow;
    companyRoleFilters->rangeHigh = rangeHigh;

    struct ProfileCached prof;
    initProfileCached(&prof);

    kv_pushp(struct PositionCached, prof.positions);

    struct PositionCached* pos = &(kv_A(prof.positions, 0));
    initPositionCached(pos);
    pos->startDate = start;
    pos->endDate = end;

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    int filterResult = ProfileValid(&performFilter);
    printf("months experience : %d\n", (int)((end - start) / 30));
    printf("filter result %d \n", filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestTenure()
{
    printf("==========================\n");
    printf("Start ProfileValid->current_tenure test\n");

    time_t rawtime = time(NULL);

    // should be selected
    // worked 7 months
    TestFilterTenureValues(rawtime - 86400 * 30 * 7, rawtime, 4, 10, CRFT_CURRENT_TENURE);
    TestFilterTenureValues(rawtime - 86400 * 30 * 7, rawtime, 0, 10, CRFT_CURRENT_TENURE);
    TestFilterTenureValues(rawtime - 86400 * 30 * 7, rawtime, 4, 0, CRFT_CURRENT_TENURE);

    // should not be selected
    TestFilterTenureValues(rawtime - 86400 * 30 * 15, rawtime, 4, 10, CRFT_CURRENT_TENURE);
    TestFilterTenureValues(rawtime - 86400 * 30 * 15, rawtime, 0, 10, CRFT_CURRENT_TENURE);
    TestFilterTenureValues(rawtime - 86400 * 30 * 2, rawtime, 4, 0, CRFT_CURRENT_TENURE);

    printf("==========================\n");
}

void TestFilterIndustryValues(const struct CompanyCached** companies, int companyCount, const char* filter, const char* filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);
    performFilter.companyCachedList = companies;
    performFilter.companyCachedLargestId = companyCount + 1;

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);

    // Settings of the filter we are testing
    companyRoleFilters->filter = strdup(filterType);
    companyRoleFilters->textValue = strdup(filter);

    struct ProfileCached prof;
    initProfileCached(&prof);
    performFilter.profile = &prof;

    kv_pushp(struct PositionCached, prof.positions);
    struct PositionCached* pos = &kv_A(prof.positions, 0);
    initPositionCached(pos);
    pos->companyId = 1;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    int filterResult = ProfileValid(&performFilter);
    printf("Filter '%s' result %d for searched industries : %s\n", filterType, filterResult, filter);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestFilterIndustry()
{
    printf("==========================\n");
    printf("Start ProfileValid->industry test\n");

#define MAX_COMPANY_AVAILABLE 1
    struct CompanyCached* companies[10];
    int companyCount = sizeof(companies) / sizeof(struct CompanyCached*);

    // At the time of testing, I did not have access to company data. Placeholder function until we have values in DB
    memset(companies, 0, sizeof(companies));
    for (size_t companyIndex = 1; companyIndex <= MAX_COMPANY_AVAILABLE; companyIndex++)
    {
        companies[companyIndex] = malloc(sizeof(struct CompanyCached));
        initCompanyCached(companies[companyIndex]);
        companies[companyIndex]->id = companyIndex;
        companies[companyIndex]->parentId = companyIndex;
        kv_push(int32_t, companies[companyIndex]->parentIndustryIds, 1);
        kv_push(int32_t, companies[companyIndex]->parentIndustryIds, 3);
        kv_push(int32_t, companies[companyIndex]->parentIndustryIds, 5);
    }

    // should be selected
    // when I made the tests, there was 1 company which had industries : 1,3,5
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "1", "industry");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "1#,#3", "industry");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "2#,#3", "industry");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "1#,#2", "industry");

    // should not be selected
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "2", "industry");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "2#,#4", "industry");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "1", "current_industry");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "1#,#3", "industry_exclude");
    TestFilterIndustryValues((const struct CompanyCached**)&companies, companyCount, "1#,#3", "previous_industry");

    for (size_t companyIndex = 1; companyIndex <= MAX_COMPANY_AVAILABLE; companyIndex++)
    {
        freeCompanyCached(companies[companyIndex]);
        free(companies[companyIndex]);
    }

    printf("==========================\n");
}

void PerformTitleTests()
{
    TestStandardize();
    TestFilterTitleIncludes();
    TestCompanyFilterIncludes();
    TestKeywordsInclude();
    TestTenure();
    TestFilterIndustry();
}
