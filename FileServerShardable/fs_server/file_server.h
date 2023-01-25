#ifndef FILE_SERVER_H
#define FILE_SERVER_H

#include <daos_definitions.h>
#include <file_lock_list.h>
#include <daos.h>
#include <stdbool.h>
#include <stdint.h>

// Types

typedef
struct
{
    void* p;
} FileServer_t;

// Constants

#define FileServer_NULL ((FileServer_t) { .p = NULL })

// Functions

FileServer_t fileServer_init();
int fileServer_run(FileServer_t server, DaosId_t serverId, DaosCount_t numServers,
    DaosCount_t numCompaniesPerFile,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections, const char *dataDir, uint32_t dirDepth, uint32_t dirCount);
void fileServer_stop(FileServer_t server);
bool fileServer_isNull(FileServer_t server);

typedef struct FileGroupData
{
    uint32_t       type;
    FileLockList_t fileList;
    Daos_t         Daos;
    DaosCount_t    numItemsPerFile;
}FileGroupData;

FileGroupData* getCreateFileGroupDao(uint32_t entryType, FileServer_t* serverData);

#endif // FILE_SERVER_H
