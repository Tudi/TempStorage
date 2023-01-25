#include <logger.h>
#include <filters.h>
#include <companyrolefilter.h>
#include <kvec.h>
#include <strings_ext.h>
#include <utils.h>
#include <boolean_parser.h>
#include <string.h>

void initCompanyRoleFilter(CompanyRoleFilter* filter)
{
    filter->filter = NULL;
    filter->modifier = NULL;
    filter->textValue = NULL;
    filter->codeValue = NULL;
    filter->rangeLow = FILTER_INVALID_INT;
    filter->rangeHigh = FILTER_INVALID_INT;
    filter->textValueAsArray = NULL;
    filter->textValueArraySize = 0;
    filter->filterType = CRFT_NOT_INITIALIZED;
    filter->projectIDListCount = 0;
    filter->projectIDList = NULL;
    filter->groupIDListCount = 0;
    filter->groupIDList = NULL;
    filter->industryIDListCount = 0;
    filter->industryIDList = NULL;
    filter->alwaysTrue = 0;
    filter->modifierNumeric = SFMV_NOT_USED_VALUE;
    filter->textValueAsCharArrayCount = 0;
    filter->textValueAsCharArray = NULL;
    filter->explainFiltering = NULL;
    filter->explainBytesWritten = 0;
    filter->explainBytesAllocated = 0;
}

void freeTextValueArray(CompanyRoleFilter* companyRoleFiler)
{
    if (companyRoleFiler->textValueAsArray != NULL)
    {
        for (size_t i = 0; i < companyRoleFiler->textValueArraySize; i++)
        {
            freeSearchedString(&companyRoleFiler->textValueAsArray[i]);
        }
        free(companyRoleFiler->textValueAsArray);
        companyRoleFiler->textValueAsArray = NULL;
        companyRoleFiler->textValueArraySize = 0;
    }
}

void freeCompanyRoleFilter(CompanyRoleFilter* filter)
{
    CHECK_AND_FREE(filter->filter);
    CHECK_AND_FREE(filter->modifier);
    CHECK_AND_FREE(filter->textValue);
    CHECK_AND_FREE(filter->codeValue);
    CHECK_AND_FREE(filter->filter);
    filter->rangeLow = FILTER_INVALID_INT;
    filter->rangeHigh = FILTER_INVALID_INT;
    freeTextValueArray(filter);
    filter->filterType = CRFT_NOT_INITIALIZED;
    CHECK_AND_FREE(filter->projectIDList);
    CHECK_AND_FREE(filter->groupIDList);
    CHECK_AND_FREE(filter->industryIDList);
    if (filter->textValueAsCharArray != NULL)
    {
        for (size_t i = 0; i < filter->textValueAsCharArrayCount; i++)
        {
            free(filter->textValueAsCharArray[i]);
        }
        free(filter->textValueAsCharArray);
        filter->textValueAsCharArray = NULL;
        filter->textValueAsCharArrayCount = 0;
    }
    CHECK_AND_FREE(filter->explainFiltering);
}

/// <summary>
/// Split the filter into sub filters. filters are separated by the "#,#" token
/// </summary>
/// <param name="companyRoleFiler"></param>
/// <param name="splitToken"></param>
void CRF_TextValueToStringCompareStore(CompanyRoleFilter* companyRoleFiler, const char* splitToken)
{
    DF_LOG_MESSAGE("started");
#ifdef PERFORM_NULL_CHECKS
    if (companyRoleFiler == NULL)
    {
        return;
    }
#endif

    // Only needed if we support reinit
    //freeTextValueArray(companyRoleFiler);

    // Does this filter have it's text value converted into an array of values ?
    // this should be called probably at the initialization of the filter
    if (companyRoleFiler->textValueAsArray == NULL)
    {
        char* sourceSplit[MAX_SPLIT_RESULTS];
        size_t splitCount = 0;
        int splitErr = StrSplit(companyRoleFiler->textValue, splitToken, (char**)&sourceSplit, &splitCount, MAX_SPLIT_RESULTS);
        if (splitErr == 0 && splitCount > 0)
        {
            companyRoleFiler->textValueAsArray = (SearchedString*)malloc(sizeof(SearchedString) * splitCount);
            if (companyRoleFiler->textValueAsArray == NULL)
            {
                return;
            }
            for (size_t i = 0; i < splitCount; i++)
            {
                DF_LOG_MESSAGE("Seached word before standardization : '%s'", sourceSplit[i]);
                initSearchedString(&companyRoleFiler->textValueAsArray[i], sourceSplit[i]);
                for (size_t wordIndex = 0; wordIndex < companyRoleFiler->textValueAsArray[i].wordCount; wordIndex++)
                {
                    DF_LOG_MESSAGE("\t\t is quoted : %d, '%s'", companyRoleFiler->textValueAsArray[i].isQuoteEnclosed, 
                        companyRoleFiler->textValueAsArray[i].wordDescriptors[wordIndex].str);
                }
                free(sourceSplit[i]);
            }
            companyRoleFiler->textValueArraySize = splitCount;
        }
    }
}

/// <summary>
/// Split the "textvalue" based on separator. Convert the list of strings to a list of numbers.
/// If the list contained no "greater than 0" number, the filter will be considered "non functional"
/// </summary>
/// <param name="filter"></param>
/// <param name="companyRoleFiler"></param>
/// <returns>Error code</returns>
int CRF_ConvertCodeValueToProjectIds(CompanyRoleFilter* companyRoleFiler, const char * splitToken)
{
    DF_LOG_MESSAGE("started");
    if (companyRoleFiler->codeValue == NULL)
    {
        companyRoleFiler->filterType = CRFT_NOT_USED;
        DF_LOG_MESSAGE("Filter requires codeValue to have a value.Found NULL");
        return 1;
    }
    char* sourceSplit[MAX_SPLIT_RESULTS];
    size_t splitCount = 0;
    int splitErr = StrSplit(companyRoleFiler->codeValue, splitToken, (char**)&sourceSplit, &splitCount, MAX_SPLIT_RESULTS);
    if (splitErr == 0 && splitCount > 0)
    {
        // was not expecting this function to be called more than once
        if (companyRoleFiler->projectIDList != NULL)
        {
            DF_LOG_MESSAGE("!! projectIDList was expected to be NULL !!");
            free(companyRoleFiler->projectIDList);
            companyRoleFiler->projectIDList = NULL;
        }

        size_t requiredMemAmount = sizeof(unsigned int) * splitCount;
        companyRoleFiler->projectIDList = (unsigned int*)malloc(requiredMemAmount);
        if (companyRoleFiler->projectIDList == NULL)
        {
            companyRoleFiler->filterType = CRFT_NOT_USED;
            DF_LOG_MESSAGE("Error: Could not allocate %zd memory for projectIDList", requiredMemAmount);
            return 1;
        }
        else
        {
            companyRoleFiler->projectIDListCount = 0;

            // convert the text values to ProjectID
            for (size_t i = 0; i < splitCount; i++)
            {
                int projectID = atoi(sourceSplit[i]);
                DF_LOG_MESSAGE("Converted project id '%s' to %d", sourceSplit[i], projectID);
                free(sourceSplit[i]);
                if (projectID != 0)
                {
                    companyRoleFiler->projectIDList[i] = projectID;
                    companyRoleFiler->projectIDListCount++;
                }
            }

            //if for some reason all values were 0, we can't make the search
            if (companyRoleFiler->projectIDListCount == 0)
            {
                DF_LOG_MESSAGE("No valid project IDs were found for filter %s", companyRoleFiler->filter);
                companyRoleFiler->filterType = CRFT_NOT_USED;
            }
        }
    }
    return 0;
}

/// <summary>
/// Convert token separated string array into C string array
/// </summary>
/// <param name="companyRoleFiler"></param>
/// <param name="splitToken"></param>
/// <returns></returns>
int CRF_TextValueToStringArray(CompanyRoleFilter* companyRoleFiler, const char* splitToken)
{
    char* sourceSplit[MAX_SPLIT_RESULTS];
    size_t splitCount = 0;
    int splitErr = StrSplit(companyRoleFiler->textValue, splitToken, (char**)&sourceSplit, &splitCount, MAX_SPLIT_RESULTS);
    if (splitErr == 0 && splitCount > 0)
    {
        companyRoleFiler->textValueAsCharArray = (char**)malloc(sizeof(char*) * splitCount);
        if (companyRoleFiler->textValueAsCharArray == NULL)
        {
            DF_LOG_MESSAGE("Error: Could not allocate %zd memory for textValueAsCharArray", sizeof(char*) * splitCount);
            return 1;
        }
        for (size_t i = 0; i < splitCount; i++)
        {
            // don't use empty strings
            if (sourceSplit[i][0] == 0)
            {
                free(sourceSplit[i]);
                continue;
            }
            companyRoleFiler->textValueAsCharArray[i] = sourceSplit[i];
            StrToLower(companyRoleFiler->textValueAsCharArray[i]);
        }
        companyRoleFiler->textValueAsCharArrayCount = splitCount;
    }
    return 0;
}

int CRF_TextValueToBooleanFilter(CompanyRoleFilter* companyRoleFiler)
{
    char* booleanStr = companyRoleFiler->textValue;
    StrToLower(booleanStr);
    DF_LOG_MESSAGE("input string '%s'", booleanStr);

    // get the words of the search string
    char* searchStrCopy = strdup(booleanStr);
    if (searchStrCopy == NULL)
    {
        DF_LOG_MESSAGE("Error: Could not allocate %zd memory for booleanFilter", strlen(booleanStr) + 1);
        return 1;
    }

    StrReplaceAllToSmaller(searchStrCopy, "(", "");
    StrReplaceAllToSmaller(searchStrCopy, ")", "");
    StrReplaceAllToSmaller(searchStrCopy, " and ", " or ");
    StrReplaceAllToSmaller(searchStrCopy, " not ", " or ");

    // reuse existing function to split the words
    DF_LOG_MESSAGE("extract words from '%s'", searchStrCopy);
    char* tempStore = companyRoleFiler->textValue;
    companyRoleFiler->textValue = searchStrCopy;
    CRF_TextValueToStringArray(companyRoleFiler, " or ");
    companyRoleFiler->textValue = tempStore;
    free(searchStrCopy);
    searchStrCopy = NULL;

    if (companyRoleFiler->textValueAsCharArrayCount == 0)
    {
        DF_LOG_MESSAGE("Error : failed to extract words from '%s'", searchStrCopy);
        companyRoleFiler->filterType = CRFT_NOT_USED;
        return 2;
    }

    // need to make sure words are properly formatted
    for (size_t i = 0; i < companyRoleFiler->textValueAsCharArrayCount; i++)
    {
        StrTrimScoringClient(companyRoleFiler->textValueAsCharArray[i]);
    }
    StrTrimScoringClient(booleanStr);

    // make sure every word is unique
    for (size_t i = 0; i < companyRoleFiler->textValueAsCharArrayCount; i++)
    {
        // since we trimmed strings, this should not happen. Safe coding
        if (companyRoleFiler->textValueAsCharArray[i][0] == 0)
        {
            free(companyRoleFiler->textValueAsCharArray[i]);
            companyRoleFiler->textValueAsCharArray[i] = NULL;
            continue;
        }
        for (size_t j = i + 1; j < companyRoleFiler->textValueAsCharArrayCount; j++)
        {
            if (strcmp(companyRoleFiler->textValueAsCharArray[i], companyRoleFiler->textValueAsCharArray[j]) == 0)
            {
                free(companyRoleFiler->textValueAsCharArray[i]);
                companyRoleFiler->textValueAsCharArray[i] = NULL;
                break;
            }
        }
    }

    // remove empty spots
    size_t writeIndex = 0;
    for (size_t i = 0; i < companyRoleFiler->textValueAsCharArrayCount; i++)
    {
        if (companyRoleFiler->textValueAsCharArray[i] == NULL)
        {
            continue;
        }
        companyRoleFiler->textValueAsCharArray[writeIndex] = companyRoleFiler->textValueAsCharArray[i];
        writeIndex++;
    }
    companyRoleFiler->textValueAsCharArrayCount = writeIndex;

    // replace words with variables
    char varAssignment[2];
    varAssignment[0] = 'a';
    varAssignment[1] = 0;
    for (size_t i = 0; i < companyRoleFiler->textValueAsCharArrayCount; i++)
    {
        int replaceCount = StrReplaceAllToSmaller(booleanStr, companyRoleFiler->textValueAsCharArray[i], varAssignment);
        //in case one of the words contained '(' or ')' it messed up the whole process
        if (replaceCount == 0)
        {
            DF_LOG_MESSAGE("Error: Could not find boolean word '%s' in boolean string '%s'",
                companyRoleFiler->textValueAsCharArray[i], booleanStr);
            companyRoleFiler->filterType = CRFT_NOT_USED;
            return 4;
        }
        varAssignment[0]++;
        if (varAssignment[0] > 'z')
        {
            DF_LOG_MESSAGE("Error: More unique words used than booleanFilter can handle");
            companyRoleFiler->filterType = CRFT_NOT_USED;
            return 5;
        }
    }

    StrReplaceAllToSmaller(booleanStr, " and ", "&");
    StrReplaceAllToSmaller(booleanStr, " or ", "|");
    StrReplaceAllToSmaller(booleanStr, " not ", "!");
    StrReplaceAllToSmaller(booleanStr, " true ", "1");
    StrReplaceAllToSmaller(booleanStr, " false ", "0");
    StrReplaceAllToSmaller(booleanStr, " ", "");
    DF_LOG_MESSAGE("Final boolean string '%s'", booleanStr);

    //check if boolean expression is parsable
    char variableValues[256];
    memset(variableValues, 0, sizeof(variableValues));
    int ret = calculate_boolean_expression(booleanStr, variableValues);
    if (ret == BOOLEAN_PARSER_SYNTAX_ERROR)
    {
        DF_LOG_MESSAGE("Error: boolean expresion '%s' is not parsable", booleanStr);
        companyRoleFiler->filterType = CRFT_NOT_USED;
        return 3;
    }

    // make sure search words are conditioned properly. Expand titles. Remove punctuation .. this operation is done on DB string also
    for (size_t i = 0; i < companyRoleFiler->textValueAsCharArrayCount; i++)
    {
        companyRoleFiler->textValueAsCharArray[i] = StrStandardizeScoringClient(companyRoleFiler->textValueAsCharArray[i]);
        StrReplaceAllToSmaller(companyRoleFiler->textValueAsCharArray[i], "\"", ""); // unquote
    }

    return 0;
}

/// <summary>
/// Converts string filter type to numeric filter type
/// </summary>
/// <param name="filter"></param>
/// <param name="companyRoleFiler"></param>
/// <returns></returns>
int CRF_ConvertStringFilterTypeToInt(CompanyRoleFilter* companyRoleFiler)
{
    DF_LOG_MESSAGE("started");

    companyRoleFiler->filterType = CRFT_NOT_RECOGNIZED;

    if (companyRoleFiler->filter == NULL)
    {
        DF_LOG_MESSAGE("unexpected NULL filter string");
        return 1;
    }

    struct FilterNameToValueStruct
    {
        const char* name;
        CompanyRoleFilterTypes val;
        void* handlerFunction; // not used. But maybe someone will insist on this idea
    };
    static const struct FilterNameToValueStruct conversionArray[] = {
        {.name = "name_include", .val = CRFT_NAME_INCLUDES},
        {.name = "name_exclude", .val = CRFT_NAME_DOES_NOT_INCLUDE},
        {.name = "current_title_include", .val = CRFT_CURRENT_TITLE_INCLUDE},
        {.name = "current_title_exclude", .val = CRFT_CURRENT_TITLE_EXCLUDE},
        {.name = "previous_title_include", .val = CRFT_PREVIOUS_TITLE_INCLUDE},
        {.name = "previous_title_exclude", .val = CRFT_PREVIOUS_TITLE_EXCLUDE},
        {.name = "current_previous_title_include", .val = CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE},
        {.name = "current_previous_title_exclude", .val = CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE},
        {.name = "current_cmp_include", .val = CRFT_CURRENT_COMPANY_INCLUDE},
        {.name = "current_cmp_exclude", .val = CRFT_CURRENT_COMPANY_EXCLUDE},
        {.name = "previous_cmp_include", .val = CRFT_PREVIOUS_COMPANY_INCLUDE},
        {.name = "previous_cmp_exclude", .val = CRFT_PREVIOUS_COMPANY_EXCLUDE},
        {.name = "current_previous_cmp_include", .val = CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE},
        {.name = "current_previous_cmp_exclude", .val = CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE},
//        {.name = "tags", .val = CRFT_TAGS_INCLUDE},
//        {.name = "tags_exclude", .val = CRFT_TAGS_EXCLUDE},
        {.name = "keywords", .val = CRFT_KEYWORDS_INCLUDE},
        {.name = "keywords_exclude", .val = CRFT_KEYWORDS_EXCLUDE},
        {.name = "messaged", .val = CRFT_MESSAGED_NEWER_THAN},
        {.name = "replied", .val = CRFT_REPLIED_NEWER_THAN},
//        {.name = "ats_last_activity", .val = CRFT_ATS_LAST_ACTIVITY},
//        {.name = "ats_status", .val = CRFT_ATS_STATUS},
//        {.name = "ats_exists", .val = CRFT_ATS_EXISTS},
        {.name = "experience", .val = CRFT_EXPERIENCE},
        {.name = "current_industry", .val = CRFT_CURRENT_INDUSTRY_INCLUDE},
        {.name = "current_industry_exclude", .val = CRFT_CURRENT_INDUSTRY_EXCLUDE},
        {.name = "previous_industry", .val = CRFT_PREVIOUS_INDUSTRY_INCLUDE},
        {.name = "previous_industry_exclude", .val = CRFT_PREVIOUS_INDUSTRY_EXCLUDE},
        {.name = "industry", .val = CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE},
        {.name = "industry_exclude", .val = CRFT_PREVIOUS_CURRENT_INDUSTRY_EXCLUDE},
        {.name = "current_tenure", .val = CRFT_CURRENT_TENURE},
        {.name = "relevant_experience", .val = CRFT_RELEVANT_EXPERIENCE},
        {.name = "total_experience", .val = CRFT_TOTAL_EXPERIENCE},
        {.name = "current_cmp_fn_include", .val = CRFT_CURRENT_COMPANY_FUNCTION_INCLUDE},
        {.name = "current_cmp_fn_exclude", .val = CRFT_CURRENT_COMPANY_FUNCTION_EXCLUDE},
        {.name = "previous_cmp_fn_include", .val = CRFT_PREVIOUS_COMPANY_FUNCTION_INCLUDE},
        {.name = "previous_cmp_fn_exclude", .val = CRFT_PREVIOUS_COMPANY_FUNCTION_EXCLUDE},
        {.name = "current_previous_cmp_fn_include", .val = CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_INCLUDE},
        {.name = "current_previous_cmp_fn_exclude", .val = CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_EXCLUDE},
        {.name = "current_cmp_nre_include", .val = CRFT_CURRENT_COMPANY_NRE_INCLUDE},
        {.name = "current_cmp_nre_exclude", .val = CRFT_CURRENT_COMPANY_NRE_EXCLUDE},
        {.name = "previous_cmp_nre_include", .val = CRFT_PREVIOUS_COMPANY_NRE_INCLUDE},
        {.name = "previous_cmp_nre_exclude", .val = CRFT_PREVIOUS_COMPANY_NRE_EXCLUDE},
        {.name = "current_previous_cmp_nre_include", .val = CRFT_PREVIOUS_CURRENT_COMPANY_NRE_INCLUDE},
        {.name = "current_previous_cmp_nre_exclude", .val = CRFT_PREVIOUS_CURRENT_COMPANY_NRE_EXCLUDE},
        {.name = "projects_include", .val = CRFT_PROJECTS_INCLUDE},
        {.name = "projects_exclude", .val = CRFT_PROJECTS_EXCLUDE},
        {.name = "groups_include", .val = CRFT_GROUPS_INCLUDE},
        {.name = "groups_exclude", .val = CRFT_GROUPS_EXCLUDE},
        {.name = "country_include", .val = CRFT_COUNTRY_ID_INCLUDE},
        {.name = "country_exclude", .val = CRFT_COUNTRY_ID_EXCLUDE},
        {.name = "reply_filter", .val = CRFT_REPLY_FILTER},
        {.name = "linkedin_include", .val = CRFT_PROFILE_ID_INCLUDE},
        {.name = "linkedin_exclude", .val = CRFT_PROFILE_ID_EXCLUDE},
        {.name = "state_include", .val = CRFT_STATE_ID_INCLUDE},
        {.name = "state_exclude", .val = CRFT_STATE_ID_EXCLUDE},
        {.name = "campaigns_include", .val = CRFT_CAMPAIGN_INCLUDE},
        {.name = "campaigns_exclude", .val = CRFT_CAMPAIGN_EXCLUDE},
        {.name = "talentpools_include", .val = CRFT_TALENTPOOL_INCLUDE},
        {.name = "talentpools_exclude", .val = CRFT_TALENTPOOL_EXCLUDE},
        {.name = NULL, .val = CRFT_NOT_RECOGNIZED},
    };

    // Convert from string to value
    int index = 0;
    while (conversionArray[index].name != NULL)
    {
        if (strcmp(companyRoleFiler->filter, conversionArray[index].name) == 0)
        {
            companyRoleFiler->filterType = conversionArray[index].val;
            break;
        }
        index++;
    }

    DF_LOG_MESSAGE("filter type %d from string %s", companyRoleFiler->filterType, companyRoleFiler->filter);

    return 0;
}
