#include <file_server.h>
#include <error_reporting.h>  
#include <logger.h>
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
#define SERVER_ID_ARG_NAME                 "server_id"
#define NUM_SERVERS_ARG_NAME               "num_servers"
#define NUM_ENTRIES_PER_FILE_ARG_NAME      "num_entries_per_file"
#define PORT_ARG_NAME                      "port"
#define NUM_CONNECTIONS_ARG_NAME           "num_connections"
#define REQUEST_ARRIVAL_TIMEOUT_ARG_NAME   "request_arrival_timeout"
#define CONNECTION_TIMEOUT_ARG_NAME        "connection_timeout"
#define DATA_DIR_ARG_NAME                  "data_dir"
#define DIR_DEPTH_ARG_NAME                 "dir_depth"
#define DIR_COUNT_ARG_NAME                 "dir_count"

//
// Variables
//

static const uint32_t noMaxValue               = 0;
static const uint32_t maxNumServers            = UINT32_MAX;
static const uint16_t maxNumConnections        = 1000;
static const uint32_t maxRequestArrivalTimeout = 5 * 60 * 1000; // 5 minutes
static const uint32_t maxConnectionTimeout     = 5 * 60 * 1000; // 5 minutes
static const uint32_t maxDirDepth              = 5;
static const uint32_t maxDirCount              = 1000;

static FileServer_t fileServer = FileServer_NULL;

//
// Local prototypes
//

static bool parseCommandLine(int argc, char* argv[], unsigned int* logLevel, uint32_t* serverId, uint32_t* numServers,
    uint32_t* numEntriesPerFile,
    uint16_t* serverPort, uint32_t* requestArrivalTimeout, uint32_t* connectionTimeout,
    uint16_t* numConnections, const char **dataDir, uint32_t* dirDepth, uint32_t* dirCount);
static bool parseLogLevelArgument(const char* argName, const char* argValueStr, unsigned int* logLevel);
static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue, uint32_t* argValue);
static void printUsage(const char* programName);
static void signalHandler(int sigNum);
static void printCurrentDirectory();

//
// Functions
//

int runFileServerApp(int argc, char* argv[])
{
    unsigned int logLevel           = NULL_LOG_MSG;
    uint32_t serverId               = 0;
    uint32_t numServers             = 0;
    uint32_t numEntriesPerFile      = 0;
    uint16_t serverPort             = 0;
    uint32_t requestArrivalTimeout  = 0;
    uint32_t connectionTimeout      = 0;
    uint16_t numConnections         = 0;
    const char *dataDir             = NULL;
    uint32_t dirDepth               = 0;
    uint32_t dirCount               = 0;

    if(!parseCommandLine(argc, argv, &logLevel, &serverId, &numServers,
        &numEntriesPerFile,
        &serverPort, &requestArrivalTimeout, &connectionTimeout,
        &numConnections, &dataDir, &dirDepth, &dirCount))
    {
        printUsage(argv[0]);
        return EINVAL;
    }

    logger_setLogLevel(logLevel);

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Log level: %s\n", logger_logLevelString(logLevel));

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Starting file server...");

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    fileServer = fileServer_init();
    if(fileServer_isNull(fileServer))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileServer_init() failed.");
        return ENOMEM;
    }

    // Printing current directory (temporary code).
    printCurrentDirectory();
    LOG_MESSAGE(INFO_LOG_MSG, "Executable: \"%s\"", argv[0]);

    int ret = fileServer_run(fileServer, serverId, numServers,
        numEntriesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections, dataDir, dirDepth, dirCount);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileServer_run() returned %d.", ret);
        return ENOTRECOVERABLE;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int exitCode = runFileServerApp(argc, argv);
    return exitCode;
}

//
// Local functions
//

#define LOG_LEVEL                 1
#define SERVER_ID                 2
#define NUM_SERVERS               3
#define NUM_ENTRIES_PER_FILE      4
#define PORT                      5
#define REQUEST_ARRIVAL_TIMEOUT   6
#define CONNECTION_TIMEOUT        7
#define NUM_CONNECTIONS           8
#define DATA_DIR                  9
#define DIR_DEPTH                 10
#define DIR_COUNT                 11

static bool parseCommandLine(int argc, char* argv[], unsigned int* logLevel, uint32_t* serverId, uint32_t* numServers,
    uint32_t* numEntriesPerFile,
    uint16_t* serverPort, uint32_t* requestArrivalTimeout, uint32_t* connectionTimeout,
    uint16_t* numConnections, const char** dataDir, uint32_t* dirDepth, uint32_t* dirCount)
{
    *logLevel                = NULL_LOG_MSG;
    *numServers              = 0;
    *serverId                = 0;
    *numEntriesPerFile       = 0;
    *serverPort              = 0;
    *requestArrivalTimeout   = 0;
    *connectionTimeout       = 0;
    *numConnections          = 0;
    *dataDir                 = NULL;
    *dirDepth                = maxDirDepth;
    *dirCount                = maxDirCount;

    static struct option longOptions[] =
    {
        { LOG_LEVEL_ARG_NAME,                 required_argument, NULL, LOG_LEVEL },
        { SERVER_ID_ARG_NAME,                 required_argument, NULL, SERVER_ID },
        { NUM_SERVERS_ARG_NAME,               required_argument, NULL, NUM_SERVERS },
        { NUM_ENTRIES_PER_FILE_ARG_NAME,      required_argument, NULL, NUM_ENTRIES_PER_FILE },
        { PORT_ARG_NAME,                      required_argument, NULL, PORT },
        { REQUEST_ARRIVAL_TIMEOUT_ARG_NAME,   required_argument, NULL, REQUEST_ARRIVAL_TIMEOUT },
        { CONNECTION_TIMEOUT_ARG_NAME,        required_argument, NULL, CONNECTION_TIMEOUT },
        { NUM_CONNECTIONS_ARG_NAME,           required_argument, NULL, NUM_CONNECTIONS },
        { DATA_DIR_ARG_NAME,                  required_argument, NULL, DATA_DIR },
        { DIR_DEPTH_ARG_NAME,                 required_argument, NULL, DIR_DEPTH },
        { DIR_COUNT_ARG_NAME,                 required_argument, NULL, DIR_COUNT },
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

            case SERVER_ID:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxNumServers, &argValue);
                if(!argError) { *serverId = argValue; }
                break;

            case NUM_SERVERS:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxNumServers, &argValue);
                if(!argError) { *numServers = argValue; }
                break;

            case NUM_ENTRIES_PER_FILE:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, noMaxValue, &argValue);
                if(!argError) { *numEntriesPerFile = argValue; }
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

            case DATA_DIR:
                if (optarg != NULL) {
                    *dataDir = optarg;
                }
                else {
                    argError = true;
                }
                break;

            case DIR_DEPTH:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxDirDepth, &argValue);
                if (!argError) { *dirDepth = argValue; }
                break;

            case DIR_COUNT:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxDirCount, &argValue);
                if (!argError) { *dirCount = argValue; }
                break;

            default:
                return false;
        }
    }

    if (*dirDepth > maxDirDepth)
    {
        fprintf(stderr, "Error: invalid value (%u) for argument "DIR_DEPTH_ARG_NAME".\n", *dirDepth);
        return false;
    }
    if (*dirCount > maxDirCount)
    {
        fprintf(stderr, "Error: invalid value (%u) for argument "DIR_COUNT_ARG_NAME".\n", *dirCount);
        return false;
    }
    if((*logLevel == NULL_LOG_MSG) || (*serverId == 0) || (*numServers == 0)
        || (*numEntriesPerFile == 0) || (*dataDir == NULL)
        || (*serverPort == 0) || (*requestArrivalTimeout == 0) || (*connectionTimeout == 0)
        || (*numConnections == 0))
    { 
        return false; 
    }

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
    fprintf(stderr, "Usage: %s --" LOG_LEVEL_ARG_NAME " <STRING> --" SERVER_ID_ARG_NAME " <NUMBER>"
        " --" NUM_SERVERS_ARG_NAME " <NUMBER>"
        " --" NUM_ENTRIES_PER_FILE_ARG_NAME " <NUMBER>"
        " --" PORT_ARG_NAME " <NUMBER> --" REQUEST_ARRIVAL_TIMEOUT_ARG_NAME " <NUMBER>"
        " --" CONNECTION_TIMEOUT_ARG_NAME " <NUMBER> --" NUM_CONNECTIONS_ARG_NAME " <NUMBER>"
        " --" DATA_DIR_ARG_NAME " <STRING>" 
        " --" DIR_DEPTH_ARG_NAME " <NUMBER>"
        " --" DIR_COUNT_ARG_NAME " <NUMBER>\n"
        "Arguments (all are required, the order is irrelevant):\n"
        "    --" LOG_LEVEL_ARG_NAME " : log level.\n"
        "        (must be one of the following: critical, attention, info, debug)\n"
        "    --" SERVER_ID_ARG_NAME " : server id.\n"
        "        (must be greater than 0)\n"
        "    --" NUM_SERVERS_ARG_NAME " : number of servers.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" NUM_ENTRIES_PER_FILE_ARG_NAME " : maximum number of entries stored per file.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" PORT_ARG_NAME " : TCP port to be used by server to listen to requests.\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" REQUEST_ARRIVAL_TIMEOUT_ARG_NAME " : maximum waiting period in milliseconds for request to arrive.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" CONNECTION_TIMEOUT_ARG_NAME " : timeout period in milliseconds for connection to be dropped\n"
        "        in case of inactivity.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" NUM_CONNECTIONS_ARG_NAME " : maximum number of pending connections to be stored\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" DATA_DIR_ARG_NAME " : existing path where files will be saved\n"
        "    --" DIR_DEPTH_ARG_NAME " : subdirectory depth in data path(maximum value = %u)\n"
        "    --" DIR_COUNT_ARG_NAME " : subdirectory count in data path(maximum value = %u)\n",
        programName, maxNumServers, UINT32_MAX, (unsigned short)UINT16_MAX,
        maxRequestArrivalTimeout, maxConnectionTimeout, maxNumConnections,
        maxDirDepth, maxDirCount);
}

static void signalHandler(int sigNum)
{
    fprintf(stdout, "Signal handler - Received signal: %d \"%s\".\n", sigNum, strsignal(sigNum));
    fileServer_stop(fileServer);
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
