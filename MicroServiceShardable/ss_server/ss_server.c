#include <ss_server.h>
#include <request_processing.h>
#include <mt_queue.h>
#include <profiling/profiling.h>
#include <utils.h>
#include <score_manager.h>
#include <score_file.h>
#include <file_lock_list.h>
#include <request_response_definitions.h>
#include <error_reporting.h>
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
#include <signal.h>
#include <fcntl.h>
#include <logger.h>
#include <string.h>

//
// Types
//

typedef
struct
{
    uint32_t requestArrivalTimeout;
    uint32_t connectionTimeout;

    MtQueue_t socketQueue;
    FileLockList_t fileLockList; 
    const char* similarityPaths[SSFT_MAX_SCORE_FILE_TYPE];
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
} ProcessServerData_t;

//
// Local prototypes
//

// Server data

static int initProcessServerData(ProcessServerData_t* data,
    const char* companyDirectory,
    const char* industryDirectory,
    const char* titleDirectory,
    const char* profileDirectory,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections);
static void cleanupProcessServerData(ProcessServerData_t* data);

// Request processing

static void zeroRequestProcessingData(RequestProcessingData_t* data);
static int initRequestProcessingData(RequestProcessingData_t* data, 
    const char* companyDirectory,
    const char* industryDirectory,
    const char* titleDirectory,
    const char* profileDirectory,
    uint32_t requestArrivalTimeout, uint32_t connectionTimeout, 
    uint16_t numRequestThreads);
static void cleanupRequestProcessingData(RequestProcessingData_t* data);
static void* requestProcessingThread(void* arg);

// Socket

static int initServerSocket(uint16_t serverPort, uint16_t numConnections, uint32_t timeoutInMs);
static int configureSocketTimeout(int sockt, uint32_t timeoutInMs);
static void closeSocket(void* arg);
static void socketQueueThreadCleanupHandler(void* arg);

// Others

static void threadCleanupHandler(void *arg);

//
// External interface
//

ScoreServer_t scoreServer_init()
{
    ScoreServer_t server = ScoreServer_NULL;

    ProcessServerData_t* data = (ProcessServerData_t*) malloc(sizeof(ProcessServerData_t));
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: scoreServer_init() - malloc(size = "
            "%zu bytes) failed.", sizeof(ProcessServerData_t));
        return server;
    }

    // Initial zero values

    data->endEvent             = -1;
    data->sockt                = -1;
    data->numConnections       = 0;
    data->requestThreadIds     = NULL;

    zeroRequestProcessingData(&data->reqProcessingData);

    server.p = data;

    return server;
}

int scoreServer_run(ScoreServer_t server,
    const char* companyDirectory,
    const char* industryDirectory,
    const char* titleDirectory,
    const char* profileDirectory,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Started...");

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Execution parameters:\n"
        "    company directory : \"%s\"\n"
        "    industry directory : \"%s\"\n"
        "    title directory : \"%s\"\n"
        "    profile directory : \"%s\"\n"
        "    port : %hu\n"
        "    request arrival timeout : %lu ms\n"
        "    connection timeout : %lu ms\n"
        "    number of connections : %hu\n",
        companyDirectory,
        industryDirectory,
        titleDirectory,
        profileDirectory,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections);

    signal(SIGPIPE, SIG_IGN);

    ProcessServerData_t* data = (ProcessServerData_t*) server.p;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for server.");
        return 1;
    }

    int ret = initProcessServerData(data, 
        companyDirectory,
        industryDirectory,
        titleDirectory,
        profileDirectory,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: initProcessServerData() returned %d.", ret);
        return 2;
    }

    if(listen(data->sockt, data->numConnections) != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: listen(backlogSize = %d) failed. errno = %d (\"%s\").",
            (int) data->numConnections, errno, strerror(errno));
        cleanupProcessServerData(data);
        return 5;
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
            return 6;
        }
        else if(sd > 0 && FD_ISSET(data->endEvent, &workingSet))
        {
            uint64_t eventCounter = 0;
            read(data->endEvent, &eventCounter, sizeof(eventCounter));
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
                mtQueue_push(data->reqProcessingData.socketQueue, (void*) (uint64_t) clientSocket);
                pthread_cleanup_pop(0);
            }
            else if(errno != EWOULDBLOCK)
            {
                LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: accept() failed. errno = %d (\"%s\").",
                    errno, strerror(errno));
                cleanupProcessServerData(data);
                return 6;
            }
        }
    }

    cleanupProcessServerData(data);

    // in case we are only testing filters without actually requesting results
    PrintProfilingStatus();

    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return 0;
}

void scoreServer_stop(ScoreServer_t server)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Begin.");

    ProcessServerData_t* data = (ProcessServerData_t*) server.p;
    if(data != NULL)
    {
        uint64_t eventCounter = 1;
        write(data->endEvent, &eventCounter, sizeof(eventCounter));
    }
    
    LOG_MESSAGE(INFO_LOG_MSG, "End.");
}

bool scoreServer_isNull(ScoreServer_t server)
{
    return server.p == NULL;
}

//
// Local functions
//

//
// Server data
//

static int initProcessServerData(ProcessServerData_t* data, 
    const char* companyDirectory,
    const char* industryDirectory,
    const char* titleDirectory,
    const char* profileDirectory,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    // End event

    data->endEvent = eventfd(0, EFD_NONBLOCK);
    if(data->endEvent < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: eventfd() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
        return 1;
    }

    // score manager internal state setup 
    scoreManagerInit();

    // Request processing

    LOG_MESSAGE(DEBUG_LOG_MSG, "initRequestProcessingData().");

    int ret = initRequestProcessingData(&data->reqProcessingData, 
        companyDirectory,
        industryDirectory,
        titleDirectory,
        profileDirectory,
        requestArrivalTimeout, connectionTimeout,
        numConnections);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: initRequestProcessingData() returned %d.", ret);
        cleanupProcessServerData(data);
        return 2;
    }

    // Socket

    LOG_MESSAGE(DEBUG_LOG_MSG, "initServerSocket().");

    uint32_t timeout = requestArrivalTimeout > connectionTimeout ? requestArrivalTimeout : connectionTimeout;

    data->sockt = initServerSocket(serverPort, numConnections, timeout);
    if(data->sockt < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: initServerSocket() returned %d.", data->sockt);
        cleanupProcessServerData(data);
        return 3;
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
        return 4;
    } 

    LOG_MESSAGE(DEBUG_LOG_MSG, "Creating requestThreadIds.");

    for(uint16_t i = 0; i < data->numRequestThreads; ++i)
    {
        if(pthread_create(&data->requestThreadIds[i], NULL, requestProcessingThread, &data->reqProcessingData) != 0)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: pthread_create(%hu) failed. errno = %d (\"%s\").",
                i, errno, strerror(errno));
            data->numRequestThreads = i;
            cleanupProcessServerData(data);
            return 5;
        }
    }

    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return 0;
}

static void cleanupProcessServerData(ProcessServerData_t* data)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Begin.");

    for(uint16_t i = 0; i < data->numRequestThreads; ++i)
    {
        if (data->requestThreadIds[i] != 0) // happens if server fails to initialize
        {
            pthread_cancel(data->requestThreadIds[i]);
            LOG_MESSAGE(DEBUG_LOG_MSG, "Request thread %hu cancelled.", i);
        }
    }

    for(uint16_t i = 0; i < data->numRequestThreads; ++i)
    {
        if (data->requestThreadIds[i] != 0) // happens if server fails to initialize
        {
            pthread_join(data->requestThreadIds[i], NULL);
            LOG_MESSAGE(DEBUG_LOG_MSG, "Request thread %hu joined.", i);
        }
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

    LOG_MESSAGE(DEBUG_LOG_MSG, "cleanupRequestProcessingData().");
    cleanupRequestProcessingData(&data->reqProcessingData);

    LOG_MESSAGE(DEBUG_LOG_MSG, "free(data).");
    free(data);

    scoreManagerShutdown();

    LOG_MESSAGE(INFO_LOG_MSG, "End.");
}

//
// Request processing
//

static void zeroRequestProcessingData(RequestProcessingData_t* data)
{
    data->requestArrivalTimeout = 0;
    data->connectionTimeout     = 0;

    data->socketQueue = MtQueue_NULL;
	
	data->fileLockList = FileLockList_NULL; 
}

static int initRequestProcessingData(RequestProcessingData_t* data,
    const char* companyDirectory,
    const char* industryDirectory,
    const char* titleDirectory,
    const char* profileDirectory,
    uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numRequestThreads)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    zeroRequestProcessingData(data);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing socket queue.");

    data->socketQueue = mtQueue_init(numRequestThreads, closeSocket);
    if(mtQueue_isNull(data->socketQueue))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: mtQueue_init() failed.");
        return 1;
    }

    data->requestArrivalTimeout = requestArrivalTimeout;
    data->connectionTimeout     = connectionTimeout;
    data->similarityPaths[SSFT_COMPANY_SIMILARITY_SCORE] = companyDirectory;
    data->similarityPaths[SSFT_INDUSTRY_SIMILARITY_SCORE] = industryDirectory;
    data->similarityPaths[SSFT_TITLE_SIMILARITY_SCORE] = titleDirectory;
    data->similarityPaths[SSFT_PROFILE_SIMILARITY_SCORE] = profileDirectory;

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing file lock list.");

    data->fileLockList = fileLockList_init(numRequestThreads * MAX_ID_ARRAY_SIZE);
    if(fileLockList_isNull(data->fileLockList))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileLockList_init(profiles, capacity = %hu) failed.",
            (uint16_t) (numRequestThreads * MAX_ID_ARRAY_SIZE));
        cleanupRequestProcessingData(data);
        return 2;
    }
	
    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return 0;
}

static void cleanupRequestProcessingData(RequestProcessingData_t* data)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    fileLockList_free(data->fileLockList);
    LOG_MESSAGE(DEBUG_LOG_MSG, "fileLockList_free completed.");
 
    mtQueue_free(data->socketQueue);
    LOG_MESSAGE(DEBUG_LOG_MSG, "mtQueue_free() completed.");

    zeroRequestProcessingData(data);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
}

static void* requestProcessingThread(void* arg)
{
    pthread_cleanup_push(threadCleanupHandler, "requestProcessingThread exited.");

    LOG_MESSAGE(INFO_LOG_MSG, "Thread started.");

    RequestProcessingData_t* data = (RequestProcessingData_t*) arg;

    while(true)
    {
        int sockt = -1;
        pthread_cleanup_push(socketQueueThreadCleanupHandler, &data->socketQueue);
        sockt = (int) (uint64_t) mtQueue_pop(data->socketQueue);
        pthread_cleanup_pop(0);

        LOG_MESSAGE(DEBUG_LOG_MSG, "Connection selected for work.");

        uint32_t timeout = data->requestArrivalTimeout > data->connectionTimeout
            ? data->requestArrivalTimeout : data->connectionTimeout;
        int ret = configureSocketTimeout(sockt, timeout);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: configureSocketTimeout(timeout = %u ms) returned %d.",
                ret, timeout);
            close(sockt);
            continue;
        }

        bool closeConnection = false;

        int64_t processing_duration = 0;
        int64_t processing_count = 0;
        while(closeConnection == false)
        {
            int64_t processing_start = GetProfilingStamp();
            ret = processRequest(data->fileLockList, sockt, data->requestArrivalTimeout, data->connectionTimeout, data->similarityPaths, &closeConnection);
            if(ret != 0) {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: processRequest() returned %d.", ret);
            }
            processing_duration += GetProfilingStamp() - processing_start;
            processing_count++;
        }

        close(sockt);

        // perform system sanity check : active connections, memory usage, avg processing time
        if (processing_count)
        {
            perform_SystemSanityChecks((int)mtQueue_size(data->socketQueue), BUFFER_EXTEND_MIN_SIZE, 
                processing_duration / processing_count / MS_TO_NS);
        }
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
        return -1;
    }

    int ret = configureSocketTimeout(sockt, timeoutInMs);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: configureSocketTimeout(timeout = %u ms) returned %d.",
            ret, timeoutInMs);
        close(sockt);
        return -2;
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
        return -3;
    }

    if(fcntl(sockt, F_SETFL, O_NONBLOCK) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fcntl(F_SETFL, O_NONBLOCK) failed. errno ="
            " %d (\"%s\").", errno, strerror(errno));
        close(sockt);
        return -4;
    }

    return sockt;
}

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

static int configureSocketTimeout(int sockt, uint32_t timeoutInMs)
{
    struct timeval timeout
        = { .tv_sec = timeoutInMs / 1000, .tv_usec = (timeoutInMs % 1000) * 1000 };

    if(setsockopt(sockt, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_SOCKET, SO_SNDTIMEO) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return 1;
    }

    if(setsockopt(sockt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_SOCKET, SO_RCVTIMEO) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return 2;
    }

    int numRetries = 3;

    if(setsockopt(sockt, IPPROTO_TCP, TCP_SYNCNT, &numRetries, sizeof(numRetries)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(IPPROTO_TCP, TCP_SYNCNT) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return 3;
    }

    if(setsockopt(sockt, SOL_TCP, TCP_USER_TIMEOUT, &timeoutInMs, sizeof(timeoutInMs)) < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: setsockopt(SOL_TCP, TCP_USER_TIMEOUT) failed. "
            "errno = %d (\"%s\").", errno, strerror(errno));
        return 4;
    }

    return 0;
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
