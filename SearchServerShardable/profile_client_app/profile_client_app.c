#include <profile_client.h>
#include <common/request_response_definitions.h>
#include <logger.h>
#include <utils.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

//
// Types
//

typedef enum {
    INVALID_REQ              = 0,
    GET_PROFILE_REQ          = 1,
    SAVE_PROFILE_REQ         = 2,
    START_SEARCH_REQ         = 3,
    SEARCH_STATUS_REQ        = 4,
    SEARCH_RESULTS_REQ       = 5,
    GET_COMPANY_REQ          = 6,
    SAVE_COMPANY_REQ         = 7,
    END_SEARCH_REQ           = 8,
    GET_MULTI_PROFILES_REQ   = 9,
    GET_SYSTEM_INFO_REQ      = 10,
    SET_SERVER_LOG_LEVEL     = 11,
    SAVE_MULTI_PROFILES_REQ  = 12,
    SAVE_MULTI_COMPANIES_REQ = 13,
} RequestCode_t;

//
// Prototypes
//

static bool parseCommandLine(int argc, char* argv[], const char** serverAddress, uint16_t* serverPort,
    RequestCode_t* requestCode, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned,
    const char** jsonFilename, const char** similaritiesFilename, uint32_t* limit, uint32_t* logLevel);
static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t minValue, uint32_t maxValue, uint32_t* argValue);
static bool parseNumbersListArgument(const char* argName, const char* argValueStr,
    uint32_t maxValue, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned);
static void printUsage(const char* programName);
static int readFile(const char* filename, void* buffer, uint32_t bufferSize, uint32_t* numBytesRead);
static int adjustJsonData(char* jsonData, uint32_t* jsonDataSize, uint32_t jsonDataCapacity);

//
// Functions
//

int main(int argc, char* argv[])
{
    const char* serverAddress = NULL;
    uint16_t serverPort       = 0;
    RequestCode_t requestCode = INVALID_REQ;
    int32_t ids[MAX_NUM_ITEMS_IN_REQUEST] = { 0u };
    uint32_t idsSize          = MAX_NUM_ITEMS_IN_REQUEST;
    uint32_t numIdsReturned   = 0;
    const char* jsonFilename  = NULL;
    const char* similaritiesFilename = NULL;
    uint32_t limit            = 0;
    uint32_t logLevel         = LOG_MSG_LEVEL_MAX + 1;

    if(!parseCommandLine(argc, argv, &serverAddress, &serverPort, &requestCode, ids, idsSize,
        &numIdsReturned, &jsonFilename, &similaritiesFilename, &limit, &logLevel))
    {
        printUsage(argv[0]);
        return EINVAL;
    }

    signal(SIGPIPE, SIG_IGN);

    uint8_t responseCode = NULL_RESPONSE;

    uint32_t jsonSendDataCapacity = REQUEST_MAX_SIZE;
    char* jsonSendData = calloc(REQUEST_MAX_SIZE, sizeof(char));
    if(jsonSendData == NULL)
    {
        fprintf(stderr, "Error: jsonSendData = calloc(capacity = %u, item size = %zu bytes) failed.\n",
            REQUEST_MAX_SIZE, sizeof(char));
        return ENOMEM;
    }

    uint32_t jsonReceiveDataCapacity = REQUEST_MAX_SIZE;
    char* jsonReceiveData = calloc(REQUEST_MAX_SIZE, sizeof(char));
    if(jsonReceiveData == NULL)
    {
        fprintf(stderr, "Error: jsonSendData = calloc(capacity = %u, item size = %zu bytes) failed.\n",
            REQUEST_MAX_SIZE, sizeof(char));
        free(jsonSendData);
        return ENOMEM;
    }

    uint32_t similaritiesDataCapacity = REQUEST_MAX_SIZE;
    uint8_t* similaritiesData = calloc(REQUEST_MAX_SIZE, sizeof(uint8_t));
    if(similaritiesData == NULL)
    {
        fprintf(stderr, "Error: similaritiesData = calloc(capacity = %u, item size = %zu bytes)"
            " failed.\n", REQUEST_MAX_SIZE, sizeof(uint8_t));
        free(jsonReceiveData);
        free(jsonSendData);
        return ENOMEM;
    }

    int ret   = 0;
    int sockt = -1;

    int funcRet = connectToServer(serverAddress, serverPort, &sockt);
    if(funcRet != 0)
    {
        fprintf(stderr, "Error: connectToServer(address = %s, port = %hu) returned %d.\n",
            serverAddress, serverPort, funcRet);
        free(similaritiesData);
        free(jsonReceiveData);
        free(jsonSendData);
        return EIO;
    }

    switch(requestCode)
    {
    case GET_PROFILE_REQ:
    {
        uint32_t receivedDataSize = 0;

        int funcRet = getProfileFromServer(sockt, ids[0], &responseCode, jsonReceiveData, jsonReceiveDataCapacity,
            &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getProfileFromServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("JSON Profile:\n\n%s\n", jsonReceiveData);
    }
    break;

    case GET_MULTI_PROFILES_REQ:
    {
        uint32_t receivedDataSize = 0;

        int funcRet = getMultipleProfilesFromServer(sockt, ids, numIdsReturned,
            &responseCode, jsonReceiveData, jsonReceiveDataCapacity, &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getMultipleProfilesFromServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("JSON Profiles:\n\n%s\n", jsonReceiveData);
    }
    break;

    case SAVE_PROFILE_REQ:
    {
        uint32_t sentDataSize = 0;

        int funcRet = readFile(jsonFilename, jsonSendData, jsonSendDataCapacity, &sentDataSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        if(sentDataSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            break;
        }

        funcRet = adjustJsonData(jsonSendData, &sentDataSize, jsonSendDataCapacity);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: adjustJsonData() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        funcRet = sendProfileToServer(sockt, jsonSendData, sentDataSize, &responseCode);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: sendProfileToServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }
    }
    break;

    case START_SEARCH_REQ:
    {
        uint32_t sentDataSize = 0;

        int funcRet = readFile(jsonFilename, jsonSendData, jsonSendDataCapacity, &sentDataSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile(jsonFile = %s) returned %u.\n", jsonFilename, funcRet);
            ret = EIO;
            break;
        }

        if(sentDataSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            break;
        }

        funcRet = adjustJsonData(jsonSendData, &sentDataSize, jsonSendDataCapacity);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: adjustJsonData() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        uint32_t similaritiesDataSize = 0;

        funcRet = readFile(similaritiesFilename, similaritiesData, similaritiesDataCapacity, &similaritiesDataSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile(similaritiesFile = %s) returned %d.\n", similaritiesFilename, funcRet);
            ret = EIO;
            break;
        }

        uint32_t searchId = 0;

        funcRet = startSearchInServer(sockt, jsonSendData, sentDataSize,
            similaritiesData, similaritiesDataSize, limit, &responseCode, &searchId);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: startSearch() returned %d - server response = %u.\n", funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("Search id: %u\n", searchId);
    }
    break;

    case SEARCH_STATUS_REQ:
    {
        uint8_t completionPercentage = 0;

        int funcRet = getSearchStatusFromServer(sockt, ids[0], &responseCode, &completionPercentage);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getSearchStatus() returned %d - server response = %u.\n", funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("Search status: %u%%\n", (uint32_t) completionPercentage);
    }
    break;

    case SEARCH_RESULTS_REQ:
    {
        uint32_t receivedDataSize = 0;

        int funcRet = getSearchResultsFromServer(sockt, ids[0], &responseCode, jsonReceiveData,
            jsonReceiveDataCapacity, &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getSearchResults() returned %d - server response = %u.\n", funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("Search results:\n\n%s\n", jsonReceiveData);
    }
    break;

    case END_SEARCH_REQ:
    {
        int funcRet = endSearchInServer(sockt, ids[0], &responseCode);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: endSearchInServer() returned %d - server response = %u.\n", funcRet, responseCode);
            ret = EPROTO;
            break;
        }
    }
    break;

    case GET_COMPANY_REQ:
    {
        uint32_t receivedDataSize = 0;

        int funcRet = getCompanyFromServer(sockt, ids[0], &responseCode, jsonReceiveData, jsonReceiveDataCapacity,
            &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getCompanyFromServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("JSON Company:\n\n%s\n", jsonReceiveData);
    }
    break;

    case SAVE_COMPANY_REQ:
    {
        uint32_t sentDataSize = 0;

        int funcRet = readFile(jsonFilename, jsonSendData, jsonSendDataCapacity, &sentDataSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        if(sentDataSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            break;
        }

        funcRet = adjustJsonData(jsonSendData, &sentDataSize, jsonSendDataCapacity);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: adjustJsonData() returned %u.\n", funcRet);
            ret = EIO;
            break;
        }

        funcRet = sendCompanyToServer(sockt, jsonSendData, sentDataSize, &responseCode);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: sendCompanyToServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }
    }
    break;

    case GET_SYSTEM_INFO_REQ:
    {
        uint32_t receivedDataSize = 0;

        int funcRet = getSystemInfoFromServer(sockt, &responseCode, jsonReceiveData, jsonReceiveDataCapacity,
            &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getSystemInfoFromServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("JSON System Info:\n\n%s\n", jsonReceiveData);
    }
    break;

    case SET_SERVER_LOG_LEVEL:
    {
        int funcRet = setServerLogLevel(sockt, logLevel, &responseCode);
        if ((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getSearchStatus() returned %d - server response = %u.\n", funcRet, responseCode);
            ret = EPROTO;
            break;
        }
    }
    break;

    case SAVE_MULTI_PROFILES_REQ:
    {
        uint32_t sentDataSize = 0;

        int funcRet = readFile(jsonFilename, jsonSendData, jsonSendDataCapacity, &sentDataSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        if(sentDataSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            break;
        }

        funcRet = adjustJsonData(jsonSendData, &sentDataSize, jsonSendDataCapacity);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: adjustJsonData() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        uint32_t receivedDataSize = 0;

        funcRet = sendMultipleProfilesToServer(sockt, jsonSendData, sentDataSize, &responseCode,
            jsonReceiveData, jsonReceiveDataCapacity, &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: sendMultipleProfilesToServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("JSON Response:\n\n%s\n", jsonReceiveData);
    }
    break;

    case SAVE_MULTI_COMPANIES_REQ:
    {
        uint32_t sentDataSize = 0;

        int funcRet = readFile(jsonFilename, jsonSendData, jsonSendDataCapacity, &sentDataSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        if(sentDataSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            break;
        }

        funcRet = adjustJsonData(jsonSendData, &sentDataSize, jsonSendDataCapacity);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: adjustJsonData() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        uint32_t receivedDataSize = 0;

        funcRet = sendMultipleCompaniesToServer(sockt, jsonSendData, sentDataSize, &responseCode,
            jsonReceiveData, jsonReceiveDataCapacity, &receivedDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: sendMultipleCompaniesToServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("JSON Response:\n\n%s\n", jsonReceiveData);
    }
    break;

    default:
        ret = EINVAL;
    break;
    }

    free(similaritiesData);
    free(jsonReceiveData);
    free(jsonSendData);

    funcRet = endConnectionToServer(sockt, &responseCode);
    if(funcRet != 0)
    {
        fprintf(stderr, "Error: endConnectionToServer() returned %d - server response = %u.\n", funcRet, responseCode);
        ret = EIO;
    }

    return ret;
}

#define SERVER_ADDR_ARG       1
#define SERVER_PORT_ARG       2
#define REQUEST_ARG           3
#define ID_ARG                4
#define JSON_FILE_ARG         5
#define SIMILARITIES_FILE_ARG 6
#define LIMIT_ARG             7
#define LOG_LEVEL_ARG         8

static bool parseCommandLine(int argc, char* argv[], const char** serverAddress, uint16_t* serverPort,
    RequestCode_t* requestCode, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned,
    const char** jsonFilename, const char** similaritiesFilename, uint32_t* limit, uint32_t* logLevel)
{
    *serverAddress        = NULL;
    *serverPort           = 0;
    *requestCode          = INVALID_REQ;
    *numIdsReturned       = 0;
    *jsonFilename         = NULL;
    *similaritiesFilename = NULL;
    *limit                = 0;
    *logLevel             = LOG_MSG_LEVEL_MAX + 1;

    static struct option longOptions[] =
    {
        { "server_addr", required_argument, NULL, SERVER_ADDR_ARG },
        { "server_port", required_argument, NULL, SERVER_PORT_ARG },
        { "request",     required_argument, NULL, REQUEST_ARG },
        { "id",          required_argument, NULL, ID_ARG },
        { "json_file",   required_argument, NULL, JSON_FILE_ARG },
        { "sim_file",    required_argument, NULL, SIMILARITIES_FILE_ARG },
        { "limit",       required_argument, NULL, LIMIT_ARG },
        { "log_level",   required_argument, NULL, LOG_LEVEL_ARG },
        { NULL,          0,                 NULL, 0 }
    };

    const char* requestCodeString = NULL;
    const char* logLevelCodeString = NULL;

    int optionIndex   = -1;
    bool argError     = false;
    uint32_t argValue = 0;

    opterr = 0;
    int option = -1;

    while(((option = getopt_long(argc, argv, "", longOptions, &optionIndex)) != -1)
        && (argError == false))
    {
        switch(option)
        {
            case SERVER_ADDR_ARG:
                if(optarg != NULL) {
                    *serverAddress = optarg;
                } else {
                    argError = true;
                }
                break;

            case SERVER_PORT_ARG:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, 1, UINT16_MAX, &argValue);
                if(!argError) { *serverPort = (uint16_t) argValue; }
                break;

            case REQUEST_ARG:
                if(optarg != NULL) {
                    requestCodeString = optarg;
                } else {
                    argError = true;
                }
                break;

            case ID_ARG:
                argError = !parseNumbersListArgument(longOptions[optionIndex].name, optarg,
                    UINT32_MAX, ids, idsSize, numIdsReturned);
                break;

            case JSON_FILE_ARG:
                if(optarg != NULL) {
                    *jsonFilename = optarg;
                } else {
                    argError = true;
                }
                break;

            case SIMILARITIES_FILE_ARG:
                if(optarg != NULL) {
                    *similaritiesFilename = optarg;
                } else {
                    argError = true;
                }
                break;

            case LIMIT_ARG:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, 1, UINT32_MAX, &argValue);
                if(!argError) { *limit = argValue; }
                break;

            case LOG_LEVEL_ARG:
                if (optarg != NULL) {
                    logLevelCodeString = optarg;
                }
                else {
                    argError = true;
                }
                break;

            default:
                return false;
        }
    }

    if ((*serverAddress == 0) || (*serverPort == 0) || (requestCodeString == NULL))
        return false;

    if(strcmp("get_profile", requestCodeString) == 0) {
        *requestCode = GET_PROFILE_REQ;
    } else if(strcmp("save_profile", requestCodeString) == 0) {
        *requestCode = SAVE_PROFILE_REQ;
    } else if(strcmp("start_search", requestCodeString) == 0) {
        *requestCode = START_SEARCH_REQ;
    } else if(strcmp("search_status", requestCodeString) == 0) {
        *requestCode = SEARCH_STATUS_REQ;
    } else if(strcmp("search_results", requestCodeString) == 0) {
        *requestCode = SEARCH_RESULTS_REQ;
    } else if(strcmp("get_company", requestCodeString) == 0) {
        *requestCode = GET_COMPANY_REQ;
    } else if(strcmp("save_company", requestCodeString) == 0) {
        *requestCode = SAVE_COMPANY_REQ;
    } else if(strcmp("end_search", requestCodeString) == 0) {
        *requestCode = END_SEARCH_REQ;
    } else if(strcmp("get_multi_profiles", requestCodeString) == 0) {
        *requestCode = GET_MULTI_PROFILES_REQ;
    } else if(strcmp("system_info", requestCodeString) == 0) {
        *requestCode = GET_SYSTEM_INFO_REQ;
    } else if(strcmp("set_log_level", requestCodeString) == 0) {
        *requestCode = SET_SERVER_LOG_LEVEL;
    } else if(strcmp("save_multi_profiles", requestCodeString) == 0) {
        *requestCode = SAVE_MULTI_PROFILES_REQ;
    } else if(strcmp("save_multi_companies", requestCodeString) == 0) {
        *requestCode = SAVE_MULTI_COMPANIES_REQ;
    } else {
        return false;
    }

    if (logLevelCodeString != NULL)
    {
        if (strcmp("critical", logLevelCodeString) == 0) {
            *logLevel = CRITICAL_LOG_MSG;
        }
        else if (strcmp("attention", logLevelCodeString) == 0) {
            *logLevel = ATTENTION_LOG_MSG;
        }
        else if (strcmp("info", logLevelCodeString) == 0) {
            *logLevel = INFO_LOG_MSG;
        }
        else if (strcmp("debug", logLevelCodeString) == 0) {
            *logLevel = DEBUG_LOG_MSG;
        }
        else {
            return false;
        }
    }

    if((*requestCode == GET_PROFILE_REQ) || (*requestCode == SEARCH_STATUS_REQ) || (*requestCode == SEARCH_RESULTS_REQ)
        || (*requestCode == GET_COMPANY_REQ) || (*requestCode == END_SEARCH_REQ)) {
        return (*numIdsReturned == 1) && (*jsonFilename == NULL) && (*similaritiesFilename == NULL) && (*limit == 0)
            && (*logLevel == (LOG_MSG_LEVEL_MAX + 1));
    } else if(*requestCode == GET_MULTI_PROFILES_REQ) {
        return (*numIdsReturned > 0) && (*jsonFilename == NULL) && (*similaritiesFilename == NULL) && (*limit == 0)
            && (*logLevel == (LOG_MSG_LEVEL_MAX + 1));
    } else if((*requestCode == SAVE_PROFILE_REQ) || (*requestCode == SAVE_COMPANY_REQ)
        || (*requestCode == SAVE_MULTI_PROFILES_REQ) || (*requestCode == SAVE_MULTI_COMPANIES_REQ)) {
        return (*numIdsReturned == 0) && (*jsonFilename != NULL) && (*similaritiesFilename == NULL) && (*limit == 0)
            && (*logLevel == (LOG_MSG_LEVEL_MAX + 1));
    } else if(*requestCode == START_SEARCH_REQ) {
        return (*numIdsReturned == 0) && (*jsonFilename != NULL) && (*similaritiesFilename != NULL) && (*limit != 0)
            && (*logLevel == (LOG_MSG_LEVEL_MAX + 1));
    } else if(*requestCode == SET_SERVER_LOG_LEVEL) {
        return (*numIdsReturned == 0) && (*jsonFilename == NULL) && (*similaritiesFilename == NULL) && (*limit == 0)
            && (*logLevel <= LOG_MSG_LEVEL_MAX);
    } else { // *requestCode == GET_SYSTEM_INFO_REQ
        return (*numIdsReturned == 0) && (*jsonFilename == NULL) && (*similaritiesFilename == NULL) && (*limit == 0)
            && (*logLevel == (LOG_MSG_LEVEL_MAX + 1));
    }

    return false;
}

static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t minValue, uint32_t maxValue,
    uint32_t* argValue)
{
    char* endPtr = NULL;

    errno = 0;
    uint32_t value = strtoul(argValueStr, &endPtr, 10);    
    if(errno != 0)
    {
        fprintf(stderr, "Error: invalid value (%s) for argument \"%s\". errno = %d (\"%s\").\n",
            argValueStr, argName, errno, strerror(errno));
        return false;
    }
    
    if(*endPtr != '\0')
    {
        fprintf(stderr, "Error: invalid value (%s) for argument \"%s\".\n", argValueStr, argName);
        return false;
    }

    if(value < minValue)
    {
        fprintf(stderr, "Error: value (%s) for argument \"%s\" is smaller than minimum value (%u).\n",
            argValueStr, argName, minValue);
        return false;
    }

    if(maxValue > 0)
    {
        if(value > maxValue)
        {
            fprintf(stderr, "Error: value (%s) for argument \"%s\" is greater than maximum value (%u).\n",
                argValueStr, argName, maxValue);
            return false;
        }
    }

    *argValue = value;

    return true;
}

static bool parseNumbersListArgument(const char* argName, const char* argValueStr,
    uint32_t maxValue, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned)
{
    *numIdsReturned = 0;

    uint32_t i = 0;
    uint32_t auxNumIdsReturned = 0;

    const char* beginPtr = argValueStr;
    char* endPtr         = NULL;

    for(i = 0; i < idsSize; ++i)
    {
        uint32_t value = strtoul(beginPtr, &endPtr, 10);
        if((*endPtr != ',') && (*endPtr != '\0'))
        {
            fprintf(stderr, "Error: invalid character '%c' in value for argument \"%s\".\n",
                *endPtr, argName);
            return false;
        }

        if(value == 0)
        {
            fprintf(stderr, "Error: invalid 0 (zero) number in value for argument \"%s\".\n",
                argName);
            return false;
        }

        if(maxValue > 0)
        {
            if(value > maxValue)
            {
                fprintf(stderr, "Error: number (%u) in value for argument \"%s\" is greater than"
                    " maximum (%u).\n", value, argName, maxValue);
                return false;
            }
        }

        ids[i] = value;
        ++auxNumIdsReturned;

        if(*endPtr == '\0') { break; }

        beginPtr = endPtr + 1;
    }

    if(*endPtr == ',')
    {
        fprintf(stderr, "Error: too many numbers (maximum = %u) in value \"%s\" for"
            " argument \"%s\".\n", idsSize, argValueStr, argName);
        return false;
    }

    if(*endPtr != '\0')
    {
        fprintf(stderr, "Error: invalid character '%c' in value for argument \"%s\".\n",
            *endPtr, argName);
        return false;
    }

    *numIdsReturned = auxNumIdsReturned;

    return true;
}

static void printUsage(const char* programName)
{
    fprintf(stderr, "Usage: %s --server_addr <STRING> --server_port <PORT> --request <STRING>"
        " --id <NUMBER> --json_file <STRING> --limit <NUMBER>\n\n"
        "Arguments:\n\n:"
        "Required:\n"
        "  --server_addr : server IPv4 address.\n"
        "  --server_port : server port.\n"
        "      (must be greater than 0 and maximum value = %hu)\n"
        "  --request : string indicating the operation to be performed:\n"
        "      (Only the arguments listed in each request should be used.)\n"
        "      get_profile : get profile identified by --id argument.\n"
        "      save_profile : send profile. Data is read from file identified by --json_file.\n"
        "      get_multi_profiles : get multiple profiles. List of ids provided by --id argument.\n"
        "            Each id must be separated by , (comma) with no space between them.\n"
        "            (number of ids must be greater than 0 and below or equal %u)\n"
        "      save_multi_profiles : send multiple profiles. Data is read from file identified by --json_file.\n"
        "      start_search : start search.\n"
        "            JSON part of criteria is read from file identified by --json_file.\n"
        "            Similarities part of criteria is read from file identified by --sim_file.\n"
        "            Maximum number of results is given by --limit.\n"
        "      search_status : get status of search identified by --id argument.\n"
        "      search_results : get results of search identified by --id argument.\n" 
        "      end_search : end search identified by --id argument.\n"
        "      get_company : get company identified by --id argument.\n"
        "      save_company : send company. Data is read from file identified by --json_file.\n"
        "      save_multi_companies : send multiple companies. Data is read from file identified by --json_file.\n"
        "      system_info : retrieve system information.\n"
        "      set_log_level : set server logging level.\n\n"
        "Optional:\n"
        "  --id : id of data to be retrieved.\n"
        "      (must be greater than 0 and maximum value = %u)\n"
        "  --json_file : path to existing file with a json data to be sent.\n"
        "  --sim_file : path to existing file with a similarity scores data to be sent.\n"
        "  --limit : maximum limit of items to be retrieved.\n"
        "      (must be greater than 0 and maximum value = %u)\n"
        "  --log_level : log level the server will use. \n"
        "      (critical/attention/info/debug).\n",
        programName, UINT16_MAX, MAX_NUM_ITEMS_IN_REQUEST, UINT32_MAX, UINT32_MAX);
}

static int readFile(const char* filename, void* buffer, uint32_t bufferSize, uint32_t* numBytesRead)
{
    int fd = open(filename, O_RDONLY);
    if(fd < 0)
    {
        fprintf(stderr, "readFile(%s) - open() failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return 1;
    }

    off_t fileSize = lseek(fd, 0, SEEK_END);
    if(fileSize < 0)
    {
        fprintf(stderr, "readFile(%s) - fseek(0, SEEK_END) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        close(fd);
        return 2;
    }

    if(lseek(fd, 0, SEEK_SET) < 0)
    {
        fprintf(stderr, "readFile(%s) - fseek(0, SEEK_SET) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        close(fd);
        return 3;
    }

    if(fileSize > bufferSize)
    {
        fprintf(stderr, "readFile(%s) - error: file size (%zu bytes) > buffer size (%u bytes).",
            filename, fileSize, bufferSize);
        close(fd);
        return 4;
    }

    *numBytesRead = read(fd, buffer, fileSize);
    if(*numBytesRead != fileSize)
    {
        fprintf(stderr, "readFile(%s) - read(%zu bytes) failed. errno = %d (\"%s\").",
            filename, fileSize, errno, strerror(errno));
        close(fd);
        return 5;
    }

    close(fd);
    return 0;
}

static int adjustJsonData(char* jsonData, uint32_t* jsonDataSize, uint32_t jsonDataCapacity)
{
    if((*jsonDataSize != 0) && (jsonData[*jsonDataSize - 1] == '\0')) { return 0; }

    if(*jsonDataSize == jsonDataCapacity)
    {
        fprintf(stderr, "adjustJsonData() - jsonData is not NULL terminated and "
            "there is no space left to add it. Maximum size (%u characteres) reached.\n",
            jsonDataCapacity);
        return 1;
    }

    jsonData[*jsonDataSize] = '\0';
    ++*jsonDataSize;

    return 0;
}
