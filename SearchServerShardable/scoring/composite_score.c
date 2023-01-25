#include <composite_score.h>
#include <scoring_definitions.h>
#include <search_filter.h>
#include <similarity_score.h>
#include <company_cached.h>
#include <profiling.h>
#include <logger.h>
#include <math.h>

//#define _PRINT_WHAT_IS_HAPPENING_IN_SCORING_
#ifdef _PRINT_WHAT_IS_HAPPENING_IN_SCORING_
    #define DS_LOG_MESSAGE(...) LOG_MESSAGE(__VA_ARGS__)
    void DS_LOG_START_END_DATE(time_t pStartDate, time_t pEndDate, const char *msg)
    {
        time_t diff = pEndDate - pStartDate;
        struct tm startDate, endDate;

        localtime_r(&pStartDate, &startDate);
        localtime_r(&pEndDate, &endDate);
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "%s Start %04d-%02d-%02d -> %04d-%02d-%02d. Diff days %f", msg,
            1900 + startDate.tm_year, startDate.tm_mon + 1, startDate.tm_mday,
            1900 + endDate.tm_year, endDate.tm_mon + 1, endDate.tm_mday, (float)diff/(float)NUMBER_SECONDS_IN_DAY);
    }
#else
    #define DS_LOG_MESSAGE(...)
    #define DS_LOG_START_END_DATE(startDate, endDate, msg)
#endif

void initCompositeScoreFloat(CompositeScoreFloat* score)
{
    score->total = 0;
    score->heuristicScore = 0;
    score->companyScore = 0;
    score->experienceScore = 0;
    score->skillsScore = 0;
    score->jobTitleScore = 0;
    score->relevantExperience = 0;
}

void freeCompositeScoreFloat(CompositeScoreFloat* score)
{
}

void initScoringCompositeScore(ScoringCompositeScore* score)
{
    score->companyRole = NULL;
    score->profile = NULL;
//    kv_init(score->companyIds);
    kv_init(score->filterTitles);
//    kv_init(score->filterIndustries);
    initCompositeScoreFloat(&score->scores);
    initPerformFilter(&score->filter);
    score->filterResult = -1;
    score->companyCachedList = NULL;
    score->companyCachedLargestId = 0;
    score->timeStamp = 0;
}

void freeScoringCompositeScore(ScoringCompositeScore* score)
{
    score->companyRole = NULL; // this is shared between worked threads and will be deallocated by "someone else"
    score->profile = NULL;
//    kv_destroy(score->companyIds);

    for (size_t indexTitles = 0; indexTitles < kv_size(score->filterTitles); indexTitles++)
    {
        SearchedString* title = &kv_A(score->filterTitles, indexTitles);
        freeSearchedString(title);
    }

    kv_destroy(score->filterTitles);
    kv_init(score->filterTitles); // make sure size does not point to invalid value

//    kv_destroy(score->filterIndustries);
    freeCompositeScoreFloat(&score->scores);
    freePerformFilter(&score->filter);
    // this is a shared resource and we should not touch it(only read it)
    score->companyCachedList = NULL;
    score->companyCachedLargestId = 0;
}

/// <summary>
/// Check if this profile is filtered out by role?
/// </summary>
/// <param name="score"></param>
/// <returns></returns>
static inline size_t checkValid(ScoringCompositeScore* score)
{
    /*
    if (score->profile->Unavailable)
    {
        score->scores->Unavailable = 1;
        return 0;
    }
    */

    if (score->companyRole->role >= 0)
    {
        const size_t indexActionedMax = kv_size(score->profile->actioned);
        for (size_t indexActioned = 0; indexActioned < indexActionedMax; indexActioned++)
        {
            const int32_t role = kv_A(score->profile->actioned, indexActioned);
            if (role == score->companyRole->role)
            {
                DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile role does not match score role.");
                return 0;
            }
        }
    }
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile %d is valid for filtering / scoring.", score->profile->id);
    return 1;
}

static void relevantWorkExperience(ScoringCompositeScore* score)
{
    int totalWorkExperience = 0;

    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile has %u positions.", kv_size(score->profile->positions));

    // these positions will get reparsed
    if (kv_size(score->profile->positions) > 0 && kv_size(score->filterTitles) > 0)
    {
        const struct PositionCached* selectedPositions[MaxSliceAlloc];
        size_t selectedPositionCount = 0;
        size_t selectedPositionsMax;
        if (kv_size(score->profile->positions) <= MaxSliceAlloc)
        {
            selectedPositionsMax = kv_size(score->profile->positions);
        }
        else
        {
            selectedPositionsMax = MaxSliceAlloc;
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Profile (id = %d) has %u positions, but "
                "scoring will only use %u.", score->profile->id,
                kv_size(score->profile->positions), MaxNumPositionsProcessedBySearch);
        }
        const time_t timeStamp = score->timeStamp; // allow the compiler to store it in a register

        const size_t indexTitlesMax = kv_size(score->filterTitles);
        for (size_t indexPosition = 0; indexPosition < selectedPositionsMax; indexPosition++)
        {
            const struct PositionCached* pos = &kv_A(score->profile->positions, indexPosition);
            DBString dbString;
            dbString.str = pos->title;
            int related = 0;
            for (size_t indexTitles = 0; indexTitles < indexTitlesMax; indexTitles++)
            {
                const SearchedString* title = &kv_A(score->filterTitles, indexTitles);
                int res = IsSearchiStringInDbString(title, &dbString);
                DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Check if string \"%s\" is in \"%s\"-%zu, result %d.",
                    title->strOriginal.str, dbString.str, title->wordCount, res);
                if (res == 1)
                {
                    related = 1;
                    break;
                }
            }

            if (!related)
            {
                continue;
            }

            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile has related experience. nr %zu.",
                selectedPositionCount);
            int checkPos = 1;

            if (selectedPositionCount > 0)
            {
                time_t prevAddedStamp = selectedPositions[selectedPositionCount - 1]->startDate;
                time_t curStamp = pos->startDate;
                if (curStamp > prevAddedStamp)
                {
                    checkPos = 0;
                }
            }

            if ((pos->startDate != getDate1_time_t()) && (checkPos == 1))
            {
                selectedPositions[selectedPositionCount] = pos;
                selectedPositionCount++;
            }
            else
            {
                DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Related experience start date is default=%d or is "
                    "greater than lastposition=%d.", pos->startDate == getDate1_time_t(), checkPos);
            }
        }

        if (selectedPositionCount > 0)
        {
            totalWorkExperience
                = (int)GetPositionCachedDuration(selectedPositions[0], timeStamp);
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile has %zu related experiences. Pos 0 experience %d",
                selectedPositionCount, totalWorkExperience);

            for (size_t indexPosition = 0; indexPosition < selectedPositionCount - 1;
                indexPosition++)
            {
                const struct PositionCached* posCurr = selectedPositions[indexPosition + 1];
                const struct PositionCached* posPrev = selectedPositions[indexPosition + 0];

                time_t curStartDate;
                if (posCurr->startDate == getDate1_time_t()) // we already made sure this is not happening
                {
                    curStartDate = timeStamp;
                }
                else
                {
                    curStartDate = posCurr->startDate;
                }

                time_t curEndDate;
                if (posCurr->endDate == getDate1_time_t())
                {
                    curEndDate = timeStamp;
                }
                else
                {
                    curEndDate = posCurr->endDate;
                }

                time_t prevStartDate = posPrev->startDate; 
                if (curEndDate > prevStartDate)
                {
                    DS_LOG_START_END_DATE(curStartDate, prevStartDate, "from prev position to cur");
                    totalWorkExperience += (int)(((prevStartDate - curStartDate) / NUMBER_SECONDS_IN_DAY));
                }
                else
                {
                    DS_LOG_START_END_DATE(curStartDate, curEndDate, "from cur position to cur");
                    totalWorkExperience += (int)(((curEndDate - curStartDate) / NUMBER_SECONDS_IN_DAY));
                }
            }
        }
        else
        {
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile has zero suitable positions.");
        }
    }

    score->scores.relevantExperience = CONVERT_DAYS_XP_TO_MONTHS(float, totalWorkExperience);

    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile relevant experience is %f months converted from %d days.",
        score->scores.relevantExperience, totalWorkExperience);
}

static void InitializeFilterTitles(ScoringCompositeScore* score)
{
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Filter has %u subfilters.", kv_size(score->filter.filters));

    for (size_t index = 0; index < kv_size(score->filter.filters); index++)
    {
        CompanyRoleFilter* crf = &kv_A(score->filter.filters, index);
        if (crf->filterType == CRFT_CURRENT_TITLE_INCLUDE
            || crf->filterType == CRFT_PREVIOUS_TITLE_INCLUDE
            || crf->filterType == CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE)
        {
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Subfilters has %zu subsub filters.",
                crf->textValueArraySize);

            for (size_t indexFilter = 0; indexFilter < crf->textValueArraySize; indexFilter++)
            {
                SearchedString* srcValue = &crf->textValueAsArray[indexFilter];
                kv_pushp(SearchedString, score->filterTitles);
                SearchedString* dstValue
                    = &kv_A(score->filterTitles, kv_size(score->filterTitles) - 1);
                initSearchedString(dstValue, srcValue->strOriginal.str);
                // copy all the values from the src store
                dstValue->isQuoteEnclosed = srcValue->isQuoteEnclosed;

                DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Adding subsubfilter \"%s\".",
                    dstValue->strOriginal.str);
            }
        }
    }
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Score has %u filterTitles.", kv_size(score->filterTitles));
}

/// <summary>
/// Setup filtering so that we can perform as fast as possible millions of times : convert strings to numeric
/// </summary>
/// <param name="score"></param>
static inline void CheckInitializeFilters(ScoringCompositeScore* score)
{
    if (score->filter.isInitialized == 0)
    {
        score->filter.companyCachedList = score->companyCachedList;
        score->filter.companyCachedLargestId = score->companyCachedLargestId;
        score->filter.filterCompanyID = score->companyRole->organizationID;

        // Set up filter parameters
        for (size_t index = 0; index < kv_size(score->companyRole->filters); index++)
        {
            struct SearchFilter* sourceFilter = &kv_A(score->companyRole->filters, index);
            // add a new filter to "performfilter"
            kv_pushp(CompanyRoleFilter, score->filter.filters);
            CompanyRoleFilter* destFilter = &kv_A(score->filter.filters, index);
            // copy all values to the new filter. We might change the values inside the performfilter
            initCompanyRoleFilter(destFilter);
            if (sourceFilter->name != NULL)
            {
                destFilter->filter = strdup(sourceFilter->name);
            }
            if (sourceFilter->modifier != NULL)
            {
                destFilter->modifier = strdup(sourceFilter->modifier);
            }
            if (sourceFilter->textValue != NULL)
            {
                destFilter->textValue = strdup(sourceFilter->textValue);
            }
            if (sourceFilter->codeValue != NULL)
            {
                destFilter->codeValue = strdup(sourceFilter->codeValue);
            }
            destFilter->rangeLow = sourceFilter->rangeLow;
            destFilter->rangeHigh = sourceFilter->rangeHigh;
        }

        // use the same timestamp both in filtering and scoring over the duration of the whole filtering process
        score->filter.timeStamp = score->timeStamp;

        // copy values to remove a pointer redirection. These are shared values. Do not try to change them !
        *(BitField*)(&score->bfLocalities) = score->companyRole->bfLocalities;

        // convert filter string values to numerical values where possible. This will change the "filters" and should only be run once
        initPerformFilterPreSearch(&score->filter);

        // Now that we have the filters, initialize filter for relevantExperience
        InitializeFilterTitles(score);
    }
}

/// <summary>
/// 
/// </summary>
/// <param name="score"></param>
/// <returns></returns>
static int checkFilters(ScoringCompositeScore* score)
{
    // From one search to another, this is all that is changed : profile + relevant experience
    score->filter.profile = score->profile;
//    filter.filterCompanyID = score->companyRole->companyID;
    score->filter.relevantExperience = score->scores.relevantExperience;

    // Do the actual filtering : 0 means one of the filters rejected this profile to be scored. By default all profiles will be selected for scoring
    int ret = ProfileValid(&score->filter);

    return ret;
}


static void calculateHCompanyScore(ScoringCompositeScore* score)
{
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Started.");
    if (score->filter.companyFilterPresent == 1 && score->filter.companyAI == 0)
    {
        score->scores.companyScore = 1;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Company filter without companyAI. Ending scoring.");
        return;
    }

    // lack of positions means profile can not have a company score
    if (kv_size(score->profile->positions) == 0)
    {
        score->scores.companyScore = 0;
        return;
    }

    size_t MaxPositionsToCheck;
    if (kv_size(score->profile->positions) <= MaxSliceAlloc)
    {
        MaxPositionsToCheck = kv_size(score->profile->positions);
    }
    else
    {
        MaxPositionsToCheck = MaxSliceAlloc;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Profile (id = %d) has %u positions, but "
            "scoring will only use %u.", score->profile->id,
            kv_size(score->profile->positions), MaxNumPositionsProcessedBySearch);
    }

    float weights[MaxSliceAlloc];
    float scores[MaxSliceAlloc];
    size_t valuesCount = 0;

    float total = 0;
    const time_t timeStamp = score->timeStamp; // allow the compiler to store it in a register

    for (size_t indexPosition = 0; indexPosition < MaxPositionsToCheck; indexPosition++)
    {
        struct PositionCached* pos = &kv_A(score->profile->positions, indexPosition);

        // check for invalid values
        int companyId = pos->companyId;
        if ((companyId == 0) || (pos->startDate == getDate1_time_t()))
        {
            continue;
        }

        // Get the parent company id
        if (companyId <= score->companyCachedLargestId && score->companyCachedList[companyId] != NULL)
        {
            companyId = score->companyCachedList[companyId]->parentId;
        }
        else
        {
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Missing company data for id %d.", companyId);
        }

        // get the most similar company
        float companyScore = GetCompanySimilarityScore(companyId, &score->companyRole->similarityScores);
        if (companyScore == 0.0f)
        {
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Company %d does not have a similarity score.", companyId);
            continue;
        }

        time_t now = timeStamp;
        time_t endDate;
        if (pos->endDate == getDate1_time_t())
        {
            endDate = now;
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Position is ongoing. Will use current timestamp.");
        }
        else
        {
            endDate = pos->endDate;
        }

        DS_LOG_START_END_DATE(pos->startDate, pos->endDate, "");

        time_t duration = now - endDate;
        float durationMonths = CONVERT_SECONDS_XP_TO_MONTHS(float, duration);
        float coeff = 1 / exp(0.02 * durationMonths);
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Duration months %f(from now), coeff %f, duration %d.",
            durationMonths, coeff, (int)duration);

        duration = endDate - pos->startDate;
        durationMonths = CONVERT_SECONDS_XP_TO_MONTHS(float, duration);
        if (durationMonths < 0)
        {
            durationMonths = 0;
        }

        float como = durationMonths * coeff;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Duration months %f, como %f, score %f, duration %d.",
            durationMonths, como, companyScore * como, (int)duration);
        if (como != 0)
        {
            total += como;
            weights[valuesCount] = como;
            scores[valuesCount] = companyScore * coeff;
            valuesCount++;
        }
    }

    if (valuesCount == 0)
    {
        score->scores.companyScore = 0;
        return;
    }

    float combined = 0;
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Total score is %f. %u values.", total, valuesCount);

    for (size_t index = 0; index < valuesCount; index++)
    {
        float companyScore = scores[index];
        float weight = weights[index];

        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Combined %f, adding %f, from score %f, weight %f, total %f.",
            combined, companyScore* (weight / total), score, weight, total);
        combined += companyScore * (weight / total);
    }
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Company score is %f.", combined);

    score->scores.companyScore = (float)(combined);
}

static void calculateHJobTitleScore(ScoringCompositeScore* score)
{
    if (score->filter.titleFilterPresent == 1 && score->filter.titleAI == 0)
    {
        score->scores.jobTitleScore = 1;
        return;
    }

    // lack of positions means profile can not have a company score
    if (kv_size(score->profile->positions) == 0)
    {
        score->scores.jobTitleScore = 0;
        return;
    }

    size_t MaxPositionsToCheck;
    if (kv_size(score->profile->positions) <= MaxSliceAlloc)
    {
        MaxPositionsToCheck = kv_size(score->profile->positions);
    }
    else
    {
        MaxPositionsToCheck = MaxSliceAlloc;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Profile (id = %d) has %u positions, but "
            "scoring will only use %u.", score->profile->id,
            kv_size(score->profile->positions), MaxNumPositionsProcessedBySearch);
    }

    float weights[MaxSliceAlloc];
    float scores[MaxSliceAlloc];
    size_t valuesCount = 0;

    float total = 0;
    const time_t timeStamp = score->timeStamp; // allow the compiler to store it in a register

    for (size_t indexPosition = 0; indexPosition < MaxPositionsToCheck; indexPosition++)
    {
        struct PositionCached* pos = &kv_A(score->profile->positions, indexPosition);

        // check for invalid values
        if (pos->startDate == getDate1_time_t())
        {
            continue;
        }

        float scoreVal = GetTitleSimilarityScore(score->profile->id, pos->parentTitletId,
            &score->companyRole->similarityScores);
        if (scoreVal == 0.0)
        {
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "(Profile id = %d, title id = %d) does not have a similarity score.",
                score->profile->id, pos->titleId);
            continue;
        }
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "pos %zu, Title '%d' has similarity score %f", indexPosition, pos->parentTitletId, scoreVal);
        time_t now = timeStamp;
        time_t endDate;
        if (pos->endDate == getDate1_time_t())
        {
            endDate = now;
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Position is ongoing. Will use current timestamp.");
        }
        else
        {
            endDate = pos->endDate;
        }

        DS_LOG_START_END_DATE(pos->startDate, pos->endDate, "");

        time_t duration = now - endDate;
        float durationMonths = CONVERT_SECONDS_XP_TO_MONTHS(float, duration);
        float coeff = 1 / exp(0.02 * durationMonths);
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Duration months %f(from now), coeff %f, duration %lld.",
            durationMonths, coeff, duration);

        duration = endDate - pos->startDate;
        durationMonths = CONVERT_SECONDS_XP_TO_MONTHS(float, duration);
        if (durationMonths < 0)
        {
            durationMonths = 0;
        }

        float como = durationMonths * coeff;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Duration months %f, como %f, similarityScore %f, score %f, duration %lld.",
            durationMonths, como, scoreVal, scoreVal * coeff, duration);
        if (como != 0)
        {
            total += como;
            weights[valuesCount] = como;
            scores[valuesCount] = scoreVal * coeff;
            valuesCount++;
        }
    }

    if (valuesCount == 0)
    {
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "No suitable positions were found for score. Setting to 0.");
        score->scores.jobTitleScore = 0;
        return;
    }

    float combined = 0;
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Total score is %f. %u values.", total, valuesCount);

    for (size_t index = 0; index < valuesCount; index++)
    {
        float score = scores[index];
        float weight = weights[index];

        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Combined %f, adding %f, from score %f, weight %f, total %f.",
            combined, score * (weight / total), score, weight, total);
        combined += score * (weight / total);
    }
    DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Title score is %f.", combined);

    score->scores.jobTitleScore = (float)(combined);
}

static inline float Logistic(float value, float min, float max, float scale, float dropout)
{
    if (value > max)
    {
        return 1;
    }
    if (value < min)
    {
        return 0;
    }
    float xmax = max - min;
    float xval = value - min;
    float xone = xmax - dropout;
    float xmid = xmax / 2;
    float y = 99;
    float k = log(1 / (scale / y - 1)) / ((xone)-xmid);
    return scale / (1 + exp(-k * (xval - xmid)));
}

static inline void CalculateExperience(float low, float high, float val, float* res, int* ind)
{
    if (low == 0 && high == 0)
    {
        *res = 1;
        *ind = 0;
        return;
    }
    float lowScore;
    float highScore;
    if (val >= low)
    {
        lowScore = 1;
    }
    else
    {
        lowScore = (Logistic(val, 0, low, 100, 0.1)) / 100;
    }
    if (val < high || high == 0)
    {
        highScore = 1;
    }
    if (val > high && high != 0)
    {
        float diff = val - high;
        float dist;
        if (diff > high)
        {
            dist = high;
            high = val;
        }
        else
        {
            dist = high - diff;
        }
        highScore = Logistic(dist, 0, high, 100, 0.1) / 100;
    }
    if (lowScore == highScore)
    {
        *res = lowScore;
        *ind = 0;
        return;
    }
    if (lowScore < highScore)
    {
        *res = lowScore;
        *ind = -1;
        return;
    }
    if (lowScore > highScore)
    {
        *res = highScore;
        *ind = 1;
        return;
    }
}

static void calculateHExperienceScore(ScoringCompositeScore* score)
{
    if (score->filter.calculateRelevantExperienceScore != NULL)
    {
        float low = (float)(score->filter.calculateRelevantExperienceScore->rangeLow);
        float high = (float)(score->filter.calculateRelevantExperienceScore->rangeHigh);
        //        m.scores.ExperienceType = "relevant"
        float res = 0.0;
        int ind = 0;
        CalculateExperience(low, high, score->scores.relevantExperience, &res, &ind);
//        m.scores.ExperienceScoreInd = rInd
        score->scores.experienceScore = (float)res;
    }
    else if (score->filter.calculateTotalExperienceScore != NULL)
    {
        float low = (float)(score->filter.calculateTotalExperienceScore->rangeLow);
        float high = (float)(score->filter.calculateTotalExperienceScore->rangeHigh);
        float actual = (float)(score->profile->totalExperienceMonths);
//        m.scores.ExperienceType = "total"
        float res = 0.0;
        int ind = 0;
        CalculateExperience(low, high, actual, &res, &ind);
//        m.scores.ExperienceScoreInd = rInd
        score->scores.experienceScore = (float)res;
    }
}

static void calculateHSkillsScore(ScoringCompositeScore* score)
{
    if (score->filter.industryFilterPresent == 1 && score->filter.industryAI == 0)
    {
        score->scores.skillsScore = 1.0f;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Industry filter is present without AI. Skipping skillScore.");
        return;
    }
    if (score->filter.keywordsFilterPresent)
    {
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Keywords filter is present. Skipping skillScore.");
        score->scores.skillsScore = 1.0f;
        return;
    }

    // lack of positions means profile can not have a company score
    if (kv_size(score->profile->positions) == 0)
    {
        score->scores.skillsScore = 0;
        return;
    }

    size_t MaxPositionsToCheck;
    if (kv_size(score->profile->positions) <= MaxSliceAlloc)
    {
        MaxPositionsToCheck = kv_size(score->profile->positions);
    }
    else
    {
        MaxPositionsToCheck = MaxSliceAlloc;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Profile (id = %d) has %u positions, but "
            "scoring will only use %u.", score->profile->id,
            kv_size(score->profile->positions), MaxNumPositionsProcessedBySearch);
    }

    score->scores.skillsScore = 0.0f;
    const time_t timeStamp = score->timeStamp; // allow the compiler to store it in a register

    for (size_t indexPosition = 0; indexPosition < MaxPositionsToCheck; indexPosition++)
    {
        struct PositionCached* pos = &kv_A(score->profile->positions, indexPosition);

        // check for invalid values
        int companyId = pos->companyId;
        if ((companyId == 0) || (pos->startDate == getDate1_time_t()))
        {
            continue;
        }

        // Get the parent company id
        if (companyId <= score->companyCachedLargestId && score->companyCachedList[companyId] != NULL)
        {
            companyId = score->companyCachedList[companyId]->parentId;
        }

        if (companyId > score->companyCachedLargestId || score->companyCachedList[companyId] == NULL)
        {
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Missing company data for id %d.", companyId);
            continue;
        }
        const struct CompanyCached* comp = score->companyCachedList[companyId];
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Company %d has %d industries.", companyId, kv_size(comp->industries));
        if (kv_size(comp->parentIndustryIds) == 0)
        {
            continue;
        }

        time_t now = timeStamp;
        time_t endDate;
        if (pos->endDate == getDate1_time_t())
        {
            endDate = now;
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Position is ongoing. Will use current timestamp.");
        }
        else
        {
            endDate = pos->endDate;
        }

        DS_LOG_START_END_DATE(pos->startDate, pos->endDate, "");

        time_t duration = now - endDate;
        float durationMonths = CONVERT_SECONDS_XP_TO_MONTHS(float, duration);
        float coeff = 1 / exp(0.02 * durationMonths);
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Duration months %f(from now), coeff %f, duration %d.",
            durationMonths, coeff, (int)duration);

        for (size_t indexInd = 0; indexInd < kv_size(comp->parentIndustryIds); indexInd++)
        {
            // get the most similar company
            float industryScore = GetIndustrySimilarityScore(kv_A(comp->parentIndustryIds, indexInd), &score->companyRole->similarityScores);
            if (industryScore == 0.0f)
            {
                DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Industry %d does not have a similarity score.", kv_A(comp->parentIndustryIds, indexInd));
                continue;
            }
            float como = industryScore * coeff;
            DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Industry %d, industry score %f, coeff %f, como %f, cur score %f.", kv_A(comp->parentIndustryIds, indexInd),
                industryScore, coeff, como, score->scores.skillsScore);
            if (como > score->scores.skillsScore)
            {
                score->scores.skillsScore = como;
            }
        }
    }
}

static void calculateHTotalScore(ScoringCompositeScore* score)
{
    const float CompanyHWeight = 0.5f;
    const float JobTitleHWeight = 0.2f;
    const float ExperienceHWeight = 0.15f;
    const float SkillsHWeight = 0.15f;

    float totalWeight = 0;
    if (score->filter.companyFilterPresent == 1)
    {
        totalWeight += CompanyHWeight;
        score->scores.heuristicScore += score->scores.companyScore * CompanyHWeight;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Company filter present, adding weighted score %f.",
            score->scores.companyScore * CompanyHWeight);
    }
    if (score->filter.titleFilterPresent == 1)
    {
        totalWeight += JobTitleHWeight;
        score->scores.heuristicScore += score->scores.jobTitleScore * JobTitleHWeight;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Title filter present, adding weighted score %f.",
            score->scores.jobTitleScore * JobTitleHWeight);
    }
    if (score->filter.calculateRelevantExperienceScore != NULL || score->filter.calculateTotalExperienceScore != NULL)
    {
        totalWeight += ExperienceHWeight;
        score->scores.heuristicScore += score->scores.experienceScore * ExperienceHWeight;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Experience filter present, adding weighted score %f.",
            score->scores.experienceScore * ExperienceHWeight);
    }
    if (score->filter.industryFilterPresent == 1 || score->filter.keywordsFilterPresent == 1)
    {
        totalWeight += SkillsHWeight;
        score->scores.heuristicScore += score->scores.skillsScore * SkillsHWeight;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Industry filter present, adding weighted score %f.",
            score->scores.skillsScore * SkillsHWeight);
    }
    if (score->filter.companyFilterPresent == 0 && score->filter.titleFilterPresent == 0 && score->filter.calculateRelevantExperienceScore == NULL
        && score->filter.calculateTotalExperienceScore == NULL && score->filter.industryFilterPresent == 0)
    {
        score->scores.total = 1;
        score->scores.heuristicScore = 1;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "No filters, settings scores to 1.");
    }
    else
    {
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "heuristical = %f. Weight = %f. Prev total = %f",
            score->scores.heuristicScore, totalWeight, score->scores.total);
        if (totalWeight != 0)
        {
            score->scores.total = score->scores.heuristicScore / totalWeight;
        }
        score->scores.heuristicScore = score->scores.total;
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Total score is %f, heuristical score is %f.",
            score->scores.total, score->scores.heuristicScore);
    }
}

void setScoringCompositeScoreSearchCriteria(ScoringCompositeScore* score, struct SearchCriteria* searchCriteria, time_t timeStampOverride)
{
    // store filter values. These are shared values !
    score->companyRole = searchCriteria;

    // initialize timestamp so that multiple threads use a very similar timestamp
    if (timeStampOverride == 0)
    {
        score->timeStamp = time(NULL);
    }
    else
    {
        score->timeStamp = timeStampOverride;
    }

    // In case this is the first time we are using filtering, make sure to convert as many string values to numeric as possible
    CheckInitializeFilters(score);
}

/// <summary>
/// Main function of scoring. If filters allow it, the profile will receive some score values
/// </summary>
/// <param name="score"></param>
/// <returns>score->scores will be filled out with values</returns>
runCompareReturnCode runCompare(ScoringCompositeScore* score)
{
    /// If profile locality Id is not found in requested locality array, profile is not suitable for scoring
    if (score->bfLocalities.size != 0)
    {
        size_t isProfileLocalityRequested;
        BitFieldHasValue(score->bfLocalities, score->profile->localityId, isProfileLocalityRequested);
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "%s:%d: bitfield found=%d, searched locality id %d for profile %d\n",
            __FUNCTION__, __LINE__, isProfileLocalityRequested != 0, score->profile->localityId, score->profile->id);
        // if profile is not within the requested locality list
        if (isProfileLocalityRequested == 0)
        {
            return RCRC_NOT_A_VALID_PROFILE;
        }
    }

    // this is a role filter on profile->actioned
    if (!checkValid(score))
    {
        return RCRC_NOT_A_VALID_PROFILE;
    }

    StartInlinedProfilingThreadSafe(PE_RELEVANT_EXPERIENCE);

    // make sure that scores are reset. Required if the ScoringCompositeScore is reused
    initCompositeScoreFloat(&score->scores);

    // title filter based sum of experiences
    if (score->filter.calculateRelevantExperienceScore != NULL)
    {
        relevantWorkExperience(score);
    }

    EndInlinedProfilingCalcMaxThreadSafe(PE_RELEVANT_EXPERIENCE);

    // any of the filters deny this profile from receiving score ?
    StartInlinedProfilingThreadSafe(PE_FILTERING);

    score->filterResult = checkFilters(score);

    EndInlinedProfilingCalcMaxThreadSafe(PE_FILTERING);

    if (score->filterResult == 0)
    {
        DS_LOG_MESSAGE(DEBUG_LOG_MSG, "Profile was rejected by filters.");
        return RCRC_REJECTED_BY_FILTERING; // did not pass filtering
    }

    StartInlinedProfilingThreadSafe(PE_SCORING);

    // filters did not need relevant experience, so we delayed calculation just in case filters rejected this profile
    if (score->filter.calculateRelevantExperienceScore == NULL)
    {
        relevantWorkExperience(score);
    }

    calculateHCompanyScore(score);
    calculateHJobTitleScore(score);
    calculateHExperienceScore(score);
    calculateHSkillsScore(score);
    calculateHTotalScore(score);

    EndInlinedProfilingCalcMaxThreadSafe(PE_SCORING);

    return RCRC_NO_ERRORS;
}


runCompareReturnCode runCompare_explain(ScoringCompositeScore* score)
{
    score->filter.profile = score->profile;
    score->filter.relevantExperience = score->scores.relevantExperience;
    return ProfileValid_explain(&score->filter);
}
