#ifndef _PROFILING_H_
#define _PROFILING_H_

// this should be commented out unless you want to create intentionally a special build
#define ENABLE_PROFILING

#define ENABLE_SUB_THREAD_PROFILING    // these use "mutexes" which bloat performance time
#define SUBSTRACT_MUTEX_LOCK_COST      // since mutex lock is a fixed overhead, it will influence small functions a lot

#define S_TO_NS     1000000000UL
#define MS_TO_NS     1000000UL

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
    PE_SAVE_SIMILARITY_SCORE,
    PE_LOAD_SIMILARITY_SCORE_FILE,
    PE_MERGE_SIMILARITY_SCORE_FILES,
    PE_GET_SIMILARITY_SCORE,
    PE_GET_SIMILARITY_SCORE_COUNT,
    PE_GEN_MTDATA,
    PE_SPLIT_MT_WORK,
    PE_MT_MERGE_SLICES,
    PE_MT_MERGE_2ARRAYS,
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
    #include <stdint.h>

    #define DisableNonUsedWarning(name)
    void PrintProfilingStatus();

    void ProfilingAddSectionTime(const int section, const int64_t time, const int64_t eventCount);
    void ProfilingAddSectionTimeThreadSafe(const int section, const int64_t time, const int64_t count);

    #define MakeVariableStartName(name) start_##name
    #define MakeVariableCountName(name) count_##name

    // generic macro to profile a code section
    #define StartInlinedProfiling(name) int64_t MakeVariableStartName(name) = GetProfilingStamp(); 
    #define EndInlinedProfiling(name) if(name>=PE_MAX_VALUE) printf("Profiled event out of bounds : %s:%d",__FUNCTION__,__LINE__); \
        ProfilingAddSectionTime(name, GetProfilingStamp() - MakeVariableStartName(name), 1);
    
    // separate macro because I can imagine this not being used a lot of times
    #define StartInlinedProfilingThreadSafe(name) int64_t MakeVariableStartName(name) = GetProfilingStamp(); 
    #define EndInlinedProfilingThreadSafe(name) ProfilingAddSectionTimeThreadSafe(name, GetProfilingStamp() - MakeVariableStartName(name), 1);

    #define StartInlinedProfilingThreadSafe2(name) int64_t MakeVariableStartName(name) = GetProfilingStamp(); int64_t MakeVariableCountName(name) = 0;
    #define IncreaseProfilingEventCount2(name, count) MakeVariableCountName(name) += count;
    #define EndInlinedProfilingThreadSafe2(name) ProfilingAddSectionTimeThreadSafe(name, GetProfilingStamp() - MakeVariableStartName(name), MakeVariableCountName(name));
#endif

#define MS_TO_NS    1000000UL
// unified way to obtain precise time without the need to add multiple includes to the project
int64_t GetProfilingStamp(); 

#endif
