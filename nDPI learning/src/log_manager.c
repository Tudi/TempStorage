#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include "log_manager.h"
#include "ini_file_handler.h"

LogManager gLogManager = {
    .isInitialized = 0,
    .outFile = NULL,
    .severityFilterFile = LogSeveritySever,
    .destinationFilter = LDF_LOCAL | LDF_STDOUT,
    .consoleLock = PTHREAD_MUTEX_INITIALIZER
};

__thread ThreadLogBuffer gThreadLogBuffer = { .offset = 0 };

static void get_current_timestamp(char *buffer, size_t size, uint64_t *seconds, uint64_t *nanoseconds) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    *seconds = ts.tv_sec;
    *nanoseconds = ts.tv_nsec;

    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

void init_log_manager(const char* app_dir) {
    if (gLogManager.isInitialized)
        return;

    gLogManager.outFile = NULL;

    // logging will have a huge performance hit. Disable it for live sessions
    if (get_ini_bool_value("Global", "EnableLogging", 0) == 0) {
        return;
    }
    const char* loglevel = get_ini_value("Global", "LogSeverity");
    if (loglevel) {
        if (strcmp(loglevel, "Debug") == 0) {
            gLogManager.severityFilterFile = LogSeverityDebug;
        }
        else if (strcmp(loglevel, "Info") == 0) {
            gLogManager.severityFilterFile = LogSeverityInfo;
        }
        else if (strcmp(loglevel, "Warn") == 0) {
            gLogManager.severityFilterFile = LogSeverityWarn;
        }
        else if (strcmp(loglevel, "Error") == 0) {
            gLogManager.severityFilterFile = LogSeverityError;
        }
        else if (strcmp(loglevel, "Fatal") == 0) {
            gLogManager.severityFilterFile = LogSeverityFatal;
        }
    }

    char full_path[PATH_MAX];
    char timestamp[64];

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    // Format just the date-based log filename
    strftime(timestamp, sizeof(timestamp), "log_%Y-%m-%d.log.txt", &tm_info);

    // Combine path and timestamp into full path
    snprintf(full_path, sizeof(full_path), "%s/Logs/%s", app_dir, timestamp);

    gLogManager.outFile = fopen(full_path, "a");
    gLogManager.isInitialized = (gLogManager.outFile != NULL);
}

void destroy_log_manager() {
    FlushThreadLogBuffer();
    if (gLogManager.outFile) {
        fclose(gLogManager.outFile);
        gLogManager.outFile = NULL;
    }
    gLogManager.isInitialized = 0;
}

void AddLogEntryV(LogDestinationFlags dest, LogSeverityValue severity, LogSourceGroups source, const char *msgFormat, ...) {
    if (!gLogManager.isInitialized || (int)severity < (int)gLogManager.severityFilterFile)
        return;

    char timestamp[64];
    uint64_t seconds, nanoseconds;
    get_current_timestamp(timestamp, sizeof(timestamp), &seconds, &nanoseconds);

    va_list args;
    va_start(args, msgFormat);
    char msg[MAX_LOG_LINE_LEN-100];
    vsnprintf(msg, sizeof(msg), msgFormat, args);
    va_end(args);

    char finalLine[MAX_LOG_LINE_LEN];
    snprintf(finalLine, sizeof(finalLine), "%s %" PRIu64 ":%06" PRIu64 ":%" PRId64 ":%d:%s",
        timestamp, seconds, nanoseconds, (int64_t)severity, (int)source, msg);

    pthread_mutex_lock(&gLogManager.consoleLock);

    if ((dest & LDF_LOCAL) && gLogManager.outFile) {
        fputs(finalLine, gLogManager.outFile);
        fflush(gLogManager.outFile);
    }
    if (dest & LDF_STDOUT) {
        fputs(finalLine, stdout);
    }
    if (dest & LDF_STDERR) {
        fputs(finalLine, stderr);
    }

    pthread_mutex_unlock(&gLogManager.consoleLock);
}

void AddLogEntryBuffered(LogSeverityValue severity, LogSourceGroups source, const char* fmt, ...) {
    if (!gLogManager.isInitialized || (int)severity < (int)gLogManager.severityFilterFile) {
        return;
    }

    char timestamp[64];
    uint64_t sec, nsec;
    get_current_timestamp(timestamp, sizeof(timestamp), &sec, &nsec);

    char logline[MAX_LOG_LINE_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(logline, sizeof(logline), fmt, args);
    va_end(args);

    static __thread int thread_id = 0;
    if (thread_id == 0) {
        thread_id = (int)pthread_self();
    }
    int remaining_bytes = MAX_BUFFER_SIZE - gThreadLogBuffer.offset;
    int n = snprintf(gThreadLogBuffer.buffer + gThreadLogBuffer.offset,
        remaining_bytes,
        "%s %" PRIu64 ":%06" PRIu64 ":%d:%d:%d:%s",
        timestamp, sec, nsec, severity, source, thread_id, logline);

    if (n >= remaining_bytes) {
        fputs("AddLogEntryBuffered: log entry truncated due to buffer size\n", stderr);
    }

    gThreadLogBuffer.offset += n;

    if (gThreadLogBuffer.offset > MAX_BUFFER_SIZE - MAX_LOG_LINE_LEN) {
        FlushThreadLogBuffer();
    }
}

void FlushThreadLogBuffer() {
    pthread_mutex_lock(&gLogManager.consoleLock);
    if (gLogManager.outFile) {
        fwrite(gThreadLogBuffer.buffer, 1, gThreadLogBuffer.offset, gLogManager.outFile);
        fflush(gLogManager.outFile);
    }
    gThreadLogBuffer.offset = 0;
    pthread_mutex_unlock(&gLogManager.consoleLock);
}