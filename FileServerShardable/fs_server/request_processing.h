#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

#include <file_lock_list.h>
#include <daos.h>
#include <file_server.h>

int processRequest(FileServer_t *serverData,
    int sockt, uint32_t requestArrivalTimeout,
    uint32_t connectionTimeout, bool* closeConnection);

#endif // REQUEST_PROCESSING_H
