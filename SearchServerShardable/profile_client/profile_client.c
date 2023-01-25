#include <profile_client.h>
#include <common/request_response_definitions.h>
#include <logger.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

//
// Types
//

typedef void (*NetworkToHostConverter)(void*);

//
// Local prototypes
//

// Get / Save operations

static int getItemFromServer(int sockt, uint8_t request, DaosId_t id, uint8_t* responseCode,
    char* jsonProfile, uint32_t jsonProfileSize, uint32_t* receivedJsonProfileSize);
static int sendItemToServer(int sockt, uint8_t request, const char* jsonProfile,
    uint32_t jsonProfileSize, uint8_t* responseCode);
int sendMultipleItemsToServer(int sockt, uint8_t request, const char* jsonItems, uint32_t jsonItemsSize,
    uint8_t* responseCode, char* jsonResponse, uint32_t jsonResponseSize, uint32_t* receivedJsonResponseSize);

// Send operations

static int sendZeroDataRequest(int sockt, uint8_t request);
static int sendUint8Request(int sockt, uint8_t request, uint8_t data);
static int sendUint32Request(int sockt, uint8_t request, uint32_t data);
static int sendInt32ArrayRequest(int sockt, uint8_t request, const int32_t* data, uint32_t count);
static int sendJsonRequest(int sockt, uint8_t request, const char* buffer, uint32_t bufferSize,
        const uint32_t* extraData);

// Receive operations

static int receiveZeroDataResponse(int sockt, uint8_t* responseCode);
static int receiveUintResponse(int sockt, uint8_t* responseCode, void* data, size_t dataSize,
    NetworkToHostConverter converter);
static int receiveJsonResponse(int sockt, uint8_t* responseCode, char* buffer, uint32_t bufferSize,
    uint32_t* receivedBufferSize);

#define RECEIVE_UINT8_RESPONSE(SOCKT, RESPONSE_CODE, DATA) \
    receiveUintResponse((SOCKT), (RESPONSE_CODE), (DATA), sizeof(*DATA), networkUint8ToHost)
#define RECEIVE_UINT32_RESPONSE(SOCKT, RESPONSE_CODE, DATA) \
    receiveUintResponse((SOCKT), (RESPONSE_CODE), (DATA), sizeof(*DATA), networkUint32ToHost)

// Others

static void networkUint8ToHost(void* data);
static void networkUint32ToHost(void* data);

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
    int ret = sendZeroDataRequest(sockt, END_CONNECTION);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendZeroDataRequest() returned %d.", ret);
        close(sockt);
        return 1;
    }

    ret = receiveZeroDataResponse(sockt, responseCode);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveZeroDataResponse() returned %d.", ret);
        close(sockt);
        return 2;
    }

    close(sockt);

    return 0;
}

int getProfileFromServer(int sockt, DaosId_t profileId, uint8_t* responseCode, char* jsonProfile,
    uint32_t jsonProfileSize, uint32_t* receivedJsonProfileSize)
{
    return getItemFromServer(sockt, GET_PROFILE, profileId,
        responseCode, jsonProfile, jsonProfileSize, receivedJsonProfileSize);
}

int getMultipleProfilesFromServer(int sockt, const DaosId_t* profileIds, uint32_t numProfileIds,
    uint8_t* responseCode, char* jsonProfiles, uint32_t jsonProfilesSize, uint32_t* receivedJsonProfileSize)
{
    int ret = sendInt32ArrayRequest(sockt, GET_MULTIPLE_PROFILES, profileIds, numProfileIds);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendInt32ArrayRequest() returned %d.", ret);
        return 1;
    }

    ret = receiveJsonResponse(sockt, responseCode, jsonProfiles, jsonProfilesSize, receivedJsonProfileSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveJsonResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}

int sendProfileToServer(int sockt, const char* jsonProfile, uint32_t jsonProfileSize,
    uint8_t* responseCode)
{
    return sendItemToServer(sockt, SAVE_PROFILE, jsonProfile, jsonProfileSize, responseCode);
}

int sendMultipleProfilesToServer(int sockt, const char* jsonProfiles, uint32_t jsonProfilesSize, uint8_t* responseCode,
    char* jsonResponse, uint32_t jsonResponseSize, uint32_t* receivedJsonResponseSize)
{
    return sendMultipleItemsToServer(sockt, SAVE_MULTIPLE_PROFILES, jsonProfiles, jsonProfilesSize, responseCode,
        jsonResponse, jsonResponseSize, receivedJsonResponseSize);
}

int startSearchInServer(int sockt, const char* jsonSearch, uint32_t jsonSearchSize,
    const uint8_t* similaritiesScores, uint32_t similaritiesScoresSize, uint32_t resultsLimit,
    uint8_t* responseCode, uint32_t* searchId)
{
    // Send request size.

    uint8_t requestCode = START_SEARCH;

    uint32_t requestSize = htonl(sizeof(requestCode) + sizeof(resultsLimit) + sizeof(jsonSearchSize)
        + jsonSearchSize + sizeof(similaritiesScoresSize) + similaritiesScoresSize);

    ssize_t numBytesWritten = write(sockt, &requestSize, sizeof(requestSize));
    if(numBytesWritten != sizeof(requestSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(requestSize), numBytesWritten);
        }

        return 1;
    }

    // Send request code.

    numBytesWritten = write(sockt, &requestCode, sizeof(requestCode));
    if(numBytesWritten != sizeof(requestCode))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestCode) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestCode) failed. numBytesRequested "
                "(%zd) != numBytesWritten (%zd).", sizeof(requestCode), numBytesWritten);
        }

        return 2;
    }

    // Send results limit.

    uint32_t networkResultsLimit = htonl(resultsLimit);

    numBytesWritten
        = write(sockt, &networkResultsLimit, sizeof(networkResultsLimit));
    if(numBytesWritten != sizeof(networkResultsLimit))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(resultsLimit) failed."
                " errno = %d (\"%s\").", errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(resultsLimit) failed. numBytesRequested "
                "(%zd) != numBytesWritten (%zd).", sizeof(networkResultsLimit), numBytesWritten);
        }

        return 3;
    }

    // Send json search size.

    uint32_t networkJsonSearchSize = htonl(jsonSearchSize);

    numBytesWritten = write(sockt, &networkJsonSearchSize, sizeof(networkJsonSearchSize));
    if(numBytesWritten != sizeof(networkJsonSearchSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(jsonSearchSize) failed."
                " errno = %d (\"%s\").", errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(jsonSearchSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(networkJsonSearchSize), numBytesWritten);
        }

        return 4;
    }

    // Send JSON search.

    numBytesWritten = write(sockt, jsonSearch, jsonSearchSize);
    if(numBytesWritten != jsonSearchSize)
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(jsonSearch) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(jsonSearch) failed. numBytesRequested"
                " (%lu) != numBytesWritten (%zd).", jsonSearchSize, numBytesWritten);
        }

        return 5;
    }

    // Send similaritiesScores size.

    uint32_t networkSimilaritiesScoresSize = htonl(similaritiesScoresSize);

    numBytesWritten
        = write(sockt, &networkSimilaritiesScoresSize, sizeof(networkSimilaritiesScoresSize));
    if(numBytesWritten != sizeof(networkSimilaritiesScoresSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(similaritiesScoresSize) failed."
                " errno = %d (\"%s\").", errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(similaritiesScoresSize) failed. "
                "numBytesRequested (%zd) != numBytesWritten (%zd).",
                sizeof(networkSimilaritiesScoresSize), numBytesWritten);
        }

        return 6;
    }

    // Send Similarities Scores.

    if(similaritiesScores != NULL)
    {
        numBytesWritten = write(sockt, similaritiesScores, similaritiesScoresSize);
        if(numBytesWritten != similaritiesScoresSize)
        {
            if(numBytesWritten < 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(similaritiesScores) failed. errno = %d"
                    " (\"%s\").", errno, strerror(errno));
            }
            else
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(similaritiesScores) failed. "
                    "numBytesRequested (%zd) != numBytesWritten (%zd).",
                    similaritiesScoresSize, numBytesWritten);
            }

            return 7;
        }
    }

    // Receive response.

    int ret = RECEIVE_UINT32_RESPONSE(sockt, responseCode, searchId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: RECEIVE_UINT32_RESPONSE() returned %d.", ret);
        return 8;
    }

    return 0;
}

int getSearchStatusFromServer(int sockt, uint32_t searchId,
    uint8_t* responseCode, uint8_t* completionPercentage)
{
    int ret = sendUint32Request(sockt, GET_SEARCH_STATUS, searchId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendUint32Request() returned %d.", ret);
        return 1;
    }

    ret = RECEIVE_UINT8_RESPONSE(sockt, responseCode, completionPercentage);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: RECEIVE_UINT8_RESPONSE() returned %d.", ret);
        return 2;
    }

    return 0;
}

int getSearchResultsFromServer(int sockt, uint32_t searchId, uint8_t* responseCode,
    char* jsonSearchResults, uint32_t jsonSearchResultsSize,
    uint32_t* receivedJsonSearchResultsSize)
{
    int ret = sendUint32Request(sockt, GET_SEARCH_RESULTS, searchId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendUint32Request() returned %d.", ret);
        return 1;
    }

    ret = receiveJsonResponse(sockt, responseCode, jsonSearchResults, jsonSearchResultsSize,
        receivedJsonSearchResultsSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveJsonResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}

int endSearchInServer(int sockt, uint32_t searchId,
    uint8_t* responseCode)
{
    int ret = sendUint32Request(sockt, END_SEARCH, searchId);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendUint32Request() returned %d.", ret);
        return 1;
    }

    ret = receiveZeroDataResponse(sockt, responseCode);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveZeroDataResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}

int getCompanyFromServer(int sockt, DaosId_t companyId, uint8_t* responseCode, char* jsonProfile,
    uint32_t jsonProfileSize, uint32_t* receivedJsonProfileSize)
{
    return getItemFromServer(sockt, GET_COMPANY, companyId,
        responseCode, jsonProfile, jsonProfileSize, receivedJsonProfileSize);
}

int sendCompanyToServer(int sockt, const char* jsonCompany, uint32_t jsonCompanySize,
    uint8_t* responseCode)
{
    return sendItemToServer(sockt, SAVE_COMPANY, jsonCompany, jsonCompanySize, responseCode);
}

int sendMultipleCompaniesToServer(int sockt, const char* jsonCompanies, uint32_t jsonCompaniesSize,
    uint8_t* responseCode, char* jsonResponse, uint32_t jsonResponseSize, uint32_t* receivedJsonResponseSize)
{
    return sendMultipleItemsToServer(sockt, SAVE_MULTIPLE_COMPANIES, jsonCompanies, jsonCompaniesSize, responseCode,
        jsonResponse, jsonResponseSize, receivedJsonResponseSize);
}

int pingServer(int sockt, uint8_t* responseCode)
{
    int ret = sendZeroDataRequest(sockt, PING);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendZeroDataRequest() returned %d.", ret);
        return 1;
    }

    ret = receiveZeroDataResponse(sockt, responseCode);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveZeroDataResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}

int getSystemInfoFromServer(int sockt, uint8_t* responseCode,
    char* jsonSystemInfo, uint32_t jsonSystemInfoSize, uint32_t* receivedJsonSystemInfoSize)
{
    int ret = sendZeroDataRequest(sockt, GET_SYSTEM_INFO);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendZeroDataRequest() returned %d.", ret);
        return 1;
    }

    ret = receiveJsonResponse(sockt, responseCode, jsonSystemInfo, jsonSystemInfoSize, receivedJsonSystemInfoSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveJsonResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}

int setServerLogLevel(int sockt, uint32_t logLevel, uint8_t* responseCode)
{
    int ret = sendUint8Request(sockt, SET_LOG_LEVEL, logLevel);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendUint8Request() returned %d.", ret);
        return 1;
    }

    ret = receiveZeroDataResponse(sockt, responseCode);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveZeroDataResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}
//
// Local functions
//

//
// Get / Save operations
//

int getItemFromServer(int sockt, uint8_t request, DaosId_t id, uint8_t* responseCode,
    char* jsonProfile, uint32_t jsonProfileSize, uint32_t* receivedJsonProfileSize)
{
    int ret = sendUint32Request(sockt, request, id);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendUint32Request() returned %d.", ret);
        return 1;
    }

    ret = receiveJsonResponse(sockt, responseCode, jsonProfile, jsonProfileSize, receivedJsonProfileSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveJsonResponse() returned %d.", ret);
        return 2;
    }

    return 0;
}

int sendItemToServer(int sockt, uint8_t request, const char* jsonProfile, uint32_t jsonProfileSize,
    uint8_t* responseCode)
{
    if((jsonProfileSize == 0) || (jsonProfileSize > REQUEST_MAX_SIZE))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: jsonProfileSize == %lu.", jsonProfileSize);
        return 1;
    }

    int ret = sendJsonRequest(sockt, request, jsonProfile, jsonProfileSize, NULL);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendJsonRequest() returned %d.",ret);
        return 2;
    }

    ret = receiveZeroDataResponse(sockt, responseCode);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveZeroDataResponse() returned %d.", ret);
        return 3;
    }

    return 0;
}

int sendMultipleItemsToServer(int sockt, uint8_t request, const char* jsonItems, uint32_t jsonItemsSize,
    uint8_t* responseCode, char* jsonResponse, uint32_t jsonResponseSize, uint32_t* receivedJsonResponseSize)
{
    if((jsonItemsSize == 0) || (jsonItemsSize > REQUEST_MAX_SIZE))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: jsonItemsSize == %lu.", jsonItemsSize);
        return 1;
    }

    int ret = sendJsonRequest(sockt, request, jsonItems, jsonItemsSize, NULL);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: sendJsonRequest() returned %d.",ret);
        return 2;
    }

    ret = receiveJsonResponse(sockt, responseCode, jsonResponse, jsonResponseSize, receivedJsonResponseSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: receiveJsonResponse() returned %d.", ret);
        return 3;
    }

    return 0;
}

//
// Send operations
//

static int sendZeroDataRequest(int sockt, uint8_t request)
{
    uint32_t requestSize = htonl(sizeof(request));

    ssize_t numBytesWritten = write(sockt, &requestSize, sizeof(requestSize));
    if(numBytesWritten != sizeof(requestSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(requestSize), numBytesWritten);
        }

        return 1;
    }

    numBytesWritten = write(sockt, &request, sizeof(request));
    if(numBytesWritten != sizeof(request))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(request), numBytesWritten);
        }

        return 2;
    }

    return 0;
}

static int sendUint8Request(int sockt, uint8_t request, uint8_t networkData)
{
    uint32_t requestSize = htonl(sizeof(request) + sizeof(networkData));

    ssize_t numBytesWritten = write(sockt, &requestSize, sizeof(requestSize));
    if (numBytesWritten != sizeof(requestSize))
    {
        if (numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(requestSize), numBytesWritten);
        }

        return 1;
    }

    numBytesWritten = write(sockt, &request, sizeof(request));
    if (numBytesWritten != sizeof(request))
    {
        if (numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(request), numBytesWritten);
        }

        return 2;
    }

    numBytesWritten = write(sockt, &networkData, sizeof(networkData));
    if (numBytesWritten != sizeof(networkData))
    {
        if (numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(data) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(data) failed. numBytesRequested (%zd) != "
                "numBytesWritten (%zd).", sizeof(networkData), numBytesWritten);
        }

        return 3;
    }

    return 0;
}

static int sendUint32Request(int sockt, uint8_t request, uint32_t data)
{
    uint32_t requestSize = htonl(sizeof(request) + sizeof(data));

    ssize_t numBytesWritten = write(sockt, &requestSize, sizeof(requestSize));
    if(numBytesWritten != sizeof(requestSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(requestSize), numBytesWritten);
        }

        return 1;
    }

    numBytesWritten = write(sockt, &request, sizeof(request));
    if(numBytesWritten != sizeof(request))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(request), numBytesWritten);
        }

        return 2;
    }

    uint32_t networkData = htonl(data);

    numBytesWritten = write(sockt, &networkData, sizeof(networkData));
    if(numBytesWritten != sizeof(networkData))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(data) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(data) failed. numBytesRequested (%zd) != "
                "numBytesWritten (%zd).", sizeof(networkData), numBytesWritten);
        }

        return 3;
    }

    return 0;
}

static int sendInt32ArrayRequest(int sockt, uint8_t request, const int32_t* data, uint32_t count)
{
    if(count > MAX_NUM_ITEMS_IN_REQUEST)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: count (%lu) > maximum (%lu).", count, MAX_NUM_ITEMS_IN_REQUEST);
        return 1;
    }

    uint32_t dataSize = count * sizeof(*data);
    uint32_t requestSize = htonl(sizeof(request) + sizeof(count) + dataSize);

    ssize_t numBytesWritten = write(sockt, &requestSize, sizeof(requestSize));
    if(numBytesWritten != sizeof(requestSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(requestSize), numBytesWritten);
        }

        return 2;
    }

    numBytesWritten = write(sockt, &request, sizeof(request));
    if(numBytesWritten != sizeof(request))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(request), numBytesWritten);
        }

        return 3;
    }

    uint32_t networkCount = htonl(count);

    numBytesWritten = write(sockt, &networkCount, sizeof(networkCount));
    if(numBytesWritten != sizeof(networkCount))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(count) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(count) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(networkCount), numBytesWritten);
        }

        return 4;
    }

    uint32_t networkData[MAX_NUM_ITEMS_IN_REQUEST] = { 0 };

    for(size_t i = 0; i < count; ++i) {
        networkData[i] = htonl(data[i]);
    }

    numBytesWritten = write(sockt, networkData, dataSize);
    if(numBytesWritten != dataSize)
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(data) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(data) failed. numBytesRequested (%zd) != "
                "numBytesWritten (%zd).", dataSize, numBytesWritten);
        }

        return 5;
    }

    return 0;
}

static int sendJsonRequest(int sockt, uint8_t request, const char* buffer, uint32_t bufferSize,
    const uint32_t* extraData)
{
    uint32_t requestSize = htonl(sizeof(request) + bufferSize
        + (extraData == NULL ? 0 : sizeof(*extraData)));

    ssize_t numBytesWritten = write(sockt, &requestSize, sizeof(requestSize));
    if(numBytesWritten != sizeof(requestSize))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(requestSize) failed. numBytesRequested"
                " (%zd) != numBytesWritten (%zd).", sizeof(requestSize), numBytesWritten);
        }

        return 1;
    }

    numBytesWritten = write(sockt, &request, sizeof(request));
    if(numBytesWritten != sizeof(request))
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(request) failed. numBytesRequested (%zd)"
                " != numBytesWritten (%zd).", sizeof(request), numBytesWritten);
        }

        return 2;
    }

    if(extraData != NULL)
    {
        uint32_t networkExtraData = htonl(*extraData);

        numBytesWritten = write(sockt, &networkExtraData, sizeof(networkExtraData));
        if(numBytesWritten != sizeof(networkExtraData))
        {
            if(numBytesWritten < 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(extraData) failed. errno = %d"
                    " (\"%s\").", errno, strerror(errno));
            }
            else
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(extraData) failed. numBytesRequested"
                    " (%zd) != numBytesWritten (%zd).", sizeof(networkExtraData), numBytesWritten);
            }

            return 3;
        }
    }

    numBytesWritten = write(sockt, buffer, bufferSize);
    if(numBytesWritten != bufferSize)
    {
        if(numBytesWritten < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(buffer) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: write(buffer) failed. numBytesRequested"
                " (%lu) != numBytesWritten (%zd).", bufferSize, numBytesWritten);
        }

        return 4;
    }

    return 0;
}

//
// Receive operations
//

static int receiveZeroDataResponse(int sockt, uint8_t* responseCode)
{
    uint32_t responseSizeBuffer = 0;

    ssize_t numBytesRead = read(sockt, &responseSizeBuffer, sizeof(responseSizeBuffer));
    if(numBytesRead != sizeof(responseSizeBuffer))
    {
        if(numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. errno = %d"
                " (\"%s\").", errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. numBytesRequested"
                " (%zd) != numBytesRead (%zd).", sizeof(responseSizeBuffer), numBytesRead);
        }

        return 1;
    }

    uint32_t responseSize = htonl(responseSizeBuffer);
    if(responseSize != sizeof(*responseCode))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: responseSize (%lu) != expected response size"
            " (%zu).", responseSize, sizeof(*responseCode));
        return 2;
    }

    numBytesRead = read(sockt, responseCode, sizeof(*responseCode));
    if(numBytesRead != sizeof(*responseCode))
    {
        if(numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseCode) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseCode) failed. numBytesRequested"
                " (%zd) != numBytesRead (%zd).", sizeof(*responseCode), numBytesRead);
        }

        return 3;
    }

    return 0;
}

static int receiveUintResponse(int sockt, uint8_t* responseCode, void* data, size_t dataSize,
    NetworkToHostConverter converter)
{
    uint32_t responseSizeBuffer = 0;

    ssize_t numBytesRead = read(sockt, &responseSizeBuffer, sizeof(responseSizeBuffer));
    if(numBytesRead != sizeof(responseSizeBuffer))
    {
        if(numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. numBytesRequested"
                " (%zd) != numBytesRead (%zd).", sizeof(responseSizeBuffer), numBytesRead);
        }

        return 1;
    }

    uint32_t responseSize = htonl(responseSizeBuffer);

    if(responseSize < sizeof(*responseCode))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: responseSize (%lu) < minimum size (%zu).",
            responseSize, sizeof(*responseCode));
        return 2;
    }

    numBytesRead = read(sockt, responseCode, sizeof(*responseCode));
    if(numBytesRead != sizeof(*responseCode))
    {
        if(numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseCode) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseCode) failed. numBytesRequested"
                " (%zd) != numBytesRead (%zd).", sizeof(*responseCode), numBytesRead);
        }

        return 3;
    }

    --responseSize;

    if((responseSize != 0) && (responseSize != dataSize))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: responseSize (%lu). Expected sizes: 0 or %zu.",
            responseSize, dataSize);
        return 4;
    }

    if(responseSize > 0)
    {
        numBytesRead = read(sockt, data, dataSize);
        if(numBytesRead != dataSize)
        {
            if(numBytesRead < 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(data) failed. errno = %d (\"%s\").",
                    errno, strerror(errno));
            }
            else
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(data) failed. numBytesRequested"
                    " (%lu) != numBytesRead (%zd).", dataSize, numBytesRead);
            }

            return 5;
        }

        converter(data);
    }

    return 0;
}

static int receiveJsonResponse(int sockt, uint8_t* responseCode, char* buffer, uint32_t bufferSize,
    uint32_t* receivedBufferSize)
{
    uint32_t responseSizeBuffer = 0;

    ssize_t numBytesRead = read(sockt, &responseSizeBuffer, sizeof(responseSizeBuffer));
    if(numBytesRead != sizeof(responseSizeBuffer))
    {
        if(numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseSize) failed. numBytesRequested"
                " (%zd) != numBytesRead (%zd).", sizeof(responseSizeBuffer), numBytesRead);
        }

        return 1;
    }

    uint32_t responseSize = htonl(responseSizeBuffer);
    if(responseSize < sizeof(*responseCode))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: responseSize (%lu) < minimum size (%zu).",
            responseSize, sizeof(*responseCode));
        return 2;
    }

    numBytesRead = read(sockt, responseCode, sizeof(*responseCode));
    if(numBytesRead != sizeof(*responseCode))
    {
        if(numBytesRead < 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseCode) failed. errno = %d (\"%s\").",
                errno, strerror(errno));
        }
        else
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(responseCode) failed. numBytesRequested"
                " (%zd) != numBytesRead (%zd).", sizeof(*responseCode), numBytesRead);
        }

        return 3;
    }

    --responseSize;
    *receivedBufferSize = 0;

    if(responseSize > 0)
    {
        if(bufferSize < responseSize)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: bufferSize (%lu) < responseSize (%lu).",
                bufferSize, responseSize);
            return 4;
        }

        numBytesRead = 0;
        do {
            int numBytesReadNow = read(sockt, &buffer[numBytesRead], responseSize - numBytesRead);
            if (numBytesReadNow <= 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(buffer) failed. errno = %d (\"%s\").",
                    errno, strerror(errno));
                break;
            }
            numBytesRead += numBytesReadNow;
        } while (numBytesRead < responseSize);

        if(numBytesRead != responseSize)
        {
            if(numBytesRead < 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(buffer) failed. errno = %d (\"%s\").",
                    errno, strerror(errno));
            }
            else
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: read(buffer) failed. numBytesRequested"
                    " (%lu) != numBytesRead (%zd).", responseSize, numBytesRead);
            }

            return 5;
        }

        *receivedBufferSize = responseSize;
    }

    return 0;
}

//
// Others
//

static void networkUint8ToHost(void* data) { }

static void networkUint32ToHost(void* data)
{
    uint32_t* u32 = (uint32_t*) data;
    *u32 = ntohl(*u32);
}
