#ifndef LOGGER_H
#define LOGGER_H

//
// Constants
//

#define NULL_LOG_MSG       0xffff
#define CRITICAL_LOG_MSG   0
#define ATTENTION_LOG_MSG  1
#define INFO_LOG_MSG       2
#define DEBUG_LOG_MSG      3

#define LOG_MSG_LEVEL_MAX  DEBUG_LOG_MSG
#define NUM_LOG_MSG_LEVELS (DEBUG_LOG_MSG + 1)

//
// Variables
//

extern unsigned int logger_level;

//
// Functions
//

void logger_setLogLevel(unsigned int level);

const char* logger_logLevelString(unsigned int level);

void logger_logMessage(const char* format, int logMsgLevel, const char* functionName,
    const char* filename, int lineNumber, ...);

#define LOG_MESSAGE(LOG_MSG_LEVEL, FORMAT, ...) \
    do { \
        if(LOG_MSG_LEVEL <= logger_level) \
            logger_logMessage(FORMAT, LOG_MSG_LEVEL, __FUNCTION__, \
                __FILE__, __LINE__, ##__VA_ARGS__); \
    } while(0)

#endif // LOGGER_H
