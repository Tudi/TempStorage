#ifndef _COPANY_ROLE_FILTER_H_
#define _COPANY_ROLE_FILTER_H_

/// <summary>
/// These are used to convert string values received from GO managers into binary values. Lookup map will be used to handle filters
/// </summary>
typedef enum CompanyRoleFilterTypes
{
    CRFT_NON_USED_VALUE, // avoid 0 values
    CRFT_NAME_INCLUDES,
    CRFT_NAME_DOES_NOT_INCLUDE,
    CRFT_CURRENT_TITLE_INCLUDE,
    CRFT_CURRENT_TITLE_EXCLUDE,
    CRFT_PREVIOUS_TITLE_INCLUDE,
    CRFT_PREVIOUS_TITLE_EXCLUDE,
    CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE,
    CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE,
    CRFT_CURRENT_COMPANY_INCLUDE,
    CRFT_CURRENT_COMPANY_EXCLUDE,
    CRFT_PREVIOUS_COMPANY_INCLUDE,
    CRFT_PREVIOUS_COMPANY_EXCLUDE,
    CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE,
    CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE,
    CRFT_TAGS_INCLUDE,
    CRFT_TAGS_EXCLUDE,
    CRFT_KEYWORDS_INCLUDE,
    CRFT_KEYWORDS_EXCLUDE,
    CRFT_KEYWORDS_BOOLEAN_INCLUDE,
    CRFT_KEYWORDS_BOOLEAN_EXCLUDE,
    CRFT_MESSAGED_NEWER_THAN,
    CRFT_MESSAGED_EXCLUDE_OLDER_THAN,
    CRFT_REPLIED_NEWER_THAN,
    CRFT_REPLIED_EXCLUDE_OLDER_THAN,
    CRFT_ATS_LAST_ACTIVITY,
    CRFT_ATS_STATUS,
    CRFT_ATS_EXISTS,
    CRFT_EXPERIENCE,
    CRFT_EXPERIENCE_BETWEEN,
    CRFT_EXPERIENCE_GREATER,
    CRFT_EXPERIENCE_SMALLER,
    CRFT_CURRENT_INDUSTRY_INCLUDE,
    CRFT_CURRENT_INDUSTRY_EXCLUDE,
    CRFT_PREVIOUS_INDUSTRY_INCLUDE,
    CRFT_PREVIOUS_INDUSTRY_EXCLUDE,
    CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE,
    CRFT_PREVIOUS_CURRENT_INDUSTRY_EXCLUDE,
    CRFT_CURRENT_TENURE,
    CRFT_CURRENT_TENURE_BETWEEN,
    CRFT_CURRENT_TENURE_GREATER,
    CRFT_CURRENT_TENURE_SMALLER,
    CRFT_RELEVANT_EXPERIENCE,
    CRFT_TOTAL_EXPERIENCE,
    CRFT_TOTAL_EXPERIENCE_BETWEEN,
    CRFT_TOTAL_EXPERIENCE_GREATER,
    CRFT_TOTAL_EXPERIENCE_SMALLER,
    CRFT_CURRENT_COMPANY_FUNCTION_INCLUDE,
    CRFT_CURRENT_COMPANY_FUNCTION_EXCLUDE,
    CRFT_PREVIOUS_COMPANY_FUNCTION_INCLUDE,
    CRFT_PREVIOUS_COMPANY_FUNCTION_EXCLUDE,
    CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_INCLUDE,
    CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_EXCLUDE,
    CRFT_CURRENT_COMPANY_NRE_INCLUDE,
    CRFT_CURRENT_COMPANY_NRE_EXCLUDE,
    CRFT_PREVIOUS_COMPANY_NRE_INCLUDE,
    CRFT_PREVIOUS_COMPANY_NRE_EXCLUDE,
    CRFT_PREVIOUS_CURRENT_COMPANY_NRE_INCLUDE,
    CRFT_PREVIOUS_CURRENT_COMPANY_NRE_EXCLUDE,
    CRFT_PROJECTS_INCLUDE,
    CRFT_PROJECTS_EXCLUDE,
    CRFT_GROUPS_INCLUDE,
    CRFT_GROUPS_EXCLUDE,
    CRFT_COUNTRY_ID_INCLUDE,
    CRFT_COUNTRY_ID_EXCLUDE,
    CRFT_REPLY_FILTER,
    CRFT_REPLY_EXCLUDE_FILTER,
    CRFT_PROFILE_ID_INCLUDE,
    CRFT_PROFILE_ID_EXCLUDE,
    CRFT_STATE_ID_INCLUDE,
    CRFT_STATE_ID_EXCLUDE,
    CRFT_CAMPAIGN_INCLUDE,
    CRFT_CAMPAIGN_EXCLUDE,
    CRFT_TALENTPOOL_INCLUDE,
    CRFT_TALENTPOOL_EXCLUDE,
    // special values for sanity checks
    CRFT_NOT_INITIALIZED,
    CRFT_NOT_RECOGNIZED,
    CRFT_NOT_USED, // some conditions block this filter to be executed. Why does it even exist ?
}CompanyRoleFilterTypes;

/// <summary>
/// Xuan said these values are no longer used or needed, but until this day, they are actually used.
/// It's a very small cost to keep them and it would be a risk to delete them
/// </summary>
typedef enum SearchFilterModifierValues // these are strings converted to numeric values
{
    SFMV_NOT_USED_VALUE, // avoid 0 values
    SFMV_LESS_THAN, // if performfilter->modifier was "less than"
    SFMV_NEGATE, // if performfilter->modifier contains "not"
}SearchFilterModifierValues;

/// <summary>
/// Search parameters received from GO managers. This structure should be hidden by "score" interface
/// There is a very similar structure WITHOUT the additional optimization variables
/// </summary>
typedef struct CompanyRoleFilter {
    CompanyRoleFilterTypes filterType; // converted value of "char *filter" to numeric. Could also use function pointers. Debatable
    struct SearchedString* textValueAsArray;
    char** textValueAsCharArray;
    unsigned int* projectIDList;
    unsigned int* groupIDList;
    unsigned int* industryIDList;

    size_t textValueArraySize;
    size_t textValueAsCharArrayCount;
    size_t projectIDListCount;
    size_t groupIDListCount;
    size_t industryIDListCount;
    SearchFilterModifierValues modifierNumeric; // converted from "modifier"
    int alwaysTrue; // In case of AI filters, every profile will be selected

    // values received from search JSON. Some of the values might get updated, so a local copy will be performed
    long long rangeLow;                // RangeLow            int              `json:"range_low"`
    long long rangeHigh;               // RangeHigh           int              `json:"range_high"`
    char* filter;                      // Filter              string       `json:"filter"`    -> contains a string which field to check against a profile. Also contains how to check the value
    char* modifier;                    // Modifier            string       `json:"modifier"`
    char* textValue;                   // TextValue           string       `json:"text_value"`    -> the value that should be used against the profile field
    char* codeValue;                   // CodeValue           string       `json:"code_value"`
    char* explainFiltering;            // when a filter is passed, explain why it happened
    size_t explainBytesWritten;
    size_t explainBytesAllocated;
}CompanyRoleFilter;

/// <summary>
/// Constructor
/// </summary>
/// <param name="filter"></param>
void initCompanyRoleFilter(CompanyRoleFilter* filter);

/// <summary>
/// Destructor
/// </summary>
/// <param name="filter"></param>
void freeCompanyRoleFilter(CompanyRoleFilter* filter);

/// <summary>
/// Split the filter into sub filters. filters are separated by the "#,#" token
/// </summary>
/// <param name="krf"></param>
/// <param name="splitToken"></param>
void CRF_TextValueToStringCompareStore(CompanyRoleFilter* krf, const char* splitToken);

/// <summary>
/// Split the "textvalue" based on separator. Convert the list of strings to a list of numbers.
/// If the list contained no "greater than 0" number, the filter will be considered "non functional"
/// </summary>
/// <param name="filter"></param>
/// <param name="krf"></param>
/// <returns>Error code</returns>
int CRF_ConvertCodeValueToProjectIds(CompanyRoleFilter* krf, const char* splitToken);

/// <summary>
/// Converts string filter type to numeric filter type
/// </summary>
/// <param name="filter"></param>
/// <param name="krf"></param>
/// <returns></returns>
int CRF_ConvertStringFilterTypeToInt(CompanyRoleFilter* krf);

/// <summary>
/// used by destructor to destroy field values
/// </summary>
/// <param name="krf"></param>
void freeTextValueArray(CompanyRoleFilter* krf);

/// <summary>
/// Convert an array of strings separated by token to C array of strings
/// </summary>
/// <param name="krf"></param>
/// <param name="splitToken"></param>
/// <returns></returns>
int CRF_TextValueToStringArray(CompanyRoleFilter* krf, const char* splitToken);

/// <summary>
/// Do multiple conversions to extract words, replace with variables, replace operators
/// </summary>
/// <param name="krf"></param>
/// <param name="splitToken"></param>
/// <returns></returns>
int CRF_TextValueToBooleanFilter(CompanyRoleFilter* krf);

#endif