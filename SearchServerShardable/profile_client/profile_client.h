#ifndef PROFILE_CLIENT_H
#define PROFILE_CLIENT_H

#include <daos_definitions.h>
#include <stdint.h>

int connectToServer(const char* serverAddress, uint16_t serverPort, int* sockt);
int endConnectionToServer(int sockt, uint8_t* responseCode);

int getProfileFromServer(int sockt, DaosId_t profileId, uint8_t* responseCode, char* jsonProfile,
    uint32_t jsonProfileSize, uint32_t* receivedJsonProfileSize);
int getMultipleProfilesFromServer(int sockt, const DaosId_t* profileIds, uint32_t numProfileIds,
    uint8_t* responseCode, char* jsonProfiles, uint32_t jsonProfilesSize, uint32_t* receivedJsonProfileSize);
int sendProfileToServer(int sockt, const char* jsonProfile, uint32_t jsonProfileSize, uint8_t* responseCode);
int sendMultipleProfilesToServer(int sockt, const char* jsonProfiles, uint32_t jsonProfilesSize, uint8_t* responseCode,
    char* jsonResponse, uint32_t jsonResponseSize, uint32_t* receivedJsonResponseSize);
int startSearchInServer(int sockt, const char* jsonSearch, uint32_t jsonSearchSize,
    const uint8_t* similaritiesScores, uint32_t similaritiesScoresSize, uint32_t resultsLimit,
    uint8_t* responseCode, uint32_t* searchId);
int getSearchStatusFromServer(int sockt, uint32_t searchId, uint8_t* responseCode,
    uint8_t* completionPercentage);
int getSearchResultsFromServer(int sockt, uint32_t searchId, uint8_t* responseCode,
    char* jsonSearchResults, uint32_t jsonSearchResultsSize,
    uint32_t* receivedJsonSearchResultsSize);
int endSearchInServer(int sockt,uint32_t searchId, uint8_t* responseCode);
int getCompanyFromServer(int sockt, DaosId_t companyId, uint8_t* responseCode,
    char* jsonProfile, uint32_t jsonProfileSize, uint32_t* receivedJsonProfileSize);
int sendCompanyToServer(int sockt, const char* jsonCompany, uint32_t jsonCompanySize, uint8_t* responseCode);
int sendMultipleCompaniesToServer(int sockt, const char* jsonCompanies, uint32_t jsonCompaniesSize,
    uint8_t* responseCode, char* jsonResponse, uint32_t jsonResponseSize, uint32_t* receivedJsonResponseSize);
int pingServer(int sockt, uint8_t* responseCode);
int getSystemInfoFromServer(int sockt, uint8_t* responseCode,
    char* jsonSystemInfo, uint32_t jsonSystemInfoSize, uint32_t* receivedJsonSystemInfoSize);
int setServerLogLevel(int sockt, uint32_t logLevel, uint8_t* responseCode);

#endif // PROFILE_CLIENT_H
