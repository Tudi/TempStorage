#include <filters.h>
#include <profile_cached.h>
#include <strings_ext.h>
//#include <stdarg.h>
//#include <setjmp.h>
#include <string.h>
#include <stdio.h>

void TestInitDeinit()
{
#define FILTER_COUNT 3
#define COMPANY_FILTER_COUNT FILTER_COUNT
    PerformFilter* performFilters[FILTER_COUNT];

    // Allocate memory
    for (int i = 0; i < FILTER_COUNT; i++)
    {
        performFilters[i] = (PerformFilter*)malloc(sizeof(PerformFilter));
        initPerformFilter(performFilters[i]);
    }
    for (int i = 0; i < COMPANY_FILTER_COUNT; i++)
    {
        kv_pushp(CompanyRoleFilter, performFilters[0]->filters);
        initCompanyRoleFilter(&kv_A(performFilters[0]->filters, kv_size(performFilters[0]->filters) - 1));
    }

    // test if we deallocate everything ok
    for (int i = 0; i < FILTER_COUNT; i++)
    {
        freePerformFilter(performFilters[i]);
        free(performFilters[i]);
    }
}

void TestFilterInit()
{
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    for (int i = 0; i < COMPANY_FILTER_COUNT; i++)
    {
        kv_pushp(CompanyRoleFilter, performFilter.filters);
        initCompanyRoleFilter(&kv_A(performFilter.filters, kv_size(performFilter.filters) - 1));
    }
    (&kv_A(performFilter.filters,0))->filter = strdup("name_include");

    initPerformFilterPreSearch(&performFilter);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
}

void TestProfileInitDeinit()
{
    struct ProfileCached prof;
    initProfileCached(&prof);

    prof.fullName = strdup("John Doe of united states ");

    freeProfileCached(&prof);
}

void TestFilterNameIncludesValues(char *name, char* search, int filterType)
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
    if (filterType == CRFT_NAME_INCLUDES)
    {
        companyRoleFilters->filter = strdup("name_include");
    }
    else if (filterType == CRFT_NAME_DOES_NOT_INCLUDE)
    {
        companyRoleFilters->filter = strdup("name_exclude"); 
    }

    if (search != NULL)
    {
        companyRoleFilters->textValue = strdup(search);
    }

    struct ProfileCached prof;
    initProfileCached(&prof);

    if (name != NULL)
    {
        prof.fullName = strdup(name);
    }

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    if (companyRoleFilters->textValueAsArray != NULL)
    {
        int filterResult = ProfileValid(&performFilter);
        printf("name '%s', filter result %d, filter strings : \n", prof.fullName, filterResult);
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
    }

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestFilterNameIncludes()
{
    printf("==========================\n");
    printf("Start ProfileValid->name_include test\n");
    
    TestFilterNameIncludesValues("John Doe of  USA", "john", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "hn D", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "\"Doe\"", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "of us", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "\"of us\"", CRFT_NAME_INCLUDES);

    // check for multi word search
    TestFilterNameIncludesValues("John Doe of  USA", "\"usa\"#,#john", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "alabama#,#carrot", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "john#,#usa", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "#,#usa", CRFT_NAME_INCLUDES);
    TestFilterNameIncludesValues("John Doe of  USA", "john#,#\"\"", CRFT_NAME_INCLUDES);

    printf("==========================\n");
}

void TestFilterNameExcludes()
{
    printf("==========================\n");
    printf("Start ProfileValid->name_exclude test\n");

    TestFilterNameIncludesValues("John Doe  of  USA", "john", CRFT_NAME_DOES_NOT_INCLUDE);
    TestFilterNameIncludesValues("John Doe  of  USA", "alan", CRFT_NAME_DOES_NOT_INCLUDE);
    TestFilterNameIncludesValues("John Doe  of  USA", "\"Doe\"", CRFT_NAME_DOES_NOT_INCLUDE);
    TestFilterNameIncludesValues("John Doe  of  USA", "of us", CRFT_NAME_DOES_NOT_INCLUDE);
    TestFilterNameIncludesValues("John Doe  of  USA", "\"alan\"", CRFT_NAME_DOES_NOT_INCLUDE);

    printf("==========================\n");
}

void TestFilterMessagedRepliedValues(time_t dbStamp, int rangeHigh, const char *modifier, int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    performFilter.filterCompanyID = 1;

    // Settings of the filter we are testing
    if (filterType == CRFT_MESSAGED_NEWER_THAN)
    {
        companyRoleFilters->filter = strdup("messaged");
    }
    else if (filterType == CRFT_REPLIED_NEWER_THAN)
    {
        companyRoleFilters->filter = strdup("replied");
    }

    companyRoleFilters->rangeHigh = rangeHigh;
    if (modifier != NULL)
    {
        companyRoleFilters->modifier = strdup(modifier);
    }

    struct ProfileCached prof;
    initProfileCached(&prof);

    kv_pushp(struct Id_TimeValue, prof.lastMessaged);
    kv_pushp(struct Id_TimeValue, prof.lastReplied);

    struct Id_TimeValue* tv = NULL;
    
    tv = &kv_A(prof.lastMessaged, 0);
    tv->id = 1;
    tv->value = dbStamp;

    tv = &kv_A(prof.lastReplied, 0);
    tv->id = 1;
    tv->value = dbStamp;

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    struct tm tms;

    localtime_r(&tv->value, &tms);
    int filterResult = ProfileValid(&performFilter);
    printf("date '%d-%d-%d', range %d, filter result : %d\n",
        1900 + tms.tm_year, tms.tm_mon + 1, tms.tm_mday, rangeHigh, filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestMessagedReplied()
{
    printf("==========================\n");
    printf("Start ProfileValid->messaged + ProfileValid->replied test\n");

    time_t rawtime = time(NULL);

    // should be selected
    TestFilterMessagedRepliedValues((int)rawtime, (int)((rawtime - 86400 * 10) / 86400), "less than", CRFT_MESSAGED_NEWER_THAN);
    TestFilterMessagedRepliedValues((int)(rawtime - 10 * 86400), (int)((rawtime) / 86400), "mango", CRFT_MESSAGED_NEWER_THAN);

    // should not be selected
    TestFilterMessagedRepliedValues((int)(rawtime - 10 * 86400), (int)((rawtime) / 86400), "less than", CRFT_MESSAGED_NEWER_THAN);

    // should be selected
    TestFilterMessagedRepliedValues((int)(rawtime), (int)((rawtime - 86400 * 10) / 86400), "less than", CRFT_REPLIED_NEWER_THAN);
    TestFilterMessagedRepliedValues((int)(rawtime - 10 * 86400), (int)((rawtime) / 86400), "mango", CRFT_REPLIED_NEWER_THAN);

    // should not be selected
    TestFilterMessagedRepliedValues((int)(rawtime - 10 * 86400), (int)((rawtime) / 86400), "less than", CRFT_REPLIED_NEWER_THAN);

    printf("==========================\n");
}

void TestFilterExperienceValues(int low, int high, int searched, int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    performFilter.filterCompanyID = 1;

    // Settings of the filter we are testing
    if (filterType == CRFT_EXPERIENCE)
    {
        companyRoleFilters->filter = strdup("experience");
    }

    companyRoleFilters->rangeLow = low;
    companyRoleFilters->rangeHigh = high;

    struct ProfileCached prof;
    initProfileCached(&prof);
    prof.totalExperienceMonths = searched * 12;

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    int filterResult = ProfileValid(&performFilter);
    printf("isbetween '%d-%d-%d : %d\n", low, searched, high, filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestExperience()
{
    printf("==========================\n");
    printf("Start ProfileValid->experience test\n");

    // should be selected
    TestFilterExperienceValues(5, 10, 7, CRFT_EXPERIENCE);
    TestFilterExperienceValues(0, 10, 7, CRFT_EXPERIENCE);
    TestFilterExperienceValues(5, 0, 7, CRFT_EXPERIENCE);

    // should not be selected
    TestFilterExperienceValues(5, 10, 2, CRFT_EXPERIENCE);
    TestFilterExperienceValues(5, 10, 12, CRFT_EXPERIENCE);
    TestFilterExperienceValues(0, 10, 12, CRFT_EXPERIENCE);
    TestFilterExperienceValues(5, 0, 2, CRFT_EXPERIENCE);

    printf("==========================\n");
}

void TestFilterRelevantExperienceValues(int low, int high, int searched, int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    performFilter.filterCompanyID = 1;

    // Settings of the filter we are testing
    if (filterType == CRFT_RELEVANT_EXPERIENCE)
    {
        companyRoleFilters->filter = strdup("relevant_experience");
    }

    companyRoleFilters->rangeLow = low;
    companyRoleFilters->rangeHigh = high;
    performFilter.relevantExperience = (float)searched;

    struct ProfileCached prof;
    initProfileCached(&prof);

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    int filterResult = ProfileValid(&performFilter);
    printf("isbetween '%d-%d-%d : %d\n", low, searched * 12, high, filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
}

void TestRelevantExperience()
{
    printf("==========================\n");
    printf("Start ProfileValid->relevant_experience test\n");

    // should be selected
    TestFilterRelevantExperienceValues(5 * 12, 10 * 12, 7, CRFT_RELEVANT_EXPERIENCE);
    TestFilterRelevantExperienceValues(0 * 12, 10 * 12, 7, CRFT_RELEVANT_EXPERIENCE);
    TestFilterRelevantExperienceValues(5 * 12, 0 * 12, 7, CRFT_RELEVANT_EXPERIENCE);

    // should not be selected
    TestFilterRelevantExperienceValues(5 * 12, 10 * 12, 2, CRFT_RELEVANT_EXPERIENCE);
    TestFilterRelevantExperienceValues(5 * 12, 10 * 12, 12, CRFT_RELEVANT_EXPERIENCE);
    TestFilterRelevantExperienceValues(0 * 12, 10 * 12, 12, CRFT_RELEVANT_EXPERIENCE);
    TestFilterRelevantExperienceValues(5 * 12, 0 * 12, 2, CRFT_RELEVANT_EXPERIENCE);

    printf("==========================\n");
}

void TestFilterTotalExperienceValues(int low, int high, int searched, int filterType)
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
    if (filterType == CRFT_TOTAL_EXPERIENCE)
    {
        companyRoleFilters->filter = strdup("total_experience");
    }

    companyRoleFilters->rangeLow = low;
    companyRoleFilters->rangeHigh = high;

    struct ProfileCached prof;
    initProfileCached(&prof);
    prof.totalExperienceMonths = searched * 12;

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    int filterResult = ProfileValid(&performFilter);
    printf("isbetween '%d-%d-%d : %d\n", low, searched, high, filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestTotalExperience()
{
    printf("==========================\n");
    printf("Start ProfileValid->total_experience test\n");

    // should be selected
    TestFilterTotalExperienceValues(5 * 12, 10 * 12, 7, CRFT_TOTAL_EXPERIENCE);
    TestFilterTotalExperienceValues(0 * 12, 10 * 12, 7, CRFT_TOTAL_EXPERIENCE);
    TestFilterTotalExperienceValues(5 * 12, 0 * 12, 7, CRFT_TOTAL_EXPERIENCE);

    // should not be selected
    TestFilterTotalExperienceValues(5 * 12, 10 * 12, 2, CRFT_TOTAL_EXPERIENCE);
    TestFilterTotalExperienceValues(5 * 12, 10 * 12, 12, CRFT_TOTAL_EXPERIENCE);
    TestFilterTotalExperienceValues(0 * 12, 10 * 12, 12, CRFT_TOTAL_EXPERIENCE);
    TestFilterTotalExperienceValues(5 * 12, 0 * 12, 2, CRFT_TOTAL_EXPERIENCE);

    printf("==========================\n");
}

void TestFilterProjectValues(const int CompanyID, const int *Projects, const int ProjectsCount, const int profCompanyID, const char *filter, const int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    performFilter.filterCompanyID = CompanyID;

    // Settings of the filter we are testing
    if (filterType == CRFT_PROJECTS_INCLUDE)
    {
        companyRoleFilters->filter = strdup("projects_include");
    }
    if (filterType == CRFT_PROJECTS_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("projects_exclude");
    }

    companyRoleFilters->codeValue = strdup(filter);

    struct ProfileCached prof;
    initProfileCached(&prof);
    performFilter.profile = &prof;

    for (int i = 0; i < ProjectsCount; i++)
    {
        kv_pushp(struct Id_Int32Value, prof.projects);
        struct Id_Int32Value* val = &kv_A(prof.projects, i);
        val->id = Projects[i];
        val->value = profCompanyID;
    }

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    int filterResult = ProfileValid(&performFilter);
    printf("searched company %d project %d result : %d\n", CompanyID, Projects[0], filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestFilterProjects()
{
    printf("==========================\n");
    printf("Start ProfileValid->projects_include test\n");

    int CompanyID = 1;
    int Projects[3] = { 1,2,3 };
    // should be selected
    TestFilterProjectValues(CompanyID, Projects, 3, CompanyID, "1", CRFT_PROJECTS_INCLUDE);
    TestFilterProjectValues(CompanyID, Projects, 3, CompanyID, "2", CRFT_PROJECTS_INCLUDE);

    // should not be selected
    TestFilterProjectValues(CompanyID, Projects, 3, CompanyID + 1, "2", CRFT_PROJECTS_INCLUDE);
    TestFilterProjectValues(CompanyID, Projects, 3, CompanyID , "4", CRFT_PROJECTS_INCLUDE);

    // test multi select values
    TestFilterProjectValues(CompanyID, Projects, 3, CompanyID, "1#,#4", CRFT_PROJECTS_INCLUDE);
    TestFilterProjectValues(CompanyID, Projects, 3, CompanyID, "5#,#4", CRFT_PROJECTS_INCLUDE);
    printf("==========================\n");
}

void TestFilterGroupValues(const int CompanyID, const int* Projects, const int ProjectsCount, const int* Groups, const int GroupsCount,
    const int profCompanyID, const char* filter, const char *inProjectValues, const int filterType)
{
    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    performFilter.filterCompanyID = CompanyID;

    // Settings of the filter we are testing
    if (filterType == CRFT_GROUPS_INCLUDE)
    {
        companyRoleFilters->filter = strdup("groups_include");
    }
    if (filterType == CRFT_GROUPS_EXCLUDE)
    {
        companyRoleFilters->filter = strdup("groups_exclude");
    }

    companyRoleFilters->codeValue = strdup(filter);

    struct ProfileCached prof;
    initProfileCached(&prof);
    performFilter.profile = &prof;

    if (Projects != NULL)
    {
        for (int i = 0; i < ProjectsCount; i++)
        {
            kv_pushp(struct Id_Int32Value, prof.projects);
            struct Id_Int32Value* val = &kv_A(prof.projects, i);
            val->id = Projects[i];
            val->value = profCompanyID;
        }
    }

    if (Groups != NULL)
    {
        for (int i = 0; i < GroupsCount; i++)
        {
            kv_pushp(struct Id_Int32Value, prof.groups);
            struct Id_Int32Value* val = &kv_A(prof.groups, i);
            val->id = Groups[i];
            val->value = profCompanyID;
        }
    }

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    int filterResult = ProfileValid(&performFilter);
    printf("searched company %d group %d result : %d\n", CompanyID, Groups[0], filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestFilterGroups()
{
    printf("==========================\n");
    printf("Start ProfileValid->groups_include test\n");

    int CompanyID = 1;
    int Projects[3] = { 1, 2, 3 };
    int Groups[3] = { 4, 5, 6 };
    // should be selected
    TestFilterGroupValues(CompanyID, NULL, 3, Groups, 3, CompanyID, "4", NULL, CRFT_GROUPS_INCLUDE);
    TestFilterGroupValues(CompanyID, NULL, 3, Groups, 3, CompanyID, "5", "7", CRFT_GROUPS_INCLUDE);
    TestFilterGroupValues(CompanyID, Projects, 3, Groups, 3, CompanyID, "7", "2", CRFT_GROUPS_INCLUDE); // because it is the list of projects

    // should not be selected
    TestFilterGroupValues(CompanyID, NULL, 3, Groups, 3, CompanyID + 1, "2", NULL, CRFT_GROUPS_INCLUDE);
    TestFilterGroupValues(CompanyID, Projects, 3, Groups, 3, CompanyID, "7", NULL, CRFT_GROUPS_INCLUDE);

    // test multi select values
    TestFilterGroupValues(CompanyID, NULL, 3, Groups, 3, CompanyID, "1#,#4", NULL, CRFT_GROUPS_INCLUDE); // selected
    TestFilterGroupValues(CompanyID, Projects, 3, Groups, 3, CompanyID, "8#,#7", "9,10", CRFT_GROUPS_INCLUDE); // not selected
    printf("==========================\n");
}

void TestFilterReplyFilterValues(time_t dbStamp, int days, int posReplyModifier, const char* modifier, int filterType)
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
    if (filterType == CRFT_REPLY_FILTER)
    {
        companyRoleFilters->filter = strdup("reply_filter");
    }

    performFilter.filterCompanyID = 1;
    performFilter.replyInDays = days;
    performFilter.positiveReplyModifier = posReplyModifier;

    if (modifier != NULL)
    {
        companyRoleFilters->modifier = strdup(modifier);
    }

    struct ProfileCached prof;
    initProfileCached(&prof);

    kv_pushp(struct Id_TimeValue, prof.lastReplied);
    kv_pushp(struct Id_TimeValue, prof.lastPositiveReply);

    struct Id_TimeValue* tv = NULL;

    tv = &kv_A(prof.lastReplied, 0);
    tv->id = 1;
    tv->value = dbStamp;

    tv = &kv_A(prof.lastPositiveReply, 0);
    tv->id = 1;
    tv->value = dbStamp;

    // Peform the search
    performFilter.profile = &prof;

    // Prepare the filter before search
    initPerformFilterPreSearch(&performFilter);

    // Do an actual filtering
    struct tm tms;

    localtime_r(&tv->value, &tms);
    int filterResult = ProfileValid(&performFilter);
    printf("message date '%d-%d-%d', less than %d days ?, filter result : %d\n",
        1900 + tms.tm_year, tms.tm_mon + 1, tms.tm_mday, days, filterResult);

    // Free filter session and all it's sub filters
    freePerformFilter(&performFilter);
    freeProfileCached(&prof);
}

void TestFilterReplyFilter()
{
    printf("==========================\n");
    printf("Start ProfileValid->reply_filter test\n");

    time_t rawtime = time(NULL);

    // should be selected
    TestFilterReplyFilterValues((int)rawtime, 10, 0, "", CRFT_REPLY_FILTER);
    TestFilterReplyFilterValues((int)(rawtime - 10 * 86400), 15, 1, "", CRFT_REPLY_FILTER);

    // should not be selected
    TestFilterReplyFilterValues((int)(rawtime - 10 * 86400), 5, 1, "", CRFT_REPLY_FILTER);
    TestFilterReplyFilterValues((int)rawtime, 10, 0, "nOt", CRFT_REPLY_FILTER);

    printf("==========================\n");
}

void TestFilterModifierFilter()
{
    printf("==========================\n");
    printf("Start check if filter modifier filters get removed from check loop test\n");

    CompanyRoleFilter* companyRoleFilters;
    PerformFilter performFilter;

    // Init main filter session
    initPerformFilter(&performFilter);

    //init sub filters
    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    companyRoleFilters->filter = strdup("ai_company"); // should get removed from check loop

    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    companyRoleFilters->filter = strdup("reply_filter");

    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    companyRoleFilters->filter = strdup("most_current_title"); // should get removed from check loop

    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    companyRoleFilters->filter = strdup("name_include");
    companyRoleFilters->textValue = strdup("a#,#b#,#c");

    kv_pushp(CompanyRoleFilter, performFilter.filters);
    companyRoleFilters = &kv_A(performFilter.filters, kv_size(performFilter.filters) - 1);
    initCompanyRoleFilter(companyRoleFilters);
    companyRoleFilters->filter = strdup("replies_in_days"); // should get removed from check loop

    // do the actual preparations that remove the "non used" fitlers
    initPerformFilterPreSearch(&performFilter);

    freePerformFilter(&performFilter);
    printf("==========================\n");
}

void PerformTitleTests();
int main(int argc, char* argv[])
{
    TestInitDeinit();
    TestFilterInit();
    TestProfileInitDeinit();
    TestFilterNameIncludes();
    TestFilterNameExcludes();
    TestMessagedReplied();
    TestExperience();
    PerformTitleTests();
    TestRelevantExperience();
    TestTotalExperience();
    TestFilterProjects(); 
    TestFilterGroups();
    TestFilterReplyFilter();
    TestFilterModifierFilter();
    return 0;
}
