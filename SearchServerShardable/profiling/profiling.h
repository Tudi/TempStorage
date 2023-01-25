#ifndef _PROFILING_H_
#define _PROFILING_H_

#include <stdint.h>

// this should be commented out unless you want to create intentionally a special build
#define ENABLE_PROFILING
#define WARNING_ON_SEARCH_TIME_EXCEED_MS  4000 // we do not expect searches to take this long
#define ENABLE_SUB_THREAD_PROFILING    // these use "mutexes" which bloat performance time
#define SUBSTRACT_MUTEX_LOCK_COST      // since mutex lock is a fixed overhead, it will influence small functions a lot

#if !defined(ENABLE_PROFILING) || !defined(ENABLE_SUB_THREAD_PROFILING)
    #define StartInlinedProfilingThreadSafe(name)
    #define EndInlinedProfilingThreadSafe(name)
    #define StartInlinedProfilingThreadSafe2(name)
    #define IncreaseProfilingEventCount2(name, count)
    #define EndInlinedProfilingThreadSafe2(name)
#endif


/// <summary>
/// Used by inlined profiling. Marks both start and the end of a profiled section
/// </summary>
typedef enum ProfilingEvents
{
    PE_PROFILE_INSERT,
    PE_COMPANY_INSERT,
    PE_PROFILE_LOADING,
    PE_COMPANY_LOADING,
    PE_SEARCH_SETUP,
    PE_RELEVANT_EXPERIENCE,
    PE_FILTERING,
    PE_SCORING,

    PE_FILTER_NAME_INCLUDE,
    PE_FILTER_NAME_EXCLUDE,
    PE_FILTER_CUR_TITLE_INCLUDE,
    PE_FILTER_CUR_TITLE_EXCLUDE,
    PE_FILTER_PREV_TITLE_INCLUDE,
    PE_FILTER_PREV_TITLE_EXCLUDE,
    PE_FILTER_TITLE_INCLUDE,
    PE_FILTER_TITLE_EXCLUDE,
    PE_FILTER_CUR_COMPANY_INCLUDE,
    PE_FILTER_CUR_COMPANY_EXCLUDE,
    PE_FILTER_PREV_COMPANY_INCLUDE,
    PE_FILTER_PREV_COMPANY_EXCLUDE,
    PE_FILTER_COMPANY_INCLUDE,
    PE_FILTER_COMPANY_EXCLUDE,
    PE_FILTER_KEYWORDS_INCLUDE,
    PE_FILTER_KEYWORDS_EXCLUDE,
    PE_FILTER_KEYWORDS_BOOLEAN_INC,
    PE_FILTER_KEYWORDS_BOOLEAN_EXC,
    PE_FILTER_MESSAGED_EXCLUDE,
    PE_FILTER_REPLIED_EXCLUDE,
    PE_FILTER_EXPERIENCE,
    PE_FILTER_CUR_INDUSTRY_INCLUDE,
    PE_FILTER_CUR_INDUSTRY_EXCLUDE,
    PE_FILTER_PREV_INDUSTRY_INCLUDE,
    PE_FILTER_PREV_INDUSTRY_EXCLUDE,
    PE_FILTER_INDUSTRY_INCLUDE,
    PE_FILTER_INDUSTRY_EXCLUDE,
    PE_FILTER_TENURE,
    PE_FILTER_RELEVANT_EXPERIENCE,
    PE_FILTER_TOTAL_EXPERIENCE,
    PE_FILTER_PROJECTS_INCLUDE,
    PE_FILTER_PROJECTS_EXCLUDE,
    PE_FILTER_GROUPS_INCLUDE,
    PE_FILTER_GROUPS_EXCLUDE,
    PE_FILTER_REPLY,
    PE_FILTER_EXCLUDE_REPLY,
    PE_FILTER_CUR_FUNCTION_INCLUDE,
    PE_FILTER_CUR_FUNCTION_EXCLUDE,
    PE_FILTER_PREV_FUNCTION_INCLUDE,
    PE_FILTER_PREV_FUNCTION_EXCLUDE,
    PE_FILTER_FUNCTION_INCLUDE,
    PE_FILTER_FUNCTION_EXCLUDE,
    PE_FILTER_CUR_NRE_INCLUDE,
    PE_FILTER_CUR_NRE_EXCLUDE,
    PE_FILTER_PREV_NRE_INCLUDE,
    PE_FILTER_PREV_NRE_EXCLUDE,
    PE_FILTER_COUNTRY_INCLUDE,
    PE_FILTER_COUNTRY_EXCLUDE,
    PE_FILTER_NRE_INCLUDE,
    PE_FILTER_NRE_EXCLUDE,
    PE_FILTER_PID_INCLUDE,
    PE_FILTER_PID_EXCLUDE,
    PE_FILTER_STATE_INCLUDE,
    PE_FILTER_STATE_EXCLUDE,
    PE_FILTER_CAMPAIGN_INCLUDE,
    PE_FILTER_CAMPAIGN_EXCLUDE,
    PE_FILTER_TALENTPOOL_INCLUDE,
    PE_FILTER_TALENTPOOL_EXCLUDE,

    PE_THREADED_SORT,
    PE_THREADED_EXPLAIN,
    PE_THREADED_SEARCH, // mutex is locked inside search_engine.c
    PE_RESULT_MERGE,
    PE_SEARCH_REQUEST_FULL,
    PE_SERVER_SHUTDOWN,
    PE_PROFILER_PROFILING,
    PE_MAX_VALUE // required to be able to summarize values
}ProfilingEvents;

#ifndef ENABLE_PROFILING
    #define StartInlinedProfiling(name) 
    #define IncreaseProfilingEventCount(name, count)
    #define EndInlinedProfiling(name) 
    #define PrintProfilingStatus()
    #define DisableNonUsedWarning(name) (void)name;
    #define PrintMemoryUsage()
#else
    #include <stdio.h>

    #define DisableNonUsedWarning(name)
    void PrintProfilingStatus();

    void ProfilingAddSectionTime(const int section, const int64_t time, const int64_t eventCount);
    void ProfilingAddSectionTimeThreadSafe(const int section, const int64_t time, const int64_t count);
    void ProfilingAddSectionTimeThreadSafeWithMax(const int section, const int64_t time, const int64_t count);

    #define MakeVariableStartName(name) start_##name
    #define MakeVariableCountName(name) count_##name

    // generic macro to profile a code section
    #define StartInlinedProfiling(name) int64_t MakeVariableStartName(name) = GetProfilingStamp(); 
    #define EndInlinedProfiling(name) if(name>=PE_MAX_VALUE) printf("Profiled event out of bounds : %s:%d",__FUNCTION__,__LINE__); \
        ProfilingAddSectionTime(name, GetProfilingStamp() - MakeVariableStartName(name), 1);
    
    // separate macro because I can imagine this not being used a lot of times
    #define StartInlinedProfilingThreadSafe(name) int64_t MakeVariableStartName(name) = GetProfilingStamp(); 
    #define EndInlinedProfilingThreadSafe(name) ProfilingAddSectionTimeThreadSafe(name, GetProfilingStamp() - MakeVariableStartName(name), 1);
    #define EndInlinedProfilingCalcMaxThreadSafe(name) ProfilingAddSectionTimeThreadSafeWithMax(name, GetProfilingStamp() - MakeVariableStartName(name), 1);

    #define StartInlinedProfilingThreadSafe2(name) int64_t MakeVariableStartName(name) = GetProfilingStamp(); int64_t MakeVariableCountName(name) = 0;
    #define IncreaseProfilingEventCount2(name, count) MakeVariableCountName(name) += count;
    #define EndInlinedProfilingThreadSafe2(name) ProfilingAddSectionTimeThreadSafe(name, GetProfilingStamp() - MakeVariableStartName(name), MakeVariableCountName(name));

    int PrintMemoryUsage();
#endif

#define MS_TO_NS    1000000UL
// unified way to obtain precise time without the need to add multiple includes to the project
int64_t GetProfilingStamp(); 

#endif
