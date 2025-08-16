#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

/* 
* This is a simple logger class because there was no need to create a bigger one
* If new features are required, feel free to use something like log4c
* Todo :
*   - add option for remote log pushing for alerts
*   - add alert options
*/

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdarg.h>

#define MAX_THREAD_COUNT 64
#define MAX_LOG_LINE_LEN 16000
#define MAX_BUFFER_SIZE (10*MAX_LOG_LINE_LEN)

typedef struct {
    char buffer[MAX_BUFFER_SIZE];
    size_t offset;
} ThreadLogBuffer;

typedef enum {
    LDF_NONE  = 0x00,
    LDF_LOCAL = 0x01,
    LDF_STDOUT = 0x02,
    LDF_STDERR = 0x04,
} LogDestinationFlags;

typedef enum {
    LogSeverityDebug = 1,
    LogSeverityInfo,
    LogSeverityWarn,
    LogSeverityError,
    LogSeverityFatal,
    LogSeveritySever,
} LogSeverityValue;

typedef enum {
    LogSourceGeneral = 1,
    LogSourceLogging,
    LogSourceNetwork,
    LogSourceParser,
    LogSourceStorage,
    LogSourcePluginTLS,
    LogSourceBufferPool,
    LogSourceWorkerThread,
    LogSourceMySQL,
    LogSourceAllocStats,
    LogSourceNDPI,
    LogSourcePluginJA3,
    LogSourcePluginJA4,
    LogSourcePluginSHA1,
    LogSourcePluginS_IP,
    LogSourcePluginSNI,
    LogSourcePluginGlobal,
} LogSourceGroups;

typedef struct {
    int isInitialized;
    FILE *outFile;
    int severityFilterFile;
    int destinationFilter;
    pthread_mutex_t consoleLock;
} LogManager;

extern LogManager gLogManager;

void init_log_manager(const char *app_dir);
void destroy_log_manager();
void AddLogEntryV(LogDestinationFlags dest, LogSeverityValue severity, LogSourceGroups source,
                  const char *msgFormat, ...);
void AddLogEntryBuffered(LogSeverityValue severity, LogSourceGroups source, const char* fmt, ...);

// difference : macro will not evalaute __VA_ARGS__ unless the filter is hit
#define AddLogEntry(dest, sev, src, fmt, ...) \
    if( gLogManager.outFile != NULL && (dest & LDF_LOCAL) && (int)sev >= (int)gLogManager.severityFilterFile){ \
        AddLogEntryV(dest, sev, src, fmt, ##__VA_ARGS__);}

#define AddLogEntryB(dest, sev, src, fmt, ...) \
    if( (int)sev >= (int)gLogManager.severityFilterFile && gLogManager.outFile != NULL && (dest & LDF_LOCAL)){ \
        AddLogEntryBuffered(sev, src, fmt, ##__VA_ARGS__);}

// help with threads mumbo jumboing logs
extern __thread ThreadLogBuffer gThreadLogBuffer;
void FlushThreadLogBuffer();

#endif // LOG_MANAGER_H
