#include <request_processing.h>
#include <daos.h>
#include <profile_persistent.h>
#include <company.h>
#include <profile_functions.h>
#include <company_functions.h>
#include <request_response_definitions.h>
#include <composite_score.h>
#include <search_engine.h>
#include <search_criteria.h>
#include <system_info_data.h>
#include <utils.h>
#include <k_utils.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <json_tokener.h>
#include <profiling.h>

//
// Types
//

typedef struct
{
    struct timeval original;
    struct timeval remaining;
} Timeout_t;

//
// Constants
//

static const char* requestDescriptions[MAX_REQUEST_CODE + 1] = {
    "INVALID",
    "GET_PROFILE",
    "SAVE_PROFILE",
    "START_SEARCH",
    "GET_SEARCH_STATUS",
    "GET_SEARCH_RESULTS",
    "GET_COMPANY",
    "SAVE_COMPANY",
    "END_SEARCH",
    "GET_MULTIPLE_PROFILES",
    "END_CONNECTION",
    "PING",
    "GET_SYSTEM_INFO",
    "SET_SERVER_LOG_LEVEL",
    "SAVE_MULTIPLE_PROFILES",
    "SAVE_MULTIPLE_COMPANIES",
};

static const char* profilesList_Key  = "profiles";
static const char* companiesList_Key = "companies";

//
// Local prototypes
//

// Request processing

static int processGetRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList,
    Daos_t daos, const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processSaveRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList, Daos_t daos,
    CacheEngine_t cacheEngine, const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection, ProfilingEvents saveEventType,
    const DaosCount_t numServers, const DaosId_t serverId);
static int processGetMultipleRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList,
    Daos_t daos, const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int retrieveItemAndReturnJson(DaosCount_t numItemsPerFile, FileLockList_t fileList, Daos_t daos,
    const ItemFunctions_t* itemFunctions, DaosId_t id, struct json_object** jsonObj);
static int processSaveMultipleRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList,
    Daos_t daos, CacheEngine_t cacheEngine, const DaosCount_t numServers, const DaosId_t serverId,
    const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, const char* jsonListKey, bool* closeConnection,
    ProfilingEvents saveEventType);
static int sendResponseWithIds(const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    uint8_t responseCode, void* persistentList, uint32_t persistentListFirstIndex,  uint32_t persistentListCount,
    struct json_object* responseJsonObj, struct json_object* notSavedJsonObj);

static int processStartSearchRequest(SearchEngine_t searchEngine, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processGetSearchStatusRequest(SearchEngine_t searchEngine, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processGetSearchResultsRequest(SearchEngine_t searchEngine, int sockt,
    Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processEndSearchRequest(SearchEngine_t searchEngine, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection);
static int processEndConnectionRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection);
static int processPingRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection);
static int processGetSystemInfoRequest(CacheEngine_t profileCache, CacheEngine_t companyCache, uint16_t numConnections,
    int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection);
static int processSetLogLevelRequest(int sockt, Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize,
    bool* closeConnection);

static int sendSimpleResponse(int sockt, Timeout_t* timeout, uint8_t responseCode);

// Socket

static int sendToSocket(int sockt, Timeout_t* timeout, const void* buffer, ssize_t bufferSize);
static int receiveFromSocket(int sockt, Timeout_t* timeout, void* buffer, ssize_t bufferSize);

//
// External interface
//

int processRequest(DaosCount_t numServers, DaosId_t serverId,
    DaosCount_t numProfilesPerFile, DaosCount_t numCompaniesPerFile,
    FileLockList_t profileFileList, Daos_t profileDaos, CacheEngine_t profileCache,
    FileLockList_t companyFileList, Daos_t companyDaos, CacheEngine_t companyCache,
    SearchEngine_t searchEngine, int sockt, uint16_t numConnections, uint32_t requestArrivalTimeout,
    uint32_t connectionTimeout, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "serverId = %lu.", serverId);

    *closeConnection = false;

    Timeout_t workingReqArrivalTimeout = {
        { .tv_sec = requestArrivalTimeout / 1000,.tv_usec = (requestArrivalTimeout % 1000) * 1000 },
        { .tv_sec = requestArrivalTimeout / 1000,.tv_usec = (requestArrivalTimeout % 1000) * 1000 }
    };

    // Process request size.

    uint32_t requestSizeBuffer = 0;

    int ret = receiveFromSocket(sockt, &workingReqArrivalTimeout, &requestSizeBuffer,
        sizeof(requestSizeBuffer));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveFromSocket(request size, size = %zu) "
            "returned %d.", sizeof(requestSizeBuffer), ret);
        *closeConnection = true;
        return 1;
    }

    uint32_t requestSize = ntohl(requestSizeBuffer);
    LOG_MESSAGE(DEBUG_LOG_MSG, "Request size = %u bytes.", requestSize);

    if(requestSize == 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: request size is 0 bytes.");
        *closeConnection = true;
        return 2;
    }

    if(requestSize > REQUEST_MAX_SIZE)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: request size (%u bytes) is greater than "
            "maximum (%u bytes).", requestSize, REQUEST_MAX_SIZE);
        *closeConnection = true;
        return 3;
    }

    uint8_t *requestBuffer = malloc(requestSize);
    if (requestBuffer == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to allocate %d bytes.", requestSize);
        *closeConnection = true;
        return 4;
    }

    // Receive request body.

    Timeout_t workingConnTimeout = {
        { .tv_sec = connectionTimeout / 1000, .tv_usec = (connectionTimeout % 1000) * 1000 },
        { .tv_sec = connectionTimeout / 1000, .tv_usec = (connectionTimeout % 1000) * 1000 }
    };

    ret = receiveFromSocket(sockt, &workingConnTimeout, requestBuffer, requestSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveFromSocket(request body, size = %lu bytes) "
            "returned %d.", requestSize, ret);
        free(requestBuffer);
        *closeConnection = true;
        return 5;
    }

    // Process request.

    uint8_t requestCode = requestBuffer[0];

    if((requestCode == 0) || (requestCode > MAX_REQUEST_CODE))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unrecognized request code (%x).", requestCode);
        free(requestBuffer);
        *closeConnection = true;
        return 0;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Request (%hu) \"%s\".", (unsigned short) requestCode,
        requestDescriptions[requestCode]);

    const char* functionCalled = NULL;

    switch(requestCode)
    {
    case GET_PROFILE:
        functionCalled = "processGetRequest()";
        ret = processGetRequest(numProfilesPerFile, profileFileList, profileDaos, getProfileFunctions(),
            sockt, &workingConnTimeout, &requestBuffer[1], requestSize - 1, closeConnection);
        break;

    case SAVE_PROFILE:
        functionCalled = "processSaveRequest()";
        ret = processSaveRequest(numProfilesPerFile, profileFileList, profileDaos, profileCache,
            getProfileFunctions(), sockt, &workingConnTimeout, &requestBuffer[1], requestSize - 1,
            closeConnection, PE_PROFILE_INSERT, numServers, serverId);
        break;

    case START_SEARCH:
        functionCalled = "processStartSearchRequest()";
        ret = processStartSearchRequest(searchEngine, sockt, &workingConnTimeout, &requestBuffer[1],
            requestSize - 1, closeConnection);
        break;

    case GET_SEARCH_STATUS:
        functionCalled = "processGetSearchStatusRequest()";
        ret = processGetSearchStatusRequest(searchEngine, sockt, &workingConnTimeout,
            &requestBuffer[1], requestSize - 1, closeConnection);
        break;

    case GET_SEARCH_RESULTS:
        functionCalled = "processGetSearchResultsRequest()";
        ret = processGetSearchResultsRequest(searchEngine, sockt, &workingConnTimeout,
            &requestBuffer[1], requestSize - 1, closeConnection);
        break;

    case GET_COMPANY:
        functionCalled = "processGetRequest()";
        ret = processGetRequest(numCompaniesPerFile, companyFileList, companyDaos,
            getCompanyFunctions(), sockt, &workingConnTimeout, &requestBuffer[1],
            requestSize - 1, closeConnection);
        break;

    case SAVE_COMPANY: 
        functionCalled = "processSaveRequest()";
        ret = processSaveRequest(numCompaniesPerFile, companyFileList, companyDaos, companyCache,
            getCompanyFunctions(), sockt, &workingConnTimeout, &requestBuffer[1], requestSize - 1,
            closeConnection, PE_COMPANY_INSERT, numServers, serverId);
        break;

    case END_SEARCH:
        functionCalled = "processEndSearchRequest()";
        ret = processEndSearchRequest(searchEngine, sockt, &workingConnTimeout, &requestBuffer[1],
            requestSize - 1, closeConnection);
        break;

    case GET_MULTIPLE_PROFILES:
        functionCalled = "processGetMultipleRequest()";
        ret = processGetMultipleRequest(numProfilesPerFile, profileFileList, profileDaos,
            getProfileFunctions(), sockt, &workingConnTimeout, &requestBuffer[1], requestSize - 1,
            closeConnection);
        break;

    case END_CONNECTION:
        functionCalled = "processEndConnectionRequest()";
        ret = processEndConnectionRequest(sockt, &workingConnTimeout, requestSize - 1, closeConnection);
        break;

    case PING:
        functionCalled = "processPingRequest()";
        ret = processPingRequest(sockt, &workingConnTimeout, requestSize - 1, closeConnection);
        break;

    case GET_SYSTEM_INFO:
        functionCalled = "processGetSystemInfoRequest()";
        ret = processGetSystemInfoRequest(profileCache, companyCache, numConnections, sockt, &workingConnTimeout,
            requestSize - 1, closeConnection);
        break;

    case SET_LOG_LEVEL:
        functionCalled = "processSetLogLevelRequest()";
        ret = processSetLogLevelRequest(sockt, &workingConnTimeout, &requestBuffer[1], requestSize - 1,
            closeConnection);
        break;

    case SAVE_MULTIPLE_PROFILES:
        functionCalled = "processSaveMultipleRequest()";
        ret = processSaveMultipleRequest(numProfilesPerFile, profileFileList, profileDaos, profileCache,
            numServers, serverId, getProfileFunctions(), sockt, &workingConnTimeout, &requestBuffer[1],
            requestSize - 1, profilesList_Key, closeConnection, PE_PROFILE_INSERT);
        break;

    case SAVE_MULTIPLE_COMPANIES:
        functionCalled = "processSaveMultipleRequest()";
        ret = processSaveMultipleRequest(numCompaniesPerFile, companyFileList, companyDaos, companyCache,
            numServers, serverId, getCompanyFunctions(), sockt, &workingConnTimeout, &requestBuffer[1],
            requestSize - 1, companiesList_Key, closeConnection, PE_COMPANY_INSERT);
        break;

    default:
        functionCalled = "Invalid request function";
        ret = 9999;
        break;
    }

    free(requestBuffer);

    if(ret == 0)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Succeeded.");
        return 0;
    }
    else
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: %s returned %d.", functionCalled, ret);
        return 7;
    }
}

//
// Local functions
//

//
// Request processing
//

static int processGetRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList, Daos_t daos,
    const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process id.

    if(bufferSize != sizeof(DaosId_t))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != sizeof(DaosId_t) (%lu).",
            bufferSize, sizeof(DaosId_t));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    const DaosId_t* networkId = (const DaosId_t*) buffer;
    DaosId_t id = NETWORK_TO_DAOS_ID(*networkId);

    LOG_MESSAGE(DEBUG_LOG_MSG, "id = %lu.", id);

    // Retrieve item.

    struct json_object* jsonObj = NULL;

    int ret = retrieveItemAndReturnJson(numItemsPerFile, fileList, daos, itemFunctions, id, &jsonObj);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: retrieveItemAndReturnJson(id = %lu) returned %d.", id, ret);
        json_object_put(jsonObj);
        sendSimpleResponse(sockt, timeout, ERROR_NONEXISTENT_ITEM_RESPONSE);
        return 2;
    }

    // Generate JSON string.

    size_t jsonStrLength = 0;
    const char* jsonStr = json_object_to_json_string_length(jsonObj, JSON_C_TO_STRING_SPACED, &jsonStrLength);

    // Send response.

    // Send response size.

    uint8_t responseCode = SUCCESS_RESPONSE;
    uint32_t responseSize = htonl(sizeof(responseCode) + jsonStrLength);

    ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) "
            "returned %d.", sizeof(responseSize), ret);
        json_object_put(jsonObj);
        return 3;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) "
            "returned %d.", sizeof(responseCode), ret);
        json_object_put(jsonObj);
        return 4;
    }

    // Send JSON string.

    ret = sendToSocket(sockt, timeout, jsonStr, jsonStrLength);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(json response, size = %zu) "
            "returned %d.", jsonStrLength, ret);
        json_object_put(jsonObj);
        return 5;
    }

    json_object_put(jsonObj);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
   
    return 0;
}

static int processSaveRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList, Daos_t daos,
    CacheEngine_t cacheEngine, const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection, ProfilingEvents saveEventType,
    const DaosCount_t numServers, const DaosId_t serverId)
{
    DisableNonUsedWarning(saveEventType);
    StartInlinedProfiling(saveEventType);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Parse JSON string received.

    struct json_tokener* jsonTokener = json_tokener_new();

    struct json_object* jsonObj = json_tokener_parse_ex(jsonTokener, (const char*) buffer, (int) bufferSize);
    if(jsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_tokener_parse_ex() failed. Error = \"%s\".",
            json_tokener_error_desc(json_tokener_get_error(jsonTokener)));
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: JSON buffer (size = %u) = %s\n\n", bufferSize, (const char*) buffer);
        json_tokener_free(jsonTokener);
        sendSimpleResponse(sockt, timeout, ERROR_JSON_LIBRARY);
        return 1;
    }

    json_tokener_free(jsonTokener);

    // Convert JSON object to item.

    void* item = NULL;

    int ret = itemFunctions->jsonToPersistentItem(&item, jsonObj);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallItem() returned %d.", ret);
        json_object_put(jsonObj);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 2;
    }

    json_object_put(jsonObj);

    int32_t itemId = itemFunctions->getPersistentItemId(item);
    if (itemId < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: id %d is negative.", itemId);
        itemFunctions->freePersistentItem(item);
        sendSimpleResponse(sockt, timeout, ERROR_INVALID_ID);
        return 3;
    }

    // check if this item is supposed to be saved on this server shard. If not, refuse to save it
    if (itemFunctions->isDestinationServerCorrect(item, numServers, serverId) == false)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: Item id %d was not expected to be saved on server id %d."
            " Number of servers %u", itemId, serverId, numServers);
        itemFunctions->freePersistentItem(item);
        sendSimpleResponse(sockt, timeout, ERROR_INVALID_SAVE_DESTINATION);
        return 4;
    }

    // Save item.

    if(cacheEngine_ItemIdCanStoreResult(cacheEngine, item) == false)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: id %d is too large to be stored in cache.", itemId);
        itemFunctions->freePersistentItem(item);
        sendSimpleResponse(sockt, timeout, ERROR_ID_TOO_LARGE);
        return 5;
    }

    uint32_t fileId = itemId / numItemsPerFile;
    ret = fileLockList_lockFileForWrite(fileList, fileId);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_lockFileForWrite(fileId = %u) returned %d.", fileId, ret);
        itemFunctions->freePersistentItem(item);
        sendSimpleResponse(sockt, timeout, ERROR_FILE_LOCK);
        return 6;
    }

    bool isNewItem = false;

    int saveRet = daos_saveItem(daos, item, &isNewItem);
    if(saveRet != 0) {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_saveItem() returned %d.", saveRet);
    }

    ret = fileLockList_releaseFile(fileList, fileId);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_releaseFile(fileId = %u) returned %d.", fileId, ret);
        itemFunctions->freePersistentItem(item);
        sendSimpleResponse(sockt, timeout, ERROR_FILE_LOCK);
        return 7;
    }

    if(saveRet != 0)
    {
        itemFunctions->freePersistentItem(item);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 8;
    }

    ret = cacheEngine_storeItem(cacheEngine, item, isNewItem);
    itemFunctions->freePersistentItem(item);

    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_storeItem() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 9;
    }

    EndInlinedProfiling(saveEventType);

    // Send response.

    sendSimpleResponse(sockt, timeout, SUCCESS_RESPONSE);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int processGetMultipleRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList,
    Daos_t daos, const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process number of ids.

    DaosCount_t numIds = 0;

    if(bufferSize <= sizeof(numIds))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) <= sizeof(numIds) (%lu).",
            bufferSize, sizeof(numIds));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    const DaosCount_t* networkNumIds = (const DaosCount_t*) buffer;
    numIds = NETWORK_TO_DAOS_COUNT(*networkNumIds);

    if(numIds == 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid numIds = 0.");
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 2;
    }

    if(numIds > MAX_NUM_ITEMS_IN_GET_REQUEST)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: numIds (%lu) > maximum (%lu).",
            numIds, MAX_NUM_ITEMS_IN_GET_REQUEST);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 3;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "numIds = %lu.", numIds);

    buffer += sizeof(*networkNumIds);
    bufferSize -= sizeof(*networkNumIds);

    if(bufferSize != (numIds * sizeof(DaosId_t)))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != numIds (%lu)"
            " * sizeof(DaosId_t) (%zu).", bufferSize, numIds, sizeof(DaosId_t));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 4;
    }

    // Initialize JSON structures.

    struct json_object* jsonObj = json_object_new_object();
    if(jsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_object_new_object() failed.");
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 5;
    }

    struct json_object* profilesJsonObj = json_object_new_array_ext(numIds);
    if(profilesJsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_object_new_array_ext(profiles) failed.");
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        json_object_put(jsonObj);
        return 6;
    }

    const char* profiles_Key = "profiles";
    const char* notRetrieved_Key = "not_retrieved";

    json_object_object_add(jsonObj, profiles_Key, profilesJsonObj);

    struct json_object* notRetrievedJsonObj = json_object_new_array_ext(numIds);
    if(notRetrievedJsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_object_new_array_ext(notRetrieved) failed.");
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        json_object_put(jsonObj);
        return 7;
    }

    json_object_object_add(jsonObj, notRetrieved_Key, notRetrievedJsonObj);

    // Process ids.

    for(DaosCount_t i = 0;  i < numIds; ++i)
    {
        const DaosId_t* networkId = (const DaosId_t*) buffer;
        DaosId_t id = NETWORK_TO_DAOS_ID(*networkId);

        LOG_MESSAGE(DEBUG_LOG_MSG, "id = %lu.", id);

        struct json_object* itemJsonObj = NULL;

        int ret = retrieveItemAndReturnJson(numItemsPerFile, fileList, daos, itemFunctions, id, &itemJsonObj);
        if(ret == 0)
        {
            json_object_array_add(profilesJsonObj, itemJsonObj);
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: retrieveItemAndReturnJson(id = %lu)"
                " returned %d.", id, ret);
            json_object_array_add(notRetrievedJsonObj, json_object_new_int(id));
        }

        buffer += sizeof(*networkId);
    }

    // Generate JSON string.

    size_t jsonStrLength = 0;
    const char* jsonStr = json_object_to_json_string_length(jsonObj, JSON_C_TO_STRING_SPACED, &jsonStrLength);

    // Send response.

    // Send response size.

    uint8_t responseCode = SUCCESS_RESPONSE;
    uint32_t responseSize = htonl(sizeof(responseCode) + jsonStrLength);

    int ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) "
            "returned %d.", sizeof(responseSize), ret);
        json_object_put(jsonObj);
        return 8;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) "
            "returned %d.", sizeof(responseCode), ret);
        json_object_put(jsonObj);
        return 9;
    }

    // Send JSON string.

    ret = sendToSocket(sockt, timeout, jsonStr, jsonStrLength);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(json response, size = %zu) "
            "returned %d.", jsonStrLength, ret);
        json_object_put(jsonObj);
        return 10;
    }

    json_object_put(jsonObj);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
   
    return 0;
}

static int retrieveItemAndReturnJson(DaosCount_t numItemsPerFile, FileLockList_t fileList,
    Daos_t daos, const ItemFunctions_t* itemFunctions, DaosId_t id, struct json_object** jsonObj)
{
    // Retrieve item.

    uint32_t fileId = id / numItemsPerFile;
    void* item      = NULL;

    int ret = fileLockList_lockFileForRead(fileList, fileId);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_lockFileForRead(fileId = %u) returned %d.", fileId, ret);
        return 1;
    }

    int getRet = daos_getItem(daos, id, &item);
    if(getRet != 0) {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_getItem() returned %d.", ret);
    }

    ret = fileLockList_releaseFile(fileList, fileId);
    if(ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_releaseFile(fileId = %u) returned %d.", fileId, ret);
        return 2;
    }

    if(getRet != 0) {
        return 3;
    }

    // Generate JSON object.

    *jsonObj = itemFunctions->persistentItemToJson(item);
    itemFunctions->freePersistentItem(item);

    if(*jsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: marshallItem() failed.");
        return 4;
    }

    return 0;
}

static int processSaveMultipleRequest(DaosCount_t numItemsPerFile, FileLockList_t fileList,
    Daos_t daos, CacheEngine_t cacheEngine, const DaosCount_t numServers, const DaosId_t serverId,
    const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, const char* jsonListKey, bool* closeConnection,
    ProfilingEvents saveEventType)
{
    DisableNonUsedWarning(saveEventType);
    StartInlinedProfiling(saveEventType);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    if(bufferSize == 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize == 0.");
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    // Parse JSON string received.

    struct json_tokener* jsonTokener = json_tokener_new();

    struct json_object* jsonObj = json_tokener_parse_ex(jsonTokener, (const char*) buffer, (int) bufferSize);
    if(jsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_tokener_parse_ex() failed. Error = \"%s\".",
            json_tokener_error_desc(json_tokener_get_error(jsonTokener)));
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: JSON buffer (size = %u) = %s\n\n", bufferSize, (const char*) buffer);
        json_tokener_free(jsonTokener);
        sendSimpleResponse(sockt, timeout, ERROR_JSON_LIBRARY);
        return 2;
    }

    json_tokener_free(jsonTokener);

    // Retrieve array from JSON object.

    void* persistentList = NULL;

    int ret = itemFunctions->jsonToPersistentList(&persistentList, jsonObj, jsonListKey);
    json_object_put(jsonObj);

    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: jsonToPersistentList(\"%s\") failed.", jsonListKey);
        sendSimpleResponse(sockt, timeout, ERROR_JSON_LIBRARY);
        return 3;
    }

    // Initialize response JSON structures.

    struct json_object* responseJsonObj = json_object_new_object();
    if(responseJsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_object_new_object() failed.");
        itemFunctions->freePersistentList(persistentList);
        sendSimpleResponse(sockt, timeout, ERROR_JSON_LIBRARY);
        return 4;
    }

    uint32_t persistentListCount = itemFunctions->persistentListCount(persistentList);

    const char* notSaved_Key = "not_saved";

    struct json_object* notSavedJsonObj = json_object_new_array_ext(persistentListCount);
    if(notSavedJsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_object_new_array_ext(\"%s\") failed.", notSaved_Key);
        json_object_put(responseJsonObj);
        itemFunctions->freePersistentList(persistentList);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 5;
    }

    json_object_object_add(responseJsonObj, notSaved_Key, notSavedJsonObj);

    //
    // Save each item.
    //

    uint32_t currentFileId = UINT32_MAX;

    for(uint32_t i = 0; i < persistentListCount; ++i)
    {
        //
        // Perform basic checks.
        //

        void* item = itemFunctions->getItemFromPersistentList(persistentList, i);

        DaosId_t itemId = itemFunctions->getPersistentItemId(item);
        if (itemId < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: id %d is negative.", itemId);
            json_object_array_add(notSavedJsonObj, json_object_new_int(itemId));
            continue;
        }

        // Check if this item is supposed to be saved on this server shard. If not, refuse to save it.

        if (itemFunctions->isDestinationServerCorrect(item, numServers, serverId) == false)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: Item id %d was not expected to be saved on server id %d."
                " Number of servers = %u", itemId, serverId, numServers);
            json_object_array_add(notSavedJsonObj, json_object_new_int(itemId));
            continue;
        }

        // Check if id can be stored in cache.

        if(cacheEngine_ItemIdCanStoreResult(cacheEngine, item) == false)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: id %d is too large to be stored in cache.", itemId);
            json_object_array_add(notSavedJsonObj, json_object_new_int(itemId));
            continue;
        }

        //
        // Save item.
        //

        uint32_t fileId = itemId / numItemsPerFile;

        // Only acquire new file lock if destination file is different from previous.

        if(fileId != currentFileId)
        {
            if(currentFileId != UINT32_MAX)
            {
                ret = fileLockList_releaseFile(fileList, currentFileId);
                if(ret != 0)
                {
                    LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_releaseFile(fileId = %u) returned %d.",
                        currentFileId, ret);
                    itemFunctions->freePersistentList(persistentList);
                    sendResponseWithIds(itemFunctions, sockt, timeout, ERROR_FILE_LOCK,
                        persistentList, i, persistentListCount, responseJsonObj, notSavedJsonObj);
                    return 6;
                }
            }

            ret = fileLockList_lockFileForWrite(fileList, fileId);
            if(ret != 0)
            {
                LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_lockFileForWrite(fileId = %u) returned %d.",
                    fileId, ret);
                itemFunctions->freePersistentList(persistentList);
                sendResponseWithIds(itemFunctions, sockt, timeout, ERROR_FILE_LOCK,
                    persistentList, i, persistentListCount, responseJsonObj, notSavedJsonObj);
                return 7;
            }

            currentFileId = fileId;
        }

        // Save item to file.

        bool isNewItem = false;

        ret = daos_saveItem(daos, item, &isNewItem);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: daos_saveItem() returned %d.", ret);
            json_object_array_add(notSavedJsonObj, json_object_new_int(itemId));
            continue;
        }

        // Save item to cache.

        ret = cacheEngine_storeItem(cacheEngine, item, isNewItem);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_storeItem() returned %d.", ret);
            json_object_array_add(notSavedJsonObj, json_object_new_int(itemId));
        }
    }

    itemFunctions->freePersistentList(persistentList);

    if(currentFileId != UINT32_MAX)
    {
        ret = fileLockList_releaseFile(fileList, currentFileId);
        if(ret != 0)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: fileLockList_releaseFile(fileId = %u) returned %d.",
                currentFileId, ret);
            sendResponseWithIds(itemFunctions, sockt, timeout, ERROR_FILE_LOCK,
                persistentList, persistentListCount, persistentListCount, responseJsonObj, notSavedJsonObj);
            return 8;
        }
    }

    sendResponseWithIds(itemFunctions, sockt, timeout, SUCCESS_RESPONSE,
        persistentList, persistentListCount, persistentListCount, responseJsonObj, notSavedJsonObj);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
   
    EndInlinedProfiling(saveEventType);

    return 0;
}

static int sendResponseWithIds(const ItemFunctions_t* itemFunctions, int sockt, Timeout_t* timeout,
    uint8_t responseCode, void* persistentList, uint32_t persistentListFirstIndex,  uint32_t persistentListCount,
    struct json_object* responseJsonObj, struct json_object* notSavedJsonObj)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    // Add remaining item ids to list.

    for(uint32_t i = persistentListFirstIndex; i < persistentListCount; ++i)
    {
        void* item = itemFunctions->getItemFromPersistentList(persistentList, i);

        DaosId_t itemId = itemFunctions->getPersistentItemId(item);

        json_object_array_add(notSavedJsonObj, json_object_new_int(itemId));
    }

    // Generate response JSON string.

    size_t responseJsonStrLength = 0;
    const char* responseJsonStr
        = json_object_to_json_string_length(responseJsonObj, JSON_C_TO_STRING_SPACED, &responseJsonStrLength);

    //
    // Send response.
    //

    // Send response size.

    uint32_t responseSize = htonl(sizeof(responseCode) + responseJsonStrLength);

    int ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) returned %d.",
            sizeof(responseSize), ret);
        json_object_put(responseJsonObj);
        return 1;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) returned %d.",
            sizeof(responseCode), ret);
        json_object_put(responseJsonObj);
        return 2;
    }

    // Send JSON string.

    ret = sendToSocket(sockt, timeout, responseJsonStr, responseJsonStrLength);
    json_object_put(responseJsonObj);

    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(json response, size = %zu) returned %d.",
            responseJsonStrLength, ret);
        return 3;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int processStartSearchRequest(SearchEngine_t searchEngine, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    StartInlinedProfiling(PE_SEARCH_SETUP);
    int64_t requestParseDurationStartStamp = GetProfilingStamp();

    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    DaosCount_t resultsLimit        = 0;
    uint32_t jsonStringSize         = 0;
    uint32_t similaritiesScoresSize = 0;

    // Basic buffer size check.

    uint32_t minimumBufferSize
        = sizeof(resultsLimit) + sizeof(jsonStringSize) + 1 + sizeof(similaritiesScoresSize);

    if(bufferSize < minimumBufferSize)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%lu) < minimum size required (%lu).",
            bufferSize, minimumBufferSize);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    // Process results limit.

    const DaosCount_t* networkResultsLimit = (const DaosCount_t*) buffer;

    resultsLimit = NETWORK_TO_DAOS_COUNT(*networkResultsLimit);
    if(resultsLimit == 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid results limit = 0.");
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 2;
    }

    LOG_MESSAGE(INFO_LOG_MSG, "Results limit = %lu.", resultsLimit);

    // Process JSON string size.

    buffer += sizeof(resultsLimit);
    bufferSize -= sizeof(resultsLimit);
    const uint32_t* networkJsonSize = (const uint32_t*) buffer;

    jsonStringSize = ntohl(*networkJsonSize);
    if(jsonStringSize == 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: invalid json string size = 0.");
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 3;
    }

    uint32_t remainingBufferSize
        = bufferSize - sizeof(jsonStringSize) - sizeof(similaritiesScoresSize);
    if(jsonStringSize > remainingBufferSize)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json string size (%lu) > remaining buffer size"
            " (%lu).", jsonStringSize, remainingBufferSize);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 4;
    }

    buffer += sizeof(jsonStringSize);
    bufferSize -= sizeof(jsonStringSize);
    const char* jsonString = (const char*) buffer;

    // Process similaritiesScores size.

    buffer += jsonStringSize;
    bufferSize -= jsonStringSize;

    const uint32_t* networkSimilaritiesScoresSize = (const uint32_t*) buffer;
    similaritiesScoresSize = ntohl(*networkSimilaritiesScoresSize);

    buffer += sizeof(similaritiesScoresSize);
    bufferSize -= sizeof(similaritiesScoresSize);
    const uint8_t* similaritiesScores = buffer;

    if(similaritiesScoresSize != bufferSize)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: similaritiesScores size (%lu) != remaining"
            " buffer size (%lu).", similaritiesScoresSize, bufferSize);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 5;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "JSON string size = %lu, similaritiesScores size = %lu.",
        jsonStringSize, similaritiesScoresSize);

    // Parse JSON string.

    if(jsonString[jsonStringSize - 1] != '\0')
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json string is not NULL terminated.");
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 6;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Search JSON: %s", jsonString);

    struct json_tokener* jsonTokener = json_tokener_new();

    struct json_object* jsonObj = json_tokener_parse_ex(jsonTokener, jsonString, (int) jsonStringSize);
    if(jsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: json_tokener_parse_ex() failed. Error = \"%s\".",
            json_tokener_error_desc(json_tokener_get_error(jsonTokener)));
        json_tokener_free(jsonTokener);
        sendSimpleResponse(sockt, timeout, ERROR_JSON_LIBRARY);
        return 7;
    }

    json_tokener_free(jsonTokener);

    // Convert JSON object to SearchCriteria.

    struct SearchCriteria* searchCriteria = malloc(sizeof(struct SearchCriteria));
    if(searchCriteria == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.",
            sizeof(struct SearchCriteria));
        json_object_put(jsonObj);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 8;
    }

    initSearchCriteria(searchCriteria);

    if(!unmarshallSearchCriteria(searchCriteria, jsonObj, similaritiesScores,
        similaritiesScoresSize))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallSearchCriteria() failed.");
        json_object_put(jsonObj);
        freeSearchCriteria(searchCriteria);
        free(searchCriteria);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 9;
    }

    json_object_put(jsonObj);

    // Start search.

    SearchId_t searchId = 0;
    int64_t requestParseDuration = GetProfilingStamp() - requestParseDurationStartStamp;

    int ret = searchEngine_startSearch(searchEngine, searchCriteria, resultsLimit, &searchId,
        requestParseDuration);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchEngine_startSearch() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 10;
    }
    
    EndInlinedProfiling(PE_SEARCH_SETUP);

    //
    // Send response.
    //

    // Send response size.

    SearchId_t networkSearchId = SEARCH_ID_TO_NETWORK(searchId);

    uint8_t responseCode = SUCCESS_RESPONSE;
    uint32_t responseSize = htonl(sizeof(responseCode) + sizeof(networkSearchId));

    ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) "
            "returned %d.", sizeof(responseSize), ret);
        return 11;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) "
            "returned %d.", sizeof(responseCode), ret);
        return 12;
    }

    // Send search id.

    ret = sendToSocket(sockt, timeout, &networkSearchId, sizeof(networkSearchId));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(searchId, size = %zu) returned %d.",
            sizeof(searchId), ret);
        return 13;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int processGetSearchStatusRequest(SearchEngine_t searchEngine, int sockt, Timeout_t* timeout,
    const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process search id.

    if(bufferSize != sizeof(SearchId_t))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != sizeof(SearchId_t) (%lu).",
            bufferSize, sizeof(SearchId_t));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    const SearchId_t* networkSearchId = (const SearchId_t*) buffer;
    SearchId_t searchId = NETWORK_TO_SEARCH_ID(*networkSearchId);

    LOG_MESSAGE(INFO_LOG_MSG, "searchId = %lu.", searchId);

    // Get search status.

    uint8_t completionPercentage = 0;

    int ret = searchEngine_getSearchStatus(searchEngine, searchId, &completionPercentage);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchEngine_getSearchStatus() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 2;
    }

    // Send response.

    // Send response size.

    uint8_t responseCode = SUCCESS_RESPONSE;
    uint32_t responseSize = htonl(sizeof(responseCode) + sizeof(completionPercentage));

    ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) "
            "returned %d.", sizeof(responseSize), ret);
        return 3;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) "
            "returned %d.", sizeof(responseCode), ret);
        return 4;
    }

    // Send completion percentage.

    ret = sendToSocket(sockt, timeout, &completionPercentage, sizeof(completionPercentage));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(completionPercentage, size = %zu)"
            " returned %d.", sizeof(searchId), ret);
        return 5;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
   
    return 0;
}

static int processGetSearchResultsRequest(SearchEngine_t searchEngine, int sockt,
    Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    StartInlinedProfiling(PE_RESULT_MERGE);

    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process search id.

    if(bufferSize != sizeof(SearchId_t))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != sizeof(SearchId_t) (%zu).",
            bufferSize, sizeof(SearchId_t));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    const SearchId_t* networkSearchId = (const SearchId_t*) buffer;
    SearchId_t searchId = NETWORK_TO_SEARCH_ID(*networkSearchId);

    // Get search results.

    struct json_object* jsonObj = NULL;

    int ret = searchEngine_getSearchResults(searchEngine, searchId, &jsonObj);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchEngine_getSearchResults() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 2;
    }

    size_t jsonStrLength = 0;
    const char* jsonStr = json_object_to_json_string_length(jsonObj, JSON_C_TO_STRING_SPACED, &jsonStrLength);

    EndInlinedProfiling(PE_RESULT_MERGE);
    PrintProfilingStatus();

    // Send response.

    // Send response size.

    uint8_t responseCode = SUCCESS_RESPONSE;
    uint32_t responseSize = htonl(sizeof(responseCode) + jsonStrLength);

    ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) "
            "returned %d.", sizeof(responseSize), ret);
        return 3;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) "
            "returned %d.", sizeof(responseCode), ret);
        return 4;
    }

    // Send JSON string.

    ret = sendToSocket(sockt, timeout, jsonStr, jsonStrLength);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(json response, size = %zu) "
            "returned %d.", jsonStrLength, ret);
        return 5;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
   
    return 0;
}

static int processEndSearchRequest(SearchEngine_t searchEngine, int sockt,
    Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process search id.

    if(bufferSize != sizeof(SearchId_t))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != sizeof(SearchId_t) (%zu).",
            bufferSize, sizeof(SearchId_t));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    const SearchId_t* networkSearchId = (const SearchId_t*) buffer;
    SearchId_t searchId = NETWORK_TO_SEARCH_ID(*networkSearchId);

    // Remove completed search results.

    int ret = searchEngine_removeSearchResults(searchEngine, searchId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: searchEngine_removeSearchResults() returned %d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 2;
    }

    // Send response.

    sendSimpleResponse(sockt, timeout, SUCCESS_RESPONSE);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int processEndConnectionRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    *closeConnection = true;

    // Process buffer size.

    if(bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    // Send response.

    sendSimpleResponse(sockt, timeout, SUCCESS_RESPONSE);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int processPingRequest(int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process buffer size.

    if(bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    // Send response.

    sendSimpleResponse(sockt, timeout, SUCCESS_RESPONSE);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int processGetSystemInfoRequest(CacheEngine_t profileCache, CacheEngine_t companyCache, uint16_t numConnections,
    int sockt, Timeout_t* timeout, uint32_t bufferSize, bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");
   
    // Process buffer size.

    if(bufferSize != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != 0.", bufferSize);
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    // Get profile cache information.

    struct SystemInfoData sysInfo;

    initSystemInfoData(&sysInfo);

    sysInfo.config.numConnections = numConnections;

    int ret = cacheEngine_getInfo(profileCache, &sysInfo.persistentItems.profiles.total,
        &sysInfo.persistentItems.profiles.minId, &sysInfo.persistentItems.profiles.maxId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_getInfo(profiles) returned $%d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        freeSystemInfoData(&sysInfo);
        return 2;
    }

    ret = cacheEngine_getInfo(companyCache, &sysInfo.persistentItems.companies.total,
        &sysInfo.persistentItems.companies.minId, &sysInfo.persistentItems.companies.maxId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: cacheEngine_getInfo(companies) returned $%d.", ret);
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        freeSystemInfoData(&sysInfo);
        return 3;
    }

    // Prepare JSON structures.

    struct json_object* sysInfoJsonObj = marshallSystemInfoData(&sysInfo);
    freeSystemInfoData(&sysInfo);

    if(sysInfoJsonObj == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: marshallSystemInfoData() failed.");
        sendSimpleResponse(sockt, timeout, ERROR_GENERIC_RESPONSE);
        return 4;
    }

    // Generate JSON string.

    size_t sysInfoJsonStrLength = 0;
    const char* sysInfoJsonStr
        = json_object_to_json_string_length(sysInfoJsonObj, JSON_C_TO_STRING_SPACED, &sysInfoJsonStrLength);

    // Send response.

    // Send response size.

    uint8_t responseCode = SUCCESS_RESPONSE;
    uint32_t responseSize = htonl(sizeof(responseCode) + sysInfoJsonStrLength);

    ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response size, size = %zu) returned %d.",
            sizeof(responseSize), ret);
        json_object_put(sysInfoJsonObj);
        return 5;
    }

    // Send response code.

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(response code, size = %zu) returned %d.",
            sizeof(responseCode), ret);
        json_object_put(sysInfoJsonObj);
        return 6;
    }

    // Send JSON string.

    ret = sendToSocket(sockt, timeout, sysInfoJsonStr, sysInfoJsonStrLength);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(json response, size = %zu) returned %d.",
            sysInfoJsonStrLength, ret);
        json_object_put(sysInfoJsonObj);
        return 7;
    }

    json_object_put(sysInfoJsonObj);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");
   
    return 0;
}

static int processSetLogLevelRequest(int sockt, Timeout_t* timeout, const uint8_t* buffer, uint32_t bufferSize,
    bool* closeConnection)
{
    LOG_MESSAGE(DEBUG_LOG_MSG, "Begin.");

    // Process buffer size.
    if (bufferSize != sizeof(unsigned char))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%u) != sizeof(unsigned int) (%zu).",
            bufferSize, sizeof(unsigned int));
        sendSimpleResponse(sockt, timeout, ERROR_MALFORMED_REQUEST_RESPONSE);
        return 1;
    }

    const uint8_t logLevel = buffer[0];

    if (logLevel > LOG_MSG_LEVEL_MAX)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: log level (%u) > max log level (%zu).",
            logLevel, LOG_MSG_LEVEL_MAX);
        sendSimpleResponse(sockt, timeout, ERROR_INVALID_VALUE);
        return 2;
    }

    logger_setLogLevel(logLevel);

    sendSimpleResponse(sockt, timeout, SUCCESS_RESPONSE);

    LOG_MESSAGE(DEBUG_LOG_MSG, "End.");

    return 0;
}

static int sendSimpleResponse(int sockt, Timeout_t* timeout, uint8_t responseCode)
{
    uint32_t responseSize = htonl(sizeof(responseCode));

    int ret = sendToSocket(sockt, timeout, &responseSize, sizeof(responseSize));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(responseSize, size = %zu) "
            "returned %d.", sizeof(responseSize), ret);
        return 1;
    }

    ret = sendToSocket(sockt, timeout, &responseCode, sizeof(responseCode));
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendToSocket(responseCode, size = %zu) "
            "returned %d.", sizeof(responseCode), ret);
        return 2;
    }

    return 0;
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

    return 0;
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

    return 0;
}
