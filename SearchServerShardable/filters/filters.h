#ifndef _FILTERS_H_
#define _FILTERS_H_ 

#include <kvec.h>
#include <companyrolefilter.h>
#include <search_filter.h>

//#define _PRINT_WHAT_IS_HAPPENING_IN_FILTERS_
#ifdef _PRINT_WHAT_IS_HAPPENING_IN_FILTERS_
    #include <stdio.h>
    #define NONULLSTR(x) ((x)==NULL?("NULL"):(x))
    #define DF_LOG_MESSAGE(...)    LOG_MESSAGE(DEBUG_LOG_MSG, __VA_ARGS__)
    #define DF_ASSERT(x, ...) { if(!(x)) {LOG_MESSAGE(DEBUG_LOG_MSG, __VA_ARGS__);} }
#else
    #define DF_LOG_MESSAGE(...)
    #define DF_ASSERT(x, ...)
#endif

// This is just for the sake of debugging. To make sure filter values are set. These should not be practically used
#define FILTER_INVALID_INT          (-0xBAD)
#define FILTER_INVALID_FLOAT        (-0xBAD)
// To avoid strange situations. These limits in theory will never be hit
#define MAX_SPLIT_RESULTS            1000

/// <summary>
/// Structure that holds a search session state. Search thread will swap "profile" from 1 search to another
/// </summary>
typedef struct PerformFilter {
    // these filters will be deleted on destruct !
    kvec_t(CompanyRoleFilter) filters;                     // Filters                       []models.CompanyRoleFilter
    // only a reference to a memory cached profile
    const struct ProfileCached *profile;                   // Profile                       models.Profile
    int filterCompanyID;                                   // FilterCompanyID               int
    float relevantExperience;                              // RelevantExperience            float32
    char companyAI;                                        // CompanyAI                     bool
    char titleAI;                                          // TitleAI                       bool
    char experienceAI;                                     // ExperienceAI                  bool
    char industryAI;                                       // IndustryAI                    bool
    char mostCurrentTitle;                                 // MostCurrentTitle              bool
    char keywordBoolean;                                   // KeywordBoolean                bool
    char positiveReply;                                    // PositiveReply                 bool
    char positiveReplyModifier;                            // PositiveReplyModifier         bool
    int replyInDays;                                       // ReplyInDays                   int

    int isInitialized;                                     // No need to check for string values at every API call
    int companyFilterPresent;
    int titleFilterPresent;
    CompanyRoleFilter* calculateRelevantExperienceScore;   // does not need to be destroyed. They are using shared memory
    CompanyRoleFilter* calculateTotalExperienceScore;      // does not need to be destroyed. They are using shared memory
    int industryFilterPresent;
    int keywordsFilterPresent;
    const struct CompanyCached** companyCachedList;        // passed down from scoring module
    size_t companyCachedLargestId;                         // max elements in the company array
    time_t timeStamp;                                      // Created at initialization to avoid calling time() millions of time
    kvec_t(SearchFilterExplain) filterExplained;           // what values triggered this filter to include the profile
}PerformFilter;

/// <summary>
/// Constructor
/// </summary>
/// <param name="filter"></param>
void initPerformFilter(PerformFilter* filter);

/// <summary>
/// Destructor
/// </summary>
/// <param name="filter"></param>
void freePerformFilter(PerformFilter* filter);

/// <summary>
/// Called before the search is performed. This function will make sure NULL checks will not be required while search is performed
/// This function should be called only once per search session. Probably will be handled automatically by scoring interface
/// </summary>
/// <param name="filter">Structure used for filtering</param>
/// <returns>Error code</returns>
int initPerformFilterPreSearch(PerformFilter* filter);

/// <summary>
/// Worker thread replaces the value of "profile" and calls this function
/// </summary>
/// <param name="filter">Structure contains all the data to be able to decide if a profile should be included in result set</param>
/// <returns>1 = profile should be included in result set</returns>
int ProfileValid(const PerformFilter* filter);

// generate info which values made the filters return 1 ( or almost 1 )
int ProfileValid_explain(PerformFilter* filter);
#endif // _FILTERS_H_