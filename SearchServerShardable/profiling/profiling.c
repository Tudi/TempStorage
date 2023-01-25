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

#define S_TO_NS     1000000000UL

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

ProfiledCodeSectionSummary gProfiledCodeSection[PE_MAX_VALUE] = { 0 };

const char* gProfiledCodeSectionNames[PE_MAX_VALUE] = {
    [PE_PROFILE_INSERT]               = "Insert profile                 ",
    [PE_COMPANY_INSERT]               = "Insert company                 ",
    [PE_PROFILE_LOADING]              = "Load all profiles              ",
    [PE_COMPANY_LOADING]              = "Load all companies             ",
    [PE_RELEVANT_EXPERIENCE]          = "RelevantExperience calc        ",
    [PE_SCORING]                      = "Scoring calc                   ",
    [PE_FILTERING]                    = "Filtering                      ",

    [PE_FILTER_NAME_INCLUDE]          = "*FILTER_NAME_INCLUDE           ",
    [PE_FILTER_NAME_EXCLUDE]          = "*FILTER_NAME_EXCLUDE           ",
    [PE_FILTER_CUR_TITLE_INCLUDE]     = "*FILTER_CUR_TITLE_INCLUDE      ",
    [PE_FILTER_CUR_TITLE_EXCLUDE]     = "*FILTER_CUR_TITLE_EXCLUDE      ",
    [PE_FILTER_PREV_TITLE_INCLUDE]    = "*FILTER_PREV_TITLE_INCLUDE     ",
    [PE_FILTER_PREV_TITLE_EXCLUDE]    = "*FILTER_PREV_TITLE_EXCLUDE     ",
    [PE_FILTER_TITLE_INCLUDE]         = "*FILTER_TITLE_INCLUDE          ",
    [PE_FILTER_TITLE_EXCLUDE]         = "*FILTER_TITLE_EXCLUDE          ",
    [PE_FILTER_CUR_COMPANY_INCLUDE]   = "*FILTER_CUR_COMPANY_INCLUDE    ",
    [PE_FILTER_CUR_COMPANY_EXCLUDE]   = "*FILTER_CUR_COMPANY_EXCLUDE    ",
    [PE_FILTER_PREV_COMPANY_INCLUDE]  = "*FILTER_PREV_COMPANY_INCLUDE   ",
    [PE_FILTER_PREV_COMPANY_EXCLUDE]  = "*FILTER_PREV_COMPANY_EXCLUDE   ",
    [PE_FILTER_COMPANY_INCLUDE]       = "*FILTER_COMPANY_INCLUDE        ",
    [PE_FILTER_COMPANY_EXCLUDE]       = "*FILTER_COMPANY_EXCLUDE        ",
    [PE_FILTER_KEYWORDS_INCLUDE]      = "*FILTER_KEYWORDS_INCLUDE       ",
    [PE_FILTER_KEYWORDS_EXCLUDE]      = "*FILTER_KEYWORDS_EXCLUDE       ",
    [PE_FILTER_KEYWORDS_BOOLEAN_INC]  = "*FILTER_KEYWORDS_BOOLEAN_INCLUD",
    [PE_FILTER_KEYWORDS_BOOLEAN_EXC]  = "*FILTER_KEYWORDS_BOOLEAN_EXCLUD",
    [PE_FILTER_MESSAGED_EXCLUDE]      = "*FILTER_MESSAGED_EXCLUDE       ",
    [PE_FILTER_REPLIED_EXCLUDE]       = "*FILTER_REPLIED_EXCLUDE        ",
    [PE_FILTER_EXPERIENCE]            = "*FILTER_EXPERIENCE             ",
    [PE_FILTER_CUR_INDUSTRY_INCLUDE]  = "*FILTER_CUR_INDUSTRY_INCLUDE   ",
    [PE_FILTER_CUR_INDUSTRY_EXCLUDE]  = "*FILTER_CUR_INDUSTRY_EXCLUDE   ",
    [PE_FILTER_PREV_INDUSTRY_INCLUDE] = "*FILTER_PREV_INDUSTRY_INCLUDE  ",
    [PE_FILTER_PREV_INDUSTRY_EXCLUDE] = "*FILTER_PREV_INDUSTRY_EXCLUDE  ",
    [PE_FILTER_INDUSTRY_INCLUDE]      = "*FILTER_INDUSTRY_INCLUDE       ",
    [PE_FILTER_INDUSTRY_EXCLUDE]      = "*FILTER_INDUSTRY_EXCLUDE       ",
    [PE_FILTER_TENURE]                = "*FILTER_TENURE                 ",
    [PE_FILTER_RELEVANT_EXPERIENCE]   = "*FILTER_RELEVANT_EXPERIENCE    ",
    [PE_FILTER_TOTAL_EXPERIENCE]      = "*FILTER_TOTAL_EXPERIENCE       ",
    [PE_FILTER_PROJECTS_INCLUDE]      = "*FILTER_PROJECTS_INCLUDE       ",
    [PE_FILTER_PROJECTS_EXCLUDE]      = "*FILTER_PROJECTS_EXCLUDE       ",
    [PE_FILTER_GROUPS_INCLUDE]        = "*FILTER_GROUPS_INCLUDE         ",
    [PE_FILTER_GROUPS_EXCLUDE]        = "*FILTER_GROUPS_EXCLUDE         ",
    [PE_FILTER_REPLY]                 = "*FILTER_REPLY                  ",
    [PE_FILTER_EXCLUDE_REPLY]         = "*FILTER_EXCLUDE_REPLY          ",
    [PE_FILTER_CUR_FUNCTION_INCLUDE]  = "*FILTER_CUR_COMP_FUNCTION_INC  ",
    [PE_FILTER_CUR_FUNCTION_EXCLUDE]  = "*FILTER_CUR_COMP_FUNCTION_EXCL ",
    [PE_FILTER_PREV_FUNCTION_INCLUDE] = "*FILTER_PREV_COMP_FUNCTION_INC ",
    [PE_FILTER_PREV_FUNCTION_EXCLUDE] = "*FILTER_PREV_COMP_FUNCTION_EXCL",
    [PE_FILTER_FUNCTION_INCLUDE]      = "*FILTER_COMP_FUNCTION_INCLUDE  ",
    [PE_FILTER_FUNCTION_EXCLUDE]      = "*FILTER_COMP_FUNCTION_EXCLUDE  ",
    [PE_FILTER_CUR_NRE_INCLUDE]       = "*FILTER_CUR_COMP_NRE_INC       ",
    [PE_FILTER_CUR_NRE_EXCLUDE]       = "*FILTER_CUR_COMP_NRE_EXCL      ",
    [PE_FILTER_PREV_NRE_INCLUDE]      = "*FILTER_PREV_COMP_NRE_INC      ",
    [PE_FILTER_PREV_NRE_EXCLUDE]      = "*FILTER_PREV_COMP_NRE_EXCL     ",
    [PE_FILTER_COUNTRY_INCLUDE]       = "*FILTER_COUNTRY_INCLUDE        ",
    [PE_FILTER_COUNTRY_EXCLUDE]       = "*FILTER_COUNTRY_EXCLUDE        ",
    [PE_FILTER_NRE_INCLUDE]           = "*FILTER_COMP_NRE_INCLUDE       ",
    [PE_FILTER_NRE_EXCLUDE]           = "*FILTER_COMP_NRE_EXCLUDE       ",
    [PE_FILTER_PID_INCLUDE]           = "*FILTER_PROFILE_ID_INCLUDE     ",
    [PE_FILTER_PID_EXCLUDE]           = "*FILTER_PROFILE_ID_EXCLUDE     ",
    [PE_FILTER_STATE_INCLUDE]         = "*FILTER_STATE_INCLUDE          ",
    [PE_FILTER_STATE_EXCLUDE]         = "*FILTER_STATE_EXCLUDE          ",
    [PE_FILTER_CAMPAIGN_INCLUDE]      = "*FILTER_CAMPAIGN_INCLUDE       ",
    [PE_FILTER_CAMPAIGN_EXCLUDE]      = "*FILTER_CAMPAIGN_EXCLUDE       ",
    [PE_FILTER_TALENTPOOL_INCLUDE]    = "*FILTER_TALENTPOOL_INCLUDE     ",
    [PE_FILTER_TALENTPOOL_EXCLUDE]    = "*FILTER_TALENTPOOL_EXCLUDE     ",

    [PE_SEARCH_SETUP]                 = "Parse search query             ",
    [PE_THREADED_SEARCH]              = "In threads Filtering+Scoring   ",
    [PE_THREADED_SORT]                = "In threads Sort results        ",
    [PE_THREADED_EXPLAIN]             = "In threads explain filtering   ",
    [PE_RESULT_MERGE]                 = "Final result merge+sort+format ",
    [PE_SEARCH_REQUEST_FULL]          = "Full search request            ",
    [PE_SERVER_SHUTDOWN]              = "Shutdown cleanup               ",
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
}

void inline ProfilingAddSectionTimeThreadSafe(const int section, const int64_t sectionTime, const int64_t count)
{
    __atomic_fetch_add(&gProfiledCodeSection[section].sumTimeSpent, sectionTime, __ATOMIC_SEQ_CST);
    __atomic_fetch_add(&gProfiledCodeSection[section].timesExecuted, count, __ATOMIC_SEQ_CST);
}

void inline ProfilingAddSectionTimeThreadSafeWithMax(const int section, const int64_t sectionTime, const int64_t count)
{
    __atomic_fetch_add(&gProfiledCodeSection[section].sumTimeSpent, sectionTime, __ATOMIC_SEQ_CST);
    __atomic_fetch_add(&gProfiledCodeSection[section].timesExecuted, count, __ATOMIC_SEQ_CST);
    
    // not threadsafe, but relatively sparse usage for now
    if (gProfiledCodeSection[section].maxTime < sectionTime)
    {
        gProfiledCodeSection[section].maxTime = sectionTime;
    }
}

int PrintMemoryUsage()
{
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if(pageSize < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sysconf(_SC_PAGE_SIZE) - errno = %d (\"%s\").\n",
            errno, strerror(errno));
        return 1;
    }

    const char* statmPath = "/proc/self/statm";

    FILE* f = fopen(statmPath, "r");
    if(f == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) - errno = %d (\"%s\").",
            statmPath, errno, strerror(errno));
        return 2;
    }

    unsigned long virtualNumPages  = 0;
    unsigned long residentNumPages = 0;
    unsigned long shareNumPages    = 0;
    unsigned long textNumPages     = 0;
    unsigned long libNumPages      = 0;
    unsigned long dataNumPages     = 0;
    unsigned long dtNumPages       = 0;

    if(fscanf(f, "%lu %lu %lu %lu %lu %lu %lu", &virtualNumPages, &residentNumPages,
        &shareNumPages, &textNumPages, &libNumPages, &dataNumPages, &dtNumPages) != 7)
    {
        if(ferror(f)) {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fscanf(%s) - errno = %d (\"%s\").",
                statmPath, errno, strerror(errno));
        } else {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fscanf(%s).", statmPath);
        }
        fclose(f);
        return 3;
    }

    fclose(f);

    unsigned long virtualSizeInBytes  = virtualNumPages * pageSize;
    unsigned long residentSizeInBytes = residentNumPages * pageSize;

    LOG_MESSAGE(INFO_LOG_MSG, "Memory usage:\n\n"
            "Virtual size: %lu bytes, %lu KB, %lu MB\n"
            "Resident size: %lu bytes, %lu KB, %lu MB\n",
            virtualSizeInBytes, virtualSizeInBytes / 1024, virtualSizeInBytes / (1024 * 1024),
            residentSizeInBytes, residentSizeInBytes / 1024, residentSizeInBytes / (1024 * 1024));

    return 0;
}

void PrintProfilingStatus()
{
    // Locking memory has a high cost. Substract average lock cost
    int64_t memory_lock_time_cost = 0;
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

    // Profile the profiler
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
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex, 
        "|_______________________________|_______________|____________|________________|____________|\n");
    // write this to logging module in 1 operation to avoid it breaking up by multiple threads writing to logs
    LOG_MESSAGE(INFO_LOG_MSG, stringWriteBuffer);
}

#endif
