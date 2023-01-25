#include <request_processing.h>
#include <daos.h>
#include <request_response_definitions.h>
#include <logger.h>
#include <app_errors.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <profiling.h>
#include <string.h>
#include <stdlib.h>

//
// Types
//

typedef struct
{
    struct timeval original;
    struct timeval remaining;
} Timeout_t;

// Request processing

static int processGetRequest(FileServer_t *serverData, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processSaveRequest(FileServer_t *serverData, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);

static int processEndConnectionRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection);
static int processPingRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection);

static int sendSimpleResponse(int sockt, Timeout_t* timeout, uint8_t responseCode);

// Socket

static int sendToSocket(int sockt, Timeout_t* timeout, const void* buffer, ssize_t bufferSize);
static int receiveFromSocket(int sockt, Timeout_t* timeout, void* buffer, ssize_t bufferSize);

//
// Variables
//

//
// External interface
//

int processRequest(FileServer_t *serverData,
    int sockt, uint32_t requestArrivalTimeout,
    uint32_t connectionTimeout, bool* closeConnection)
{
    *closeConnection = false;

    Timeout_t workingReqArrivalTimeout = {
        {.tv_sec = requestArrivalTimeout / 1000,.tv_usec = (requestArrivalTimeout % 1000) * 1000 },
        {.tv_sec = requestArrivalTimeout / 1000,.tv_usec = (requestArrivalTimeout % 1000) * 1000 }
    };

    // Process request size.

    FSPacketHeader ph;

    StartInlinedProfiling(PE_SOCKET_READ_REQUEST);
    int ret = receiveFromSocket(sockt, &workingReqArrivalTimeout, &ph,
        sizeof(ph));

    // do not consider a bug : connections closed by client
    if (ret == ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES || ret == ERR_FS_SOCKET_READ_INTERRUPTED)
    {
        *closeConnection = true;
        return ERR_FS_NO_ERROR;
    }
    else if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveFromSocket(request size, size = %zu) "
            "returned %d.", sizeof(ph), ret);
        *closeConnection = true;
        return ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Request size = %u bytes , type (%hu) = \"%s\".", ph.size, (unsigned short)ph.type,
        ph.type < FSPT_MAX_REQUEST_CODE ? g_packet_type_str[ph.type] : "");

    if (ph.size > PACKET_MAX_SIZE)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: request size (%u bytes) is greater than "
            "maximum (%u bytes).", ph.size, PACKET_MAX_SIZE);
        *closeConnection = true;
        return ERR_FS_PACKET_SIZE_TOO_LARGE;
    }

    if ((ph.type == FSPT_NOT_USED_PACKET_TYPE) || (ph.type > FSPT_MAX_REQUEST_CODE))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unrecognized request code (%x).", ph.type);
        *closeConnection = true;
        return ERR_FS_NON_HANDLED_PACKET_TYPE;
    }

    Timeout_t workingConnTimeout = {
        {.tv_sec = connectionTimeout / 1000, .tv_usec = (connectionTimeout % 1000) * 1000 },
        {.tv_sec = connectionTimeout / 1000, .tv_usec = (connectionTimeout % 1000) * 1000 }
    };

    uint8_t* requestBuffer = NULL;
    if (ph.size)
    {
        requestBuffer = malloc(ph.size + sizeof(ph));
        if (requestBuffer == NULL)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to allocate %d bytes.", ph.size);
            *closeConnection = true;
            return ERR_FS_MEMORY_ALLOC_FAILED;
        }

        // Receive request body.

        ret = receiveFromSocket(sockt, &workingConnTimeout, &requestBuffer[sizeof(ph)], ph.size);
        if (ret != ERR_FS_NO_ERROR)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveFromSocket(request body, size = %lu bytes) "
                "returned %d.", ph.size, ret);
            free(requestBuffer);
            *closeConnection = true;
            return ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES;
        }

        // add header to the full packet
        memcpy(requestBuffer, &ph, sizeof(ph));
    }

    // If we received all data required for this packet, tell client imediatly that we are ok getting next packet
    int quickAck = 1; // only good for sparsely sent packets. Needs to be set after receive
    if (setsockopt(sockt, IPPROTO_TCP, TCP_QUICKACK, &quickAck, sizeof(quickAck)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_TCP, TCP_QUICKACK) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_TIMEOUT_FAILED;
    }

    EndInlinedProfiling(PE_SOCKET_READ_REQUEST);

    // Process request.

    switch (ph.type)
    {
    case FSPT_GET_FILE:
    {
        StartInlinedProfiling(PE_GET);
        ret = processGetRequest(serverData,
            sockt, &workingConnTimeout, requestBuffer,
            ph.size, closeConnection);
//        *closeConnection = true;
        EndInlinedProfiling(PE_GET);
    }break;

    case FSPT_SAVE_FILE:
    {
        StartInlinedProfiling(PE_SAVE);
        ret = processSaveRequest(serverData,
            sockt, &workingConnTimeout, requestBuffer, ph.size,
            closeConnection);
//        *closeConnection = true;
        EndInlinedProfiling(PE_SAVE);
    }break;

    case FSPT_END_CONNECTION:
        ret = processEndConnectionRequest(sockt, &workingConnTimeout, ph.size, closeConnection);
        break;

    case FSPT_PING:
        ret = processPingRequest(sockt, &workingConnTimeout, ph.size, closeConnection);
        break;

    default:
        ret = ERR_FS_NON_HANDLED_PACKET_TYPE;
        break;
    }

    free(requestBuffer);

#if defined(PRINT_PROFILING_EVERY_X_REQUEST) && PRINT_PROFILING_EVERY_X_REQUEST > 0
    static size_t requestCounter = 0;
    requestCounter++;
    if ((requestCounter % 10000) == 0)
        PrintProfilingStatus();
#endif

    return ret;
}

//
// Request processing
//

static int processGetRequest(FileServer_t *serverData,
    int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    FSPacketGetFile* pgf = (FSPacketGetFile*)buffer;
	if(pgf == NULL || pgf->header.size < sizeof(pgf->data))
	{
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%zd) < %zu.", bufferSize, sizeof(pgf->data));
        sendSimpleResponse(sockt, timeout, ERR_FS_MALFORMED_GET_FILE_PACKET);
        return ERR_FS_MALFORMED_GET_FILE_PACKET;
	}

    FileGroupData* fgd = getCreateFileGroupDao(pgf->data.type, serverData);
    if (fgd == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: DAO init failed.");
        sendSimpleResponse(sockt, timeout, ERR_FS_DAO_INIT_FAILED);
        return ERR_FS_DAO_INIT_FAILED;
    }

    // Retrieve item.
    void* DBBuffer = NULL;
    FSPacketSaveFile* fsb = NULL;
    uint32_t fsbSize = 0;

    uint32_t fileId = pgf->data.id / fgd->numItemsPerFile;

    fileLockList_lockFileForRead(fgd->fileList, fileId);

    int ret = daos_getItem(fgd->Daos, pgf->data.id, &DBBuffer, (void**)&fsb, &fsbSize);

    fileLockList_releaseFile(fgd->fileList, fileId);

    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_getItem() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERR_FS_ENTRY_NOT_FOUND);
        free(DBBuffer);
        return ERR_FS_ENTRY_NOT_FOUND;
    }

    fsb->header.type = FSPT_GET_FILE;
    ret = sendToSocket(sockt, timeout, fsb, fsb->header.size + sizeof(FSPacketHeader));
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) "
            "returned %d.", fsb->header.size + sizeof(FSPacketHeader), ret);
        free(DBBuffer);
        return ERR_FS_SOCKET_WRITE_INTERRUPTED;
    }
    free(DBBuffer);

    return ERR_FS_NO_ERROR;
}

static int processSaveRequest(FileServer_t *serverData,
    int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    FSPacketSaveFile* fs = (FSPacketSaveFile*)buffer;
	if(fs == NULL || fs->header.size < sizeof(fs->data) + 1)
	{
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%zd) < %zu.", bufferSize, sizeof(fs->data) + 1);
        sendSimpleResponse(sockt, timeout, ERR_FS_MALFORMED_SAVE_FILE_PACKET);
        return ERR_FS_MALFORMED_SAVE_FILE_PACKET;
	}

    FileGroupData* fgd = getCreateFileGroupDao(fs->data.type, serverData);
    if (fgd == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: DAO init failed.");
        sendSimpleResponse(sockt, timeout, ERR_FS_DAO_INIT_FAILED);
        return ERR_FS_DAO_INIT_FAILED;
    }
    // Save item.

    uint32_t fileId = fs->data.id / fgd->numItemsPerFile;
    fileLockList_lockFileForWrite(fgd->fileList, fileId);

    int ret = daos_saveItem(fgd->Daos, fs, fs->header.size + sizeof(fs->header));

    fileLockList_releaseFile(fgd->fileList, fileId);

    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_saveItem() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERR_FS_ENTRY_SAVE_FAILED);
        return ERR_FS_ENTRY_SAVE_FAILED;
    }

    // Send response.
    sendSimpleResponse(sockt, timeout, ERR_FS_NO_ERROR);

    return ERR_FS_NO_ERROR;
}

static int processEndConnectionRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection)
{
    *closeConnection = true;

    // Process buffer size.
    if (bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%zd) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERR_FS_MALFORMED_END_CONNECTION_PACKET);
        return ERR_FS_MALFORMED_END_CONNECTION_PACKET;
    }

    // Send response.
    int errResp = sendSimpleResponse(sockt, timeout, ERR_FS_NO_ERROR);

    return errResp;
}

static int processPingRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    // Process buffer size.
    if (bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%zd) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERR_FS_MALFORMED_PING_PACKET);
        return ERR_FS_MALFORMED_PING_PACKET;
    }

    // Send response.
    int errReq = sendSimpleResponse(sockt, timeout, ERR_FS_NO_ERROR);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return errReq;
}

static int sendSimpleResponse(int sockt, Timeout_t* timeout, uint8_t responseCode)
{
    FSPacketResponseCode *sendPacket = NULL;
    size_t codeLen = 1; 
    const char* responseStr = NULL;
    if (responseCode < ERR_FS_MAX_KNOWN && g_err_code_str[responseCode] != NULL)
    {
        codeLen = strnlen(g_err_code_str[responseCode], MAX_STRING_LEN) + 1;
        responseStr = g_err_code_str[responseCode];
    }
    size_t bytesToSend = sizeof(FSPacketResponseCode) + codeLen;
    sendPacket = malloc(bytesToSend);
    sendPacket->header.size = bytesToSend - sizeof(sendPacket->header);
    sendPacket->header.type = FSPT_RESPONSE_CODE;
    sendPacket->data.responseCode = responseCode;
    if (responseStr == NULL)
    {
        sendPacket->data.responseStr[0] = 0;
    }
    else
    {
        memcpy(sendPacket->data.responseStr, responseStr, codeLen);
    }

    int ret = sendToSocket(sockt, timeout, sendPacket, bytesToSend);
    free(sendPacket);

    return ret;
}

//
// Socket
//

static int sendToSocket(int sockt, Timeout_t* timeout, const void* buffer, ssize_t bufferSize)
{
    const uint8_t* sendBuffer = (const uint8_t*)buffer;

    fd_set masterSet;

    FD_ZERO(&masterSet);
    FD_SET(sockt, &masterSet);
    int maxSd = sockt;

    ssize_t numBytesToSend = bufferSize;

    while (numBytesToSend > 0)
    {
        struct timeval sendTimeout = timeout->remaining;
        fd_set workingSet = masterSet;

        int sd = select(maxSd + 1, NULL, &workingSet, NULL, &timeout->remaining);
        if (sd > 0)
        {
            if (FD_ISSET(sockt, &workingSet))
            {
                ssize_t numBytesWritten = write(sockt, sendBuffer, numBytesToSend);
                if (numBytesWritten < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - "
                            "write(block = %zd bytes) failed. errno = %d (\"%s\").",
                            bufferSize, numBytesToSend, errno, strerror(errno));
                        return ERR_FS_SOCKET_WRITE_FAILED;
                    }
                }
                else if (numBytesWritten == 0)
                {
                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: connection closed while writing. buffer size "
                        "(%zd bytes) - write(block = %zd bytes) failed.", bufferSize, numBytesWritten);
                    return ERR_FS_SOCKET_WRITE_INTERRUPTED;
                }
                else
                {
                    sendBuffer += numBytesWritten;
                    numBytesToSend -= numBytesWritten;
                }
            }
        }
        else if (sd == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select() timeout "
                "(remaining limit = %ds%dms, full exchange limit = %ds%dms).", bufferSize,
                sendTimeout.tv_sec, sendTimeout.tv_usec / 1000,
                timeout->original.tv_sec, timeout->original.tv_usec / 1000);
            return ERR_FS_SOCKET_TIMEOUT;
        }
        else // sd < 0
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select() failed."
                " errno = %d (\"%s\").", bufferSize, errno, strerror(errno));
            return ERR_FS_SOCKET_SELECT_FAILED;
        }
    }

    return ERR_FS_NO_ERROR;
}

static int receiveFromSocket(int sockt, Timeout_t* timeout, void* buffer, ssize_t bufferSize)
{
    uint8_t* receiveBuffer = (uint8_t*)buffer;

    fd_set masterSet;

    FD_ZERO(&masterSet);
    FD_SET(sockt, &masterSet);
    int maxSd = sockt;

    ssize_t numBytesToRead = bufferSize;

    while (numBytesToRead > 0)
    {
        struct timeval receiveTimeout = timeout->remaining;
        fd_set workingSet = masterSet;

        int sd = select(maxSd + 1, &workingSet, NULL, NULL, &timeout->remaining);
        if (sd > 0)
        {
            if (FD_ISSET(sockt, &workingSet))
            {
                ssize_t numBytesRead = read(sockt, receiveBuffer, numBytesToRead);
                if (numBytesRead < 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
//                        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) -"
//                            " read(block = %zd bytes) failed. errno = %d (\"%s\").",
//                            bufferSize, numBytesToRead, errno, strerror(errno));
                        return ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES;
                    }
                }
                else if (numBytesRead == 0)
                {
//                    LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: connection closed while reading. buffer size "
//                        "(%zd bytes) - read(block = %zd bytes) failed.", bufferSize, numBytesToRead);
                    return ERR_FS_SOCKET_READ_INTERRUPTED;
                }
                else
                {
                    receiveBuffer += numBytesRead;
                    numBytesToRead -= numBytesRead;
                }
            }
        }
        else if (sd == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select()"
                " timeout (remaining limit = %ds%dms, full exchange limit = %ds%dms).",
                bufferSize, receiveTimeout.tv_sec, receiveTimeout.tv_usec / 1000,
                timeout->original.tv_sec, timeout->original.tv_usec / 1000);
            return ERR_FS_SOCKET_TIMEOUT;
        }
        else // sd < 0
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: buffer size (%zd bytes) - select() failed."
                " errno = %d (\"%s\").", bufferSize, errno, strerror(errno));
            return ERR_FS_SOCKET_SELECT_FAILED;
        }
    }

    return ERR_FS_NO_ERROR;
}
