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

#ifndef MAX_STRING_LEN
    #define MAX_STRING_LEN 0x0FFFFF
#endif

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
}ProfiledCodeSectionSummary;

ProfiledCodeSectionSummary gProfiledCodeSection[PE_MAX_VALUE];

const char* gProfiledCodeSectionNames[PE_MAX_VALUE] = {
    [PE_SAVE]                   = "Save file                      ",
    [PE_GET]                    = "Get file                       ",
    [PE_SOCKET_READ_REQUEST]    = "SOCKET : Read request          ",
    [PE_HANDLE_REQUEST]         = "Handle 1 request full          ",
    [PE_SOCKET_LIFETIME]        = "Socket open-close lifetime     ",
    [PE_SERVER_SHUTDOWN]        = "Shutdown cleanup               ",
};

/// <summary>
/// Add to the summary the obtained profiled section
/// </summary>
/// <param name="section"></param>
/// <param name="time"></param>
void inline ProfilingAddSectionTime(const int section, const int64_t time, const int64_t eventCount)
{
    gProfiledCodeSection[section].sumTimeSpent += time;
    gProfiledCodeSection[section].timesExecuted += eventCount;
}

void inline ProfilingAddSectionTimeThreadSafe(const int section, const int64_t time, const int64_t count)
{
    __atomic_fetch_add(&gProfiledCodeSection[section].sumTimeSpent, time, __ATOMIC_SEQ_CST);
    __atomic_fetch_add(&gProfiledCodeSection[section].timesExecuted, count, __ATOMIC_SEQ_CST);
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
        "____________________________________________________________________________\n");
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex, 
        "|      Code section             |  Avg time ms |  Total time  |    Count   |\n");
    stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex, 
        "|_______________________________|______________|______________|____________|\n");
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
        stringWriteIndex += snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex, 
            "|%s|% 14.6f| %02ld:%02ld:%02ld:%03ld |% 12ld|\n",
            gProfiledCodeSectionNames[profiledEventType], avg, h, m_mod, s_mod, ms_mod,
            gProfiledCodeSection[profiledEventType].timesExecuted);
    }
    snprintf(&stringWriteBuffer[stringWriteIndex], stringBufferSize - stringWriteIndex, 
        "|_______________________________|______________|______________|____________|\n");
    // write this to logging module in 1 operation to avoid it breaking up by multiple threads writing to logs
    LOG_MESSAGE(INFO_LOG_MSG, stringWriteBuffer);
    FILE* fileProfilinginfo = fopen("profiling.log", "wt");
    if (fileProfilinginfo)
    {
        fwrite(stringWriteBuffer, 1, strnlen(stringWriteBuffer, MAX_STRING_LEN), fileProfilinginfo);
        fclose(fileProfilinginfo);
    }
}

#endif
