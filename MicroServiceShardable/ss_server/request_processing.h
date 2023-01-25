#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

#include <stdbool.h>
#include <stdint.h> 
#include <file_lock_list.h>

int processRequest(FileLockList_t fileLockList, int sockt, uint32_t requestArrivalTimeout,
    uint32_t connectionTimeout, const char** SimilarityPaths, bool* closeConnection);

#endif // REQUEST_PROCESSING_H
