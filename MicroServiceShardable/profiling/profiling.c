#include <profiling.h>
#include <logger.h>
#include <sys/sysinfo.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stddef.h>
#include <time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/// <summary>
/// Convert monotonic time to NanoSeconds
/// </summary>
/// <param name="TimeNow"></param>
/// <returns></returns>
int64_t static inline MonotonicToNSec(struct timespec* TimeNow)
{
    int64_t nsec = (int64_t)(TimeNow->tv_sec) * (int64_t)S_TO_NS + (int64_t)(TimeNow->tv_nsec);
    return nsec;
}

/// <summary>
/// Project scoped universal timestamp function
/// </summary>
/// <returns></returns>
int64_t inline GetProfilingStamp()
{
    struct timespec TimeNow;
    clock_gettime(CLOCK_MONOTONIC, &TimeNow);
    return MonotonicToNSec(&TimeNow);
}

#ifdef ENABLE_PROFILING
typedef struct ProfiledCodeSectionSummary
{
    int64_t sumTimeSpent;
    int64_t timesExecuted;
    int64_t maxTime;
}ProfiledCodeSectionSummary;

ProfiledCodeSectionSummary gProfiledCodeSection[PE_MAX_VALUE];

const char* gProfiledCodeSectionNames[PE_MAX_VALUE] = {
    [PE_SAVE_SIMILARITY_SCORE]           = "Process save score request     ",
    [PE_LOAD_SIMILARITY_SCORE_FILE]      = "Load data from files           ",
    [PE_MERGE_SIMILARITY_SCORE_FILES]    = "Merge data from files          ",
    [PE_GET_SIMILARITY_SCORE]            = "Process get score request      ",
    [PE_GET_SIMILARITY_SCORE_COUNT]      = "Files requested by get requests",
    [PE_GEN_MTDATA]                      = "Gen MTData                     ",
    [PE_SPLIT_MT_WORK]                   = "Split work between threads     ",
    [PE_MT_MERGE_SLICES]                 = "MT merge slices                ",
    [PE_MT_MERGE_2ARRAYS]                = "Merge 2 arrays                 ",
    [PE_PROFILER_PROFILING]              = "Profile profiler               ",
};

/// <summary>
/// Add to the summary the obtained profiled section
/// </summary>
/// <param name="section"></param>
/// <param name="time"></param>
void inline ProfilingAddSectionTime(const int section, const int64_t sectionTime, const int64_t eventCount)
{
    gProfiledCodeSection[section].sumTimeSpent += sectionTime;
    gProfiledCodeSection[section].timesExecuted += eventCount;

    if (gProfiledCodeSection[section].maxTime < sectionTime)
    {
        gProfiledCodeSection[section].maxTime = sectionTime;
    }
}

void inline ProfilingAddSectionTimeThreadSafe(const int section, const int64_t sectionTime, const int64_t count)
{
    __atomic_fetch_add(&gProfiledCodeSection[section].sumTimeSpent, sectionTime, __ATOMIC_SEQ_CST);
    __atomic_fetch_add(&gProfiledCodeSection[section].timesExecuted, count, __ATOMIC_SEQ_CST);

    // not threadsafe, but relatively sparse usage for now
    if (gProfiledCodeSection[section].maxTime < sectionTime)
    {
        gProfiledCodeSection[section].maxTime = sectionTime;
    }
}

void PrintProfilingStatus()
{
    // Locking memory has a high cost. Substract average lock cost
    int64_t memory_lock_time_cost = 0;
    // profile the profiler
#ifdef SUBSTRACT_MUTEX_LOCK_COST
    int64_t start = GetProfilingStamp();
    for (size_t counter = 0; counter < 1000; counter++)
    {
        ProfilingAddSectionTimeThreadSafe(PE_PROFILER_PROFILING, GetProfilingStamp() + GetProfilingStamp(), 1);
    }
    int64_t end = GetProfilingStamp();
    gProfiledCodeSection[PE_PROFILER_PROFILING].timesExecuted = 0;
    memory_lock_time_cost = (end - start) / 1000;
#endif

    char stringWriteBuffer[PE_MAX_VALUE * 80 + 5 * 80];
    size_t stringBufferSize = sizeof(stringWriteBuffer);
    int stringWriteIndex = 0;
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex,
        "Printing profiled code section summary :\n");
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex,
        "____________________________________________________________________________________________\n");
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex,
        "|      Code section             |  Avg time ms  |  Max time  |   Total time   |    Count   |\n");
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex,
        "|_______________________________|_______________|____________|________________|____________|\n");
    for (ssize_t profiledEventType = 0; profiledEventType < PE_MAX_VALUE; profiledEventType++)
    {
        float avg = 0;
        if (gProfiledCodeSection[profiledEventType].timesExecuted > 0)
        {
            int64_t memory_lock_cost_total = 0;
            if (gProfiledCodeSectionNames[profiledEventType][0] == '*')
            {
                memory_lock_cost_total = gProfiledCodeSection[profiledEventType].timesExecuted * memory_lock_time_cost;
            }
            avg = (float)(gProfiledCodeSection[profiledEventType].sumTimeSpent - memory_lock_cost_total)
                / (float)gProfiledCodeSection[profiledEventType].timesExecuted / (float)MS_TO_NS;
        }
        else
        {
            continue;
        }
        int64_t ms = gProfiledCodeSection[profiledEventType].sumTimeSpent / MS_TO_NS;
        int64_t ms_mod = ms % 1000;
        int64_t s = ms / 1000;
        int64_t s_mod = s % 60;
        int64_t m = s / 60;
        int64_t m_mod = m % 60;
        int64_t h = m / 60;
        int64_t msMax = gProfiledCodeSection[profiledEventType].maxTime / MS_TO_NS;
        stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex,
            "|%s|% 15.5f|% 12ld| %04ld:%02ld:%02ld:%03ld |% 12ld|\n",
            gProfiledCodeSectionNames[profiledEventType], avg, msMax, h, m_mod, s_mod, ms_mod,
            gProfiledCodeSection[profiledEventType].timesExecuted);
    }
    snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex,
        "|_______________________________|_______________|____________|________________|____________|\n");
    // write this to logging module in 1 operation to avoid it breaking up by multiple threads writing to logs
    LOG_MESSAGE(INFO_LOG_MSG, stringWriteBuffer);
}

#endif
