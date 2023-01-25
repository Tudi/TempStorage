#include <common/request_response_definitions.h>
#include <fs_client.h>
#include <getopt.h>
#include <app_errors.h>
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
    INVALID_REQ     = 0,
    GET_FILE_REQ    = 1,
    SAVE_FILE_REQ   = 2,
} RequestCode_t;

//
// Prototypes
//

static bool parseCommandLine(int argc, char* argv[], const char** serverAddress, uint16_t* serverPort,
    RequestCode_t* requestCode, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned,
    const char** fileName, uint32_t *fileType);
static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue, uint32_t* argValue);
static bool parseNumbersListArgument(const char* argName, const char* argValueStr,
    uint32_t maxValue, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned);
static void printUsage(const char* programName);
static int readFile(const char* filename, void** buffer, uint32_t* numBytesRead);

//
// Functions
//
#define MAX_NUM_ITEMS_IN_REQUEST 1

int main(int argc, char* argv[])
{
    const char* serverAddress = NULL;
    uint16_t serverPort       = 0;
    RequestCode_t requestCode = INVALID_REQ;
    int32_t ids[MAX_NUM_ITEMS_IN_REQUEST] = { 0u };
    uint32_t idsSize          = MAX_NUM_ITEMS_IN_REQUEST;
    uint32_t numIdsReturned   = 0;
    const char* fileName      = NULL;
    uint32_t fileType         = 0;

    if(!parseCommandLine(argc, argv, &serverAddress, &serverPort, &requestCode, ids, idsSize,
        &numIdsReturned, &fileName, &fileType))
    {
        printUsage(argv[0]);
        return EINVAL;
    }

    signal(SIGPIPE, SIG_IGN);

    uint8_t responseCode = ERR_FS_NO_ERROR;

    int ret   = 0;
    int sockt = -1;

    int funcRet = connectToServer(serverAddress, serverPort, &sockt);
    if(funcRet != 0)
    {
        fprintf(stderr, "Error: connectToServer(address = %s, port = %hu) returned %d.\n",
            serverAddress, serverPort, funcRet);
        return EIO;
    }

    switch(requestCode)
    {
    case GET_FILE_REQ:
    {
        uint32_t fileSize = 0;

        FSPacketSaveFile* fileContent = NULL;
        int funcRet = getFileFromServer(sockt, fileType, ids[0], &responseCode, &fileContent, &fileSize);
        if((funcRet != 0) || (responseCode != 0) || fileContent == NULL)
        {
            fprintf(stderr, "Error: getFileFromServer() returned %d - server response = %d.\n",
                funcRet, responseCode);
            ret = EPROTO;
            free(fileContent);
            break;
        }

        if (fileName != NULL)
        {
            FILE* f = fopen(fileName, "wb");
            if (f != NULL)
            {
                fwrite(fileContent->data.fileData, 1, fileContent->header.size - sizeof(fileContent->data), f);
                fclose(f);
            }
        }

        free(fileContent);
    }
    break;

    case SAVE_FILE_REQ:
    {
        uint32_t fileSize = 0;
        char *fileContent = 0;

        int funcRet = readFile(fileName, (void**)&fileContent, &fileSize);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile() returned %d.\n", funcRet);
            ret = EIO;
            free(fileContent);
            break;
        }

        if(fileSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            free(fileContent);
            break;
        }

        funcRet = sendFileToServer(sockt, fileType, ids[0], fileContent, fileSize, &responseCode);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: sendProfileToServer() returned %d - server response = %d.\n",
                funcRet, responseCode);
            ret = EPROTO;
            free(fileContent);
            break;
        }

        free(fileContent);
    }
    break;

    default:
        ret = EINVAL;
    break;
    }

/*    funcRet = endConnectionToServer(sockt, &responseCode);
    if(funcRet != 0)
    {
        fprintf(stderr, "Error: endConnectionToServer() returned %d - server response = %d.\n",
            funcRet, responseCode);
        ret = EIO;
    } */

    return ret;
}

#define SERVER_ADDR_ARG       1
#define SERVER_PORT_ARG       2
#define REQUEST_ARG           3
#define ID_ARG                4
#define JSON_FILE_ARG         5
#define FILE_TYPE_ARG         6

static bool parseCommandLine(int argc, char* argv[], const char** serverAddress, uint16_t* serverPort,
    RequestCode_t* requestCode, int32_t* ids, uint32_t idsSize, uint32_t* numIdsReturned,
    const char** fileName, uint32_t* fileType)
{
    *serverAddress        = NULL;
    *serverPort           = 0;
    *requestCode          = INVALID_REQ;
    *numIdsReturned       = 0;
    *fileName             = NULL;
    *fileType             = UINT32_MAX;

    static struct option longOptions[] =
    {
        { "server_addr", required_argument, NULL, SERVER_ADDR_ARG },
        { "server_port", required_argument, NULL, SERVER_PORT_ARG },
        { "request",     required_argument, NULL, REQUEST_ARG },
        { "id",          required_argument, NULL, ID_ARG },
        { "file_name",   required_argument, NULL, JSON_FILE_ARG },
        { "file_type",   required_argument, NULL, FILE_TYPE_ARG },
        { NULL,          0,                 NULL, 0 }
    };

    const char* requestCodeString = NULL;

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
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, UINT16_MAX, &argValue);
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
                    *fileName = optarg;
                } else {
                    argError = true;
                }
                break;

            case FILE_TYPE_ARG:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg, UINT32_MAX, &argValue);
                if (!argError) { *fileType = (uint32_t)argValue; }
                break;

            default:
            {
                fprintf(stderr, "Error: unhandled param type '%s'='%s'.\n", longOptions[optionIndex].name, optarg);
                return false;
            }
        }
    }

    if((*serverAddress == 0) || (*serverPort == 0) || (requestCodeString == NULL))
        return false;

    if(strcmp("get_file", requestCodeString) == 0) {
        *requestCode = GET_FILE_REQ;
    } else if(strcmp("save_file", requestCodeString) == 0) {
        *requestCode = SAVE_FILE_REQ;
    } else {
        fprintf(stderr, "Error: invalid value (%s) for argument request_type.\n", requestCodeString);
        return false;
    }

    if((*requestCode == GET_FILE_REQ) || (*requestCode == SAVE_FILE_REQ)) 
    {
        if(*serverAddress == NULL)
        {
            fprintf(stderr, "Error: invalid value (%s) for argument serverAddress.\n", *serverAddress);
            return false;
        }
        if (*serverPort == 0)
        {
            fprintf(stderr, "Error: invalid value (%d) for argument serverPort.\n", *serverPort);
            return false;
        }
        if (*numIdsReturned != 1)
        {
            fprintf(stderr, "Error: invalid value (%u) for argument numIdsReturned.\n", *numIdsReturned);
            return false;
        }
        if (*fileName == NULL)
        {
            fprintf(stderr, "Error: invalid value (NULL) for argument fileName.\n");
            return false;
        }
        if (*fileType == UINT32_MAX)
        {
            fprintf(stderr, "Error: invalid value (%u) for argument fileType.\n", *fileType);
            return false;
        }
        if (ids[0] == INT32_MAX)
        {
            fprintf(stderr, "Error: invalid value (%d) for argument fileName.\n", ids[0]);
            return false;
        }
        return true;
    }
    else
    {
        fprintf(stderr, "Error: invalid value (%d) for argument request_type.\n", *requestCode);
        return false;
    }

    return false;
}

static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue, uint32_t* argValue)
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
        ids[i] = INT32_MAX;
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
        "      get_file : get profile identified by --id argument.\n"
        "      save_file : send profile. Data is read from file identified by --json_file.\n"
        "Optional:\n"
        "  --file_type : type of the file to be handled.\n"
        "  --id : id of data to be handled.\n"
        "  --file_name : path to file to be handled.\n",
        programName, (unsigned short)UINT16_MAX);
}

static int readFile(const char* filename, void** buffer, uint32_t* numBytesRead)
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

    *buffer = malloc(fileSize);
    if (*buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate %lu bytes", fileSize);
        close(fd);
        return 4;
    }

    *numBytesRead = read(fd, *buffer, fileSize);
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
