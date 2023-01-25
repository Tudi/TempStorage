#include <file_server.h>
#include <request_processing.h>
#include <mt_queue.h>
#include <daos_filestore_functions.h>
#include <common/request_response_definitions.h>
#include <logger.h>
#include <profiling.h>
#include <error_reporting.h>
#include <app_errors.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MAX_DIFFERENT_ENTRY_TYPES   1000 
//
// Types
//

typedef
struct
{
    DaosId_t    serverId;
    DaosCount_t numEntriesPerFile;

    uint32_t requestArrivalTimeout;
    uint32_t connectionTimeout;
    uint16_t numConnections;

    MtQueue_t socketQueue;
} RequestProcessingData_t;

typedef
struct
{
    // Server
    int endEvent;
    int sockt;

    uint16_t   numConnections;
    uint16_t   numRequestThreads;
    pthread_t* requestThreadIds;

    // Request processing
    RequestProcessingData_t reqProcessingData;

    FileGroupData   fileGroupData[MAX_DIFFERENT_ENTRY_TYPES];
    int             fileGroupsUsed;
    pthread_mutex_t fileGroupMutex;

    char* dataDir;
    uint32_t dirDepth;
    uint32_t dirCount;
} ProcessServerData_t;

typedef struct SocketStore
{
    int socket;
    uint64_t stamp_accept;
}SocketStore;

//
// Local prototypes
//

// Server data

static int initProcessServerData(ProcessServerData_t* data, DaosId_t serverId, DaosCount_t numServers,
    DaosCount_t numEntriesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, const char* dataDir, uint32_t dirDepth, uint32_t dirCount);
static void cleanupProcessServerData(ProcessServerData_t* data);

// Request processing

static void zeroRequestProcessingData(RequestProcessingData_t* data);
static int loadRequestProcessingData(RequestProcessingData_t* data, DaosId_t serverId, DaosCount_t numServers,    
    DaosCount_t numEntriesPerFile,
    uint32_t requestArrivalTimeout, uint32_t connectionTimeout, uint16_t numConnections,
    uint16_t numRequestThreads);
static void cleanupRequestProcessingData(RequestProcessingData_t* data);
static void* requestProcessingThread(void* arg);
FileGroupData* getCreateEntryDao(uint32_t fileGroup, FileServer_t* serverData);

// Socket

static int initServerSocket(uint16_t serverPort, uint16_t numConnections, uint32_t timeoutInMs);
static int configureSocketSettings(int sockt, uint32_t timeoutInMs);
static void closeSocket(void* arg);
static void socketQueueThreadCleanupHandler(void* arg);

// Others

static void threadCleanupHandler(void *arg);

//
// External interface
//

FileServer_t fileServer_init()
{
    FileServer_t server = FileServer_NULL;

    ProcessServerData_t* data = (ProcessServerData_t*) malloc(sizeof(ProcessServerData_t));
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: fileServer_init() - malloc(size = "
            "%zu bytes) failed.", sizeof(ProcessServerData_t));
        return server;
    }

    // Initial zero values

    data->endEvent             = -1;
    data->sockt                = -1;
    data->numConnections       = 0;
    data->numRequestThreads    = 0;
    data->requestThreadIds     = NULL;
    data->fileGroupsUsed       = 0;
    data->dataDir              = NULL;
    data->dirDepth             = 0;
    data->dirCount             = 0;
    memset(&data->fileGroupData, 0, sizeof(data->fileGroupData));
    pthread_mutex_init(&data->fileGroupMutex, NULL);

    zeroRequestProcessingData(&data->reqProcessingData);

    server.p = data;

    return server;
}

int fileServer_run(FileServer_t server, DaosId_t serverId, DaosCount_t numServers,
    DaosCount_t numEntriesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, const char* dataDir, uint32_t dirDepth, uint32_t dirCount)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Started...");

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Execution parameters:\n"
        "    server id : \"%lu\"\n"
        "    number of servers : %lu\n"
        "    number of entries per file : %lu\n"
        "    port : %hu\n"
        "    request arrival timeout : %lu ms\n"
        "    connection timeout : %lu ms\n"
        "    number of connections : %hu\n"
        "    data path : %s\n"
        "    dir depth : %hu\n"
        "    dir count : %hu\n",
        serverId, numServers,
        numEntriesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections, dataDir, dirDepth, dirCount);

    signal(SIGPIPE, SIG_IGN);

    ProcessServerData_t* data = (ProcessServerData_t*) server.p;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for server.");
        return ERR_FS_PARAM_SERVERDATA_NULL;
    }

    int ret = initProcessServerData(data, serverId, numServers,
        numEntriesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections, dataDir, dirDepth, dirCount);
    if(ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: initProcessServerData() returned %d.", ret);
        return ret;
    }

    if(listen(data->sockt, data->numConnections) != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: listen(backlogSize = %d) failed. errno = %d (\"%s\").",
            (int) data->numConnections, errno, strerror(errno));
        cleanupProcessServerData(data);
        return ERR_FS_SOCKET_LISTEN_FAILED;
    }

    fd_set masterSet;

    FD_ZERO(&masterSet);
    FD_SET(data->endEvent, &masterSet);
    FD_SET(data->sockt, &masterSet);
    int maxSd = data->endEvent > data->sockt ? data->endEvent : data->sockt;

    LOG_MESSAGE(INFO_LOG_MSG, "Ready to receive and process requests.");

    while(true)
    {
        fd_set workingSet = masterSet;
        int sd = select(maxSd + 1, &workingSet, NULL, NULL, NULL);
        if(sd < 0)
        {
            if(errno == EINTR)
            {
                LOG_MESSAGE(INFO_LOG_MSG, "select() returned EINTR.");
                break;
            }

            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: select() failed. errno = %d (\"%s\").",
                errno, strerror(errno));
            cleanupProcessServerData(data);
            return ERR_FS_SOCKET_SELECT_FAILED;
        }
        else if(sd > 0 && FD_ISSET(data->endEvent, &workingSet))
        {
            uint64_t eventCounter = 0;
            int bytesRead = read(data->endEvent, &eventCounter, sizeof(eventCounter));
            if (bytesRead != sizeof(eventCounter))
            {
                LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: read event failed. errno = %d (\"%s\").",
                    errno, strerror(errno));
                cleanupProcessServerData(data);
                return ERR_FS_SOCKET_READ_INTERRUPTED;
            }
            break;
        }
        else if(sd > 0 && FD_ISSET(data->sockt, &workingSet))
        {
            struct sockaddr_in clientAddr = { 0 };
            socklen_t clientAddrLength = sizeof(clientAddr);

            int clientSocket = accept(data->sockt, (struct sockaddr*) &clientAddr, &clientAddrLength);
            if(clientSocket >= 0)
            {
                pthread_cleanup_push(socketQueueThreadCleanupHandler, &data->reqProcessingData.socketQueue);
                SocketStore* socketStore = malloc(sizeof(SocketStore));
                socketStore->socket = clientSocket;
                socketStore->stamp_accept = GetProfilingStamp(); 
                mtQueue_push(data->reqProcessingData.socketQueue, (void*)socketStore);
                pthread_cleanup_pop(0);
            }
            else if(errno != EWOULDBLOCK)
            {
                LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: accept() failed. errno = %d (\"%s\").",
                    errno, strerror(errno));
                cleanupProcessServerData(data);
                return ERR_FS_SOCKET_ACCEPT_FAILED;
            }
        }
    }

    PrintMemoryUsage();

    StartInlinedProfiling(PE_SERVER_SHUTDOWN);
    cleanupProcessServerData(data);
    EndInlinedProfiling(PE_SERVER_SHUTDOWN);

    // in case we are only testing filters without actually requesting results
    PrintProfilingStatus();

    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return ERR_FS_NO_ERROR;
}

void fileServer_stop(FileServer_t server)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Begin.");

    ProcessServerData_t* data = (ProcessServerData_t*) server.p;
    if(data != NULL)
    {
        uint64_t eventCounter = 1;
        int bytesWritten = write(data->endEvent, &eventCounter, sizeof(eventCounter));
        if (bytesWritten != sizeof(eventCounter))
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: read event failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
    }
    
    LOG_MESSAGE(INFO_LOG_MSG, "End.");
}

bool fileServer_isNull(FileServer_t server)
{
    return server.p == NULL;
}

//
// Local functions
//

//
// Server data
//

static int initProcessServerData(ProcessServerData_t* data, DaosId_t serverId, DaosCount_t numServers,
    DaosCount_t numEntriesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, const char *dataDir, uint32_t dirDepth, uint32_t dirCount)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    // End event

    data->endEvent = eventfd(0, EFD_NONBLOCK);
    if(data->endEvent < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: eventfd() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
        return ERR_FS_EVENT_CREATE_FAILED;
    }

    // Request processing
    LOG_MESSAGE(DEBUG_LOG_MSG, "loadRequestProcessingData().");

    int ret = loadRequestProcessingData(&data->reqProcessingData, serverId, numServers,
        numEntriesPerFile,
        requestArrivalTimeout, connectionTimeout, numConnections,
        numConnections);
    if(ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: loadRequestProcessingData() returned %d.", ret);
        cleanupProcessServerData(data);
        return ret;
    }

    // Socket

    LOG_MESSAGE(DEBUG_LOG_MSG, "initServerSocket().");

    uint32_t timeout = requestArrivalTimeout > connectionTimeout ? requestArrivalTimeout : connectionTimeout;

    data->sockt = initServerSocket(serverPort, numConnections, timeout);
    if(data->sockt < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: initServerSocket() returned %d.", data->sockt);
        cleanupProcessServerData(data);
        return ERR_FS_SOCKET_INIT_FAILED;
    }

    // Request threads

    LOG_MESSAGE(DEBUG_LOG_MSG, "malloc(requestThreadIds).");

    data->numConnections    = numConnections;
    data->numRequestThreads = data->numConnections;

    data->requestThreadIds = (pthread_t*) malloc(data->numRequestThreads * sizeof(pthread_t));
    if(data->requestThreadIds == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.",
            data->numRequestThreads * sizeof(pthread_t));
        cleanupProcessServerData(data);
        return ERR_FS_MEMORY_ALLOC_FAILED;
    } 

    LOG_MESSAGE(DEBUG_LOG_MSG, "Creating requestThreadIds.");

    for(uint16_t i = 0; i < data->numRequestThreads; ++i)
    {
        if(pthread_create(&data->requestThreadIds[i], NULL, requestProcessingThread, data) != 0)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: pthread_create(%hu) failed. errno = %d (\"%s\").",
                i, errno, strerror(errno));
            data->numRequestThreads = i;
            cleanupProcessServerData(data);
            return ERR_FS_THREAD_CREATE_FAILED;
        }
    }

    data->dataDir = strdup(dataDir);
    data->dirDepth = dirDepth;
    data->dirCount = dirCount;

    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return ERR_FS_NO_ERROR;
}

static void cleanupProcessServerData(ProcessServerData_t* data)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Begin.");

    for(uint16_t i = 0; i < data->numRequestThreads; ++i)
    {
        pthread_cancel(data->requestThreadIds[i]);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Request thread %hu cancelled.", i);
    }

    for(uint16_t i = 0; i < data->numRequestThreads; ++i)
    {
        pthread_join(data->requestThreadIds[i], NULL);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Request thread %hu joined.", i);
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "free(requestThreadIds).");
    free(data->requestThreadIds);

    if(data->sockt >= 0)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "close(socket).");
        close(data->sockt);
    }

    if(data->endEvent >= 0)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "close(endEvent).");
        close(data->endEvent);
    }

    for (size_t i = 0; i < data->fileGroupsUsed; i++)
    {
        daos_free(data->fileGroupData[i].Daos);
        fileLockList_free(data->fileGroupData[i].fileList);

    }
    LOG_MESSAGE(DEBUG_LOG_MSG, "daos_free completed.");
    LOG_MESSAGE(DEBUG_LOG_MSG, "fileLockList_free completed.");

    pthread_mutex_destroy(&data->fileGroupMutex);

    LOG_MESSAGE(DEBUG_LOG_MSG, "cleanupRequestProcessingData().");
    cleanupRequestProcessingData(&data->reqProcessingData);

    free(data->dataDir);

    LOG_MESSAGE(DEBUG_LOG_MSG, "free(data).");
    free(data);

    LOG_MESSAGE(INFO_LOG_MSG, "End.");
}

//
// Request processing
//

static void zeroRequestProcessingData(RequestProcessingData_t* data)
{
    data->serverId           = 0;

    data->requestArrivalTimeout = 0;
    data->connectionTimeout     = 0;
    data->numConnections        = 0;

    data->socketQueue = MtQueue_NULL;
}

static int loadRequestProcessingData(RequestProcessingData_t* data, DaosId_t serverId, DaosCount_t numServers,
    DaosCount_t numEntriesPerFile,
    uint32_t requestArrivalTimeout, uint32_t connectionTimeout, uint16_t numConnections,
    uint16_t numRequestThreads)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    zeroRequestProcessingData(data);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing socket queue.");

    data->socketQueue = mtQueue_init(numRequestThreads, closeSocket);
    if(mtQueue_isNull(data->socketQueue))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: mtQueue_init() failed.");
        return ERR_FS_MT_QUEUE_INIT_FAILED;
    }

    data->requestArrivalTimeout = requestArrivalTimeout;
    data->connectionTimeout     = connectionTimeout;
    data->numConnections        = numConnections;

    data->serverId = serverId;
    data->numEntriesPerFile = numEntriesPerFile;

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return ERR_FS_NO_ERROR;
}

static void cleanupRequestProcessingData(RequestProcessingData_t* data)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    mtQueue_free(data->socketQueue);
    LOG_MESSAGE(DEBUG_LOG_MSG, "mtQueue_free() completed.");

    zeroRequestProcessingData(data);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
}

static void* requestProcessingThread(void* arg)
{
    pthread_cleanup_push(threadCleanupHandler, "requestProcessingThread exited.");

    LOG_MESSAGE(INFO_LOG_MSG, "Thread started.");

    ProcessServerData_t* serverData = (ProcessServerData_t*)arg;
    RequestProcessingData_t* data = &serverData->reqProcessingData;
    FileServer_t sdata;
    sdata.p = serverData;

    while(true)
    {
        int sockt = -1;
        uint64_t stamp_accept = 0;
        pthread_cleanup_push(socketQueueThreadCleanupHandler, &data->socketQueue);
        SocketStore* socketStore = mtQueue_pop(data->socketQueue);
        sockt = socketStore->socket;
        stamp_accept = socketStore->stamp_accept;
        free(socketStore); 
        pthread_cleanup_pop(0);

        LOG_MESSAGE(DEBUG_LOG_MSG, "Connection selected for work.");

        uint32_t timeout = data->requestArrivalTimeout > data->connectionTimeout
            ? data->requestArrivalTimeout : data->connectionTimeout;
        int ret = configureSocketSettings(sockt, timeout);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: configureSocketSettings(timeout = %u ms) returned %d.",
                ret, timeout);
            close(sockt);
            continue;
        }

        bool closeConnection = false;

        while(closeConnection == false)
        {
            StartInlinedProfiling(PE_HANDLE_REQUEST);
            ret = processRequest(&sdata, sockt,
                data->requestArrivalTimeout, data->connectionTimeout, &closeConnection);
            if(ret != 0) {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: processRequest() returned %d.", ret);
            }
            EndInlinedProfiling(PE_HANDLE_REQUEST);
        }

        close(sockt);
        ProfilingAddSectionTimeThreadSafe(PE_SOCKET_LIFETIME, GetProfilingStamp() - stamp_accept, 1); 
        LOG_MESSAGE(DEBUG_LOG_MSG, "Connection closed.");
    }

    pthread_cleanup_pop(1);

    return NULL;
}

//
// Socket
//

static int initServerSocket(uint16_t serverPort, uint16_t numConnections, uint32_t timeoutInMs)
{
    int sockt = socket(AF_INET, SOCK_STREAM, 0);
    if(sockt < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: socket() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
        return ERR_FS_SOCKET_CREATE_FAILED;
    }

    int ret = configureSocketSettings(sockt, timeoutInMs);
    if(ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: configureSocketSettings(timeout = %u ms) returned %d.",
            ret, timeoutInMs);
        close(sockt);
        return ret;
    }

    struct sockaddr_in serverAddr = { 0 };

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);

    if(bind(sockt, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bind() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
        close(sockt);
        return ERR_FS_SOCKET_BIND_FAILED;
    }

    if(fcntl(sockt, F_SETFL, O_NONBLOCK) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fcntl(F_SETFL, O_NONBLOCK) failed. errno ="
            " %d (\"%s\").", errno, strerror(errno));
        close(sockt);
        return ERR_FS_SOCKET_CONFIG_FAILED;
    }
    return sockt;
}

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

static int configureSocketSettings(int sockt, uint32_t timeoutInMs)
{
    struct timeval timeout
        = { .tv_sec = timeoutInMs / 1000, .tv_usec = (timeoutInMs % 1000) * 1000 };

    if(setsockopt(sockt, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_SOCKET, SO_SNDTIMEO) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_WTIMEOUT_FAILED;
    }

    if(setsockopt(sockt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_SOCKET, SO_RCVTIMEO) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_RTIMEOUT_FAILED;
    }

    int numRetries = 3;

    if(setsockopt(sockt, IPPROTO_TCP, TCP_SYNCNT, &numRetries, sizeof(numRetries)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(IPPROTO_TCP, TCP_SYNCNT) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_RETRY_FAILED;
    }

    if(setsockopt(sockt, SOL_TCP, TCP_USER_TIMEOUT, &timeoutInMs, sizeof(timeoutInMs)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_TCP, TCP_USER_TIMEOUT) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_TIMEOUT_FAILED;
    }

    int naggleAlgo = 1; // network packets are packed by the server to send them all at once
    if (setsockopt(sockt, IPPROTO_TCP, TCP_NODELAY, &naggleAlgo, sizeof(naggleAlgo)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(IPPROTO_TCP, TCP_NODELAY) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_TIMEOUT_FAILED;
    }

    int quickAck = 1; // only good for sparsely sent packets. Needs to be set after receive
    if (setsockopt(sockt, SOL_TCP, TCP_QUICKACK, &quickAck, sizeof(quickAck)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_TCP, TCP_QUICKACK) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return ERR_FS_SOCKET_CONFIG_TIMEOUT_FAILED;
    }
    return ERR_FS_NO_ERROR;
}

static void closeSocket(void* arg)
{
    int sockt = (int) (int64_t) arg;
    close(sockt);
}

static void socketQueueThreadCleanupHandler(void* arg)
{
    MtQueue_t* queue = (MtQueue_t*) arg;
    MtQueueThreadCleanup(*queue);
}

//
// Others
//

static void threadCleanupHandler(void *arg)
{
    const char* msg = (const char*) arg;
    LOG_MESSAGE(INFO_LOG_MSG, msg);
}

FileGroupData* getCreateFileGroupDao(uint32_t fileGroup, FileServer_t* data)
{
    ProcessServerData_t* serverData = (ProcessServerData_t*)data->p;
    // search without threadlock
    for (size_t i = 0; i < serverData->fileGroupsUsed; i++)
    {
        if (serverData->fileGroupData[i].type == fileGroup)
        {
            return &serverData->fileGroupData[i];
        }
    }

    pthread_mutex_lock(&serverData->fileGroupMutex);
	//search again, but this time in a thread safe way
    for (size_t i = 0; i < serverData->fileGroupsUsed; i++)
    {
        if (serverData->fileGroupData[i].type == fileGroup)
        {
            FileGroupData* ret = &serverData->fileGroupData[i];
            pthread_mutex_unlock(&serverData->fileGroupMutex);
            return ret;
        }
    }

	// We can't save/load this file due to "out of locks" 
    if (serverData->fileGroupsUsed == MAX_DIFFERENT_ENTRY_TYPES)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Reached maximum DAO count.",
            MAX_DIFFERENT_ENTRY_TYPES);
        pthread_mutex_unlock(&serverData->fileGroupMutex);
        return NULL;
    }

    //looks like we need a new type
    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing files file lock list for type %u.", fileGroup);

    // create directory structure
    char numberBuffer[500];
    snprintf(numberBuffer, sizeof(numberBuffer), "%s/%u", serverData->dataDir, fileGroup);
    int err_mkdir = mkdir(numberBuffer, 0755);
    if (err_mkdir != 0 && errno != EEXIST)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: mkdir '%s' failed with errno = %d(\"%s\").", numberBuffer, errno, strerror(errno));
        pthread_mutex_unlock(&serverData->fileGroupMutex);
        return NULL;
    }

    FileGroupData* ret = &serverData->fileGroupData[serverData->fileGroupsUsed];
    serverData->fileGroupsUsed++;

    ret->type = fileGroup;
    ret->numItemsPerFile = serverData->reqProcessingData.numEntriesPerFile;
    ret->fileList = fileLockList_init(serverData->numRequestThreads);
    if (fileLockList_isNull(ret->fileList))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileLockList_init(files, capacity = %hu) failed.",
            serverData->numRequestThreads);
        pthread_mutex_unlock(&serverData->fileGroupMutex);
        return NULL;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing files daos.");

    ret->Daos = daos_init(numberBuffer, serverData->dirDepth, serverData->dirCount, "", "",
        10, serverData->reqProcessingData.numEntriesPerFile, getDaosFileStoreFunctions());
    if (daos_isNull(ret->Daos))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_init(files) failed.");
        pthread_mutex_unlock(&serverData->fileGroupMutex);
        return NULL;
    }

    pthread_mutex_unlock(&serverData->fileGroupMutex);
    return ret;
}
