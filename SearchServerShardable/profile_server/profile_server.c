#include <profile_server.h>
#include <request_processing.h>
#include <file_lock_list.h>
#include <mt_queue.h>
#include <cache_engine.h>
#include <daos.h>
#include <profile_functions.h>
#include <company_functions.h>
#include <profile_definitions.h>
#include <company_definitions.h>
#include <profiling/profiling.h>
#include <utils.h>
#include <common/request_response_definitions.h>
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

//
// Types
//

typedef
struct
{
    DaosCount_t numServers;
    DaosId_t    serverId;
    DaosCount_t numProfilesPerFile;
    DaosCount_t numCompaniesPerFile;

    uint32_t requestArrivalTimeout;
    uint32_t connectionTimeout;
    uint16_t numConnections;

    MtQueue_t socketQueue;

    FileLockList_t profileFileList;
    Daos_t         profileDaos;
    CacheEngine_t  profileCache;

    FileLockList_t companyFileList;
    Daos_t         companyDaos;
    CacheEngine_t  companyCache;

    SearchEngine_t searchEngine;
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

    pthread_t* profileCacheThreadId;
    pthread_t* companyCacheThreadId;

    // Request processing
    RequestProcessingData_t reqProcessingData;
} ProcessServerData_t;

typedef
struct
{
    CacheEngine_t cacheEngine;
    uint16_t      updatePeriod;
} CacheThreadArg_t;

//
// Local prototypes
//

// Server data

static int initProcessServerData(ProcessServerData_t* data, DaosId_t serverId, DaosCount_t numServers,
    const char* profileDirectory, DaosCount_t numProfileCacheEntries, DaosCount_t numProfilesPerFile,
    const char* companyDirectory, DaosCount_t numCompanyCacheEntries, DaosCount_t numCompaniesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, uint16_t numSearchThreads, uint16_t numLoadingThreads,
    uint16_t searchResultsExpirationInMinutes);
static void cleanupProcessServerData(ProcessServerData_t* data);

// Request processing

static void zeroRequestProcessingData(RequestProcessingData_t* data);
static int loadRequestProcessingData(RequestProcessingData_t* data, DaosId_t serverId, DaosCount_t numServers,
    const char* profileDirectory, DaosCount_t numProfileCacheEntries, DaosCount_t numProfilesPerFile,
    const char* companyDirectory, DaosCount_t numCompanyCacheEntries, DaosCount_t numCompaniesPerFile,
    uint32_t requestArrivalTimeout, uint32_t connectionTimeout, uint16_t numConnections,
    uint16_t numRequestThreads, uint16_t numSearchThreads, uint16_t numLoadingThreads,
    uint16_t searchResultsExpirationInMinutes);
static void cleanupRequestProcessingData(RequestProcessingData_t* data);
static void* requestProcessingThread(void* arg);

// Socket

static int initServerSocket(uint16_t serverPort, uint16_t numConnections, uint32_t timeoutInMs);
static int configureSocketTimeout(int sockt, uint32_t timeoutInMs);
static void closeSocket(void* arg);
static void socketQueueThreadCleanupHandler(void* arg);

// CacheEngine_t

static pthread_t* startCacheThread(CacheEngine_t cacheEngine, uint16_t updatePeriod);
static void* cacheThread(void* arg);

// Others

static void threadCleanupHandler(void *arg);

//
// External interface
//

ProfileServer_t profileServer_init()
{
    ProfileServer_t server = ProfileServer_NULL;

    ProcessServerData_t* data = (ProcessServerData_t*) malloc(sizeof(ProcessServerData_t));
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: profileServer_init() - malloc(size = "
            "%zu bytes) failed.", sizeof(ProcessServerData_t));
        return server;
    }

    // Initial zero values

    data->endEvent             = -1;
    data->sockt                = -1;
    data->numConnections       = 0;
    data->numRequestThreads    = 0;
    data->requestThreadIds     = NULL;
    data->profileCacheThreadId = NULL;
    data->companyCacheThreadId = NULL;

    zeroRequestProcessingData(&data->reqProcessingData);

    server.p = data;

    return server;
}

int profileServer_run(ProfileServer_t server, DaosId_t serverId, DaosCount_t numServers,
    const char* profileDirectory, DaosCount_t numProfileCacheEntries, DaosCount_t numProfilesPerFile,
    const char* companyDirectory, DaosCount_t numCompanyCacheEntries, DaosCount_t numCompaniesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, uint16_t numSearchThreads, uint16_t numLoadingThreads,
    uint16_t profileCacheUpdatePeriodInSecs, uint16_t companyCacheUpdatePeriodInSecs,
    uint16_t searchResultsExpirationInMinutes)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Started...");

    LOG_MESSAGE(ATTENTION_LOG_MSG, "Execution parameters:\n"
        "    server id : \"%lu\"\n"
        "    number of servers : %lu\n"
        "    profile directory : \"%s\"\n"
        "    number of profile cache entries : %lu\n"
        "    number of profiles per file : %lu\n"
        "    company directory : \"%s\"\n"
        "    number of company cache entries : %lu\n"
        "    number of companies per file : %lu\n"
        "    port : %hu\n"
        "    request arrival timeout : %lu ms\n"
        "    connection timeout : %lu ms\n"
        "    number of connections : %hu\n"
        "    number of search threads : %hu\n"
        "    number of loading threads : %hu\n"
        "    profile cache update period : %hu seconds\n"
        "    company cache update period : %hu seconds\n"
        "    search results expiration : %hu minutes\n",
        serverId, numServers,
        profileDirectory, numProfileCacheEntries, numProfilesPerFile,
        companyDirectory, numCompanyCacheEntries, numCompaniesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections, numSearchThreads, numLoadingThreads,
        profileCacheUpdatePeriodInSecs, companyCacheUpdatePeriodInSecs,
        searchResultsExpirationInMinutes);

    signal(SIGPIPE, SIG_IGN);

    ProcessServerData_t* data = (ProcessServerData_t*) server.p;
    if(data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: invalid NULL argument for server.");
        return 1;
    }

    int ret = initProcessServerData(data, serverId, numServers,
        profileDirectory, numProfileCacheEntries, numProfilesPerFile,
        companyDirectory, numCompanyCacheEntries, numCompaniesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout,
        numConnections, numSearchThreads, numLoadingThreads, searchResultsExpirationInMinutes);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: initProcessServerData() returned %d.", ret);
        return 2;
    }

    data->profileCacheThreadId = startCacheThread(data->reqProcessingData.profileCache,
        profileCacheUpdatePeriodInSecs);
    if(data->profileCacheThreadId == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: startCacheThread(profiles) failed.");
        cleanupProcessServerData(data);
        return 3;
    }

    data->companyCacheThreadId = startCacheThread(data->reqProcessingData.companyCache,
        companyCacheUpdatePeriodInSecs);
    if(data->companyCacheThreadId == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: startCacheThread(companies) failed.");
        cleanupProcessServerData(data);
        return 4;
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

    PrintMemoryUsage();

    StartInlinedProfiling(PE_SERVER_SHUTDOWN);
    cleanupProcessServerData(data);
    EndInlinedProfiling(PE_SERVER_SHUTDOWN);

    // in case we are only testing filters without actually requesting results
    PrintProfilingStatus();

    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return 0;
}

void profileServer_stop(ProfileServer_t server)
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

bool profileServer_isNull(ProfileServer_t server)
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
    const char* profileDirectory, DaosCount_t numProfileCacheEntries, DaosCount_t numProfilesPerFile,
    const char* companyDirectory, DaosCount_t numCompanyCacheEntries, DaosCount_t numCompaniesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, uint16_t numSearchThreads, uint16_t numLoadingThreads,
    uint16_t searchResultsExpirationInMinutes)
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

    // Request processing

    LOG_MESSAGE(DEBUG_LOG_MSG, "loadRequestProcessingData().");

    int ret = loadRequestProcessingData(&data->reqProcessingData, serverId, numServers,
        profileDirectory, numProfileCacheEntries, numProfilesPerFile,
        companyDirectory, numCompanyCacheEntries, numCompaniesPerFile,
        requestArrivalTimeout, connectionTimeout, numConnections,
        numConnections, numSearchThreads, numLoadingThreads, searchResultsExpirationInMinutes);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: loadRequestProcessingData() returned %d.", ret);
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

    cacheEngine_stop(data->reqProcessingData.companyCache);
    LOG_MESSAGE(DEBUG_LOG_MSG, "cacheEngine_stop(companies) completed.");

    cacheEngine_stop(data->reqProcessingData.profileCache);
    LOG_MESSAGE(DEBUG_LOG_MSG, "cacheEngine_stop(profiles) completed.");

    if(data->companyCacheThreadId != NULL)
    {
        pthread_join(*data->companyCacheThreadId, NULL);
        free(data->companyCacheThreadId);
    }

    if(data->profileCacheThreadId != NULL)
    {
        pthread_join(*data->profileCacheThreadId, NULL);
        free(data->profileCacheThreadId);
    }

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

    LOG_MESSAGE(INFO_LOG_MSG, "End.");
}

//
// Request processing
//

static void zeroRequestProcessingData(RequestProcessingData_t* data)
{
    data->numServers         = 0;
    data->serverId           = 0;
    data->numProfilesPerFile = 0;

    data->requestArrivalTimeout = 0;
    data->connectionTimeout     = 0;
    data->numConnections        = 0;

    data->socketQueue = MtQueue_NULL;

    data->profileFileList = FileLockList_NULL;
    data->profileDaos     = Daos_NULL;
    data->profileCache    = CacheEngine_NULL;

    data->companyFileList = FileLockList_NULL;
    data->companyDaos     = Daos_NULL;
    data->companyCache    = CacheEngine_NULL;

    data->searchEngine = SearchEngine_NULL;
}

static int loadRequestProcessingData(RequestProcessingData_t* data, DaosId_t serverId, DaosCount_t numServers,
    const char* profileDirectory, DaosCount_t numProfileCacheEntries, DaosCount_t numProfilesPerFile,
    const char* companyDirectory, DaosCount_t numCompanyCacheEntries, DaosCount_t numCompaniesPerFile,
    uint32_t requestArrivalTimeout, uint32_t connectionTimeout, uint16_t numConnections,
    uint16_t numRequestThreads, uint16_t numSearchThreads, uint16_t numLoadingThreads,
    uint16_t searchResultsExpirationInMinutes)
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
    data->numConnections        = numConnections;

    data->numServers = numServers;
    data->serverId = serverId;
    data->numProfilesPerFile  = numProfilesPerFile;
    data->numCompaniesPerFile = numCompaniesPerFile;

    StartInlinedProfiling(PE_PROFILE_LOADING);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing profiles file lock list.");

    data->profileFileList = fileLockList_init(numRequestThreads);
    if(fileLockList_isNull(data->profileFileList))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileLockList_init(profiles, capacity = %hu) failed.", numRequestThreads);
        cleanupRequestProcessingData(data);
        return 2;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing profiles daos.");

    data->profileDaos = daos_init(PROFILE_DAOS_VERSION, profileDirectory, PROFILE_FILE_PREFIX, PROFILE_FILE_EXTENSION,
        PROFILE_NUM_ID_DIGITS, numProfilesPerFile, getProfileFunctions());
    if(daos_isNull(data->profileDaos))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_init(profiles, version = %hu) failed.", PROFILE_DAOS_VERSION);
        cleanupRequestProcessingData(data);
        return 3;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Loading profiles cache.");

    data->profileCache = cacheEngine_load(getProfileFunctions(), data->profileDaos,
        profileDirectory, numServers, numProfileCacheEntries, numLoadingThreads);
    if(cacheEngine_isNull(data->profileCache))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_load(profiles) failed.");
        cleanupRequestProcessingData(data);
        return 4;
    }

    EndInlinedProfiling(PE_PROFILE_LOADING);

    StartInlinedProfiling(PE_COMPANY_LOADING);
    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing companies file lock list.");

    data->companyFileList = fileLockList_init(numRequestThreads);
    if(fileLockList_isNull(data->companyFileList))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fileLockList_init(companies, capacity = %hu) failed.",
            numRequestThreads);
        cleanupRequestProcessingData(data);
        return 5;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing companies daos.");

    data->companyDaos = daos_init(COMPANY_DAOS_VERSION, companyDirectory, COMPANY_FILE_PREFIX, COMPANY_FILE_EXTENSION,
        COMPANY_NUM_ID_DIGITS, numCompaniesPerFile, getCompanyFunctions());
    if(daos_isNull(data->companyDaos))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_init(companies, version = %hu) failed.", COMPANY_DAOS_VERSION);
        cleanupRequestProcessingData(data);
        return 6;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Loading companies cache.");

    data->companyCache = cacheEngine_load(getCompanyFunctions(), data->companyDaos,
        companyDirectory, numServers, numCompanyCacheEntries, numLoadingThreads);
    if(cacheEngine_isNull(data->companyCache))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_load(companies) failed.");
        cleanupRequestProcessingData(data);
        return 7;
    }

    EndInlinedProfiling(PE_COMPANY_LOADING);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Initializing search engine.");

    data->searchEngine = searchEngine_init();
    if(searchEngine_isNull(data->searchEngine))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchEngine_init() failed.");
        cleanupRequestProcessingData(data);
        return 8;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Starting search engine.");

    int ret = searchEngine_start(data->searchEngine, data->profileCache, data->companyCache,
        numSearchThreads, searchResultsExpirationInMinutes);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchEngine_start() returned %d.", ret);
        cleanupRequestProcessingData(data);
        return 9;
    }

    LOG_MESSAGE(INFO_LOG_MSG, "End.");

    return 0;
}

static void cleanupRequestProcessingData(RequestProcessingData_t* data)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    searchEngine_stop(data->searchEngine);
    LOG_MESSAGE(DEBUG_LOG_MSG, "searchEngine_stop() completed.");

    cacheEngine_free(data->companyCache);
    LOG_MESSAGE(DEBUG_LOG_MSG, "cacheEngine_free(companies) completed.");

    daos_free(data->companyDaos);
    LOG_MESSAGE(DEBUG_LOG_MSG, "daos_free(companies) completed.");

    fileLockList_free(data->companyFileList);
    LOG_MESSAGE(DEBUG_LOG_MSG, "fileLockList_free(companies) completed.");

    cacheEngine_free(data->profileCache);
    LOG_MESSAGE(DEBUG_LOG_MSG, "cacheEngine_free(profiles) completed.");

    daos_free(data->profileDaos);
    LOG_MESSAGE(DEBUG_LOG_MSG, "daos_free(profiles) completed.");

    fileLockList_free(data->profileFileList);
    LOG_MESSAGE(DEBUG_LOG_MSG, "fileLockList_free(profiles) completed.");

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

        LOG_MESSAGE(INFO_LOG_MSG, "Connection selected for work.");

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

        while(closeConnection == false)
        {
            ret = processRequest(data->numServers, data->serverId, data->numProfilesPerFile, data->numCompaniesPerFile,
                data->profileFileList, data->profileDaos, data->profileCache,
                data->companyFileList, data->companyDaos, data->companyCache,
                data->searchEngine, sockt, data->numConnections,
                data->requestArrivalTimeout, data->connectionTimeout, &closeConnection);
            if(ret != 0) {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: processRequest() returned %d.", ret);
            }
        }

        close(sockt);
        LOG_MESSAGE(INFO_LOG_MSG, "Connection closed.");
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
// CacheEngine_t
//

static pthread_t* startCacheThread(CacheEngine_t cacheEngine, uint16_t updatePeriod)
{
    pthread_t* threadId = (pthread_t*) malloc(sizeof(pthread_t));
    if(threadId == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(threadId, size = %zu bytes) failed.",
            sizeof(pthread_t));
        return NULL;
    }

    CacheThreadArg_t* threadArg = malloc(sizeof(CacheThreadArg_t));
    if(threadArg == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(threadArg, size = %zu bytes) failed.",
            sizeof(CacheThreadArg_t));
    	free(threadId);
        return NULL;
    }

    threadArg->cacheEngine = cacheEngine;
    threadArg->updatePeriod = updatePeriod;

    if(pthread_create(threadId, NULL, cacheThread, threadArg) != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: pthread_create() failed. errno = %d (\"%s\").",
            errno, strerror(errno));
        free(threadArg);
    	free(threadId);
        return NULL;
    }

    return threadId;
}

static void* cacheThread(void* arg)
{
    pthread_cleanup_push(threadCleanupHandler, "cacheThread exited.");

    LOG_MESSAGE(INFO_LOG_MSG, "Started.");

    CacheThreadArg_t* cacheThreadArg = (CacheThreadArg_t*) arg;

    int ret = cacheEngine_run(cacheThreadArg->cacheEngine, cacheThreadArg->updatePeriod);
    free(cacheThreadArg);

    if(ret != 0) {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_run() returned %d.", ret);
    }

    pthread_cleanup_pop(1);

    return NULL;
}

//
// Others
//

static void threadCleanupHandler(void *arg)
{
    const char* msg = (const char*) arg;
    LOG_MESSAGE(INFO_LOG_MSG, msg);
}
