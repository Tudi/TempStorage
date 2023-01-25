#include <profile_server.h>
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
#include <error_reporting.h>

//
// Constants
//

#define LOG_LEVEL_ARG_NAME                 "log_level"
#define SERVER_ID_ARG_NAME                 "server_id"
#define NUM_SERVERS_ARG_NAME               "num_servers"
#define PROFILE_DIR_ARG_NAME               "profile_dir"
#define NUM_PROFILE_CACHE_ENTRIES_ARG_NAME "num_profile_cache_entries"
#define NUM_PROFILES_PER_FILE_ARG_NAME     "num_profiles_per_file"
#define COMPANY_DIR_ARG_NAME               "company_dir"
#define NUM_COMPANY_CACHE_ENTRIES_ARG_NAME "num_company_cache_entries"
#define NUM_COMPANIES_PER_FILE_ARG_NAME    "num_companies_per_file"
#define PORT_ARG_NAME                      "port"
#define NUM_CONNECTIONS_ARG_NAME           "num_connections"
#define REQUEST_ARRIVAL_TIMEOUT_ARG_NAME   "request_arrival_timeout"
#define CONNECTION_TIMEOUT_ARG_NAME        "connection_timeout"
#define NUM_SEARCH_THREADS_ARG_NAME        "num_search_threads"
#define NUM_LOADING_THREADS_ARG_NAME       "num_loading_threads"
#define PROFILE_CACHE_UPDATE_ARG_NAME      "profile_cache_update"
#define COMPANY_CACHE_UPDATE_ARG_NAME      "company_cache_update"
#define SEARCH_RESULTS_EXPIRATION_ARG_NAME "search_results_expiration"

//
// Variables
//

static const uint32_t noMaxValue               = 0;
static const uint32_t maxNumServers            = 8 * 1024;
static const uint16_t maxNumConnections        = 1000;
static const uint32_t maxRequestArrivalTimeout = 5 * 60 * 1000; // 5 minutes
static const uint32_t maxConnectionTimeout     = 15 * 1000;
static const uint16_t maxNumThreads            = 1000;
static const uint32_t maxExpirationInMinutes   = 2 * 60;

static ProfileServer_t profileServer = ProfileServer_NULL;

//
// Local prototypes
//

static bool parseCommandLine(int argc, char* argv[], unsigned int* logLevel, uint32_t* serverId, uint32_t* numServers,
    const char** profileDirectory, uint32_t* numProfileCacheEntries, uint32_t* numProfilesPerFile,
    const char** companyDirectory, uint32_t* numCompanyCacheEntries, uint32_t* numCompaniesPerFile,
    uint16_t* serverPort, uint32_t* requestArrivalTimeout, uint32_t* connectionTimeout,
    uint16_t* numConnections, uint16_t* numSearchThreads, uint16_t* numLoadingThreads,
    uint16_t* profileCacheUpdatePeriodInSecs, uint16_t* companyCacheUpdatePeriodInSecs,
    uint16_t* searchResultsExpirationInMinutes);
static bool parseLogLevelArgument(const char* argName, const char* argValueStr, unsigned int* logLevel);
static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue, uint32_t* argValue);
static void printUsage(const char* programName);
static void signalHandler(int sigNum);
static void printCurrentDirectory();

//
// Functions
//

int runProfileServerApp(int argc, char* argv[])
{
    unsigned int logLevel           = NULL_LOG_MSG;
    uint32_t serverId               = UINT32_MAX;
    uint32_t numServers             = 0;
    const char* profileDirectory    = NULL;
    uint32_t numProfileCacheEntries = 0;
    uint32_t numProfilesPerFile     = 0;
    const char* companyDirectory    = NULL;
    uint32_t numCompanyCacheEntries = 0;
    uint32_t numCompaniesPerFile    = 0;
    uint16_t serverPort             = 0;
    uint32_t requestArrivalTimeout  = 0;
    uint32_t connectionTimeout      = 0;
    uint16_t numConnections         = 0;
    uint16_t numSearchThreads       = 0;
    uint16_t numLoadingThreads      = 0;
    uint16_t profileCacheUpdatePeriodInSecs = 0;
    uint16_t companyCacheUpdatePeriodInSecs = 0;
    uint16_t searchResultsExpirationInMinutes = 0;

    if(!parseCommandLine(argc, argv, &logLevel, &serverId, &numServers,
        &profileDirectory, &numProfileCacheEntries, &numProfilesPerFile,
        &companyDirectory, &numCompanyCacheEntries, &numCompaniesPerFile,
        &serverPort, &requestArrivalTimeout, &connectionTimeout,
        &numConnections, &numSearchThreads, &numLoadingThreads,
        &profileCacheUpdatePeriodInSecs, &companyCacheUpdatePeriodInSecs,
        &searchResultsExpirationInMinutes))
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

    profileServer = profileServer_init();
    if(profileServer_isNull(profileServer))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: profileServer_init() failed.");
        return ENOMEM;
    }

    // Printing current directory (temporary code).
    printCurrentDirectory();
    LOG_MESSAGE(INFO_LOG_MSG, "Executable: \"%s\"", argv[0]);

    int ret = profileServer_run(profileServer, serverId, numServers,
        profileDirectory, numProfileCacheEntries, numProfilesPerFile,
        companyDirectory, numCompanyCacheEntries, numCompaniesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections, numSearchThreads, numLoadingThreads,
        profileCacheUpdatePeriodInSecs, companyCacheUpdatePeriodInSecs,
        searchResultsExpirationInMinutes);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: profileServer_run() returned %d.", ret);
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
    int exitCode = runProfileServerApp(argc, argv);
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
#define SERVER_ID                 2
#define NUM_SERVERS               3
#define PROFILE_DIR               4
#define NUM_PROFILE_CACHE_ENTRIES 5
#define NUM_PROFILES_PER_FILE     6
#define COMPANY_DIR               7
#define NUM_COMPANY_CACHE_ENTRIES 8
#define NUM_COMPANIES_PER_FILE    9
#define PORT                      10
#define REQUEST_ARRIVAL_TIMEOUT   11
#define CONNECTION_TIMEOUT        12
#define NUM_CONNECTIONS           13
#define NUM_SEARCH_THREADS        14
#define NUM_LOADING_THREADS       15
#define PROFILE_CACHE_UPDATE      16 
#define COMPANY_CACHE_UPDATE      17 
#define SEARCH_RESULTS_EXPIRATION 18

static bool parseCommandLine(int argc, char* argv[], unsigned int* logLevel, uint32_t* serverId, uint32_t* numServers,
    const char** profileDirectory, uint32_t* numProfileCacheEntries, uint32_t* numProfilesPerFile,
    const char** companyDirectory, uint32_t* numCompanyCacheEntries, uint32_t* numCompaniesPerFile,
    uint16_t* serverPort, uint32_t* requestArrivalTimeout, uint32_t* connectionTimeout,
    uint16_t* numConnections, uint16_t* numSearchThreads, uint16_t* numLoadingThreads,
    uint16_t* profileCacheUpdatePeriodInSecs, uint16_t* companyCacheUpdatePeriodInSecs,
    uint16_t* searchResultsExpirationInMinutes)
{
    *logLevel                = NULL_LOG_MSG;
    *numServers              = 0;
    *serverId                = UINT32_MAX;
    *profileDirectory        = NULL;
    *numProfileCacheEntries  = 0;
    *numProfilesPerFile      = 0;
    *companyDirectory        = NULL;
    *numCompanyCacheEntries  = 0;
    *numCompaniesPerFile     = 0;
    *serverPort              = 0;
    *requestArrivalTimeout   = 0;
    *connectionTimeout       = 0;
    *numConnections          = 0;
    *numSearchThreads        = 0;
    *numLoadingThreads       = 0;
    *profileCacheUpdatePeriodInSecs   = 0;
    *companyCacheUpdatePeriodInSecs   = 0;
    *searchResultsExpirationInMinutes = 0;

    static struct option longOptions[] =
    {
        { LOG_LEVEL_ARG_NAME,                 required_argument, NULL, LOG_LEVEL },
        { SERVER_ID_ARG_NAME,                 required_argument, NULL, SERVER_ID },
        { NUM_SERVERS_ARG_NAME,               required_argument, NULL, NUM_SERVERS },
        { PROFILE_DIR_ARG_NAME,               required_argument, NULL, PROFILE_DIR },
        { NUM_PROFILE_CACHE_ENTRIES_ARG_NAME, required_argument, NULL, NUM_PROFILE_CACHE_ENTRIES },
        { NUM_PROFILES_PER_FILE_ARG_NAME,     required_argument, NULL, NUM_PROFILES_PER_FILE },
        { COMPANY_DIR_ARG_NAME,               required_argument, NULL, COMPANY_DIR },
        { NUM_COMPANY_CACHE_ENTRIES_ARG_NAME, required_argument, NULL, NUM_COMPANY_CACHE_ENTRIES },
        { NUM_COMPANIES_PER_FILE_ARG_NAME,    required_argument, NULL, NUM_COMPANIES_PER_FILE },
        { PORT_ARG_NAME,                      required_argument, NULL, PORT },
        { REQUEST_ARRIVAL_TIMEOUT_ARG_NAME,   required_argument, NULL, REQUEST_ARRIVAL_TIMEOUT },
        { CONNECTION_TIMEOUT_ARG_NAME,        required_argument, NULL, CONNECTION_TIMEOUT },
        { NUM_CONNECTIONS_ARG_NAME,           required_argument, NULL, NUM_CONNECTIONS },
        { NUM_SEARCH_THREADS_ARG_NAME,        required_argument, NULL, NUM_SEARCH_THREADS },
        { NUM_LOADING_THREADS_ARG_NAME,       required_argument, NULL, NUM_LOADING_THREADS },
        { PROFILE_CACHE_UPDATE_ARG_NAME,      required_argument, NULL, PROFILE_CACHE_UPDATE },
        { COMPANY_CACHE_UPDATE_ARG_NAME,      required_argument, NULL, COMPANY_CACHE_UPDATE },
        { SEARCH_RESULTS_EXPIRATION_ARG_NAME, required_argument, NULL, SEARCH_RESULTS_EXPIRATION },
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

            case PROFILE_DIR:
                if(optarg != NULL) { 
                    *profileDirectory = optarg; 
                } else { 
                    argError = true; 
                }
                break;

            case NUM_PROFILE_CACHE_ENTRIES:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, noMaxValue, &argValue);
                if(!argError) { *numProfileCacheEntries = argValue; }
                break;

            case NUM_PROFILES_PER_FILE:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, noMaxValue, &argValue);
                if(!argError) { *numProfilesPerFile = argValue; }
                break;

            case COMPANY_DIR:
                if(optarg != NULL) { 
                    *companyDirectory = optarg; 
                } else { 
                    argError = true; 
                }
                break;

            case NUM_COMPANY_CACHE_ENTRIES:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, noMaxValue, &argValue);
                if(!argError) { *numCompanyCacheEntries = argValue; }
                break;

            case NUM_COMPANIES_PER_FILE:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, noMaxValue, &argValue);
                if(!argError) { *numCompaniesPerFile = argValue; }
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

            case NUM_SEARCH_THREADS:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxNumThreads, &argValue);
                if(!argError) { *numSearchThreads = (uint16_t) argValue; }
                break;

            case NUM_LOADING_THREADS:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, maxNumThreads, &argValue);
                if(!argError) { *numLoadingThreads = (uint16_t) argValue; }
                break;

            case PROFILE_CACHE_UPDATE:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, UINT16_MAX, &argValue);
                if(!argError) { *profileCacheUpdatePeriodInSecs = (uint16_t) argValue; }
                break;

            case COMPANY_CACHE_UPDATE:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, UINT16_MAX, &argValue);
                if(!argError) { *companyCacheUpdatePeriodInSecs = (uint16_t) argValue; }
                break;

            case SEARCH_RESULTS_EXPIRATION:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg,
                    maxExpirationInMinutes, &argValue);
                if(!argError) { *searchResultsExpirationInMinutes = (uint16_t) argValue; }
                break;

            default:
                return false;
        }
    }

    if((*logLevel == NULL_LOG_MSG) || (*serverId == UINT32_MAX) || (*numServers == 0)
        || (*profileDirectory == NULL) || (*numProfileCacheEntries == 0) || (*numProfilesPerFile == 0)
        || (*companyDirectory == NULL) || (*numCompanyCacheEntries == 0) || (*numCompaniesPerFile == 0)
        || (*serverPort == 0) || (*requestArrivalTimeout == 0) || (*connectionTimeout == 0)
        || (*numConnections == 0) || (*numSearchThreads == 0) || (*numLoadingThreads == 0)
        || (*profileCacheUpdatePeriodInSecs == 0) || (*companyCacheUpdatePeriodInSecs == 0)
        || (searchResultsExpirationInMinutes == 0))
    { return false; }

    return true;
}

static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue,
    uint32_t* argValue)
{
    char* endPtr = NULL;

	errno = 0;
    uint32_t value = strtoul(argValueStr, &endPtr, 10);
    if(*endPtr != '\0')
    {
        fprintf(stderr, "Error: invalid value (%s) for argument \"%s\".\n", argValueStr, argName);
        return false;
    }

    if(errno != 0)
    {
        fprintf(stderr, "Error: invalid value (%s) for argument \"%s\". errno = %d (\"%s\").\n", argValueStr, argName, errno, strerror(errno));
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
        " --" PROFILE_DIR_ARG_NAME " <STRING> --" NUM_PROFILE_CACHE_ENTRIES_ARG_NAME " <NUMBER>"
        " --" NUM_PROFILES_PER_FILE_ARG_NAME " <NUMBER> --" COMPANY_DIR_ARG_NAME " <STRING>"
        " --" NUM_COMPANY_CACHE_ENTRIES_ARG_NAME " <NUMBER> --" NUM_COMPANIES_PER_FILE_ARG_NAME " <NUMBER>"
        " --" PORT_ARG_NAME " <NUMBER> --" REQUEST_ARRIVAL_TIMEOUT_ARG_NAME " <NUMBER>"
        " --" CONNECTION_TIMEOUT_ARG_NAME " <NUMBER> --" NUM_CONNECTIONS_ARG_NAME " <NUMBER>"
        " --" NUM_SEARCH_THREADS_ARG_NAME " <NUMBER> --" NUM_LOADING_THREADS_ARG_NAME " <NUMBER>"
        " --" PROFILE_CACHE_UPDATE_ARG_NAME " <NUMBER> --" COMPANY_CACHE_UPDATE_ARG_NAME " <NUMBER>"
        " --" SEARCH_RESULTS_EXPIRATION_ARG_NAME " <NUMBER>\n\n"
        "Arguments (all are required, the order is irrelevant):\n"
        "    --" LOG_LEVEL_ARG_NAME " : log level.\n"
        "        (must be one of the following: critical, attention, info, debug)\n"
        "    --" SERVER_ID_ARG_NAME " : server id.\n"
        "        (must be greater than 0)\n"
        "    --" NUM_SERVERS_ARG_NAME " : number of servers.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" PROFILE_DIR_ARG_NAME" : existing directory where profiles files are stored.\n"
        "    --" NUM_PROFILE_CACHE_ENTRIES_ARG_NAME " : number of entries in profile cache.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" NUM_PROFILES_PER_FILE_ARG_NAME " : maximum number of profiles stored per file.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" COMPANY_DIR_ARG_NAME " : existing directory where company files are stored.\n"
        "    --" NUM_COMPANY_CACHE_ENTRIES_ARG_NAME " : number of entries in company cache.\n"
        "        (must be greater than 0 and maximum value = %u)\n"
        "    --" NUM_COMPANIES_PER_FILE_ARG_NAME " : maximum number of companies stored per file.\n"
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
        "    --" NUM_SEARCH_THREADS_ARG_NAME " : number of threads created by server to process searches.\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" NUM_LOADING_THREADS_ARG_NAME " : number of threads created by server to load the caches.\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" PROFILE_CACHE_UPDATE_ARG_NAME " : profile cache update period in seconds.\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" COMPANY_CACHE_UPDATE_ARG_NAME " : company cache update period in seconds.\n"
        "        (must be greater than 0 and maximum value = %hu)\n"
        "    --" SEARCH_RESULTS_EXPIRATION_ARG_NAME " : expiration period in minutes for search results\n"
       "         to be discarded if not retrieved by client.\n"
        "        (must be greater than 0 and maximum value = %u)\n",
        programName, maxNumServers, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT16_MAX,
        maxRequestArrivalTimeout, maxConnectionTimeout, maxNumConnections, maxNumThreads, maxNumThreads,
        UINT16_MAX, UINT16_MAX, maxExpirationInMinutes);
}

static void signalHandler(int sigNum)
{
    fprintf(stdout, "Signal handler - Received signal: %d \"%s\".\n", sigNum, strsignal(sigNum));
    profileServer_stop(profileServer);
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
