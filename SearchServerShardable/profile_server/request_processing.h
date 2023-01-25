#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

#include <file_lock_list.h>
#include <daos.h>
#include <cache_engine.h>
#include <search_engine.h>

int processRequest(DaosCount_t numServers, DaosId_t serverId, DaosCount_t numProfilesPerFile, DaosCount_t numCompaniesPerFile,
    FileLockList_t profileFileList, Daos_t profileDaos, CacheEngine_t profileCache,
    FileLockList_t companyFileList, Daos_t companyDaos, CacheEngine_t companyCache,
    SearchEngine_t searchEngine, int sockt, uint16_t numConnections, uint32_t requestArrivalTimeout,
    uint32_t connectionTimeout, bool* closeConnection);

#endif // REQUEST_PROCESSING_H
