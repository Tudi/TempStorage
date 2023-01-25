#include <logger.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <threads.h>

//
// Constants
//

#define LOG_MSG_MAX_LENGTH 4096

static const char* nullLogLevelDescription = "NULL";

static const char* logLevelDescriptions[NUM_LOG_MSG_LEVELS] = {
    "CRITICAL",
    "ATTENTION",
    "INFO",
    "DEBUG"
};

//
// External variables
//

unsigned int logger_level = INFO_LOG_MSG;

//
// Local prototypes
//

static inline int writePrefixToLogMsg(char* logMsg, size_t logMsgSize, int logMsgLevel,
    const char* functionName);

//
// External interface
//

void logger_setLogLevel(unsigned int level)
{
    if(level <= LOG_MSG_LEVEL_MAX) {
        logger_level = level;
    }
}

const char* logger_logLevelString(unsigned int level)
{
    return level <= LOG_MSG_LEVEL_MAX ? logLevelDescriptions[level] : nullLogLevelDescription;
}

void logger_logMessage(const char* format, int logMsgLevel, const char* functionName,
    const char* filename, int lineNumber, ...)
{
    if(logMsgLevel > LOG_MSG_LEVEL_MAX) { return; }

    // Print prefix with timestamp, tid, log level and function name.

    char logMsg[LOG_MSG_MAX_LENGTH];
    size_t logMsgSize = LOG_MSG_MAX_LENGTH;

    int numCharsWritten = writePrefixToLogMsg(logMsg, logMsgSize, logMsgLevel, functionName);

    // Print log message.

    va_list varArgs;
    va_start(varArgs, lineNumber);

    int msgStartPos = numCharsWritten;
    logMsgSize -= numCharsWritten;
    numCharsWritten = vsnprintf(&logMsg[msgStartPos], logMsgSize - numCharsWritten, format, varArgs);

    va_end(varArgs);

    // Print file name and line number at the end of the log message.

    if((numCharsWritten > 0) && (numCharsWritten < logMsgSize))
    {
        msgStartPos += numCharsWritten;
        logMsgSize -= numCharsWritten;
        snprintf(&logMsg[msgStartPos], logMsgSize - numCharsWritten, " (%s:%d) \x1B[0m \n", filename, lineNumber);
    }

    fprintf(stdout, logMsg);
    fflush(stdout);
}

//
// Functions
//

#define NS_TO_MS 1000000

static inline int writePrefixToLogMsg(char* logMsg, size_t logMsgSize, int logMsgLevel,
    const char* functionName)
{
    struct timespec ts;
    struct tm tms;

    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tms);

    size_t numCharsWritten = 0;
    if (logMsgLevel == CRITICAL_LOG_MSG || logMsgLevel == ATTENTION_LOG_MSG)
    {
        numCharsWritten += snprintf(&logMsg[numCharsWritten], logMsgSize - numCharsWritten, "\x1B[31m");
    }
    else if (logMsgLevel == INFO_LOG_MSG)
    {
        numCharsWritten += snprintf(&logMsg[numCharsWritten], logMsgSize - numCharsWritten, "\x1B[32m");
    }

    numCharsWritten += strftime(&logMsg[numCharsWritten], logMsgSize - numCharsWritten, "%F %T", &tms);

    if(numCharsWritten > 0)
    {
        numCharsWritten += snprintf(&logMsg[numCharsWritten], logMsgSize - numCharsWritten,
            ":%04ld (tid %lu) [%s] %s() - ", ts.tv_nsec / NS_TO_MS, thrd_current(),
            logLevelDescriptions[logMsgLevel], functionName);
    }

    return (int) numCharsWritten;
}
