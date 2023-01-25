#include <request_processing.h>
#include <request_response_definitions.h>
#include <utils.h>
#include <score_manager.h>
#include <score_file.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <profiling.h>
#include <logger.h>
#include <string.h>
#include <app_errors.h>

//
// Types
//

typedef struct
{
    struct timeval original;
    struct timeval remaining;
} Timeout_t;


//
// Local prototypes
//

// Request processing

static int processGetRequest(FileLockList_t fileLockList, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection, const char** SimilarityPaths);

static int processSaveRequest(FileLockList_t fileLockList, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection, const char** SimilarityPaths);

static int processEndConnectionRequest(int sockt, Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processPingRequest(int sockt, Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);

static int sendSimpleResponse(int sockt, Timeout_t* timeout, uint32_t responseCode);

// Socket

static int sendToSocket(int sockt, Timeout_t* timeout, const void* buffer, ssize_t bufferSize);
static int receiveFromSocket(int sockt, Timeout_t* timeout, void* buffer, ssize_t bufferSize);

//
// External interface
//

int processRequest(FileLockList_t fileLockList, int sockt, uint32_t requestArrivalTimeout,
    uint32_t connectionTimeout, const char** SimilarityPaths, bool* closeConnection)
{
    *closeConnection = false;

    Timeout_t workingReqArrivalTimeout = {
        { .tv_sec = requestArrivalTimeout / 1000,.tv_usec = (requestArrivalTimeout % 1000) * 1000 },
        { .tv_sec = requestArrivalTimeout / 1000,.tv_usec = (requestArrivalTimeout % 1000) * 1000 }
    };

    // Process request size.

    SSPacketHeader packetHeader;

    int ret = receiveFromSocket(sockt, &workingReqArrivalTimeout, &packetHeader,
        sizeof(packetHeader));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveFromSocket(request size, size = %zu) "
            "returned %d.", sizeof(packetHeader), ret);
        *closeConnection = true;
        return ret;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Request data block size = %u bytes.", packetHeader.size);

    if(packetHeader.size > PACKET_MAX_SIZE)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: request size (%u bytes) is greater than "
            "maximum (%u bytes).", packetHeader.size, PACKET_MAX_SIZE);
        *closeConnection = true;
        return ERR_SS_PACKET_SIZE_TOO_LARGE;
    }

    uint8_t *requestBuffer = malloc(packetHeader.size);
    if (requestBuffer == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to allocate %d bytes.", packetHeader.size);
        *closeConnection = true;
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }

    // Receive request body.

    Timeout_t workingConnTimeout = {
        { .tv_sec = connectionTimeout / 1000, .tv_usec = (connectionTimeout % 1000) * 1000 },
        { .tv_sec = connectionTimeout / 1000, .tv_usec = (connectionTimeout % 1000) * 1000 }
    };

    ret = receiveFromSocket(sockt, &workingConnTimeout, requestBuffer, packetHeader.size);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveFromSocket(request body, size = %lu bytes) "
            "returned %d.", packetHeader.size, ret);
        free(requestBuffer);
        *closeConnection = true;
        return ERR_SS_SOCKET_READ_NOT_ENOUGH_BYTES;
    }

    // Process request.

    if((packetHeader.type == SSPT_NOT_USED_PACKET_TYPE) || (packetHeader.type >= SSPT_MAX_REQUEST_CODE))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unrecognized request code (%x).", packetHeader.type);
        free(requestBuffer);
        *closeConnection = true;
        return ERR_SS_UNKNOWN_PACKET_TYPE;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Request (%hu) \"%s\".", (unsigned short) packetHeader.type,
        g_packet_type_str[packetHeader.type]);

    const char* functionCalled = NULL;

    switch(packetHeader.type)
    {
    case SSPT_GET_SCORE:
        functionCalled = "processGetRequest()";
        StartInlinedProfiling(PE_GET_SIMILARITY_SCORE);
        ret = processGetRequest(fileLockList, sockt, &workingConnTimeout, requestBuffer, packetHeader.size, closeConnection, SimilarityPaths);
        EndInlinedProfiling(PE_GET_SIMILARITY_SCORE);

        PrintProfilingStatus();

        *closeConnection = true;
        break;

    case SSPT_SAVE_SCORE:
        functionCalled = "processSaveRequest()";
        StartInlinedProfiling(PE_SAVE_SIMILARITY_SCORE);
        ret = processSaveRequest(fileLockList, sockt, &workingConnTimeout, requestBuffer, packetHeader.size, closeConnection, SimilarityPaths);
        EndInlinedProfiling(PE_SAVE_SIMILARITY_SCORE);
        *closeConnection = true;
        break;

    case SSPT_END_CONNECTION:
        functionCalled = "processEndConnectionRequest()";
        ret = processEndConnectionRequest(sockt, &workingConnTimeout, requestBuffer, packetHeader.size, closeConnection);
        break;

    case SSPT_PING:
        functionCalled = "processPingRequest()";
        ret = processPingRequest(sockt, &workingConnTimeout, requestBuffer, packetHeader.size, closeConnection);
        break;
        
    default:
        functionCalled = "Invalid request function";
        ret = ERR_SS_NON_HANDLED_PACKET_TYPE;
        *closeConnection = true;
        break;
    }

    free(requestBuffer);
	
    if(ret == ERR_SS_NO_ERROR)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Succeeded.");
        return ERR_SS_NO_ERROR;
    }
    else
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: %s returned %d.", functionCalled, ret);
        return ret;
    }
}

//
// Local functions
//

//
// Request processing
//

static int processGetRequest(FileLockList_t fileLockList, int sockt, Timeout_t* timeout, const uint8_t* buffer, 
    uint32_t bufferSize, bool* closeConnection, const char** SimilarityPaths)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    uint32_t outpacketAllocated = sizeof(SSPacketScoreReply) + BUFFER_EXTEND_MIN_SIZE;
    SSPacketScoreReply* sendPacket = (SSPacketScoreReply *)malloc(outpacketAllocated);
    if (sendPacket == NULL)
    {
		LOG_MESSAGE(CRITICAL_LOG_MSG, "Failed to allocate %zu bytes memory.", outpacketAllocated);
        SSPacketScoreReply stackPacket;
        stackPacket.header.type = SSPT_REPLY_SCORE;
        stackPacket.header.size = 0;

        sendToSocket(sockt, timeout, &stackPacket, sizeof(stackPacket));
        LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }
    else
    {
        uint32_t outPacketWritten = sizeof(SSPacketHeader);
        generateScorePacket(fileLockList, buffer, bufferSize, SimilarityPaths, (uint8_t**)&sendPacket, &outPacketWritten, &outpacketAllocated);

        sendPacket->header.type = SSPT_REPLY_SCORE;
        sendPacket->header.size = outPacketWritten - sizeof(SSPacketHeader);

        int errSend = sendToSocket(sockt, timeout, sendPacket, outPacketWritten);
        free(sendPacket);
        LOG_MESSAGE(DEBUG_LOG_MSG, "End. Reply byte count %zd", outPacketWritten);
        return errSend;
    }
}

static int processSaveRequest(FileLockList_t fileLockList, int sockt, Timeout_t* timeout, const uint8_t* buffer, 
    uint32_t bufferSize, bool* closeConnection, const char** SimilarityPaths)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    SSPacketDataSendScore* scorePacket = (SSPacketDataSendScore*)buffer;

    // check if value is within range
    if (scorePacket->type == SSFT_NOT_USED_UNINITIALIZED_VALUE || scorePacket->type >= SSFT_MAX_SCORE_FILE_TYPE)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Received unknown similarity score type %d.", scorePacket->type);
        LOG_DEFAULT_APP_ERROR(ERR_SS_UNKNOWN_SCORE_TYPE);
        sendSimpleResponse(sockt, timeout, ERR_SS_UNKNOWN_SCORE_TYPE);
        return ERR_SS_UNKNOWN_SCORE_TYPE;
    }

    fileLockList_lockFileForWrite(fileLockList, scorePacket->id);
    int saveResult = saveScoreFile(scorePacket->type, SimilarityPaths[scorePacket->type], scorePacket->id, scorePacket->fileData, bufferSize - sizeof(SSPacketDataSendScore));
    fileLockList_releaseFile(fileLockList, scorePacket->id);

    // sanity check. Check if the file size is the same as the buffer received
    if (saveResult != ERR_SS_NO_ERROR)
    {
        sendSimpleResponse(sockt, timeout, saveResult);
        return saveResult;
    }

    // Send response.
    int errResp = sendSimpleResponse(sockt, timeout, ERR_SS_NO_ERROR);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return errResp;
}

static int processEndConnectionRequest(int sockt, Timeout_t* timeout, const uint8_t* buffer, 
    uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    *closeConnection = true;

    // Process buffer size.
    if(bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%zd) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERR_SS_MALFORMED_END_CONNECTION_PACKET);
        return 1;
    }

    // Send response.
    int errResp = sendSimpleResponse(sockt, timeout, ERR_SS_NO_ERROR);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return errResp;
}

static int processPingRequest(int sockt, Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process buffer size.
    if(bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%zd) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERR_SS_MALFORMED_PING_PACKET);
        return 1;
    }

    // Send response.
    int errReq = sendSimpleResponse(sockt, timeout, ERR_SS_NO_ERROR);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return errReq;
}

static int sendSimpleResponse(int sockt, Timeout_t* timeout, uint32_t responseCode)
{
    SSPacketResponseCode sendPacket;
    sendPacket.header.size = sizeof(sendPacket.data);
    sendPacket.header.type = SSPT_RESPONSE_CODE;
    sendPacket.data.responseCode = responseCode;

    return sendToSocket(sockt, timeout, &sendPacket, sizeof(sendPacket));
}

//
// Socket
//

static int sendToSocket(int sockt, Timeout_t* timeout, const void* buffer, ssize_t bufferSize)
{
    const uint8_t* sendBuffer = (const uint8_t*) buffer;

    fd_set masterSet;

    FD_ZERO(&masterSet);
    FD_SET(sockt, &masterSet);
    int maxSd = sockt;

    ssize_t numBytesToSend = bufferSize;

    while(numBytesToSend > 0)
    {
        struct timeval sendTimeout = timeout->remaining;
        fd_set workingSet = masterSet;

        int sd = select(maxSd + 1, NULL, &workingSet, NULL, &timeout->remaining);
        if(sd > 0)
        {
            if(FD_ISSET(sockt, &workingSet))
            {
                ssize_t numBytesWritten = write(sockt, sendBuffer, numBytesToSend);
                if(numBytesWritten < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - "
                            "write(block = %zd bytes) failed. errno = %d (\"%s\").",
                            bufferSize, numBytesToSend, errno, strerror(errno));
                        return 1;
                    }
                }
                else if(numBytesWritten == 0)
                {
                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: connection closed while writing. buffer size "
                        "(%zd bytes) - write(block = %zd bytes) failed.", bufferSize, numBytesWritten);
                    return 2;
                }
                else
                {
                    sendBuffer += numBytesWritten;
                    numBytesToSend -= numBytesWritten;
                }
            }
        }
        else if(sd == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select() timeout "
                "(remaining limit = %ds%dms, full exchange limit = %ds%dms).", bufferSize,
                sendTimeout.tv_sec, sendTimeout.tv_usec / 1000,
                timeout->original.tv_sec, timeout->original.tv_usec / 1000);
            return 3;
        }
        else // sd < 0
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select() failed."
                " errno = %d (\"%s\").", bufferSize, errno, strerror(errno));
            return 4;
        }
    }

    return ERR_SS_NO_ERROR;
}

static int receiveFromSocket(int sockt, Timeout_t* timeout, void* buffer, ssize_t bufferSize)
{
    uint8_t* receiveBuffer = (uint8_t*) buffer;

    fd_set masterSet;

    FD_ZERO(&masterSet);
    FD_SET(sockt, &masterSet);
    int maxSd = sockt;

    ssize_t numBytesToRead = bufferSize;

    while(numBytesToRead > 0)
    {
        struct timeval receiveTimeout = timeout->remaining;
        fd_set workingSet = masterSet;

        int sd = select(maxSd + 1, &workingSet, NULL, NULL, &timeout->remaining);
        if(sd > 0)
        {
            if(FD_ISSET(sockt, &workingSet))
            {
                ssize_t numBytesRead = read(sockt, receiveBuffer, numBytesToRead);
                if(numBytesRead < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) -"
                            " read(block = %zd bytes) failed. errno = %d (\"%s\").",
                            bufferSize, numBytesToRead, errno, strerror(errno));
                        return 1;
                    }
                }
                else if(numBytesRead == 0)
                {
                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: connection closed while reading. buffer size "
                        "(%zd bytes) - read(block = %zd bytes) failed.", bufferSize, numBytesToRead);
                    return 2;
                }
                else
                {
                    receiveBuffer += numBytesRead;
                    numBytesToRead -= numBytesRead;
                }
            }
        }
        else if(sd == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select()"
                " timeout (remaining limit = %ds%dms, full exchange limit = %ds%dms).",
                bufferSize, receiveTimeout.tv_sec, receiveTimeout.tv_usec / 1000,
                timeout->original.tv_sec, timeout->original.tv_usec / 1000);
            return 3;
        }
        else // sd < 0
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select() failed."
                " errno = %d (\"%s\").", bufferSize, errno, strerror(errno));
            return 4;
        }
    }

    return ERR_SS_NO_ERROR;
}
