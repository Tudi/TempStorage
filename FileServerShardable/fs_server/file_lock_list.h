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

typedef uint32_t FileId_t;

// Constants

#define FileLockList_NULL ((FileLockList_t) { .f = NULL })

// Functions

FileLockList_t fileLockList_init(uint16_t numEntries);
void fileLockList_free(FileLockList_t fileList);
bool fileLockList_isNull(FileLockList_t fileList);

int fileLockList_lockFileForRead(FileLockList_t fileList, FileId_t fileId);
int fileLockList_lockFileForWrite(FileLockList_t fileList, FileId_t fileId);
int fileLockList_releaseFile(FileLockList_t fileList, FileId_t fileId);

#endif // FILE_LOCK_LIST_H
