#include <error_reporting.h>
#include <logger.h>
#include <sentry.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int errorReporting_start(const char *errorDirectory)
{
    // By default, Sentry will read the environment variables: SENTRY_DSN, SENTRY_RELEASE, SENTRY_ENVIRONMENT.
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_database_path(options, errorDirectory);
    sentry_options_set_debug(options, 1);
    int ret = sentry_init(options);
    if (ret == 0)
    {
        sentry_flush(10 * 1000); // Attempt to flush all pending events within 10 seconds.
    }
    return ret;
}

int errorReporting_stop()
{
    return sentry_close();
}

static int GetMemoryUsage(unsigned long *virtualSizeInBytes, unsigned long *residentSizeInBytes)
{
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pageSize < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sysconf(_SC_PAGE_SIZE) - errno = %d (\"%s\").\n",
            errno, strerror(errno));
        return 1;
    }

    const char* statmPath = "/proc/self/statm";

    FILE* f = fopen(statmPath, "r");
    if (f == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) - errno = %d (\"%s\").",
            statmPath, errno, strerror(errno));
        return 2;
    }

    unsigned long virtualNumPages = 0;
    unsigned long residentNumPages = 0;
    unsigned long shareNumPages = 0;
    unsigned long textNumPages = 0;
    unsigned long libNumPages = 0;
    unsigned long dataNumPages = 0;
    unsigned long dtNumPages = 0;

    if (fscanf(f, "%lu %lu %lu %lu %lu %lu %lu", &virtualNumPages, &residentNumPages,
        &shareNumPages, &textNumPages, &libNumPages, &dataNumPages, &dtNumPages) != 7)
    {
        if (ferror(f)) {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fscanf(%s) - errno = %d (\"%s\").",
                statmPath, errno, strerror(errno));
        }
        else {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fscanf(%s).", statmPath);
        }
        fclose(f);
        return 3;
    }

    fclose(f);

    *virtualSizeInBytes = virtualNumPages * pageSize;
    *residentSizeInBytes = residentNumPages * pageSize;

    return 0;
}

void perform_SystemSanityChecks(int activeConnectionCount, int expectedMemUsagePerConnection, int reqProcessingTime)
{
    char printBuff[655355];
    if(activeConnectionCount > REPORT_CONNECTION_COUNT_LARGER)
    {
        snprintf(printBuff, sizeof(printBuff), "Unexpectedly large active connection count : %d larger than %d", 
            activeConnectionCount, REPORT_CONNECTION_COUNT_LARGER);
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Warning: %s", printBuff);
        sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_WARNING, NULL, printBuff);
        sentry_capture_event(event);
    }
    unsigned long virtualSizeInBytes = 0;
    unsigned long residentSizeInBytes = 0;
    GetMemoryUsage(&virtualSizeInBytes, &residentSizeInBytes);
    if (residentSizeInBytes >= (activeConnectionCount + REPORT_MEM_USAGE_COUNT_LARGER) * expectedMemUsagePerConnection)
    {
        snprintf(printBuff, sizeof(printBuff), "Unexpectedly large memory usage : %lu larger than %d",
            residentSizeInBytes, (activeConnectionCount + REPORT_MEM_USAGE_COUNT_LARGER) * expectedMemUsagePerConnection);
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Warning: %s", printBuff);
        sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_WARNING, NULL, printBuff);
        sentry_capture_event(event);
    }
    if (reqProcessingTime > REPORT_REQUEST_PROCESSING_TIME_MS)
    {
        snprintf(printBuff, sizeof(printBuff), "Unexpectedly large request processing time : %d ms larger than %d ms",
            reqProcessingTime, REPORT_REQUEST_PROCESSING_TIME_MS);
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Warning: %s", printBuff);
        sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_WARNING, NULL, printBuff);
        sentry_capture_event(event);
    }
}
