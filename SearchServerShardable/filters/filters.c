#include <logger.h>
#include <filters.h>
#include <k_utils.h>
#include <strings_ext.h>
#include <profile_cached.h>
#include <position_cached.h>
#include <companyrolefilter.h>
#include <string.h>
#include <limits.h>

void initPerformFilter(PerformFilter* filter)
{
    kv_init(filter->filters);
    filter->profile = NULL;
    filter->filterCompanyID = FILTER_INVALID_INT;
    filter->relevantExperience = FILTER_INVALID_FLOAT;
    filter->companyAI = 0;
    filter->titleAI = 0;
    filter->experienceAI = 0;
    filter->industryAI = 0;
    filter->mostCurrentTitle = 0;
    filter->keywordBoolean = 0;
    filter->positiveReply = 0;
    filter->positiveReplyModifier = 0;
    filter->replyInDays = 0;
    filter->isInitialized = 0;
    filter->companyFilterPresent = 0;
    filter->titleFilterPresent = 0;
    filter->calculateRelevantExperienceScore = NULL;
    filter->calculateTotalExperienceScore = NULL;
    filter->industryFilterPresent = 0;
    filter->keywordsFilterPresent = 0;
    filter->companyCachedList = NULL;
    filter->companyCachedLargestId = 0;
    filter->timeStamp = 0;
    kv_init(filter->filterExplained);
}

void freePerformFilter(PerformFilter* filter)
{
    for (size_t index = 0; index < kv_size(filter->filters); index++)
    {
        freeCompanyRoleFilter(&kv_A(filter->filters, index));
    }
    kv_destroy(filter->filters);
    kv_init(filter->filters);
    filter->profile = NULL;
    filter->isInitialized = 0;
    filter->companyCachedList = NULL; // this is a shared resource given by search engine
    filter->companyCachedLargestId = 0;
    for (size_t index = 0; index < kv_size(filter->filterExplained); index++)
    {
        freeSearchFilterExplain(&kv_A(filter->filterExplained, index));
    }
    kv_destroy(filter->filterExplained);
    kv_init(filter->filterExplained);
}

/// <summary>
/// The values will be used by scoring interface. Filter itself is not using these values. The values have been added for the sake of optimization
/// </summary>
/// <param name="filter"></param>
static inline void InitWhichScoresToCalculate(PerformFilter* filter, CompanyRoleFilter* companyRoleFilter)
{
    DF_LOG_MESSAGE("filter has type %d", companyRoleFilter->filterType);
    if (companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE)
    {
        DF_LOG_MESSAGE("Company filter is present");
        filter->companyFilterPresent = 1;
    }
    else if (companyRoleFilter->filterType == CRFT_CURRENT_TITLE_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_TITLE_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE)
    {
        DF_LOG_MESSAGE("Title filter is present");
        filter->titleFilterPresent = 1;
    }
    else if (companyRoleFilter->filterType == CRFT_RELEVANT_EXPERIENCE)
    {
        DF_LOG_MESSAGE("Relevant experience filter is present");
        filter->calculateRelevantExperienceScore = companyRoleFilter;
    }
    else if (companyRoleFilter->filterType == CRFT_TOTAL_EXPERIENCE)
    {
        DF_LOG_MESSAGE("Total experience filter is present");
        filter->calculateTotalExperienceScore = companyRoleFilter;
    }
    else if (companyRoleFilter->filterType == CRFT_CURRENT_INDUSTRY_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_INDUSTRY_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE)
    {
        DF_LOG_MESSAGE("Industry filter is present");
        filter->industryFilterPresent = 1;
    }
    else if (companyRoleFilter->filterType == CRFT_KEYWORDS_INCLUDE || (companyRoleFilter->filter != NULL && strstr(companyRoleFilter->filter, "keyword_boolean_filter") != NULL ))
    {
        DF_LOG_MESSAGE("Keyword filter is present");
        filter->keywordsFilterPresent = 1;
    }
}

/// <summary>
/// Should only be called by initPerformFilterPreSearch
/// Function will do a first-pass on parsing filter values. This is required because the behavior of some filters might change based on other filter values
/// </summary>
/// <param name="filter"></param>
static void ParseAndDetectFilterModifierFilters(PerformFilter* filter)
{
    for (size_t i = 0; i < kv_size(filter->filters); ++i)
    {
        CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, i);

        // We do not expect list of filters to contain NULL pointers. Something went wrong
        if (companyRoleFilter == NULL)
        {
            DF_LOG_MESSAGE("unexpected NULL filter");
            return; // Error : should not receive null filter structures
        }
        
        if (companyRoleFilter->filter != NULL)
        {
            if (strcmp(companyRoleFilter->filter, "ai_company") == 0)
                filter->companyAI = 1;
            else if (strcmp(companyRoleFilter->filter, "ai_title") == 0)
                filter->titleAI = 1;
            else if (strcmp(companyRoleFilter->filter, "ai_experience") == 0)
                filter->experienceAI = 1;
            else if (strcmp(companyRoleFilter->filter, "ai_industry") == 0)
                filter->industryAI = 1;
            else if (strcmp(companyRoleFilter->filter, "most_current_title") == 0)
                filter->mostCurrentTitle = 1;
            else if (strcmp(companyRoleFilter->filter, "keyword_boolean_filter") == 0)
                filter->keywordBoolean = 1;
            else if (strcmp(companyRoleFilter->filter, "positive_filter") == 0)
            {
                filter->positiveReply = 1;
                if (companyRoleFilter->modifier != NULL)
                {
                    StrToLower(companyRoleFilter->modifier);
                    if (strstr(companyRoleFilter->modifier, (const char*)"not") != NULL)
                    {
                        filter->positiveReplyModifier = 1;
                    }
                }
            }
            else if (strcmp(companyRoleFilter->filter, "replies_in_days") == 0)
            {
                if (companyRoleFilter->textValue != NULL)
                {
                    filter->replyInDays = atoi(companyRoleFilter->textValue);
                }
            }
            if (companyRoleFilter->modifier != NULL)
            {
                StrToLower(companyRoleFilter->modifier);
                if (strstr(companyRoleFilter->modifier, (const char*)"does not") != NULL)
                {
                    companyRoleFilter->modifierNumeric = SFMV_NEGATE;
                }
                else if (strstr(companyRoleFilter->modifier, (const char*)"less than") != NULL)
                {
                    companyRoleFilter->modifierNumeric = SFMV_LESS_THAN;
                }
            }
        }
    }
}

static void applyFilterNegation(CompanyRoleFilter* companyRoleFilter)
{
    if (companyRoleFilter->modifierNumeric != SFMV_NEGATE)
    {
        return;
    }
    DF_LOG_MESSAGE("has'modifier''doesnot'.Flipsfilterfunctionality!");
    switch (companyRoleFilter->filterType)
    {
    case CRFT_NAME_INCLUDES:
        companyRoleFilter->filterType = CRFT_NAME_DOES_NOT_INCLUDE; break;
    case CRFT_NAME_DOES_NOT_INCLUDE:
        companyRoleFilter->filterType = CRFT_NAME_INCLUDES; break;
    case CRFT_CURRENT_TITLE_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_TITLE_EXCLUDE; break;
    case CRFT_CURRENT_TITLE_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_TITLE_INCLUDE; break;
    case CRFT_PREVIOUS_TITLE_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_TITLE_EXCLUDE; break;
    case CRFT_PREVIOUS_TITLE_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_TITLE_INCLUDE; break;
    case CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE; break;
    case CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE; break;
    case CRFT_REPLY_FILTER:
        companyRoleFilter->filterType = CRFT_REPLY_EXCLUDE_FILTER; break;
    case CRFT_REPLY_EXCLUDE_FILTER:
        companyRoleFilter->filterType = CRFT_REPLY_FILTER; break;
    case CRFT_CURRENT_COMPANY_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_COMPANY_EXCLUDE; break;
    case CRFT_CURRENT_COMPANY_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_COMPANY_INCLUDE; break;
    case CRFT_PREVIOUS_COMPANY_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_COMPANY_EXCLUDE; break;
    case CRFT_PREVIOUS_COMPANY_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_COMPANY_INCLUDE; break;
    case CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE; break;
    case CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE; break;
    case CRFT_KEYWORDS_INCLUDE:
        companyRoleFilter->filterType = CRFT_KEYWORDS_EXCLUDE; break;
    case CRFT_KEYWORDS_EXCLUDE:
        companyRoleFilter->filterType = CRFT_KEYWORDS_INCLUDE; break;
    case CRFT_CURRENT_INDUSTRY_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_INDUSTRY_EXCLUDE; break;
    case CRFT_CURRENT_INDUSTRY_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_INDUSTRY_INCLUDE; break;
    case CRFT_PREVIOUS_INDUSTRY_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_INDUSTRY_EXCLUDE; break;
    case CRFT_PREVIOUS_INDUSTRY_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_INDUSTRY_INCLUDE; break;
    case CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_CURRENT_INDUSTRY_EXCLUDE; break;
    case CRFT_PREVIOUS_CURRENT_INDUSTRY_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE; break;
    case CRFT_CURRENT_COMPANY_FUNCTION_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_COMPANY_FUNCTION_EXCLUDE; break;
    case CRFT_CURRENT_COMPANY_FUNCTION_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_COMPANY_FUNCTION_INCLUDE; break;
    case CRFT_PREVIOUS_COMPANY_FUNCTION_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_COMPANY_FUNCTION_INCLUDE; break;
    case CRFT_PREVIOUS_COMPANY_FUNCTION_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_COMPANY_FUNCTION_EXCLUDE; break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_EXCLUDE; break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_INCLUDE; break;
    case CRFT_CURRENT_COMPANY_NRE_INCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_COMPANY_NRE_EXCLUDE; break;
    case CRFT_CURRENT_COMPANY_NRE_EXCLUDE:
        companyRoleFilter->filterType = CRFT_CURRENT_COMPANY_NRE_INCLUDE; break;
    case CRFT_PREVIOUS_COMPANY_NRE_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_COMPANY_NRE_EXCLUDE; break;
    case CRFT_PREVIOUS_COMPANY_NRE_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_COMPANY_NRE_INCLUDE; break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_NRE_INCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_CURRENT_COMPANY_NRE_EXCLUDE; break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_NRE_EXCLUDE:
        companyRoleFilter->filterType = CRFT_PREVIOUS_CURRENT_COMPANY_NRE_INCLUDE; break;
    default:
    {
        break;
    }
    };
}

/// <summary>
/// Called before the search is performed. This function will make sure NULL checks will not be required while search is performed
/// </summary>
/// <param name="filter">Structure used for filtering</param>
/// <returns>Error code</returns>
int initPerformFilterPreSearch(PerformFilter* filter)
{
    DF_LOG_MESSAGE("started");
//    if (filter->isInitialized != 0)
    {
//        return 0;
    }
    filter->isInitialized = 1;

    int validFiletersFound = 0;
    int filtersNotUsedForFiltering = 0;

    ParseAndDetectFilterModifierFilters(filter);

    for (size_t i = 0; i < kv_size(filter->filters); ++i)
    {
        CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, i);
        // Convert the string representation of filter type to numeric value
        CRF_ConvertStringFilterTypeToInt(companyRoleFilter);

        // I think these are no longer used. But it is within GO code so I'm adding it
        applyFilterNegation(companyRoleFilter);

        // used by scoring interface. Optimization to not check for which filter types are present
        InitWhichScoresToCalculate(filter, companyRoleFilter);

        if (companyRoleFilter->modifierNumeric == SFMV_LESS_THAN)
        {
            DF_LOG_MESSAGE("has 'modifier' 'less than'");
            if (companyRoleFilter->filterType == CRFT_MESSAGED_NEWER_THAN)
            {
                companyRoleFilter->filterType = CRFT_MESSAGED_EXCLUDE_OLDER_THAN;
                DF_LOG_MESSAGE("converted filter type %d to %d", CRFT_MESSAGED_NEWER_THAN, CRFT_MESSAGED_EXCLUDE_OLDER_THAN);
            }
            else if (companyRoleFilter->filterType == CRFT_REPLIED_NEWER_THAN)
            {
                companyRoleFilter->filterType = CRFT_REPLIED_EXCLUDE_OLDER_THAN;
                DF_LOG_MESSAGE("converted filter type %d to %d", CRFT_REPLIED_NEWER_THAN, CRFT_REPLIED_EXCLUDE_OLDER_THAN);
            }
        }

        // In case text has not yet been split, split it. Only process the search string once
        if (companyRoleFilter->filterType == CRFT_NAME_INCLUDES || companyRoleFilter->filterType == CRFT_NAME_DOES_NOT_INCLUDE)
        {
            CRF_TextValueToStringCompareStore(companyRoleFilter, (const char*)"#,#");
            // If for some reason, there are no words in the text field, than there is nothing we can do with this filter
            if (companyRoleFilter->textValueArraySize == 0 || companyRoleFilter->textValueAsArray == NULL)
            {
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_CURRENT_TITLE_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_TITLE_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_TITLE_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_TITLE_EXCLUDE
            || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE)
        {
            // Why even send such filter ? Always returns 1
            if (filter->titleAI != 0 && (companyRoleFilter->filterType == CRFT_CURRENT_TITLE_INCLUDE 
                                        || companyRoleFilter->filterType == CRFT_PREVIOUS_TITLE_INCLUDE 
                                        || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE))
            {
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
            else
            {
                CRF_TextValueToStringCompareStore(companyRoleFilter, (const char*)"#,#");
                // If for some reason, there are no words in the text field, than there is nothing we can do with this filter
                if (companyRoleFilter->textValueArraySize == 0 || companyRoleFilter->textValueAsArray == NULL)
                {
                    companyRoleFilter->filterType = CRFT_NOT_USED;
                }
            }
        }
        else if (companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_EXCLUDE
            || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE)
        {
            if (filter->companyAI != 0 && (companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_INCLUDE 
                                            || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_INCLUDE 
                                            || companyRoleFilter->filterType == CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE))
            {
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
            else
            {
                CRF_TextValueToStringCompareStore(companyRoleFilter, (const char*)"#,#");
                // If for some reason, there are no words in the text field, than there is nothing we can do with this filter
                if (companyRoleFilter->textValueArraySize == 0 || companyRoleFilter->textValueAsArray == NULL)
                {
                    companyRoleFilter->filterType = CRFT_NOT_USED;
                }
            }
        }
        else if (companyRoleFilter->filterType == CRFT_KEYWORDS_INCLUDE || companyRoleFilter->filterType == CRFT_KEYWORDS_EXCLUDE)
        {
            if (filter->keywordBoolean)
            {
                if (companyRoleFilter->filterType == CRFT_KEYWORDS_INCLUDE)
                {
                    companyRoleFilter->filterType = CRFT_KEYWORDS_BOOLEAN_INCLUDE;
                }
                else
                {
                    companyRoleFilter->filterType = CRFT_KEYWORDS_BOOLEAN_EXCLUDE;
                }
                CRF_TextValueToBooleanFilter(companyRoleFilter);
            }
            else
            {
                CRF_TextValueToStringCompareStore(companyRoleFilter, (const char*)"#,#");
                // If for some reason, there are no words in the text field, than there is nothing we can do with this filter
                if (companyRoleFilter->textValueArraySize == 0 || companyRoleFilter->textValueAsArray == NULL)
                {
                    companyRoleFilter->filterType = CRFT_NOT_USED;
                }
            }
        }
        else if (companyRoleFilter->filterType == CRFT_MESSAGED_NEWER_THAN || companyRoleFilter->filterType == CRFT_REPLIED_NEWER_THAN)
        {
            //scale time to current time - days. Looks like the range is already a Unixstamp converted to days
//            companyRoleFilter->rangeHigh = time(NULL) / 86400 - companyRoleFilter->rangeHigh;
            // I asked Xuan about it : this filter will always return true
            // For me it's a bit confusing. Why does this filter type exist if it filters nothing ?
            companyRoleFilter->alwaysTrue = 1;
            companyRoleFilter->filterType = CRFT_NOT_USED;
        }
        else if (companyRoleFilter->filterType == CRFT_EXPERIENCE)
        {
            if (filter->experienceAI)
            {
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_CURRENT_INDUSTRY_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_INDUSTRY_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_INDUSTRY_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_INDUSTRY_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_INDUSTRY_EXCLUDE)
        {
            if (filter->companyCachedList == NULL)
            {
                DF_LOG_MESSAGE("Missing company data. Industry filter can not function without it !");
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
            else if (filter->industryAI && companyRoleFilter->modifierNumeric != SFMV_NEGATE)
            {
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
            else
            {
                CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
                //those project Ids were actually industry IDs in this case
                companyRoleFilter->industryIDList = companyRoleFilter->projectIDList;
                companyRoleFilter->projectIDList = NULL;
                companyRoleFilter->industryIDListCount = companyRoleFilter->projectIDListCount;
                companyRoleFilter->projectIDListCount = 0;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_RELEVANT_EXPERIENCE)
        {
            if (filter->experienceAI)
            {
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_TOTAL_EXPERIENCE)
        {
        }        
        else if(companyRoleFilter->filterType == CRFT_PROJECTS_INCLUDE || companyRoleFilter->filterType == CRFT_PROJECTS_EXCLUDE)
        {
            if (filter->filterCompanyID == 0 || filter->filterCompanyID == FILTER_INVALID_INT)
            {
                DF_LOG_MESSAGE("project filter is missing filterCompanyID");
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
        }
        else if (companyRoleFilter->filterType == CRFT_CAMPAIGN_INCLUDE || companyRoleFilter->filterType == CRFT_CAMPAIGN_EXCLUDE)
        {
            if (filter->filterCompanyID == 0 || filter->filterCompanyID == FILTER_INVALID_INT)
            {
                DF_LOG_MESSAGE("campaign filter is missing filterCompanyID");
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
        }
        else if (companyRoleFilter->filterType == CRFT_COUNTRY_ID_INCLUDE || companyRoleFilter->filterType == CRFT_COUNTRY_ID_EXCLUDE)
        {
            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
        }
        else if (companyRoleFilter->filterType == CRFT_GROUPS_INCLUDE || companyRoleFilter->filterType == CRFT_GROUPS_EXCLUDE)
        {
            if (filter->filterCompanyID == 0 || filter->filterCompanyID == FILTER_INVALID_INT)
            {
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }

            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
            //those project Ids were actually group IDs in this case
            companyRoleFilter->groupIDList = companyRoleFilter->projectIDList;
            companyRoleFilter->groupIDListCount = companyRoleFilter->projectIDListCount;
            companyRoleFilter->projectIDList = NULL;
            companyRoleFilter->projectIDListCount = 0;
        }
        else if (companyRoleFilter->filterType == CRFT_REPLY_FILTER)
        {
            if(filter->filterCompanyID == 0 || filter->filterCompanyID == FILTER_INVALID_INT)
            {
                DF_LOG_MESSAGE("filter does not have filterCompanyID. It will always return 1 !!");
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }

            if (filter->replyInDays == 0)
            {
                DF_LOG_MESSAGE("filter does not have replyInDays. It will always return 1 !!");
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_FUNCTION_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_FUNCTION_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_FUNCTION_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_FUNCTION_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_EXCLUDE)
        {
            CRF_TextValueToStringArray(companyRoleFilter, (const char*)"#,#");
            if (companyRoleFilter->textValueAsCharArrayCount == 0)
            {
                DF_LOG_MESSAGE("Error : filter does not have filter values. It will always return 1");
                companyRoleFilter->alwaysTrue = 1;
                companyRoleFilter->filterType = CRFT_NOT_USED;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_NRE_INCLUDE || companyRoleFilter->filterType == CRFT_CURRENT_COMPANY_NRE_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_NRE_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_COMPANY_NRE_EXCLUDE
            || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_COMPANY_NRE_INCLUDE || companyRoleFilter->filterType == CRFT_PREVIOUS_CURRENT_COMPANY_NRE_EXCLUDE)
        {
            if (companyRoleFilter->rangeHigh < 0)
            {
                companyRoleFilter->rangeHigh = INT_MAX;
            }
        }
        else if (companyRoleFilter->filterType == CRFT_PROFILE_ID_INCLUDE || companyRoleFilter->filterType == CRFT_PROFILE_ID_EXCLUDE)
        {
            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
        }
        else if (companyRoleFilter->filterType == CRFT_STATE_ID_INCLUDE || companyRoleFilter->filterType == CRFT_STATE_ID_EXCLUDE)
        {
            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
        }
        else if (companyRoleFilter->filterType == CRFT_TALENTPOOL_INCLUDE || companyRoleFilter->filterType == CRFT_TALENTPOOL_EXCLUDE)
        {
            CRF_ConvertCodeValueToProjectIds(companyRoleFilter, "#,#");
        }

        if (companyRoleFilter->filterType != CRFT_NON_USED_VALUE && companyRoleFilter->filterType != CRFT_NOT_INITIALIZED && companyRoleFilter->filterType != CRFT_NOT_RECOGNIZED && companyRoleFilter->filterType < CRFT_NOT_USED)
        {
            validFiletersFound++;
        }
        else
        {
            filtersNotUsedForFiltering++;
        }
    }

    // If no timestamp override has been provided, we generate one. This is for legacy tests. 
    // Scoring will provide a timestamp override
    if (filter->timeStamp == 0)
    {
        filter->timeStamp = time(NULL);
    }

    // Some filters are only used for setup. There is no need to iterate through them millions of times
    if (filtersNotUsedForFiltering > 0)
    {
        int filterWriteIndex = 0;
        for (size_t filterReadIndex = 0; filterReadIndex < kv_size(filter->filters); ++filterReadIndex)
        {
            CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, filterReadIndex);
            if (companyRoleFilter->filterType == CRFT_NON_USED_VALUE
                || companyRoleFilter->filterType == CRFT_NOT_INITIALIZED 
                || companyRoleFilter->filterType == CRFT_NOT_RECOGNIZED 
                || companyRoleFilter->filterType == CRFT_NOT_USED
                || companyRoleFilter->alwaysTrue == 1)
            {
                DF_LOG_MESSAGE("removing useless filter %s from index %zd", companyRoleFilter->filter, filterReadIndex);
                freeCompanyRoleFilter(companyRoleFilter);
                if (companyRoleFilter->alwaysTrue == 1)
                {
                    companyRoleFilter->filterType = CRFT_NOT_USED; // make sure to report it
                }
            }
            else 
            {
                // shift value from read index so that values are consecutive
                if (filterWriteIndex != filterReadIndex)
                {
                    memcpy(&filter->filters.a[filterWriteIndex], &filter->filters.a[filterReadIndex], sizeof(CompanyRoleFilter));
                }
                filterWriteIndex++;
            }
        }
        filter->filters.n = filterWriteIndex;
    }
    return 0;
}
