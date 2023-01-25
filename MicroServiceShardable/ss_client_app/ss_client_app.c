#include <ss_client.h>
#include <request_response_definitions.h>
#include <utils.h>
#include <app_errors.h>
#include <score_file.h>
#include <logger.h>
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
// Prototypes
//

static bool parseCommandLine(int argc, char* argv[], const char** serverAddress,
    uint16_t* serverPort, uint8_t* requestCode, const char** similarity_score_filename,
    uint32_t* similarityScoreType, uint32_t* id, const char** request_ascii_data, 
    const char** file_save_req_to, const char** file_save_rep_to);
static bool parseNumberArgument(const char* argName, const char* argValueStr, uint32_t maxValue,
    uint32_t* argValue);
static void printUsage(const char* programName);
static void* convertASCIIToBinRequest(const char* asciiReq, uint32_t *binSize);
static bool parseLogLevelArgument(const char* argValueStr);
//
// Functions
//

int main(int argc, char* argv[])
{
    const char* serverAddress      = NULL;
    uint16_t serverPort            = 0;
    uint8_t requestCode            = SSPT_NOT_USED_PACKET_TYPE;
    uint32_t id                    = 0;
    const char* scoreFilename      = NULL;
    uint32_t scoreType             = 0;
    const char* request_ascii_data = NULL;
    const char* file_save_req_to   = NULL;
    const char* file_save_rep_to   = NULL;

    if(!parseCommandLine(argc, argv, &serverAddress, &serverPort, &requestCode,
        &scoreFilename, &scoreType, &id, &request_ascii_data, &file_save_req_to,
        &file_save_rep_to))
    {
        printUsage(argv[0]);
        return EINVAL;
    }

    signal(SIGPIPE, SIG_IGN);

    uint32_t responseCode = 0;

    int ret   = 0;
    int sockt = -1;
    int endConnectionPacketRequired = 0;

    int funcRet = connectToServer(serverAddress, serverPort, &sockt);
    if(funcRet != 0)
    {
        fprintf(stderr, "Error: connectToServer(address = %s, port = %hu) returned %d.\n",
            serverAddress, serverPort, funcRet);
        return EIO;
    }

    switch(requestCode)
    {
    case SSPT_GET_SCORE:
    {
/*        uint32_t similarityScoreDataSize = 0;

        int funcRet = getProfileFromServer(sockt, ids[0], &responseCode, similarityScoreData, similarityScoreDataCapacity,
            &similarityScoreDataSize);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: getProfileFromServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }

        printf("similarityScore Profile:\n\n%s\n", similarityScoreData); */
    }
    break;

    case SSPT_GET_SCORE_ASCII:
    {
        if (request_ascii_data == NULL)
        {
            fprintf(stderr, "Error: Missing ASCII data from request. Format is : count,type,id[],count,type,id[]... \n");
            ret = EINVAL;
            break;
        }
        // convert the ascii string to binary format
        uint32_t binRequestSize = 0;
        void* binRequest = convertASCIIToBinRequest(request_ascii_data, &binRequestSize);
        if(binRequest == NULL)
        {
            fprintf(stderr, "Error: Invalid ASCII data for request. Expectd format : count,type,id[],count,type,id[]... \n");
            ret = EINVAL;
            break;
        }
        // in case we wish to debug a packet ...
        if (file_save_req_to != NULL)
        {
            FILE* f = fopen(file_save_req_to, "wb");
            if (f != NULL)
            {
                fwrite(binRequest, 1, binRequestSize, f);
                fclose(f);
            }
        }
        SSPacketScoreReply* respPacket = NULL;
        uint32_t respPacketSize = 0;
        int reqError = getScoresFromServer(sockt, binRequest, binRequestSize, (void**)&respPacket, &respPacketSize);
        if (reqError != ERR_SS_NO_ERROR)
        {
            fprintf(stderr, "Error: Failed to fetch results from server. Error %d\n", reqError);
            ret = EINVAL;
        }
        if (file_save_rep_to != NULL)
        {
            FILE* f = fopen(file_save_rep_to, "wb");
            if (f != NULL)
            {
                fwrite(respPacket, 1, respPacketSize, f);
                fclose(f);
            }
        }
        // cleanup
        free(binRequest);
        free(respPacket);
    }break;

    case SSPT_SAVE_SCORE:
    {
        size_t similarityScoreDataSize = 0;

        if (scoreType == 0)
        {
            fprintf(stderr, "Error: Score type should be within valid range : 0 < %u < %d \n", scoreType, SSFT_MAX_SCORE_FILE_TYPE);
            ret = EINVAL;
            break;
        }
        if (id == 0)
        {
            fprintf(stderr, "Error: Entry Id should be withing valid range : 0 < %u < %d \n", id, INT32_MAX);
            ret = EINVAL;
            break;
        }
        FILE* similarityFile = fopen(scoreFilename, "rb");
        if (similarityFile == NULL)
        {
            fprintf(stderr, "Error: Failed to open input file %s.\n", scoreFilename);
            ret = EIO;
            break;
        }
        uint8_t* similarityScoreData = NULL;
        int funcRet = getFileContents(similarityFile, &similarityScoreData, &similarityScoreDataSize);
        fclose(similarityFile);
        if(funcRet != 0)
        {
            fprintf(stderr, "Error: readFile() returned %d.\n", funcRet);
            ret = EIO;
            break;
        }

        if(similarityScoreDataSize == 0)
        {
            fprintf(stderr, "Error: empty file.\n");
            ret = EINVAL;
            break;
        }

        funcRet = sendScoreFileToServer(sockt, scoreType, id, (char*)similarityScoreData, similarityScoreDataSize, &responseCode);
        free(similarityScoreData);
        if((funcRet != 0) || (responseCode != 0))
        {
            fprintf(stderr, "Error: sendProfileToServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EPROTO;
            break;
        }
    }
    break;

    default:
        ret = EINVAL;
    break;
    }

    if (endConnectionPacketRequired == 1)
    {
        funcRet = endConnectionToServer(sockt, &responseCode);
        if (funcRet != 0)
        {
            fprintf(stderr, "Error: endConnectionToServer() returned %d - server response = %u.\n",
                funcRet, responseCode);
            ret = EIO;
        }
    }

    return ret;
}

#define SERVER_ADDR_ARG                  1
#define SERVER_PORT_ARG                  2
#define REQUEST_ARG                      3
#define ID_ARG                           4
#define SIMILARITYSCORE_FILE_ARG         5
#define SIMILARITIES_FILE_ARG            6
#define SIMILARITY_TYPE_ARG              7
#define REQUEST_DATA_ARG                 8
#define SAVE_REQ_TO_FILE_ARG             9
#define SAVE_REPL_TO_FILE_ARG            10
#define LOG_LEVEL                        11

static bool parseCommandLine(int argc, char* argv[], const char** serverAddress,
    uint16_t* serverPort, uint8_t* requestCode, const char** similarity_score_filename,
    uint32_t* similarity_type, uint32_t* id, const char** request_ascii_data, 
    const char** file_save_req_to, const char** file_save_rep_to)
{
    *serverAddress              = NULL;
    *serverPort                 = 0;
    *requestCode                = SSPT_NOT_USED_PACKET_TYPE;
    *similarity_score_filename  = NULL;
    *similarity_type            = 0;
    *id                         = 0;
    *request_ascii_data         = NULL;
    *file_save_req_to           = NULL;
    *file_save_rep_to           = NULL;

    static struct option longOptions[] =
    {
        { "server_addr",        required_argument, NULL, SERVER_ADDR_ARG },
        { "server_port",        required_argument, NULL, SERVER_PORT_ARG },
        { "request",            required_argument, NULL, REQUEST_ARG },
        { "id",                 required_argument, NULL, ID_ARG },
        { "similarity_score_file",   required_argument, NULL, SIMILARITYSCORE_FILE_ARG },
        { "similarity_type",    required_argument, NULL, SIMILARITY_TYPE_ARG },
        { "request_ascii_data", required_argument, NULL, REQUEST_DATA_ARG },
        { "save_req_packet",    required_argument, NULL, SAVE_REQ_TO_FILE_ARG },
        { "save_repl_packet",   required_argument, NULL, SAVE_REPL_TO_FILE_ARG },
        { "log_level",          required_argument, NULL, LOG_LEVEL },
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
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg,
                    UINT16_MAX, &argValue);
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
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg,
                    UINT32_MAX, &argValue);
                if (!argError) { *id = (uint16_t)argValue; }
                break;

            case SIMILARITYSCORE_FILE_ARG:
                if(optarg != NULL) {
                    *similarity_score_filename = optarg;
                } else {
                    argError = true;
                }
                break;

            case SIMILARITY_TYPE_ARG:
                argError = !parseNumberArgument(longOptions[optionIndex].name, optarg,
                    UINT32_MAX, &argValue);
                if(!argError) { *similarity_type = argValue; }
                break;

            case REQUEST_DATA_ARG:
                if (optarg != NULL) {
                    *request_ascii_data = optarg;
                }
                else {
                    argError = true;
                }
                break;

            case SAVE_REQ_TO_FILE_ARG:
                if (optarg != NULL) {
                    *file_save_req_to = optarg;
                }
                else {
                    argError = true;
                }
                break;

            case SAVE_REPL_TO_FILE_ARG:
                if (optarg != NULL) {
                    *file_save_rep_to = optarg;
                }
                else {
                    argError = true;
                }
                break;

            case LOG_LEVEL:
                if (optarg != NULL) {
                    parseLogLevelArgument( optarg );
                }
                else {
                    argError = true;
                }
                break;

            default:
                return false;
        }
    }

    if ((*serverAddress == 0))
    {
        fprintf(stderr, "Error: Missing server address argument.\n");
        return false;
    }
    if ((*serverPort == 0))
    {
        fprintf(stderr, "Error: Missing server port argument.\n");
        return false;
    }
    if ((*requestCodeString == 0))
    {
        fprintf(stderr, "Error: Missing request code argument.\n");
        return false;
    }

    if(strcmp("get_score", requestCodeString) == 0) {
        *requestCode = SSPT_GET_SCORE;
    }
    else if (strcmp("get_score_ascii", requestCodeString) == 0) {
        *requestCode = SSPT_GET_SCORE_ASCII;
    } else if(strcmp("save_score", requestCodeString) == 0) {
        *requestCode = SSPT_SAVE_SCORE;
    } 
    else 
    {
        fprintf(stderr, "Error: Unrecognized request string %s.\n", requestCodeString);
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

static void printUsage(const char* programName)
{
    fprintf(stderr, "Usage: %s --server_addr <STRING> --server_port <PORT> --request <STRING>"
        " --id <NUMBER> --similarityScore_file <STRING> --limit <NUMBER>\n\n"
        "Arguments:\n\n:"
        "Required:\n"
        "  --server_addr : server IPv4 address.\n"
        "  --server_port : server port.\n"
        "      (must be greater than 0 and maximum value = %hu)\n"
        "  --request : string indicating the operation to be performed:\n"
        "      (Only the arguments listed in each request should be used.)\n"
        "      get_score : get profile identified by --id argument.\n"
        "      get_score_ascii : get similarity score based on ASCII format request.\n"
        "      save_score : send similarityScore. Data is read from file identified by --similarity_score_file.\n"
        "Optional:\n"
        "  --id : id of data to be saved.\n"
        "      (must be greater than 0 and maximum value = %u)\n"
        "  --similarity_score_file : path to existing file with a similarityScore data to be loaded from.\n"
        "  --similarity_type : maximum limit of items to be retrieved.\n"
        "      (must be greater than 0 and maximum value = %u)\n"
        "  --request_ascii_data : ascii format request packet : [{count1,type1,id1[]},{count2,type2,id2[]}].\n"
        "  --save_req_packet : ascii data is converted into binary packet and saved to this file.\n"
        "  --log_level : debug / info / attention / critical .\n",
        programName, (unsigned short)UINT16_MAX, UINT32_MAX, UINT32_MAX);
}

static int getNextInt(const char** str, int* out)
{
    // no more data available
    if (*(*str) == 0)
    {
        return 1;
    }
    *out = atoi(*str);
    // skip the number
    while (*(*str) != 0 && *(*str) != ',')
    {
        (*str)++;
    }
    // skip the following ',' or ' '
    while(*(*str) == ',' || *(*str) == ' ')
    {
        (*str)++;
    }
    return 0;
}

static void* convertASCIIToBinRequest(const char* asciiReq, uint32_t* binSize)
{
    if (asciiReq == NULL || binSize == NULL)
    {
        return NULL;
    }
    // count number of values present in the string
    size_t strLen = strnlen(asciiReq, MAX_STRING_LEN);
    size_t valueCount = 1;
    for (size_t i = 0; i < strLen; i++)
    {
        if (asciiReq[i] == ',')
        {
            valueCount++;
        }
    }
    char* retBuff = malloc((valueCount + 1) * sizeof(int));
    if (retBuff == NULL)
    {
        return NULL;
    }
    int bytesWritten = 0;
    int blocksParsed = 0;
    SSPacketScoreRequest* retPacket = (SSPacketScoreRequest*)retBuff;
    retPacket->header.type = SSPT_GET_SCORE; // we are generating a get score packet
    bytesWritten += sizeof(SSPacketScoreRequest);
    while(1)
    {
        int parseRet;
        SSPacketDataScoreRequestBlock* retPacketReqBlock = (SSPacketDataScoreRequestBlock * )&retBuff[bytesWritten];
        // count, type, id[]
        int count = 0;
        parseRet = getNextInt(&asciiReq, &count);
        if (parseRet != 0)
        {
            break;
        }
        retPacketReqBlock->count = count;

        int idTypes = 0;
        parseRet = getNextInt(&asciiReq, &idTypes);
        if (parseRet != 0)
        {
            break;
        }
        retPacketReqBlock->type = idTypes;

        for (size_t i = 0; i < count; i++)
        {
            int val;
            parseRet = getNextInt(&asciiReq, &val);
            if (parseRet != 0)
            {
                fprintf(stderr, "Error: Too early end of ascii data parsing\n");
                free(retBuff);
                return NULL;
            }
            retPacketReqBlock->id[i] = val;
        }
        blocksParsed++;
        bytesWritten += sizeof(SSPacketDataScoreRequestBlock) + sizeof(retPacketReqBlock->id[0]) * retPacketReqBlock->count;

        // end of data to be parsed ?
        if (*asciiReq == 0)
        {
            break;
        }
    }
    retPacket->header.size = bytesWritten - sizeof(SSPacketHeader);

    *binSize = bytesWritten;

    return retBuff;
}

static bool parseLogLevelArgument(const char* argValueStr)
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

    for (size_t i = 0; i < NUM_LOG_MSG_LEVELS; ++i)
    {
        if (strcmp(logLevelArguments[i].argument, argValueStr) == 0)
        {
            logger_setLogLevel( logLevelArguments[i].level );
            return true;
        }
    }

    fprintf(stderr, "Error: invalid value \"%s\" for log_level.\n", argValueStr);

    return false;
}