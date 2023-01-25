#include <ss_client.h>
#include <common/request_response_definitions.h>
#include <logger.h>
#include <app_errors.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

//
// Local prototypes
//

//
// External interface
//

int writeToSocket(const int sockt, const void *buffer, const size_t bufferSize)
{
    int numBytesWritten = write(sockt, buffer, bufferSize);
    if (numBytesWritten != bufferSize)
    {
        if (numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(buffer) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(buffer) failed. numBytesRequested"
                " (%lu) != numBytesWritten (%zd).", bufferSize, numBytesWritten);
        }

        return ERR_SS_SOCKET_WRITE_INTERRUPTED;
    }

    return ERR_SS_NO_ERROR;
}


static int readFromSocket(int sockt, void* retBuf, size_t size)
{
    uint8_t* buff = (uint8_t *)retBuf;
    ssize_t numBytesReadTotal = 0;
    while (numBytesReadTotal < size)
    {
        ssize_t numBytesRead = read(sockt, &buff[numBytesReadTotal], size - numBytesReadTotal);
        if (numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. errno = %d"
                " (\"%s\").", errno, strerror(errno));
            break;
        }
        if (numBytesRead == 0)
        {
            break;
        }
        numBytesReadTotal += numBytesRead;
    }
    if (numBytesReadTotal != size)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. numBytesRequested"
            " (%zd) != numBytesRead (%zd).", size, numBytesReadTotal);
        return ERR_SS_SOCKET_READ_NOT_ENOUGH_BYTES;
    }

    return ERR_SS_NO_ERROR;
}

static int readPacketFromSocket(int sockt, void** retBuf, uint32_t* retSize)
{
    SSPacketHeader header;
    int readError = readFromSocket(sockt, &header, sizeof(header));
    if (readError != ERR_SS_NO_ERROR)
    {
        return readError;
    }
    if (header.size >= PACKET_MAX_SIZE)
    {
        return ERR_SS_PACKET_SIZE_TOO_LARGE;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Reading packet size %zu type %u\n",
        sizeof(SSPacketHeader) + header.size, header.type);

    uint8_t* fullPacket = (uint8_t *)malloc(sizeof(SSPacketHeader) + header.size);
    if (fullPacket == NULL)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Failed to allocate %zu bytes\n",
            sizeof(SSPacketHeader) + header.size);
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }

    memcpy(fullPacket, &header, sizeof(header));

    *retSize = sizeof(SSPacketHeader) + header.size;
    *retBuf = fullPacket;
    if (header.size > 0)
    {
        readError = readFromSocket(sockt, &fullPacket[sizeof(SSPacketHeader)], header.size);
    }
    return readError;
}

int connectToServer(const char* address, uint16_t port, int* sockt)
{
    int auxSockt = socket(AF_INET, SOCK_STREAM, 0);
    if(auxSockt < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: socket() failed. errno = %d " "(\"%s\").",
            errno, strerror(errno));
        return 1;
    }

    struct sockaddr_in sockAddr = { 0 };

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(address);
    sockAddr.sin_port = htons(port);

    if(connect(auxSockt, (struct sockaddr*) &sockAddr, sizeof(sockAddr)) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: connect() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
        close(auxSockt);
        return 2;
    }

    *sockt = auxSockt;

    return 0;
}

int endConnectionToServer(int sockt, uint32_t* responseCode)
{
    SSPacketEndConnection sendPacket;
    sendPacket.header.size = sizeof(sendPacket.data);
    sendPacket.header.type = SSPT_END_CONNECTION;
    int ret = writeToSocket(sockt, &sendPacket, sizeof(sendPacket));

    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 1;
    }

    SSPacketResponseCode recvPacket;
    ret = readFromSocket(sockt, &recvPacket, sizeof(recvPacket));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 2;
    }

    *responseCode = recvPacket.data.responseCode;

    close(sockt);

    return 0;
}

int getScoresFromServer(int sockt, void* requestPacket,
    uint32_t requestPacketSize, void** responsePacket, uint32_t* responsePacketSize)
{
    int ret = writeToSocket(sockt, requestPacket, requestPacketSize);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 1;
    }

    ret = readPacketFromSocket(sockt, responsePacket, responsePacketSize);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: readFromSocket() returned %d.", ret);
        close(sockt);
        return 2;
    }

    return 0;
}

int sendScoreFileToServer(int sockt, uint32_t scoreType, uint32_t scoreId, const char* scoreFile, uint32_t scoreFileSize,
    uint32_t* responseCode)
{
    size_t bytesNeeded = sizeof(SSPacketSendScore) + scoreFileSize;
    char* sendPacketBuffer = (char*)malloc(bytesNeeded);
    if (sendPacketBuffer == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to allocate %zu bytes.", bytesNeeded);
        close(sockt);
        return 1;
    }

    SSPacketSendScore *sendPacket;
    sendPacket = (SSPacketSendScore*)sendPacketBuffer;

    sendPacket->header.size = sizeof(sendPacket->data) + scoreFileSize;
    sendPacket->header.type = SSPT_SAVE_SCORE;

    sendPacket->data.type = scoreType;
    sendPacket->data.id = scoreId;
    memcpy(sendPacket->data.fileData, scoreFile, scoreFileSize);

    int ret = writeToSocket(sockt, sendPacket, bytesNeeded);
    free(sendPacketBuffer);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 1;
    }

    SSPacketResponseCode recvPacket;
    ret = readFromSocket(sockt, &recvPacket, sizeof(recvPacket));
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: readFromSocket() returned %d.", ret);
        close(sockt);
        return 2;
    }

    *responseCode = recvPacket.data.responseCode;

    return 0;
}

//
// Local functions
//

//
// Get / Save operations
//

int pingServer(int sockt, uint32_t* responseCode)
{
    SSPacketPing sendPacket;
    sendPacket.header.size = sizeof(sendPacket.data);
    sendPacket.header.type = SSPT_PING;
    int ret = writeToSocket(sockt, &sendPacket, sizeof(SSPacketHeader));

    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 1;
    }

    SSPacketResponseCode recvPacket;
    ret = readFromSocket(sockt, &recvPacket, sizeof(recvPacket));
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: readFromSocket() returned %d.", ret);
        close(sockt);
        return 2;
    }

    *responseCode = recvPacket.data.responseCode;

    close(sockt);

    return 0;
}
