#ifndef PROFILE_SERVER_H
#define PROFILE_SERVER_H

#include <daos_definitions.h>
#include <stdbool.h>
#include <stdint.h>

// Types

typedef
struct
{
    void* p;
} ProfileServer_t;

// Constants

#define ProfileServer_NULL ((ProfileServer_t) { .p = NULL })

// Functions

ProfileServer_t profileServer_init();
int profileServer_run(ProfileServer_t server, DaosId_t serverId, DaosCount_t numServers,
    const char* profileDirectory, DaosCount_t numProfileCacheEntries,DaosCount_t numProfilesPerFile,
    const char* companyDirectory, DaosCount_t numCompanyCacheEntries,DaosCount_t numCompaniesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, uint16_t numSearchThreads, uint16_t numLoadingThreads,
    uint16_t profileCacheUpdatePeriodInSecs, uint16_t companyCacheUpdatePeriodInSecs,
    uint16_t searchResultsExpirationInMinutes);
void profileServer_stop(ProfileServer_t server);
bool profileServer_isNull(ProfileServer_t server);

#endif // PROFILE_SERVER_H
