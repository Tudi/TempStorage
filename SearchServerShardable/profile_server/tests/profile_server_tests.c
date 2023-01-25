#include <profile_server.h>
#include <profile_client.h>
#include <request_response_definitions.h>
#include <profile_test_data.h>
#include <assert_mt.h>
#include <utils.h>
#include <macro_utils.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

//
// Variables
//

static ProfileServer_t profileServer = ProfileServer_NULL;

static DaosId_t serverId      = 0;
static DaosCount_t numServers = 2;

static const char* profileDirectory    = "./";
static uint32_t numProfileCacheEntries = 24;
static uint32_t numProfilesPerFile     = 8;
static const char* companyDirectory    = "./";
static uint32_t numCompanyCacheEntries = 24;
static uint32_t numCompaniesPerFile    = 8;

static const char* serverAddress                 = "127.0.0.1";
static uint16_t serverPort                       = 20000;
static uint32_t requestArrivalTimeout            = 5000;
static uint32_t connectionTimeout                = 2000;
static uint16_t profileCacheUpdatePeriodInSecs   = 3;
static uint16_t companyCacheUpdatePeriodInSecs   = 4;
static uint16_t numConnections                   = 3;
static uint16_t numSearchThreads                 = 2;
static uint16_t numLoadingThreads                = 2;
static uint16_t searchResultsExpirationInMinutes = 2;

static uint16_t testExecutionInSecs = 10;

int clientSocket1 = -1;
int clientSocket2 = -1;
int clientSocket3 = -1;

//
// Prototypes
//

int test_ProfileServer_succeeds();
void test_ProfileServer_cleanup();

void* serverThread(void* arg);
void* client1Thread(void* arg);
void* client2Thread(void* arg);
void* client3Thread(void* arg);

void serverThreadCleanupHandler(void *arg);
void clientThreadBufferReleaser(void *arg);
void clientThreadSocketCloser(void *arg);
void clientThreadEnd(void *arg);

//
// Tests
//

int main(int argc, char* argv[])
{
    atexit(test_ProfileServer_cleanup);

    logger_setLogLevel(DEBUG_LOG_MSG);

    return test_ProfileServer_succeeds();
}

//
// Functions
//

int test_ProfileServer_succeeds()
{
    LOG_MESSAGE(INFO_LOG_MSG, "Test %s started...", __func__);

    signal(SIGPIPE, SIG_IGN);

    pthread_t serverThreadId;
    
    LOG_MESSAGE(INFO_LOG_MSG, "Starting server thread...");
    ASSERT_MT_INT_EQUAL(0, pthread_create(&serverThreadId, NULL, serverThread, NULL));
    LOG_MESSAGE(INFO_LOG_MSG, "Started server thread.");

    uint32_t timeForServerToStart = 1;
    LOG_MESSAGE(INFO_LOG_MSG, "Sleeping for %u seconds to give server time to start.", timeForServerToStart);
    sleep(timeForServerToStart);
    LOG_MESSAGE(INFO_LOG_MSG, "Woke up.");

    pthread_t client1ThreadId, client2ThreadId, client3ThreadId;

    LOG_MESSAGE(INFO_LOG_MSG, "Starting client threads...");
    ASSERT_MT_INT_EQUAL(0, pthread_create(&client1ThreadId, NULL, client1Thread, NULL));
    ASSERT_MT_INT_EQUAL(0, pthread_create(&client2ThreadId, NULL, client2Thread, NULL));
    ASSERT_MT_INT_EQUAL(0, pthread_create(&client3ThreadId, NULL, client3Thread, NULL));
    LOG_MESSAGE(INFO_LOG_MSG, "Started client threads.");

    LOG_MESSAGE(INFO_LOG_MSG, "Sleeping for %u seconds so test threads can work.", testExecutionInSecs);
    sleep(testExecutionInSecs);
    LOG_MESSAGE(INFO_LOG_MSG, "Woke up.");

    LOG_MESSAGE(INFO_LOG_MSG, "Stopping server.");
    profileServer_stop(profileServer);
    profileServer = ProfileServer_NULL;

    LOG_MESSAGE(INFO_LOG_MSG, "Joining threads.");
    pthread_join(client3ThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread 3 joined.");
    pthread_join(client2ThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread 2 joined.");
    pthread_join(client1ThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread 1 joined.");
    pthread_join(serverThreadId, NULL);
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread joined.");

    LOG_MESSAGE(INFO_LOG_MSG, programError ? "Test failed." : "Test passed.");

    return programError ? EINVAL : 0;
}

void test_ProfileServer_cleanup()
{
    profileServer_stop(profileServer);

    uint32_t timeForServerToStop = 1;
    LOG_MESSAGE(INFO_LOG_MSG, "Sleeping for %u seconds to give server time to stop.", timeForServerToStop);
    sleep(timeForServerToStop);
    LOG_MESSAGE(INFO_LOG_MSG, "Woke up.");
}

//
// Threads
//

void* serverThread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread begin.");

    pthread_cleanup_push(serverThreadCleanupHandler, NULL);

    profileServer = profileServer_init();
    ASSERT_MT_INT_EQUAL(0, profileServer_isNull(profileServer));
    
    int ret = profileServer_run(profileServer, serverId, numServers,
        profileDirectory, numProfileCacheEntries, numProfilesPerFile,
        companyDirectory, numCompanyCacheEntries, numCompaniesPerFile,
        serverPort, requestArrivalTimeout, connectionTimeout, numConnections, numSearchThreads, numLoadingThreads,
        profileCacheUpdatePeriodInSecs, companyCacheUpdatePeriodInSecs, searchResultsExpirationInMinutes);
    profileServer = ProfileServer_NULL;
    ASSERT_MT_INT_EQUAL(0, ret);

    pthread_cleanup_pop(1);

    return NULL;
}

void* client1Thread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread begin.");

    pthread_cleanup_push(clientThreadEnd, NULL);

    // Establish connection

    ASSERT_MT_INT_EQUAL(0, connectToServer(serverAddress, serverPort, &clientSocket1));

    pthread_cleanup_push(clientThreadSocketCloser, (void*) &clientSocket1);

    // Wait until connection is dropped.

    uint32_t timeForConnectionToDrop = (requestArrivalTimeout / 1000) + 1;
    LOG_MESSAGE(INFO_LOG_MSG, "Sleeping for %u seconds so server will drop connection.", timeForConnectionToDrop);
    sleep(timeForConnectionToDrop);
    LOG_MESSAGE(INFO_LOG_MSG, "Woke up.");

    // Ping server

    uint8_t responseCode = NULL_RESPONSE;

    ASSERT_MT_INT_NOT_EQUAL(0, pingServer(clientSocket1, &responseCode));
    ASSERT_MT_INT_NOT_EQUAL(SUCCESS_RESPONSE, responseCode);
    clientSocket1 = -1;

    pthread_cleanup_pop(0); // Close socket.
    pthread_cleanup_pop(1); // Print final message.

    return NULL;
}

void* client2Thread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread begin.");

    pthread_cleanup_push(clientThreadEnd, NULL);

    LOG_MESSAGE(INFO_LOG_MSG, "responseBuffer = calloc(capacity = %lu, item size = %zu bytes).",
        REQUEST_MAX_SIZE, sizeof(char));
    char* responseBuffer = calloc(REQUEST_MAX_SIZE, sizeof(char));
    ASSERT_MT_NOT_NULL(responseBuffer);

    pthread_cleanup_push(clientThreadBufferReleaser, responseBuffer);

    uint8_t responseCode = NULL_RESPONSE;

    // Establish connection

    ASSERT_MT_INT_EQUAL(0, connectToServer(serverAddress, serverPort, &clientSocket2));

    pthread_cleanup_push(clientThreadSocketCloser, (void*) &clientSocket2);

    // Save profile 1

    const char* profilePersistent1_JsonString = PROFILE_PERSISTENT_1_JSON_STRING;

    LOG_MESSAGE(INFO_LOG_MSG, "sendProfileToServer(profile1).");

    ASSERT_MT_INT_EQUAL(0, sendProfileToServer(clientSocket2, profilePersistent1_JsonString,
        strlen(profilePersistent1_JsonString) + 1, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);

    LOG_MESSAGE(INFO_LOG_MSG, "sendProfileToServer(profile1) passed.");

    // Get profile 1

    uint32_t receivedJsonSize = 0;

    LOG_MESSAGE(INFO_LOG_MSG, "getProfileFromServer(profile1).");

    responseCode = NULL_RESPONSE;

    ASSERT_MT_INT_EQUAL(0, getProfileFromServer(clientSocket2, PROFILE_PERSISTENT_1_ID,
        &responseCode, responseBuffer, REQUEST_MAX_SIZE, &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_STR_EQUAL(profilePersistent1_JsonString, responseBuffer);

    LOG_MESSAGE(INFO_LOG_MSG, "getProfileFromServer(profile1) passed.");

    // Save profile 5

    const char* profilePersistent5_JsonString = PROFILE_PERSISTENT_5_JSON_STRING;

    LOG_MESSAGE(INFO_LOG_MSG, "sendProfileToServer(profile5).");

    responseCode = NULL_RESPONSE;

    ASSERT_MT_INT_EQUAL(0, sendProfileToServer(clientSocket2, profilePersistent5_JsonString,
        strlen(profilePersistent5_JsonString) + 1, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);

    LOG_MESSAGE(INFO_LOG_MSG, "sendProfileToServer(profile5) passed.");

    // Get profile 5

    LOG_MESSAGE(INFO_LOG_MSG, "getProfileFromServer(profile5).");

    responseCode = NULL_RESPONSE;
    memset(responseBuffer, 0x0, REQUEST_MAX_SIZE);
    receivedJsonSize = 0;

    ASSERT_MT_INT_EQUAL(0, getProfileFromServer(clientSocket2, PROFILE_PERSISTENT_5_ID,
        &responseCode, responseBuffer, REQUEST_MAX_SIZE, &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_STR_EQUAL(profilePersistent5_JsonString, responseBuffer);

    LOG_MESSAGE(INFO_LOG_MSG, "getProfileFromServer(profile5) passed.");

    // Close connection

    LOG_MESSAGE(INFO_LOG_MSG, "endConnectionToServer().");

    ASSERT_MT_INT_EQUAL(0, endConnectionToServer(clientSocket2, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    clientSocket2 = -1;

    LOG_MESSAGE(INFO_LOG_MSG, "endConnectionToServer() passed.");

    pthread_cleanup_pop(0); // Close socket.
    pthread_cleanup_pop(1); // Free buffer.
    pthread_cleanup_pop(1); // Print final message.

    return NULL;
}

#define PROFILE_PERSISTENT_INVALID_1_ID 1011

void* client3Thread(void* arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread begin.");

    pthread_cleanup_push(clientThreadEnd, NULL);

    LOG_MESSAGE(INFO_LOG_MSG, "responseBuffer = calloc(capacity = %lu, item size = %zu bytes).",
        REQUEST_MAX_SIZE, sizeof(char));
    char* responseBuffer = calloc(REQUEST_MAX_SIZE, sizeof(char));
    ASSERT_MT_NOT_NULL(responseBuffer);

    pthread_cleanup_push(clientThreadBufferReleaser, responseBuffer);

    uint8_t responseCode = NULL_RESPONSE;

    // Establish connection

    ASSERT_MT_INT_EQUAL(0, connectToServer(serverAddress, serverPort, &clientSocket3));

    pthread_cleanup_push(clientThreadSocketCloser, (void*) &clientSocket3);

    // Save profile 2

    const char* profilePersistent2_JsonString = PROFILE_PERSISTENT_2_JSON_STRING;

    LOG_MESSAGE(INFO_LOG_MSG, "sendProfileToServer(profile2).");

    ASSERT_MT_INT_EQUAL(0, sendProfileToServer(clientSocket3, profilePersistent2_JsonString,
        strlen(profilePersistent2_JsonString) + 1, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);

    LOG_MESSAGE(INFO_LOG_MSG, "sendProfileToServer(profile2) passed.");

    // Get profile 2

    uint32_t receivedJsonSize = 0;

    LOG_MESSAGE(INFO_LOG_MSG, "getProfileFromServer(profile2).");

    responseCode = NULL_RESPONSE;
    memset(responseBuffer, 0x0, REQUEST_MAX_SIZE);
    receivedJsonSize = 0;

    ASSERT_MT_INT_EQUAL(0, getProfileFromServer(clientSocket3, PROFILE_PERSISTENT_2_ID,
        &responseCode, responseBuffer, REQUEST_MAX_SIZE, &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_STR_EQUAL(profilePersistent2_JsonString, responseBuffer);

    LOG_MESSAGE(INFO_LOG_MSG, "getProfileFromServer(profile2) passed.");

    // Save profiles 3 and 4

    const char* profilesPersistent3_and_4_JsonString
        = "{ \"profiles\" : [" PROFILE_PERSISTENT_3_JSON_STRING "," PROFILE_PERSISTENT_4_JSON_STRING "] }";

    LOG_MESSAGE(INFO_LOG_MSG, "sendMultipleProfilesToServer(profile3, profile4).");

    const char* expectedResults = "{ \"not_saved\": [ ] }";
    receivedJsonSize = 0;

    responseCode = NULL_RESPONSE;
    memset(responseBuffer, 0x0, REQUEST_MAX_SIZE);

    ASSERT_MT_INT_EQUAL(0, sendMultipleProfilesToServer(clientSocket3, profilesPersistent3_and_4_JsonString,
        strlen(profilesPersistent3_and_4_JsonString) + 1, &responseCode, responseBuffer, REQUEST_MAX_SIZE,
        &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_TRUE(receivedJsonSize > 0);
    ASSERT_MT_TRUE(receivedJsonSize <= REQUEST_MAX_SIZE);
    ASSERT_MT_STR_EQUAL(expectedResults, responseBuffer);

    LOG_MESSAGE(INFO_LOG_MSG, "sendMultipleProfilesToServer(profile3, profile4) passed.");

    LOG_MESSAGE(INFO_LOG_MSG, "Sleeping for %u seconds to give cache time to update.", profileCacheUpdatePeriodInSecs);
    sleep(profileCacheUpdatePeriodInSecs);
    LOG_MESSAGE(INFO_LOG_MSG, "Woke up.");

    // Ping server

    ASSERT_MT_INT_EQUAL(0, pingServer(clientSocket3, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);

    // Perform search

    LOG_MESSAGE(INFO_LOG_MSG, "startSearchInServer().");

    const char* jsonSearch = "{ \"role_code\": 1, \"localities\": [ ], \"organization_id\": 0, "
        "\"filters\": [ { \"filter\": \"linkedin_include\", \"modifier\": \"\","
        "\"text_value\": \"\", \"code_value\": \"22#,#24\", \"range_low\": 0,"
        " \"range_high\": 0 } ] }";
    uint32_t jsonSearchSize           = strlen(jsonSearch) + 1;
    const uint8_t* similaritiesScores = NULL;
    uint32_t similaritiesScoresSize   = 0;
    uint32_t resultsLimit = 5;
    uint32_t searchId     = 0;

    responseCode = NULL_RESPONSE;

    ASSERT_MT_INT_EQUAL(0, startSearchInServer(clientSocket3, jsonSearch, jsonSearchSize,
        similaritiesScores, similaritiesScoresSize, resultsLimit, &responseCode, &searchId));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_INT_NOT_EQUAL(0, searchId);

    LOG_MESSAGE(INFO_LOG_MSG, "startSearchInServer() passed.");

    LOG_MESSAGE(INFO_LOG_MSG, "getSearchStatusFromServer().");

    uint8_t completionPercentage = 0;

    responseCode = NULL_RESPONSE;

    uint32_t searchTime = 1;
    LOG_MESSAGE(INFO_LOG_MSG, "Sleeping for %u seconds to give server to complete search.", searchTime);
    sleep(searchTime);
    LOG_MESSAGE(INFO_LOG_MSG, "Woke up.");

    ASSERT_MT_INT_EQUAL(0, getSearchStatusFromServer(clientSocket3, searchId, &responseCode,
        &completionPercentage));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_INT_EQUAL(100, completionPercentage);

    LOG_MESSAGE(INFO_LOG_MSG, "getSearchStatusFromServer() passed.");

    LOG_MESSAGE(INFO_LOG_MSG, "getSearchResultsFromServer().");

    expectedResults = "{ \"scores\": [ { \"r\": 1, \"p\": 22, \"t\": 10000, \"hs\": 10000, "
        "\"cs\": 0, \"e\": 0, \"s\": 0, \"j\": 0, \"rel\": 0, \"filters\": [ { \"filter\": "
        "\"linkedin_include\", \"text_value\": \"22\" } ] }, { \"r\": 1, \"p\": 24, \"t\": 10000, "
        "\"hs\": 10000, \"cs\": 0, \"e\": 0, \"s\": 0, \"j\": 0, \"rel\": 0, \"filters\": "
        "[ { \"filter\": \"linkedin_include\", \"text_value\": \"24\" } ] } ], \"total_scored\": 2 }";
    receivedJsonSize = 0;

    responseCode = NULL_RESPONSE;
    memset(responseBuffer, 0x0, REQUEST_MAX_SIZE);

    ASSERT_MT_INT_EQUAL(0, getSearchResultsFromServer(clientSocket3, searchId, &responseCode,
        responseBuffer, REQUEST_MAX_SIZE, &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_TRUE(receivedJsonSize > 0);
    ASSERT_MT_TRUE(receivedJsonSize <= REQUEST_MAX_SIZE);
    ASSERT_MT_STR_EQUAL(expectedResults, responseBuffer);

    LOG_MESSAGE(INFO_LOG_MSG, "getSearchResultsFromServer() passed.");

    LOG_MESSAGE(INFO_LOG_MSG, "endSearchInServer().");

    responseCode = NULL_RESPONSE;

    ASSERT_MT_INT_EQUAL(0, endSearchInServer(clientSocket3, searchId, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);

    LOG_MESSAGE(INFO_LOG_MSG, "endSearchInServer() passed.");

    // Get multiple profiles 2, 3 and 4

    LOG_MESSAGE(INFO_LOG_MSG,
        "getMultipleProfilesFromServer([profile2, profile3, invalidProfile1, profile4]).");

    const char* expectedProfilesResponse = "{ \"profiles\": [ " PROFILE_PERSISTENT_2_JSON_STRING
        ", " PROFILE_PERSISTENT_3_JSON_STRING ", " PROFILE_PERSISTENT_4_JSON_STRING " ],"
        " \"not_retrieved\": [ " MACRO_TO_STRING(PROFILE_PERSISTENT_INVALID_1_ID) " ] }";
    const DaosId_t profileIds[] = { PROFILE_PERSISTENT_2_ID, PROFILE_PERSISTENT_3_ID,
        PROFILE_PERSISTENT_INVALID_1_ID, PROFILE_PERSISTENT_4_ID };
    uint32_t numProfileIds = ARRAY_COUNT(profileIds);

    responseCode = NULL_RESPONSE;
    memset(responseBuffer, 0x0, REQUEST_MAX_SIZE);
    receivedJsonSize = 0;

    ASSERT_MT_INT_EQUAL(0, getMultipleProfilesFromServer(clientSocket3, profileIds, numProfileIds,
        &responseCode, responseBuffer, REQUEST_MAX_SIZE, &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_TRUE(receivedJsonSize > 0);
    ASSERT_MT_TRUE(receivedJsonSize <= REQUEST_MAX_SIZE);

    ASSERT_MT_STR_EQUAL(expectedProfilesResponse, responseBuffer);

    LOG_MESSAGE(INFO_LOG_MSG,
        "getMultipleProfilesFromServer([profile2, invalidProfile1, profile4]) passed.");

    // Get statistics

    const char* expectedStatistics = "{ \"config\": { \"client_connections\": 3 }, "
        "\"persistent_items\": { \"profiles\": { \"min\": 16, \"max\": 26, \"total\": 5 }, "
        "\"companies\": { \"min\": 0, \"max\": 0, \"total\": 0 } } }";

    responseCode = NULL_RESPONSE;
    memset(responseBuffer, 0x0, REQUEST_MAX_SIZE);
    receivedJsonSize = 0;

    ASSERT_MT_INT_EQUAL(0, getSystemInfoFromServer(clientSocket3, &responseCode,
        responseBuffer, REQUEST_MAX_SIZE, &receivedJsonSize));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    ASSERT_MT_STR_EQUAL(expectedStatistics, responseBuffer);
    ASSERT_MT_TRUE(receivedJsonSize > 0);
    ASSERT_MT_TRUE(receivedJsonSize <= REQUEST_MAX_SIZE);

    // Close connection

    LOG_MESSAGE(INFO_LOG_MSG, "endConnectionToServer().");

    ASSERT_MT_INT_EQUAL(0, endConnectionToServer(clientSocket3, &responseCode));
    ASSERT_MT_INT_EQUAL(SUCCESS_RESPONSE, responseCode);
    clientSocket3 = -1;

    LOG_MESSAGE(INFO_LOG_MSG, "endConnectionToServer() passed.");

    pthread_cleanup_pop(0); // Close socket.
    pthread_cleanup_pop(1); // Free buffer.
    pthread_cleanup_pop(1); // Print final message.

    return NULL;
}

void serverThreadCleanupHandler(void *arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Server thread end.");
}

void clientThreadBufferReleaser(void *arg)
{
    free(arg);
}

void clientThreadSocketCloser(void *arg)
{
    int* sockt = (int*) arg;

    if(*sockt > 0) {
        close(*sockt);
    }
}

void clientThreadEnd(void *arg)
{
    LOG_MESSAGE(INFO_LOG_MSG, "Client thread end.");
}
