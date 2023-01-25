#include <logger.h>
#include <filters.h>
#include <k_utils.h>
#include <strings_ext.h>
#include <profile_cached.h>
#include <position_cached.h>
#include <company_cached.h>
#include <profiling.h>
#include <boolean_parser.h>
#include <search_filter.h>

#define MIN_SIZE_EXTEND_EXPLAIN_STRING 1024

static inline int isExplainStringPresent(const char* explainStr, const char* str)
{
    if (explainStr == NULL)
    {
        return 0;
    }

    size_t strLen = strlen(str);
    // check if already added
    char* foundLoc = strstr(explainStr, str);
    if (foundLoc != NULL &&
        memcmp(&foundLoc[-3], "#,#", 3) == 0 &&
        memcmp(&foundLoc[strLen + 1], "#,#", 3))
    {
        return 1;
    }
    return 0;
}

static inline void AddExplainString_explain(CompanyRoleFilter* __restrict companyRoleFilter, const char* str)
{
    // check if already added
    if (isExplainStringPresent(companyRoleFilter->explainFiltering, str) == 1)
    {
        return;
    }

    size_t strLen = strlen(str);
    DF_ASSERT(strLen > 0, "Unexpected explain filter value '%s'", str);

    size_t bytesNeeded = strLen + 1 + 3;
    if (bytesNeeded <= 1)
    {
        DF_LOG_MESSAGE("unexpected 0 size searched string");
        return;
    }
    // Allocate meory for this string to be stored
    if (companyRoleFilter->explainBytesAllocated <= companyRoleFilter->explainBytesWritten + bytesNeeded)
    {
        if (companyRoleFilter->explainFiltering == NULL)
        {
            bytesNeeded += 3; // for the token
        }
        // allocate blocks to try to avoid multi allocations
        size_t newAllocSize = companyRoleFilter->explainBytesWritten + bytesNeeded;
        if (newAllocSize < companyRoleFilter->explainBytesAllocated + MIN_SIZE_EXTEND_EXPLAIN_STRING)
        {
            newAllocSize = companyRoleFilter->explainBytesAllocated + MIN_SIZE_EXTEND_EXPLAIN_STRING;
        }

        char* newStr = realloc(companyRoleFilter->explainFiltering, newAllocSize);
        if (newStr == NULL)
        {
            companyRoleFilter->explainBytesAllocated = 0;
            companyRoleFilter->explainBytesWritten = 0;
            free(companyRoleFilter->explainFiltering);
            companyRoleFilter->explainFiltering = NULL;
            return;
        }
        // every string is enclosed in separator token
        if (companyRoleFilter->explainFiltering == NULL)
        {
            companyRoleFilter->explainFiltering = newStr;
            memcpy(&companyRoleFilter->explainFiltering[companyRoleFilter->explainBytesWritten], "#,#", 3);
            companyRoleFilter->explainBytesWritten += 3;
        }
        else
        {
            companyRoleFilter->explainFiltering = newStr;
        }
        companyRoleFilter->explainBytesAllocated = newAllocSize;
    }

    // add the actual word that was found
    memcpy(&companyRoleFilter->explainFiltering[companyRoleFilter->explainBytesWritten], str, strLen);
    companyRoleFilter->explainBytesWritten += strLen;

    // add separator token
    memcpy(&companyRoleFilter->explainFiltering[companyRoleFilter->explainBytesWritten], "#,#", 3);
    companyRoleFilter->explainBytesWritten += 3;

    // always add 0 terminator
    companyRoleFilter->explainFiltering[companyRoleFilter->explainBytesWritten] = 0;
}

static inline void AddExplainStamp_explain(CompanyRoleFilter* __restrict companyRoleFilter, time_t stamp)
{
    char stampBuffer[50];
    struct tm stampDate;
    localtime_r(&stamp, &stampDate);
    size_t numCharsWritten = strftime(stampBuffer, sizeof(stampBuffer), "%FT%T%Z", &stampDate);
    if (numCharsWritten > 0)
    {
        AddExplainString_explain(companyRoleFilter, stampBuffer);
    }
}

static inline void AddExplainInt64_explain(CompanyRoleFilter* __restrict companyRoleFilter, int64_t val)
{
    char valBuffer[50];
    size_t numCharsWritten = snprintf(valBuffer, sizeof(valBuffer), "%" PRId64, val);
    if (numCharsWritten > 0)
    {
        AddExplainString_explain(companyRoleFilter, valBuffer);
    }
}

#define TEMPLATE_FUNCTION_NAME_SUFFIX	_explain
#define TEMPLATE_FUNCTION_RETURN_1		ret = 1; // we do not return. We will inspect all values to explain behavior
#define TCCRF
#include <filters_perform.template.h>

static void reinitExplainInfo(CompanyRoleFilter* companyRoleFilter)
{
    if (companyRoleFilter->explainFiltering != NULL)
    {
        free(companyRoleFilter->explainFiltering);
        companyRoleFilter->explainFiltering = NULL;
        companyRoleFilter->explainBytesWritten = 0;
        companyRoleFilter->explainBytesAllocated = 0;
    }
}

static void BundleFilterExplain(PerformFilter* filter)
{
    const size_t maxFilters = kv_size(filter->filters);
    kv_resize(SearchFilterExplain, filter->filterExplained, maxFilters);
    for (size_t i = 0; i < maxFilters; ++i)
    {
        // In previous loop we already checked that no value is NULL in this array
        CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, i);

        // nothing to explain
        if (companyRoleFilter->explainFiltering == NULL)
        {
            continue;
        }
        if (companyRoleFilter->filter == NULL)
        {
            continue;
        }

        DF_ASSERT(strlen(companyRoleFilter->explainFiltering) > 6, "Unexpected explain filter value '%s'", companyRoleFilter->explainFiltering);

        // cut out the enclosing tokens "#,#"
        char* reducedExplainStr = &companyRoleFilter->explainFiltering[3];

        reducedExplainStr[strlen(reducedExplainStr) - 3] = 0;
        DF_LOG_MESSAGE("Adding explain info '%s'", reducedExplainStr);

        kv_pushp(SearchFilterExplain, filter->filterExplained);
        SearchFilterExplain* explainFilter = &kv_A(filter->filterExplained, kv_size(filter->filterExplained) - 1);
        initSearchFilterExplain(explainFilter);

        explainFilter->name = strdup(companyRoleFilter->filter);
        explainFilter->textValue = strdup(reducedExplainStr);
    }
    DF_LOG_MESSAGE("Have %u filters, and %u explain structs, max pos %u", kv_size(filter->filters), kv_size(filter->filterExplained), filter->filterExplained.m);
}

int ProfileValid_explain(PerformFilter* filter)
{
    DF_LOG_MESSAGE("Profile Id %d", filter->profile->id);

    int ret = 1;
    // Do we have at least 1 filter that would select this profile for next step ?
    const size_t maxFilters = kv_size(filter->filters);
    for (size_t i = 0; i < maxFilters; ++i)
    {
        // In previous loop we already checked that no value is NULL in this array
        CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, i);
        if (ConditionValid_explain(filter, companyRoleFilter) == 0)
        {
            ret = 0;
            break;
        }
    }
    // format explain info to be returned to search threads
    if (ret == 1)
    {
        BundleFilterExplain(filter);
    }
    // clean explain info
    for (size_t i = 0; i < maxFilters; ++i)
    {
        // In previous loop we already checked that no value is NULL in this array
        CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, i);
        reinitExplainInfo(companyRoleFilter);
    }
    return ret;
}
