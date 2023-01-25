#include <logger.h>
#include <filters.h>
#include <k_utils.h>
#include <strings_ext.h>
#include <profile_cached.h>
#include <position_cached.h>
#include <company_cached.h>
#include <profiling.h>
#include <boolean_parser.h>

#define AddExplainString(companyRoleFilter, str)
#define AddExplainStamp(companyRoleFilter, stamp)
#define AddExplainInt64(companyRoleFilter, val)

#define TEMPLATE_FUNCTION_RETURN_1		return 1;
#define TEMPLATE_FUNCTION_NAME_SUFFIX 
#define TCCRF   const
#include <filters_perform.template.h>

/// <summary>
/// Do the filtering. Apply each filter and their subfilters on the profile. 
/// If one of the filters returns true, the filtering will stop
/// </summary>
/// <param name="filter"></param>
/// <returns>0 means profile does not need to be included in the result set</returns>
int ProfileValid(const PerformFilter* filter)
{
    DF_LOG_MESSAGE( "Profile Id %d", filter->profile->id);

    // Do we have at least 1 filter that would select this profile for next step ?
    const size_t maxFilters = kv_size(filter->filters);
    for (size_t i = 0; i < maxFilters; ++i)
    {
        // In previous loop we already checked that no value is NULL in this array
        CompanyRoleFilter* companyRoleFilter = &kv_A(filter->filters, i);
        if (ConditionValid(filter, companyRoleFilter) == 0)
        {
            return 0;
        }
    }
    return 1;
}
