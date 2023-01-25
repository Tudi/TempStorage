#ifndef _COMPOSITE_SCORING_H_
#define _COMPOSITE_SCORING_H_

#include <kvec.h>
#include <profile_cached.h>
#include <strings_ext.h>
#include <filters.h>
#include <search_criteria.h>

/// <summary>
/// This is a temporary store because it has values in "float" instead of uint16_t. 
/// At the end of the calculations, values can be converted to the final "CompositeScore" structure
/// </summary>
typedef struct CompositeScoreFloat
{
    float total;                  // float32 `json:"t"`
    float heuristicScore;         // float32 `json:"hs"`
    float companyScore;           // float32 `json:"cs"`
    float experienceScore;        // float32 `json:"e"`
    float skillsScore;            // float32 `json:"s"`
    float jobTitleScore;          // float32 `json:"j"`
    float relevantExperience;     // float32 `json:"rel"`
}CompositeScoreFloat;

// companyrolefilter is searchfilter
typedef struct ScoringCompositeScore {
    const struct SearchCriteria *companyRole;            //    r       models.CompanyRole     // Role
    const struct ProfileCached  *profile;                //    p       models.Profile         // Profile
//    kvec_t(int)    companyIds;                         //    s       []int                  // Filter Company ID's
    kvec_t(SearchedString)      filterTitles;            //    t       []string               // Filter Titles
//    kvec_t(int)    filterIndustries;                   //    i       []int                  // Filter Industries
    CompositeScoreFloat         scores;                  //    scores  models.CompositeScore
    // Below fields are C only ( not mapped to GO )
    PerformFilter               filter;                  // no need to create a new filter just because profile changed ?
    int                         filterResult;            // this is only for the sake of debugging
    const struct CompanyCached** companyCachedList;      // passed down from search engine
    size_t                      companyCachedLargestId;  // max elements in the company array
    time_t                      timeStamp;               // Created at initialization to avoid having inconsistent scoring results due to multi threading
    const BitField              bfLocalities;            // copied from SearchCriteria. Do not deallocate it !
}ScoringCompositeScore;

void initScoringCompositeScore(ScoringCompositeScore* score);
void freeScoringCompositeScore(ScoringCompositeScore* score);

/// <summary>
/// Set search criteria and also initialize filters
/// This function should be called only once after initScoringCompositeScore and before scoring multiple profiles
/// </summary>
/// <param name="score"></param>
/// <param name="searchCriteria"></param>
void setScoringCompositeScoreSearchCriteria(ScoringCompositeScore* score, struct SearchCriteria* searchCriteria, time_t timeStampOverride);

typedef enum runCompareReturnCode
{
    RCRC_NO_ERRORS = 0,
    RCRC_NOT_A_VALID_PROFILE,
    RCRC_REJECTED_BY_FILTERING
}runCompareReturnCode;

/// <summary>
/// Calculate RelevantExperience. Check if it passes the filter test. If it passed, calculate other score types
/// You can reuse this structure and only replace the "profile" field
/// Result scores will be found in "scores" field
/// </summary>
/// <param name="score"></param>
/// <returns>score->scores will be filled out with values</returns>
runCompareReturnCode runCompare(ScoringCompositeScore* score);
runCompareReturnCode runCompare_explain(ScoringCompositeScore* score);
#endif
