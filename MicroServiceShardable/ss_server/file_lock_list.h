#ifndef FILE_LOCK_LIST_H
#define FILE_LOCK_LIST_H

#include <stdbool.h>
#include <stdint.h>

// Types

typedef
struct
{
    void* f;
} FileLockList_t;

// Constants

#define FileLockList_NULL ((FileLockList_t) { .f = NULL })

// Functions

FileLockList_t fileLockList_init(uint16_t numEntries);
void fileLockList_free(FileLockList_t fileList);
bool fileLockList_isNull(FileLockList_t fileList);

void fileLockList_lockFileForRead(FileLockList_t fileList, uint32_t fileId);
void fileLockList_lockFileForWrite(FileLockList_t fileList, uint32_t fileId);
void fileLockList_releaseFile(FileLockList_t fileList, uint32_t fileId);

#endif // FILE_LOCK_LIST_H
