#define CONCATENATE_MACRO(a,b) a ## b
#define GenerateFunctionName(Name, NameExt) CONCATENATE_MACRO(Name,NameExt)


static inline int CheckExperienceBetween(long long rangeLow, long long rangeHigh, long long experience)
{
    if (rangeLow <= experience && experience <= rangeHigh)
    {
        return 1;
    }
    return 0;
}

static inline int CheckExperienceGreaterThanEqual(const long long rangeLow, const long long rangeHigh, const long long experience)
{
    if (rangeLow <= experience)
    {
        return 1;
    }
    return 0;
}

static inline int CheckExperienceLessThanEqual(const long long rangeLow, const long long rangeHigh, const long long experience)
{
    if (experience <= rangeHigh)
    {
        return 1;
    }
    return 0;
}

static inline int CheckExperience(const long long rangeLow, const long long rangeHigh, const long long experience)
{
    DF_LOG_MESSAGE("low=%d actual=%d high=%d", (int)rangeLow, (int)experience, (int)rangeHigh);
    if (rangeLow != 0 && rangeHigh != 0)
    {
        return CheckExperienceBetween(rangeLow, rangeHigh, experience);
    }
    if (rangeLow != 0)
    {
        return CheckExperienceGreaterThanEqual(rangeLow, rangeHigh, experience);
    }
    if (rangeHigh != 0)
    {
        return CheckExperienceLessThanEqual(rangeLow, rangeHigh, experience);
    }

    return 0;
}

/// <summary>
/// A worker is expected to have worked at similar companies which are part of similar industries. 
/// We group together industries and only after that check if they are within the searched filter
/// </summary>
/// <param name="pSet"></param>
/// <param name="val"></param>
static inline void AddIndustryToSet(void* pSet, const int32_t val)
{
    kvec_t(int32_t)* set = pSet;
    const size_t maxI = kv_size(*set);
    for (size_t index = 0; index < maxI; index++)
    {
        if (kv_A(*set, index) == val)
        {
            return;
        }
    }
    kv_push(int32_t, *set, val);
    DF_LOG_MESSAGE("Adding industry %d to the set of values available", val);
}

/// <summary>
/// Looks up a company info. If company has parent, than parent company is prefered. Adds all industries to the result set
/// </summary>
/// <param name="companyId"></param>
/// <param name="pSet"></param>
static inline void MergeCompanyIndustriesIntoSet(const PerformFilter* __restrict performFilter, const int32_t companyId, void* pSet)
{
    kvec_t(int32_t)* set = pSet;
    if (companyId <= performFilter->companyCachedLargestId && performFilter->companyCachedList[companyId] != NULL)
    {
        const struct CompanyCached* company = performFilter->companyCachedList[companyId];
        if (company->parentId <= performFilter->companyCachedLargestId
            && performFilter->companyCachedList[company->parentId] != NULL)
        {
            company = performFilter->companyCachedList[company->parentId];
        }
        const size_t maxI = kv_size(company->parentIndustryIds);
        for (size_t index = 0; index < maxI; index++)
        {
            AddIndustryToSet(set, kv_A(company->parentIndustryIds, index));
        }
    }
    else
    {
        DF_LOG_MESSAGE("company %d not found, limit %zd\n", companyId, performFilter->companyCachedLargestId);
    }
}

/// <summary>
/// Compare 2 strings if they are equal. Strings are stored lower case + standardized
/// </summary>
/// <param name="scs1">String 1</param>
/// <param name="scs2">String 2</param>
/// <returns>1 if the searched string is found in DBstring</returns>
static inline int GenerateFunctionName(IsSearchiStringInDbStringLocal, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    TCCRF CompanyRoleFilter* __restrict companyRoleFilter, const SearchedString* searchedStr, const char* databaseStr)
{
    if (searchedStr->isQuoteEnclosed)
    {
        if (strcmp(databaseStr, searchedStr->strOriginal.str) == 0)
        {
            GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedStr->strOriginal.str);
            return 1;
        }
    }
    else
    {
        size_t wordsFound = 0;
        for (size_t index1 = 0; index1 < searchedStr->wordCount; index1++)
        {
            if (strstr(databaseStr, searchedStr->wordDescriptors[index1].str))
            {
                wordsFound++;
                GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
                    companyRoleFilter, searchedStr->wordDescriptors[index1].str);
            }
        }

        // did we find all the words we were looking for ?
        if (wordsFound == searchedStr->wordCount)
        {
            return 1;
        }
    }

    return 0;
}


/// <summary>
/// Like a google search on a DB string list : 
/// If quoted, it searches for exact match. 
/// else search for every word if present in db words
/// </summary>
/// <param name="source">array of words the website sent us</param>
/// <param name="sourceCount">number of entries in the array</param>
/// <param name="values">array of words in "Profile"</param>
/// <returns></returns>
static inline int GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)
(TCCRF CompanyRoleFilter* __restrict companyRoleFilter, const char* __restrict  dbString)
{
    DF_LOG_MESSAGE("started");
    int ret = 0;
    //    printf("String count in DB %d, string count searched %d\n", (int)dbStringsCount, (int)searchedStringsCount);
    const size_t searchedStringsCount = companyRoleFilter->textValueArraySize;
    const SearchedString* __restrict  searchedStrings = companyRoleFilter->textValueAsArray;
    for (size_t indexSearched = 0; indexSearched < searchedStringsCount; indexSearched++)
    {
        int strstrRet = GenerateFunctionName(IsSearchiStringInDbStringLocal, TEMPLATE_FUNCTION_NAME_SUFFIX)
            (companyRoleFilter, &searchedStrings[indexSearched], dbString);
        DF_LOG_MESSAGE("check quoted(%d) string '%s' is in '%s'- words : %d, result : %d", 
            searchedStrings[indexSearched].isQuoteEnclosed, searchedStrings[indexSearched].strOriginal.str, 
            dbString, (int)searchedStrings[indexSearched].wordCount, strstrRet);
        if (strstrRet == 1)
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

/// <summary>
/// Check if "textValue" contains any of the names the profile has
/// </summary>
/// <param name="profile"></param>
/// <param name="textValue">Names the webpage is searching for</param>
/// <returns></returns>
static inline int GenerateFunctionName(nameValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    // check the list of values we need compared to the list of values we have
    int ret = GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
        companyRoleFilter, profile->fullName);
    return ret;
}


/// <summary>
/// Check if a list of searched titles are present in the profile
/// </summary>
/// <param name="performFilter"></param>
/// <param name="profile"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(titleValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("titleValid: cur, positions %u\n", kv_size(profile->positions));
    int ret = 0;
    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (isPositionCurrent(pos))
        {
            if (GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, pos->title))
            {
                TEMPLATE_FUNCTION_RETURN_1;
            }
            if (performFilter->mostCurrentTitle)
            {
                break;
            }
        }
        // since active positions will always be at the start of the list, there is no point to check further
        else
        {
            break;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(titleValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("titleValid: prev, positions %u\n", kv_size(profile->positions));
    int ret = 0;
    size_t i = 0;
    const size_t maxI = kv_size(profile->positions);
    // since active positions will always be at the start of the list
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (!isPositionCurrent(pos))
        {
            break;
        }
    }
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
            companyRoleFilter, pos->title))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(titleValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("titleValid: cur_prev, positions %u\n", kv_size(profile->positions));
    int ret = 0;
    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
            companyRoleFilter, pos->title))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }

    return ret;
}


/// <summary>
/// Profile worked at one of the companies with "searched" name ?
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(companyValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("companyValid: cur, positions %u\n", kv_size(profile->positions));
    int ret = 0;
    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (isPositionCurrent(pos))
        {
            if (GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
                companyRoleFilter, pos->companyName))
            {
                TEMPLATE_FUNCTION_RETURN_1;
            }
        }
        // since active positions will always be at the start of the list, there is no point to check further
        else
        {
            break;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(companyValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("companyValid: prev, positions %u\n", kv_size(profile->positions));
    int ret = 0;
    size_t i = 0;
    // since active positions will always be at the start of the list
    const size_t maxI = kv_size(profile->positions);
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (!isPositionCurrent(pos))
        {
            break;
        }
    }
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        // Not having a date set means the position is current
        if (GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
            companyRoleFilter, pos->companyName))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(companyValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("companyValid: cur_prev, positions %u\n", kv_size(profile->positions));
    int ret = 0;
    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(IsSearchStringArrayInDbString, TEMPLATE_FUNCTION_NAME_SUFFIX)(
            companyRoleFilter, pos->companyName))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

/// <summary>
/// Search inside Headline, Summary, Keywords, Title, Description
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(keywordsValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* __restrict profile = performFilter->profile;
    for (size_t termIndex = 0; termIndex < companyRoleFilter->textValueArraySize; termIndex++)
    {
        const SearchedString* __restrict searchedWordsWithAND = &companyRoleFilter->textValueAsArray[termIndex];
        int wordsFound = 0;
        for (size_t wordIndex = 0; wordIndex < searchedWordsWithAND->wordCount; wordIndex++)
        {
            const char* __restrict searchedWord = searchedWordsWithAND->wordDescriptors[wordIndex].str;

            if (strstr(profile->skillsHeadlineSummaryPosDescription, searchedWord) != NULL)
            {
                wordsFound++;
                GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedWord);
                continue;
            }

            const size_t maxPositions = kv_size(profile->positions);
            for (size_t i = 0; i < maxPositions; i++)
            {
                const struct PositionCached* pos = &kv_A(profile->positions, i);
                if (strstr(pos->companyName, searchedWord))
                {
                    wordsFound++;
                    GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedWord);
                    break;
                }
                if (strstr(pos->title, searchedWord))
                {
                    wordsFound++;
                    GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedWord);
                    break;
                }
            }
        }

        if (wordsFound == searchedWordsWithAND->wordCount)
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }

    return ret;
}


/// <summary>
/// Search inside Headline, Summary, Keywords, Title, Description
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(keywordsBooleanValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* __restrict profile = performFilter->profile;
    char variableValues[256];
    memset(variableValues, 0, sizeof(variableValues));
    const size_t booleanWordCount = companyRoleFilter->textValueAsCharArrayCount;
    DF_LOG_MESSAGE("words=%zu, boolean string '%s'", booleanWordCount, companyRoleFilter->textValue);
    for (size_t boolWordIndex = 0; boolWordIndex < booleanWordCount; boolWordIndex++)
    {
        const char* searchedWord = companyRoleFilter->textValueAsCharArray[boolWordIndex];
        DF_LOG_MESSAGE("check if keyword present : \"%s\"", searchedWord);
        if (strstr(profile->skillsHeadlineSummaryPosDescription, searchedWord) != NULL)
        {
            variableValues[boolWordIndex] = 1;
            DF_LOG_MESSAGE("Found keyword in skillsHeadlineSummaryPosDescription: \"%s\"", searchedWord);
            GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedWord);
            continue;
        }

        const size_t maxPositions = kv_size(profile->positions);
        for (size_t i = 0; i < maxPositions; i++)
        {
            const struct PositionCached* pos = &kv_A(profile->positions, i);
            if (strstr(pos->companyName, searchedWord))
            {
                variableValues[boolWordIndex] = 1;
                DF_LOG_MESSAGE("Found keyword in companyName: \"%s\"", searchedWord);
                GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedWord);
                break;
            }
            if (strstr(pos->title, searchedWord))
            {
                variableValues[boolWordIndex] = 1;
                DF_LOG_MESSAGE("Found keyword in position title: \"%s\"", searchedWord);
                GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, searchedWord);
                break;
            }
        }
    }

    ret = calculate_boolean_expression(companyRoleFilter->textValue, variableValues);
    DF_LOG_MESSAGE("Result of boolean expression : %d", ret);

    return ret;
}

/// <summary>
/// Select all the profiles that received a message no more than X days. Or all the profiles that received no messages
/// </summary>
/// <param name="rangeLow"></param>
/// <param name="rangeHigh"></param>
/// <param name="companyID"></param>
/// <param name="p_dates"></param>
/// <returns></returns>
static inline int GenerateFunctionName(checkDate, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    TCCRF CompanyRoleFilter* __restrict companyRoleFilter, int filterCompanyID, const void* p_dates)
{
    int ret = 0;
    const kvec_t(struct Id_TimeValue)* dates = p_dates;
    DF_LOG_MESSAGE("has %u dates. Filtering for id %d", kv_size(*dates), filterCompanyID);
    // check if dates have the selected companyID
    const size_t maxI = kv_size(*dates);
    for (size_t i = 0; i < maxI; i++)
    {
        struct Id_TimeValue* timeValue = &kv_A(*dates, i);
        if (timeValue->id != filterCompanyID)
        {
            continue;
        }
        // is this message within the desired range ?
        long long messageUnixTimeDays = (timeValue->value / (time_t)NUMBER_SECONDS_IN_DAY);
        DF_LOG_MESSAGE("time stamp %lld - range %lld - diff %lld", 
            messageUnixTimeDays, companyRoleFilter->rangeHigh, messageUnixTimeDays - companyRoleFilter->rangeHigh);
        int isMessageOlderThanFilter = !(companyRoleFilter->rangeHigh < messageUnixTimeDays); // message is too old(smaller than given timestamp)
        if (isMessageOlderThanFilter)
        {
            TEMPLATE_FUNCTION_RETURN_1;
            GenerateFunctionName(AddExplainStamp, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, timeValue->value);
        }
    }
    return ret;
}

/// <summary>
/// Select all the profiles that received a message no more than X days. Or all the profiles that received no messages
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(messageValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    return GenerateFunctionName(checkDate, TEMPLATE_FUNCTION_NAME_SUFFIX)(
        companyRoleFilter, performFilter->filterCompanyID, &performFilter->profile->lastMessaged);
}

/// <summary>
/// Select all the profiles that sent a message no more than X days. Or all the profiles that sent no messages
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(replyValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    return GenerateFunctionName(checkDate, TEMPLATE_FUNCTION_NAME_SUFFIX)(
        companyRoleFilter, performFilter->filterCompanyID, &performFilter->profile->lastReplied);
}

/// <summary>
/// Check if experience field is withing required range
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(experienceValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    long long experience = performFilter->profile->totalExperienceMonths;
    int ret = CheckExperience(companyRoleFilter->rangeLow, companyRoleFilter->rangeHigh, experience);
    if (ret == 1)
    {
        TEMPLATE_FUNCTION_RETURN_1;
        GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, experience);
    }
    return ret;
}

/// <summary>
/// Select profile if time spent at current company is within selected limits
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(tenureValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    long long experience = 0;
    if (kv_size(performFilter->profile->positions))
    {
        experience = CONVERT_DAYS_XP_TO_MONTHS(long long, GetPositionCachedDuration(&kv_A(performFilter->profile->positions, 0), performFilter->timeStamp));
    }
    int ret = CheckExperience(companyRoleFilter->rangeLow, companyRoleFilter->rangeHigh, experience);
    if (ret == 1)
    {
        TEMPLATE_FUNCTION_RETURN_1;
        GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, experience);
    }
    return ret;
}

static inline int GenerateFunctionName(relevantValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = CheckExperience(companyRoleFilter->rangeLow, companyRoleFilter->rangeHigh, performFilter->relevantExperience);
    if (ret == 1)
    {
        TEMPLATE_FUNCTION_RETURN_1;
        GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, performFilter->relevantExperience);
    }
    return ret;
}

/// <summary>
/// Select profile if experience is within limits
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(totalValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = CheckExperience(companyRoleFilter->rangeLow, companyRoleFilter->rangeHigh, performFilter->profile->totalExperienceMonths);
    if (ret == 1)
    {
        TEMPLATE_FUNCTION_RETURN_1;
        GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, performFilter->profile->totalExperienceMonths);
    }
    return ret;
}

static inline int GenerateFunctionName(CheckHasIndustryInSet, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    TCCRF CompanyRoleFilter* __restrict companyRoleFilter, void* pIndustrySet)
{
    int ret = 0;
    kvec_t(int32_t)* industrySet = pIndustrySet;
    const size_t maxIndustries = kv_size(*industrySet);
    for (size_t indexSearchedIndustries = 0; indexSearchedIndustries < companyRoleFilter->industryIDListCount; indexSearchedIndustries++)
    {
        for (size_t indexAvailableIndustries = 0; indexAvailableIndustries < maxIndustries; indexAvailableIndustries++)
        {
            DF_LOG_MESSAGE("Checking if searched industry %d=%d available\n", (int32_t)companyRoleFilter->industryIDList[indexSearchedIndustries], kv_A(*industrySet, indexAvailableIndustries));
            if ((int32_t)companyRoleFilter->industryIDList[indexSearchedIndustries] == kv_A(*industrySet, indexAvailableIndustries))
            {
                TEMPLATE_FUNCTION_RETURN_1;
                GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->industryIDList[indexSearchedIndustries]);
            }
        }
    }
    return ret;
}

/// <summary>
/// Check if a profile's industry list contains one of the searched industries
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(industryValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("industryValid: cur, positions %u", kv_size(profile->positions));
    kvec_t(int32_t) industrySet;
    kv_init(industrySet);
    kv_resize(int32_t, industrySet, 1000);

    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (isPositionCurrent(pos))
        {
            MergeCompanyIndustriesIntoSet(performFilter, pos->companyId, &industrySet);
        }
        // since active positions will always be at the start of the list, there is no point to check further
        else
        {
            break;
        }
    }
    // check if values we are searching for are in the list of industries that were found on the company
    // As a speed improvement idea, you could break these functions up so that values fit in a cacheline : [x, x+16] segments
    int ret = GenerateFunctionName(CheckHasIndustryInSet, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, &industrySet);

    kv_destroy(industrySet);
    return ret;
}

static inline int GenerateFunctionName(industryValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("industryValid: prev, positions %u", kv_size(profile->positions));
    kvec_t(int32_t) industrySet;
    kv_init(industrySet);
    kv_resize(int32_t, industrySet, 1000);

    size_t i = 0;
    const size_t maxI = kv_size(profile->positions);
    // since active positions will always be at the start of the list
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (!isPositionCurrent(pos))
        {
            break;
        }
    }
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        MergeCompanyIndustriesIntoSet(performFilter, pos->companyId, &industrySet);
    }

    // check if values we are searching for are in the list of industries that were found on the company
    // As a speed improvement idea, you could break these functions up so that values fit in a cacheline : [x, x+16] segments
    int ret = GenerateFunctionName(CheckHasIndustryInSet, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, &industrySet);

    kv_destroy(industrySet);
    return ret;
}

static inline int GenerateFunctionName(industryValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const struct ProfileCached* __restrict profile = performFilter->profile;
    DF_LOG_MESSAGE("industryValid: cur_prev, positions %u", kv_size(profile->positions));
    kvec_t(int32_t) industrySet;
    kv_init(industrySet);
    kv_resize(int32_t, industrySet, 1000);

    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        MergeCompanyIndustriesIntoSet(performFilter, pos->companyId, &industrySet);
    }

    // check if values we are searching for are in the list of industries that were found on the company
    // As a speed improvement idea, you could break these functions up so that values fit in a cacheline : [x, x+16] segments
    int ret = GenerateFunctionName(CheckHasIndustryInSet, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, &industrySet);

    kv_destroy(industrySet);
    return ret;
}

/// <summary>
/// Select profile if it has a specific project from a specific company
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(EvaluateProject, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    DF_LOG_MESSAGE("profile has %u projects, filter has %zd projects", kv_size(performFilter->profile->projects), companyRoleFilter->projectIDListCount);
    const size_t maxIndexProjects = kv_size(performFilter->profile->projects);
    const size_t maxIndexProjectIdsSearched = companyRoleFilter->projectIDListCount;
    for (size_t indexProjects = 0; indexProjects < maxIndexProjects; indexProjects++)
    {
        const struct Id_Int32Value* iv = &kv_A(performFilter->profile->projects, indexProjects);
        for (size_t indexSearched = 0; indexSearched < maxIndexProjectIdsSearched; indexSearched++)
        {
            DF_LOG_MESSAGE("profile project id %d, filter project id %d, profile company %d, filter company %d", iv->id, companyRoleFilter->projectIDList[indexSearched], iv->value, performFilter->filterCompanyID);
            if (iv->id == companyRoleFilter->projectIDList[indexSearched])
            {
                if (iv->value == performFilter->filterCompanyID)
                {
                    TEMPLATE_FUNCTION_RETURN_1;
                    GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->projectIDList[indexSearched]);
                }
            }
        }
    }
    return ret;
}


/// <summary>
/// Select profile if it has a specific group from a specific company
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(EvaluateGroup, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const size_t maxIndexGroups = kv_size(performFilter->profile->groups);
    for (size_t indexGroups = 0; indexGroups < maxIndexGroups; indexGroups++)
    {
        const struct Id_Int32Value* iv = &kv_A(performFilter->profile->groups, indexGroups);
        for (size_t indexSearched = 0; indexSearched < companyRoleFilter->groupIDListCount; indexSearched++)
        {
            if (iv->id == companyRoleFilter->groupIDList[indexSearched])
            {
                if (iv->value == performFilter->filterCompanyID)
                {
                    TEMPLATE_FUNCTION_RETURN_1;
                    GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->groupIDList[indexSearched]);
                }
            }
        }
    }
    return ret;
}

/// <summary>
/// Search to see if profile is part of a group. In case he is not in a group, there is an option to also check in companies( why not create 2 separate filters ?)
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(groupValidInclude, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int filterResult = GenerateFunctionName(EvaluateGroup, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
    return filterResult;
}

static inline int GenerateFunctionName(groupValidExclude, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    return !GenerateFunctionName(EvaluateGroup, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
}


/// <summary>
/// Check if we received a reply, from a specific company, sooner than X days
/// </summary>
/// <param name="replyVect"></param>
/// <param name="days"></param>
/// <param name="CompanyId"></param>
/// <returns></returns>
static inline int GenerateFunctionName(CheckReply, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    TCCRF CompanyRoleFilter* __restrict companyRoleFilter, const void* replyVect, const time_t days, const int CompanyId, const time_t timeStamp)
{
    int ret = 0;
    const kvec_t(struct Id_TimeValue)* lastReplied = replyVect;

    // replies not older than this
    time_t limitLow = timeStamp - days * 24 * 60 * 60;

    DF_LOG_MESSAGE("profile has %u selected replies. Days before now %ld. Now stamp %ld. Our stamp %ld",
        kv_size(*lastReplied), days, time(NULL), timeStamp);
    const size_t maxIndexMessage = kv_size(*lastReplied);
    for (size_t indexReply = 0; indexReply < maxIndexMessage; indexReply++)
    {
        struct Id_TimeValue* reply = &kv_A(*lastReplied, indexReply);
        DF_LOG_MESSAGE("profile company id %d, filter company id %d", reply->id, CompanyId);
        if (reply->id != CompanyId)
        {
            continue;
        }

        DF_LOG_MESSAGE("profile time %ld, filter time %ld", reply->value, limitLow);
        if (limitLow < reply->value)
        {
            TEMPLATE_FUNCTION_RETURN_1;
            GenerateFunctionName(AddExplainStamp, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, reply->value);
        }
    }

    return ret;
}

static inline int GenerateFunctionName(replyFilterValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    const void* valueVect = &performFilter->profile->lastReplied;

    if (performFilter->positiveReply && !performFilter->positiveReplyModifier)
    {
        valueVect = &performFilter->profile->lastPositiveReply;
    }

    kvec_t(struct Id_TimeValue) repliedFiltered;
    kv_init(repliedFiltered);
    if (performFilter->positiveReplyModifier)
    {
        size_t maxVectCount = kv_size(performFilter->profile->lastReplied);
        kv_resize(struct Id_TimeValue, repliedFiltered, maxVectCount);
        // From the list of replies to this company
        const size_t maxIndexReplied = kv_size(performFilter->profile->lastReplied);
        const size_t maxIndexPositiveReplied = kv_size(performFilter->profile->lastPositiveReply);
        for (size_t indexReply = 0; indexReply < maxIndexReplied; indexReply++)
        {
            const struct Id_TimeValue* reply = &kv_A(performFilter->profile->lastReplied, indexReply);
            if (reply->id != performFilter->filterCompanyID)
            {
                continue;
            }
            for (size_t indexPositiveReply = 0; indexPositiveReply < maxIndexPositiveReplied; indexPositiveReply++)
            {
                const struct Id_TimeValue* posReply = &kv_A(performFilter->profile->lastPositiveReply, indexPositiveReply);
                if (reply->id != posReply->id)
                {
                    continue;
                }
                // select the companies that gave a positive reply
                if (posReply->value != getDate1_time_t())
                {
                    kv_pushp(struct Id_TimeValue, repliedFiltered);
                    DF_LOG_MESSAGE("adding selected time %u", kv_size(repliedFiltered));
                    struct Id_TimeValue* newVal = &kv_A(repliedFiltered, kv_size(repliedFiltered) - 1);
                    newVal->id = posReply->id;
                    newVal->value = posReply->value;
                }
            }
        }
        valueVect = &repliedFiltered;
    }

    int ret = GenerateFunctionName(CheckReply, TEMPLATE_FUNCTION_NAME_SUFFIX)(
        companyRoleFilter, valueVect, performFilter->replyInDays, performFilter->filterCompanyID, performFilter->timeStamp);

    kv_destroy(repliedFiltered);

    return ret;
}

static inline int GenerateFunctionName(checkStageForCompany, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter, size_t companyId)
{
    DF_LOG_MESSAGE("checkStageForCompany: company %zd. Max loaded %zd", (companyId), performFilter->companyCachedLargestId);
    if (companyId <= performFilter->companyCachedLargestId && performFilter->companyCachedList[companyId] != NULL)
    {
        DF_LOG_MESSAGE("checkStageForCompany: company %zd exists", (companyId));
        const struct CompanyCached* company = performFilter->companyCachedList[companyId];
        if (company->parentId <= performFilter->companyCachedLargestId
            && performFilter->companyCachedList[company->parentId] != NULL)
        {
            company = performFilter->companyCachedList[company->parentId];
        }
        else
        {
            DF_LOG_MESSAGE("checkStageForCompany: company %zd has missing parent", (companyId));
            return 0;
        }

        const char* stage = company->stage;
        // no point searching in empty string
        if (stage[0] == 0)
        {
            DF_LOG_MESSAGE("checkStageForCompany: company %zd does not have stage set", (companyId));
            return 0;
        }
        // check if this stage was requested by filter
        const size_t arrayCount = companyRoleFilter->textValueAsCharArrayCount;
        for (size_t i = 0; i < arrayCount; i++)
        {
            DF_LOG_MESSAGE("checkStageForCompany: compare '%s' with '%s'", stage, companyRoleFilter->textValueAsCharArray[i]);
            if (strcmp(stage, companyRoleFilter->textValueAsCharArray[i]) == 0)
            {
                GenerateFunctionName(AddExplainString, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, stage);
                return 1;
            }
        }
    }
    return 0;
}


static inline int GenerateFunctionName(stageValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* profile = performFilter->profile;
    DF_LOG_MESSAGE("stageValid: cur, profile id %d, position count %d", profile->id, kv_size(profile->positions));

    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (isPositionCurrent(pos))
        {
            if (GenerateFunctionName(checkStageForCompany, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter, pos->companyId))
            {
                TEMPLATE_FUNCTION_RETURN_1;
            }
        }
        // since active positions will always be at the start of the list, there is no point to check further
        else
        {
            break;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(stageValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* profile = performFilter->profile;
    DF_LOG_MESSAGE("stageValid: prev, profile id %d, position count %d", profile->id, kv_size(profile->positions));
    size_t i = 0;
    // since active positions will always be at the start of the list
    const size_t maxI = kv_size(profile->positions);
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (!isPositionCurrent(pos))
        {
            break;
        }
    }
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(checkStageForCompany, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter, pos->companyId))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(stageValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* profile = performFilter->profile;
    DF_LOG_MESSAGE("stageValid: cur_prev, profile id %d, position count %d", profile->id, kv_size(profile->positions));
    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(checkStageForCompany, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter, pos->companyId))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(countryValidIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    DF_LOG_MESSAGE("profile id %u , filter has %zd ids", performFilter->profile->id, companyRoleFilter->projectIDListCount);
    const size_t countryId = performFilter->profile->countryId;
    const size_t maxIndexProjectIdsSearched = companyRoleFilter->projectIDListCount;
    for (size_t indexSearched = 0; indexSearched < maxIndexProjectIdsSearched; indexSearched++)
    {
        if (countryId == companyRoleFilter->projectIDList[indexSearched])
        {
            TEMPLATE_FUNCTION_RETURN_1;
            GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->projectIDList[indexSearched]);
        }
    }
    return ret;
}


static inline int GenerateFunctionName(CheckNumberEmployees, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter, size_t companyId)
{
    int ret = 0;
    DF_LOG_MESSAGE("CheckNumberEmployees: company %zd, max %zd", (companyId), performFilter->companyCachedLargestId);
    if (companyId <= performFilter->companyCachedLargestId && performFilter->companyCachedList[companyId] != NULL)
    {
        DF_LOG_MESSAGE("CheckNumberEmployees: company %zd exists", (companyId));
        const struct CompanyCached* company = performFilter->companyCachedList[companyId];
        if (company->parentId <= performFilter->companyCachedLargestId
            && performFilter->companyCachedList[company->parentId] != NULL)
        {
            company = performFilter->companyCachedList[company->parentId];
        }
        else
        {
            DF_LOG_MESSAGE("CheckNumberEmployees: company %zd has missing parent", (companyId));
            return 0;
        }

        const int32_t numEmployees = company->numEmployees;
        // no point searching in empty string
        if (numEmployees < 0)
        {
            DF_LOG_MESSAGE("checkStageForCompany: company %zd does not have numEmployees set", (companyId));
            return 0;
        }
        // check if this stage was requested by filter
        DF_LOG_MESSAGE("checkStageForCompany: compare %5<%d<%d", companyRoleFilter->rangeLow, numEmployees, companyRoleFilter->rangeHigh);
        if (companyRoleFilter->rangeLow <= numEmployees && numEmployees <= companyRoleFilter->rangeHigh)
        {
            TEMPLATE_FUNCTION_RETURN_1;
            GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, numEmployees);
        }
    }
    return ret;
}

static inline int GenerateFunctionName(numberEmployeesValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* profile = performFilter->profile;
    DF_LOG_MESSAGE("numberEmployeesValid: profile %d, pos size %d",
        profile->id, kv_size(profile->positions));

    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (isPositionCurrent(pos))
        {
            if (GenerateFunctionName(CheckNumberEmployees, TEMPLATE_FUNCTION_NAME_SUFFIX)(
                performFilter, companyRoleFilter, pos->companyId))
            {
                TEMPLATE_FUNCTION_RETURN_1;
            }
        }
        // since active positions will always be at the start of the list, there is no point to check further
        else
        {
            break;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(numberEmployeesValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* profile = performFilter->profile;
    DF_LOG_MESSAGE("numberEmployeesValid: profile %d, pos size %d",
        profile->id, kv_size(profile->positions));
    size_t i = 0;
    // since active positions will always be at the start of the list
    const size_t maxI = kv_size(profile->positions);
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (!isPositionCurrent(pos))
        {
            break;
        }
    }
    for (; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(CheckNumberEmployees, TEMPLATE_FUNCTION_NAME_SUFFIX)(
            performFilter, companyRoleFilter, pos->companyId))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

static inline int GenerateFunctionName(numberEmployeesValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    const struct ProfileCached* profile = performFilter->profile;
    DF_LOG_MESSAGE("numberEmployeesValid: profile %d, pos size %d",
        profile->id, kv_size(profile->positions));
    const size_t maxI = kv_size(profile->positions);
    for (size_t i = 0; i < maxI; i++)
    {
        const struct PositionCached* pos = &kv_A(profile->positions, i);
        if (GenerateFunctionName(CheckNumberEmployees, TEMPLATE_FUNCTION_NAME_SUFFIX)(
            performFilter, companyRoleFilter, pos->companyId))
        {
            TEMPLATE_FUNCTION_RETURN_1;
        }
    }
    return ret;
}

/// <summary>
/// Is this profile selected for scoring ?
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(profileIdIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    DF_LOG_MESSAGE("profile id %u , filter has %zd ids", performFilter->profile->id, companyRoleFilter->projectIDListCount);
    const size_t profileId = performFilter->profile->id;
    const size_t maxIndexProjectIdsSearched = companyRoleFilter->projectIDListCount;
    for (size_t indexSearched = 0; indexSearched < maxIndexProjectIdsSearched; indexSearched++)
    {
        if (profileId == companyRoleFilter->projectIDList[indexSearched])
        {
            TEMPLATE_FUNCTION_RETURN_1;
            GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->projectIDList[indexSearched]);
        }
    }
    return ret;
}

static inline int GenerateFunctionName(stateIdIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    DF_LOG_MESSAGE("profile id %u , filter has %zd ids", performFilter->profile->id, companyRoleFilter->projectIDListCount);
    const size_t profileId = performFilter->profile->stateId;
    const size_t maxIndexProjectIdsSearched = companyRoleFilter->projectIDListCount;
    for (size_t indexSearched = 0; indexSearched < maxIndexProjectIdsSearched; indexSearched++)
    {
        if (profileId == companyRoleFilter->projectIDList[indexSearched])
        {
            TEMPLATE_FUNCTION_RETURN_1;
            GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->projectIDList[indexSearched]);
        }
    }
    return ret;
}

/// <summary>
/// Select profile if it has a specific campaign from a specific company
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(campaignIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    DF_LOG_MESSAGE("profile has %u campaigns, filter has %zd campaigns", kv_size(performFilter->profile->campaigns), companyRoleFilter->projectIDListCount);
    const size_t maxIndexCampaigns = kv_size(performFilter->profile->campaigns);
    const size_t maxIndexProjectIdsSearched = companyRoleFilter->projectIDListCount;
    for (size_t indexCampaigns = 0; indexCampaigns < maxIndexCampaigns; indexCampaigns++)
    {
        const struct Id_Int32Value* iv = &kv_A(performFilter->profile->campaigns, indexCampaigns);
        for (size_t indexSearched = 0; indexSearched < maxIndexProjectIdsSearched; indexSearched++)
        {
            DF_LOG_MESSAGE("profile campaign id %d, filter campaign id %d, profile company %d, filter company %d", iv->id, companyRoleFilter->projectIDList[indexSearched], iv->value, performFilter->filterCompanyID);
            if (iv->id == companyRoleFilter->projectIDList[indexSearched])
            {
                if (iv->value == performFilter->filterCompanyID)
                {
                    TEMPLATE_FUNCTION_RETURN_1;
                    GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->projectIDList[indexSearched]);
                }
            }
        }
    }
    return ret;
}

/// <summary>
/// Select the profile if it has one of the requested talent pool IDs
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns></returns>
static inline int GenerateFunctionName(talentPoolIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    int ret = 0;
    DF_LOG_MESSAGE("profile has %u talentpools, filter has %zd talentpoolIds", kv_size(performFilter->profile->talentPools), companyRoleFilter->projectIDListCount);
    const size_t maxIndexTalentPool = kv_size(performFilter->profile->talentPools);
    const size_t maxIndexTalentPoolIdsSearched = companyRoleFilter->projectIDListCount;
    for (size_t indexTalentPool = 0; indexTalentPool < maxIndexTalentPool; indexTalentPool++)
    {
        const int32_t talentPoolId = kv_A(performFilter->profile->talentPools, indexTalentPool);
        for (size_t indexSearched = 0; indexSearched < maxIndexTalentPoolIdsSearched; indexSearched++)
        {
            DF_LOG_MESSAGE("profile talentpool id %d, filter talentpool id %d", talentPoolId, companyRoleFilter->projectIDList[indexSearched]);
            if (talentPoolId == companyRoleFilter->projectIDList[indexSearched])
            {
                TEMPLATE_FUNCTION_RETURN_1;
                GenerateFunctionName(AddExplainInt64, TEMPLATE_FUNCTION_NAME_SUFFIX)(companyRoleFilter, companyRoleFilter->projectIDList[indexSearched]);
            }
        }
    }
    return ret;
}

/// <summary>
/// Checks if based on one specific filter, the profile should be included into the search result set
/// </summary>
/// <param name="performFilter"></param>
/// <param name="companyRoleFilter"></param>
/// <returns>0 means profile does not need to be present in the result set. 1 means check next filter to see if it returns 0</returns>
static inline int GenerateFunctionName(ConditionValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(
    const PerformFilter* __restrict performFilter, TCCRF CompanyRoleFilter* __restrict companyRoleFilter)
{
    DF_LOG_MESSAGE("started");
#ifdef _PRINT_WHAT_IS_HAPPENING_IN_FILTERS_
    if (companyRoleFilter->textValueAsArray != NULL)
    {
        DF_LOG_MESSAGE("arrived in ConditionValid, filter type is %d-'%s', profile name is %s, filter value %s\n", companyRoleFilter->filterType, NONULLSTR(companyRoleFilter->filter),
            NONULLSTR(performFilter->profile->fullName), companyRoleFilter->textValueAsArray[0].strOriginal.str);
    }
    else
    {
        DF_LOG_MESSAGE("arrived in ConditionValid, filter type is %d-'%s', profile name is %s\n", companyRoleFilter->filterType, NONULLSTR(companyRoleFilter->filter),
            NONULLSTR(performFilter->profile->fullName));
    }
#endif
    int ret = 1;
    switch (companyRoleFilter->filterType)
    {
    case CRFT_NAME_INCLUDES:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_NAME_INCLUDE);
        ret = GenerateFunctionName(nameValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_NAME_INCLUDE);
    } break;
    case CRFT_NAME_DOES_NOT_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_NAME_EXCLUDE);
        ret = !GenerateFunctionName(nameValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_NAME_EXCLUDE);
    } break;
    case CRFT_CURRENT_TITLE_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_TITLE_INCLUDE);
        ret = GenerateFunctionName(titleValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_TITLE_INCLUDE);
    } break;
    case CRFT_CURRENT_TITLE_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_TITLE_EXCLUDE);
        ret = !GenerateFunctionName(titleValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_TITLE_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_TITLE_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_TITLE_INCLUDE);
        ret = GenerateFunctionName(titleValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_TITLE_INCLUDE);
    } break;
    case CRFT_PREVIOUS_TITLE_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_TITLE_EXCLUDE);
        ret = !GenerateFunctionName(titleValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_TITLE_EXCLUDE);
    } break;
    case CRFT_CURRENT_PREVIOUS_TITLE_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_TITLE_INCLUDE);
        ret = GenerateFunctionName(titleValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_TITLE_INCLUDE);
    } break;
    case CRFT_CURRENT_PREVIOUS_TITLE_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_TITLE_EXCLUDE);
        ret = !GenerateFunctionName(titleValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_TITLE_EXCLUDE);
    } break;
    case CRFT_CURRENT_COMPANY_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_COMPANY_INCLUDE);
        ret = GenerateFunctionName(companyValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_COMPANY_INCLUDE);
    } break;
    case CRFT_CURRENT_COMPANY_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_COMPANY_EXCLUDE);
        ret = !GenerateFunctionName(companyValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_COMPANY_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_COMPANY_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_COMPANY_INCLUDE);
        ret = GenerateFunctionName(companyValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_COMPANY_INCLUDE);
    } break;
    case CRFT_PREVIOUS_COMPANY_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_COMPANY_EXCLUDE);
        ret = !GenerateFunctionName(companyValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_COMPANY_EXCLUDE);
    } break;
    case CRFT_CURRENT_PREVIOUS_COMPANY_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_COMPANY_INCLUDE);
        ret = GenerateFunctionName(companyValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_COMPANY_INCLUDE);
    } break;
    case CRFT_CURRENT_PREVIOUS_COMPANY_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_COMPANY_EXCLUDE);
        ret = !GenerateFunctionName(companyValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_COMPANY_EXCLUDE);
    } break;

    case CRFT_TAGS_INCLUDE:
    case CRFT_TAGS_EXCLUDE:
        break;

    case CRFT_KEYWORDS_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_INCLUDE);
        ret = GenerateFunctionName(keywordsValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_INCLUDE);
    } break;
    case CRFT_KEYWORDS_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_EXCLUDE);
        ret = !GenerateFunctionName(keywordsValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_EXCLUDE);
    } break;
    case CRFT_KEYWORDS_BOOLEAN_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_BOOLEAN_INC);
        ret = GenerateFunctionName(keywordsBooleanValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_BOOLEAN_INC);
    } break;
    case CRFT_KEYWORDS_BOOLEAN_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_BOOLEAN_EXC);
        ret = !GenerateFunctionName(keywordsBooleanValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_KEYWORDS_BOOLEAN_EXC);
    } break;
    case CRFT_MESSAGED_EXCLUDE_OLDER_THAN: // do not include profiles that have messages older than LimitHigh
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_MESSAGED_EXCLUDE);
        ret = GenerateFunctionName(messageValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_MESSAGED_EXCLUDE);
    } break;
    case CRFT_REPLIED_EXCLUDE_OLDER_THAN: // do not include profiles that have replies older than LimitHigh
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_REPLIED_EXCLUDE);
        ret = GenerateFunctionName(replyValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_REPLIED_EXCLUDE);
    } break;

    case CRFT_ATS_LAST_ACTIVITY:
    case CRFT_ATS_STATUS:
    case CRFT_ATS_EXISTS:
        break;

    case CRFT_EXPERIENCE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_EXPERIENCE);
        ret = GenerateFunctionName(experienceValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_EXPERIENCE);
    } break;
    case CRFT_CURRENT_INDUSTRY_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_INDUSTRY_INCLUDE);
        ret = GenerateFunctionName(industryValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_INDUSTRY_INCLUDE);
    } break;
    case CRFT_CURRENT_INDUSTRY_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_INDUSTRY_EXCLUDE);
        ret = !GenerateFunctionName(industryValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_INDUSTRY_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_INDUSTRY_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_INDUSTRY_INCLUDE);
        ret = GenerateFunctionName(industryValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_INDUSTRY_INCLUDE);
    } break;
    case CRFT_PREVIOUS_INDUSTRY_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_INDUSTRY_EXCLUDE);
        ret = !GenerateFunctionName(industryValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_INDUSTRY_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_CURRENT_INDUSTRY_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_INDUSTRY_INCLUDE);
        ret = GenerateFunctionName(industryValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_INDUSTRY_INCLUDE);
    } break;
    case CRFT_PREVIOUS_CURRENT_INDUSTRY_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_INDUSTRY_EXCLUDE);
        ret = !GenerateFunctionName(industryValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_INDUSTRY_EXCLUDE);
    } break;
    case CRFT_CURRENT_TENURE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_TENURE);
        ret = GenerateFunctionName(tenureValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_TENURE);
    } break;
    case CRFT_RELEVANT_EXPERIENCE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_RELEVANT_EXPERIENCE);
        ret = GenerateFunctionName(relevantValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_RELEVANT_EXPERIENCE);
    } break;
    case CRFT_TOTAL_EXPERIENCE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_TOTAL_EXPERIENCE);
        ret = GenerateFunctionName(totalValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_TOTAL_EXPERIENCE);
    } break;

    case CRFT_CURRENT_COMPANY_FUNCTION_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_FUNCTION_INCLUDE);
        ret = GenerateFunctionName(stageValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_FUNCTION_INCLUDE);
    } break;
    case CRFT_CURRENT_COMPANY_FUNCTION_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_FUNCTION_EXCLUDE);
        ret = !GenerateFunctionName(stageValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_FUNCTION_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_COMPANY_FUNCTION_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_FUNCTION_INCLUDE);
        ret = GenerateFunctionName(stageValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_FUNCTION_INCLUDE);
    } break;
    case CRFT_PREVIOUS_COMPANY_FUNCTION_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_FUNCTION_EXCLUDE);
        ret = !GenerateFunctionName(stageValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_FUNCTION_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_FUNCTION_INCLUDE);
        ret = GenerateFunctionName(stageValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_FUNCTION_INCLUDE);
    } break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_FUNCTION_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_FUNCTION_EXCLUDE);
        ret = !GenerateFunctionName(stageValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_FUNCTION_EXCLUDE);
    } break;
    case CRFT_CURRENT_COMPANY_NRE_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_NRE_INCLUDE);
        ret = GenerateFunctionName(numberEmployeesValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_NRE_INCLUDE);
    } break;
    case CRFT_CURRENT_COMPANY_NRE_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CUR_NRE_EXCLUDE);
        ret = !GenerateFunctionName(numberEmployeesValid_current, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CUR_NRE_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_COMPANY_NRE_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_NRE_INCLUDE);
        ret = GenerateFunctionName(numberEmployeesValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_NRE_INCLUDE);
    } break;
    case CRFT_PREVIOUS_COMPANY_NRE_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PREV_NRE_EXCLUDE);
        ret = !GenerateFunctionName(numberEmployeesValid_previous, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PREV_NRE_EXCLUDE);
    } break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_NRE_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_NRE_INCLUDE);
        ret = GenerateFunctionName(numberEmployeesValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_NRE_INCLUDE);
    } break;
    case CRFT_PREVIOUS_CURRENT_COMPANY_NRE_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_NRE_EXCLUDE);
        ret = !GenerateFunctionName(numberEmployeesValid_cur_prev, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_NRE_EXCLUDE);
    } break;
    case CRFT_PROJECTS_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PROJECTS_INCLUDE);
        ret = GenerateFunctionName(EvaluateProject, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PROJECTS_INCLUDE);
    } break;
    case CRFT_PROJECTS_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PROJECTS_EXCLUDE);
        ret = !GenerateFunctionName(EvaluateProject, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PROJECTS_EXCLUDE);
    } break;
    case CRFT_GROUPS_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_GROUPS_INCLUDE);
        ret = GenerateFunctionName(groupValidInclude, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_GROUPS_INCLUDE);
    } break;
    case CRFT_GROUPS_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_GROUPS_EXCLUDE);
        ret = GenerateFunctionName(groupValidExclude, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter); // The negation is inside the specially made function
        EndInlinedProfilingThreadSafe(PE_FILTER_GROUPS_EXCLUDE);
    } break;
    case CRFT_COUNTRY_ID_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_COUNTRY_INCLUDE);
        ret = GenerateFunctionName(countryValidIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_COUNTRY_INCLUDE);
    } break;
    case CRFT_COUNTRY_ID_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_COUNTRY_EXCLUDE);
        ret = !GenerateFunctionName(countryValidIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_COUNTRY_EXCLUDE);
    } break;
    case CRFT_REPLY_FILTER:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_REPLY);
        ret = GenerateFunctionName(replyFilterValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_REPLY);
    } break;
    case CRFT_REPLY_EXCLUDE_FILTER:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_REPLY);
        ret = !GenerateFunctionName(replyFilterValid, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_REPLY);
    } break;
    case CRFT_PROFILE_ID_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PID_INCLUDE);
        ret = GenerateFunctionName(profileIdIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PID_INCLUDE);
    } break;
    case CRFT_PROFILE_ID_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_PID_EXCLUDE);
        ret = !GenerateFunctionName(profileIdIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_PID_EXCLUDE);
    } break;
    case CRFT_STATE_ID_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_STATE_INCLUDE);
        ret = GenerateFunctionName(stateIdIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_STATE_INCLUDE);
    } break;
    case CRFT_STATE_ID_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_STATE_EXCLUDE);
        ret = !GenerateFunctionName(stateIdIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_STATE_EXCLUDE);
    } break;
    case CRFT_CAMPAIGN_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CAMPAIGN_INCLUDE);
        ret = GenerateFunctionName(campaignIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CAMPAIGN_INCLUDE);
    } break;
    case CRFT_CAMPAIGN_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_CAMPAIGN_EXCLUDE);
        ret = !GenerateFunctionName(campaignIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_CAMPAIGN_EXCLUDE);
    } break;
    case CRFT_TALENTPOOL_INCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_TALENTPOOL_INCLUDE);
        ret = GenerateFunctionName(talentPoolIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_TALENTPOOL_INCLUDE);
    } break;
    case CRFT_TALENTPOOL_EXCLUDE:
    {
        StartInlinedProfilingThreadSafe(PE_FILTER_TALENTPOOL_EXCLUDE);
        ret = !GenerateFunctionName(talentPoolIncludes, TEMPLATE_FUNCTION_NAME_SUFFIX)(performFilter, companyRoleFilter);
        EndInlinedProfilingThreadSafe(PE_FILTER_TALENTPOOL_EXCLUDE);
    } break;
    default: {
        DF_LOG_MESSAGE("Unknown filter type %d. Should have never got here", companyRoleFilter->filterType);
        return 1; // Do not include this profile into the result set. We have no idea how to filter it
    } // should signal some error or something
    };

    return ret;
}

#undef TEMPLATE_FUNCTION_NAME_SUFFIX
#undef TEMPLATE_FUNCTION_RETURN_1
#undef TCCRF
