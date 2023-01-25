#include <fs_client.h>
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
// Types
//

//
// Local prototypes
//
static int writeToSocket(const int sockt, const void *buffer, const size_t bufferSize)
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

        return ERR_FS_SOCKET_WRITE_INTERRUPTED;
    }

    return ERR_FS_NO_ERROR;
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
        return ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES;
    }

    return ERR_FS_NO_ERROR;
}

static int readPacketFromSocket(int sockt, void** retBuf, uint32_t* retSize)
{
    FSPacketHeader header;
    int readError = readFromSocket(sockt, &header, sizeof(header));
    if (readError != ERR_FS_NO_ERROR)
    {
        return readError;
    }
    if (header.size >= PACKET_MAX_SIZE)
    {
        return ERR_FS_PACKET_SIZE_TOO_LARGE;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Reading packet size %zu type %u\n",
        sizeof(FSPacketHeader) + header.size, header.type);

    uint8_t* fullPacket = (uint8_t *)malloc(sizeof(FSPacketHeader) + header.size);
    if (fullPacket == NULL)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Failed to allocate %zu bytes\n",
            sizeof(FSPacketHeader) + header.size);
        LOG_DEFAULT_APP_ERROR(ERR_FS_MEMORY_ALLOC_FAILED);
        return ERR_FS_MEMORY_ALLOC_FAILED;
    }

    memcpy(fullPacket, &header, sizeof(header));

    *retSize = sizeof(FSPacketHeader) + header.size;
    *retBuf = fullPacket;
    if (header.size > 0)
    {
        readError = readFromSocket(sockt, &fullPacket[sizeof(FSPacketHeader)], header.size);
    }
    return readError;
}  

//
// External interface
//

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

int endConnectionToServer(int sockt, uint8_t* responseCode)
{
    FSPacketEndConnection sendPacket;
    sendPacket.header.size = sizeof(sendPacket.data);
    sendPacket.header.type = FSPT_END_CONNECTION;
    int ret = writeToSocket(sockt, &sendPacket, sizeof(sendPacket));

    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 1;
    }

    FSPacketResponseCode recvPacket;
    ret = readFromSocket(sockt, &recvPacket, sizeof(recvPacket));
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 2;
    }

    *responseCode = recvPacket.data.responseCode;

    close(sockt);

    return 0;
}

int getFileFromServer(int sockt, uint8_t entryType, uint32_t entryId, uint8_t* responseCode, FSPacketSaveFile** recvPacket,
    uint32_t* receivedfileSize)
{
    FSPacketGetFile pgf;
    pgf.header.size = sizeof(FSPacketDataGet);
    pgf.header.type = FSPT_GET_FILE;
    pgf.data.type = entryType;
    pgf.data.id = entryId;
    int ret = writeToSocket(sockt, &pgf, sizeof(pgf));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendJsonRequest() returned %d.",ret);
        return 2;
    }

    ret = readPacketFromSocket(sockt, (void** )recvPacket, receivedfileSize);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: readFromSocket() returned %d.", ret);
        close(sockt);
        return 2;
    }

    return 0;
}

int sendFileToServer(int sockt, uint8_t entryType, uint32_t entryId, const char* file, uint32_t fileSize,
    uint8_t* responseCode)
{
    if((fileSize == 0) || (fileSize > PACKET_MAX_SIZE))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileSize == %lu.", fileSize);
        return 1;
    }

    uint32_t totalNetworkDataSize = sizeof(FSPacketSaveFile)+fileSize;
    FSPacketSaveFile *psf = (FSPacketSaveFile*)malloc(totalNetworkDataSize);
    psf->header.size = fileSize + sizeof(FSPacketDataSave);
    psf->header.type = FSPT_SAVE_FILE;
    psf->data.type = entryType;
    psf->data.id = entryId;
    memcpy(&psf->data.fileData, file, fileSize);
    int ret = writeToSocket(sockt, psf, totalNetworkDataSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendJsonRequest() returned %d.",ret);
        free(psf);
        return 2;
    }

    FSPacketResponseCode recvPacket;
    ret = readFromSocket(sockt, &recvPacket, sizeof(recvPacket));
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: readFromSocket() returned %d.", ret);
        close(sockt);
        free(psf);
        return 2;
    }

    *responseCode = recvPacket.data.responseCode; 
    free(psf);

    return 0;
}

int pingServer(int sockt, uint8_t* responseCode)
{
    FSPacketPing sendPacket;
    sendPacket.header.size = sizeof(sendPacket.data);
    sendPacket.header.type = FSPT_PING;
    int ret = writeToSocket(sockt, &sendPacket, sizeof(FSPacketHeader));

    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeToSocket() returned %d.", ret);
        close(sockt);
        return 1;
    }

    FSPacketResponseCode recvPacket;
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

