#include <ss_server.h>
#include <logger.h>
#include <request_response_definitions.h> 
#include <error_reporting.h> 
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

//
// Constants
//

#define LOG_LEVEL_ARG_NAME                 "log_level"
#define COMPANY_DIR_ARG_NAME               "company_dir"
#define INDUSTRY_DIR_ARG_NAME              "industry_dir"
#define TITLE_DIR_ARG_NAME                 "title_dir"
#define PROFILE_DIR_ARG_NAME               "profile_dir"
#define PORT_ARG_NAME                      "port"
#define NUM_CONNECTIONS_ARG_NAME           "num_connections"
#define REQUEST_ARRIVAL_TIMEOUT_ARG_NAME   "request_arrival_timeout"
#define CONNECTION_TIMEOUT_ARG_NAME        "connection_timeout"

//
// Variables
//

static const uint16_t maxNumConnections        = 200;
static const uint32_t maxRequestArrivalTimeout = 120 * 1000;
static const uint32_t maxConnectionTimeout     = 15 * 1000;

static ScoreServer_t scoreServer = ScoreServer_NULL;

//
// Local prototypes
//

static bool parseCommandLine(int argc, char* argv[], unsigned int* logLevel,
    const char** companyDirectory,
    const char** industryDirectory,
    const char** titleDirectory,
    const char** profileDirectory,
    uint16_t* serverPort, uint32_t* requestArrivalTimeout, uint32_t* connectionTimeout,
    uint16_t* numConnections);
static bool parseLogLevelArgument(const char* argName, const char* argValueStr, unsigned int* logLevel);
static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue, uint32_t* argValue);
static void printUsage(const char* programName);
static void signalHandler(int sigNum);
static void printCurrentDirectory();

//
// Functions
//

int runScoringSimilarityServiceApp(int argc, char* argv[])
{
    unsigned int logLevel           = NULL_LOG_MSG;
    const char* companyDirectory    = NULL;
    const char* industryDirectory   = NULL;
    const char* titleDirectory      = NULL;
    const char* profileDirectory    = NULL;
    uint16_t serverPort             = 0;
    uint32_t requestArrivalTimeout  = 0;
    uint32_t connectionTimeout      = 0;
    uint16_t numConnections         = 0;

    if(!parseCommandLine(argc, argv, &logLevel,
        &companyDirectory,
        &industryDirectory,
        &titleDirectory,
        &profileDirectory,
        &serverPort, &requestArrivalTimeout, &connectionTimeout,
        &numConnections))
    {
        printUsage(argv[0]);
        return EINVAL;
    }

    logger_setLogLevel(logLevel);

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Log level: %s\n", logger_logLevelString(logLevel));

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Starting profile server...");

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    scoreServer = scoreServer_init();
    if(scoreServer_isNull(scoreServer))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: scoreServer_init() failed.");
        return ENOMEM;
    }

    // Printing current directory (temporary code).
    printCurrentDirectory();
    LOG_MESSAGE(INFO_LOG_MSG, "Executable: \"%s\"", argv[0]);

    int ret = scoreServer_run(scoreServer,
        companyDirectory,
        industryDirectory,
        titleDirectory,
        profileDirectory,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: scoreServer_run() returned %d.", ret);
        return ENOTRECOVERABLE;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    const char* errorDirEnvVarName = "ERROR_DIR";
    const char* errorDir = getenv(errorDirEnvVarName);
    if (errorDir == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getenv(\"%s\") returned NULL.", errorDirEnvVarName);
        return EINVAL;
    }
    int ret = errorReporting_start(errorDir);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: errorReporting_start() returned %d.", ret);
    }
    int exitCode = runScoringSimilarityServiceApp(argc, argv);
    ret = errorReporting_stop();
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: errorReporting_stop() returned %d.", ret);
    }
    return exitCode;
}

//
// Local functions
//

#define LOG_LEVEL                 1
#define COMPANY_DIR               2
#define INDUSTRY_DIR              3
#define TITLE_DIR                 4
#define PROFILE_DIR               5
#define PORT                      10
#define REQUEST_ARRIVAL_TIMEOUT   11
#define CONNECTION_TIMEOUT        12
#define NUM_CONNECTIONS           13

static bool parseCommandLine(int argc, char* argv[], unsigned int* logLevel,
    const char** companyDirectory,
    const char** industryDirectory,
    const char** titleDirectory,
    const char** profileDirectory,
    uint16_t* serverPort, uint32_t* requestArrivalTimeout, uint32_t* connectionTimeout,
    uint16_t* numConnections)
{
    *logLevel                = NULL_LOG_MSG;
    *companyDirectory        = NULL;
    *industryDirectory       = NULL;
    *titleDirectory          = NULL;
    *profileDirectory        = NULL;
    *serverPort              = 0;
    *requestArrivalTimeout   = 0;
    *connectionTimeout       = 0;
    *numConnections          = 0;

    static struct option longOptions[] =
    {
        { LOG_LEVEL_ARG_NAME,                 required_argument, NULL, LOG_LEVEL },
        { COMPANY_DIR_ARG_NAME,               required_argument, NULL, COMPANY_DIR },
        { INDUSTRY_DIR_ARG_NAME,              required_argument, NULL, INDUSTRY_DIR },
        { TITLE_DIR_ARG_NAME,                 required_argument, NULL, TITLE_DIR },
        { PROFILE_DIR_ARG_NAME,               required_argument, NULL, PROFILE_DIR },
        { PORT_ARG_NAME,                      required_argument, NULL, PORT },
        { REQUEST_ARRIVAL_TIMEOUT_ARG_NAME,   required_argument, NULL, REQUEST_ARRIVAL_TIMEOUT },
        { CONNECTION_TIMEOUT_ARG_NAME,        required_argument, NULL, CONNECTION_TIMEOUT },
        { NUM_CONNECTIONS_ARG_NAME,           required_argument, NULL, NUM_CONNECTIONS },
        { NULL,                               0,                 NULL, 0 }
    };

    int optionIndex = -1;
    bool argError = false;
    uint32_t argValue = 0;

    opterr = 0;
    int option = -1;

    while(((option = getopt_long(argc, argv, "", longOptions, &optionIndex)) != -1)
        && (argError == false))
    {
        switch(option)
        {
            case LOG_LEVEL:
                argError = !parseLogLevelArgument(longOptions[optionIndex].name, optarg, logLevel);
                break;

            case COMPANY_DIR:
                if (optarg != NULL) {
                    *companyDirectory = optarg;
                    if ((*companyDirectory)[strnlen(*companyDirectory, MAX_STRING_LEN) - 1] == '\\' ||
                        (*companyDirectory)[strnlen(*companyDirectory, MAX_STRING_LEN) - 1] == '/')
                    {
                        ((char*)(*companyDirectory))[strnlen(*companyDirectory, MAX_STRING_LEN) - 1] = 0;
                    }
                }
                else {
                    argError = true;
                }
                break;

            case INDUSTRY_DIR:
                if (optarg != NULL) {
                    *industryDirectory = optarg;
                    if ((*industryDirectory)[strnlen(*industryDirectory, MAX_STRING_LEN) - 1] == '\\' ||
                        (*industryDirectory)[strnlen(*industryDirectory, MAX_STRING_LEN) - 1] == '/')
                    {
                        ((char*)(*industryDirectory))[strnlen(*industryDirectory, MAX_STRING_LEN) - 1] = 0;
                    }
                }
                else {
                    argError = true;
                }
                break;

            case TITLE_DIR:
                if (optarg != NULL) {
                    *titleDirectory = optarg;
                    if ((*titleDirectory)[strnlen(*titleDirectory, MAX_STRING_LEN) - 1] == '\\' ||
                        (*titleDirectory)[strnlen(*titleDirectory, MAX_STRING_LEN) - 1] == '/')
                    {
                        ((char*)(*titleDirectory))[strnlen(*titleDirectory, MAX_STRING_LEN) - 1] = 0;
                    }
                }
                else {
                    argError = true;
                }
                break;

            case PROFILE_DIR:
                if(optarg != NULL) { 
                    *profileDirectory = optarg; 
                    if ((*profileDirectory)[strnlen(*profileDirectory, MAX_STRING_LEN) - 1] == '\\' ||
                        (*profileDirectory)[strnlen(*profileDirectory, MAX_STRING_LEN) - 1] == '/')
                    {
                        ((char*)(*profileDirectory))[strnlen(*profileDirectory, MAX_STRING_LEN) - 1] = 0;
                    }
                } else { 
                    argError = true; 
                }
                break;

            case PORT:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, UINT16_MAX, &argValue);
                if(!argError) { *serverPort = (uint16_t) argValue; }
                break;

            case REQUEST_ARRIVAL_TIMEOUT:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg,
                    maxRequestArrivalTimeout, &argValue);
                if(!argError) { *requestArrivalTimeout = argValue; }
                break;

            case CONNECTION_TIMEOUT:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg,
                    maxConnectionTimeout, &argValue);
                if(!argError) { *connectionTimeout = argValue; }
                break;

            case NUM_CONNECTIONS:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxNumConnections, &argValue);
                if(!argError) { *numConnections = (uint16_t) argValue; }
                break;

            default:
                return false;
        }
    }

    if((*logLevel == NULL_LOG_MSG) 
        || (*companyDirectory == NULL)
        || (*industryDirectory == NULL)
        || (*titleDirectory == NULL)
        || (*profileDirectory == NULL)
        || (*serverPort == 0) || (*requestArrivalTimeout == 0) || (*connectionTimeout == 0)
        || (*numConnections == 0))
    { return false; }

    return true;
}

static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue,
    uint32_t* argValue)
{
    char* endPtr = NULL;

    uint32_t value = strtoul(argValueStr, &endPtr, 10);
    if(*endPtr != '\0')
    {
        fprintf(stderr, "Error: invalid value (%s) for argument \"%s\".\n", argValueStr, argName);
        return false;
    }

    if(value == 0)
    {
        fprintf(stderr, "Error: invalid 0 (zero) value for argument \"%s\".\n", argName);
        return false;
    }

    if(maxValue > 0)
    {
        if(value > maxValue)
        {
            fprintf(stderr, "Error: value (%s) for argument \"%s\" is greater than maximum value"
                " (%u).\n", argValueStr, argName, maxValue);
            return false;
        }
    }

    *argValue = value;

    return true;
}

static bool parseLogLevelArgument(const char* argName, const char* argValueStr,
    unsigned int* logLevel)
{
    struct {
        const char* argument;
        unsigned int level;
    } logLevelArguments[NUM_LOG_MSG_LEVELS] = {
        { "critical",  CRITICAL_LOG_MSG },
        { "attention", ATTENTION_LOG_MSG },
        { "info",      INFO_LOG_MSG },
        { "debug",     DEBUG_LOG_MSG }
    };

    for(size_t i = 0; i < NUM_LOG_MSG_LEVELS; ++i)
    {
        if(strcmp(logLevelArguments[i].argument, argValueStr) == 0)
        {
            *logLevel = logLevelArguments[i].level;
            return true;
        }
    }

    fprintf(stderr, "Error: invalid value \"%s\" for argument \"%s\".\n", argValueStr, argName);
    *logLevel = NULL_LOG_MSG;

    return false;
}

static void printUsage(const char* programName)
{
    fprintf(stderr, "Usage: %s --" LOG_LEVEL_ARG_NAME 
        " --" COMPANY_DIR_ARG_NAME " <STRING>"
        " --" INDUSTRY_DIR_ARG_NAME " <STRING>"
        " --" TITLE_DIR_ARG_NAME " <STRING>"
        " --" PROFILE_DIR_ARG_NAME " <STRING>"
        " --" PORT_ARG_NAME " <NUMBER> --" REQUEST_ARRIVAL_TIMEOUT_ARG_NAME " <NUMBER>"
        " --" CONNECTION_TIMEOUT_ARG_NAME " <NUMBER> --" NUM_CONNECTIONS_ARG_NAME " <NUMBER> \n"
        "Arguments (all are required, the order is irrelevant):\n"
        "    --" LOG_LEVEL_ARG_NAME " : log level.\n"
        "        (must be one of the following: critical, attention, info, debug)\n"
        "    --" COMPANY_DIR_ARG_NAME " : existing directory where company files are stored.\n"
        "    --" INDUSTRY_DIR_ARG_NAME " : existing directory where company files are stored.\n"
        "    --" TITLE_DIR_ARG_NAME " : existing directory where company files are stored.\n"
        "    --" PROFILE_DIR_ARG_NAME" : existing directory where profiles files are stored.\n"
        "    --" PORT_ARG_NAME " : TCP port to be used by server to listen to requests.\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" REQUEST_ARRIVAL_TIMEOUT_ARG_NAME " : maximum waiting period in milliseconds for request to arrive.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" CONNECTION_TIMEOUT_ARG_NAME " : timeout period in milliseconds for connection to be dropped\n"
        "        in case of inactivity.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" NUM_CONNECTIONS_ARG_NAME " : maximum number of pending connections to be stored\n"
        "        (must be greater than 0 and maximum value = %hu)\n",
        programName, (unsigned short)UINT16_MAX,
        maxRequestArrivalTimeout, maxConnectionTimeout, maxNumConnections);
}

static void signalHandler(int sigNum)
{
    fprintf(stdout, "Signal handler - Received signal: %d \"%s\".\n", sigNum, strsignal(sigNum));
    scoreServer_stop(scoreServer);
}

static void printCurrentDirectory()
{
    char currentDirectory[PATH_MAX + 1];

    char* result = getcwd(currentDirectory, PATH_MAX + 1);
    if(result == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getcwd() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
    }
    else
    {
        LOG_MESSAGE(INFO_LOG_MSG, "Current directory: \"%s\"", currentDirectory);
    }
}
