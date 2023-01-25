#include <file_lock_list.h>
#include <pthread.h>
#include <kvec.h>

//
// Types
//

typedef
struct
{
    FileId_t fileId;
    uint16_t useCount;
    pthread_rwlock_t rwlock;
} FileEntry_t;

typedef FileEntry_t* FileEntryPtr_t;

typedef
struct
{
    pthread_mutex_t mutex;
    kvec_t(FileEntry_t) entries;
} FileLockListData_t;

typedef int (*LockFunction_t)(pthread_rwlock_t*);

//
// Constants
//

#define FILEID_MAX UINT32_MAX

//
// Local prototypes
//

static int lockFile(FileLockListData_t* fileLockListData, FileId_t fileId, LockFunction_t lockFunction);

//
// External interface
//

FileLockList_t fileLockList_init(uint16_t numEntries)
{
    FileLockList_t fileList = FileLockList_NULL;

    FileLockListData_t* fileLockListData = (FileLockListData_t*) malloc(sizeof(FileLockListData_t));
    if(fileLockListData == NULL) { return fileList; }

    pthread_mutex_init(&fileLockListData->mutex, NULL);
    kv_init(fileLockListData->entries);
    kv_resize(FileEntry_t, fileLockListData->entries, numEntries);

    uint16_t i = 0;
    for(; i < numEntries; ++i)
    {
        (void) kv_a(FileEntry_t, fileLockListData->entries, i);

        kv_A(fileLockListData->entries, i).fileId = FILEID_MAX;
        kv_A(fileLockListData->entries, i).useCount = 0;
        pthread_rwlock_init(&(kv_A(fileLockListData->entries, i).rwlock), NULL);
    }

    fileList.f = fileLockListData;

    return fileList;
}

void fileLockList_free(FileLockList_t fileList)
{
    FileLockListData_t* fileLockListData = (FileLockListData_t*) fileList.f;
    if(fileLockListData == NULL) { return; }

    size_t i = 0;
    for(; i < kv_size(fileLockListData->entries); ++i) {
        pthread_rwlock_destroy(&kv_A(fileLockListData->entries, i).rwlock);
    }

    pthread_mutex_destroy(&fileLockListData->mutex);
    kv_destroy(fileLockListData->entries);
    kv_init(fileLockListData->entries);
    free(fileLockListData);
}

bool fileLockList_isNull(FileLockList_t fileList)
{
    return fileList.f == NULL;
}

int fileLockList_lockFileForRead(FileLockList_t fileList, FileId_t fileId)
{
    return lockFile(fileList.f, fileId, pthread_rwlock_rdlock);
}

int fileLockList_lockFileForWrite(FileLockList_t fileList, FileId_t fileId)
{
    return lockFile(fileList.f, fileId, pthread_rwlock_wrlock);
}

int fileLockList_releaseFile(FileLockList_t fileList, FileId_t fileId)
{
    FileLockListData_t* fileLockListData = (FileLockListData_t*) fileList.f;

    FileEntry_t* existingEntry = NULL;

    pthread_mutex_lock(&fileLockListData->mutex);

    for(size_t i = 0; i < kv_size(fileLockListData->entries); ++i)
    {
        if(kv_A(fileLockListData->entries, i).fileId == fileId)
        {
            existingEntry = &kv_A(fileLockListData->entries, i);
            break;
        }
    }

    if(existingEntry == NULL)
    {
        pthread_mutex_unlock(&fileLockListData->mutex);
        return 1;
    }

    if(--(existingEntry->useCount) == 0) {
        existingEntry->fileId = FILEID_MAX;
    }

    pthread_rwlock_unlock(&existingEntry->rwlock);

    pthread_mutex_unlock(&fileLockListData->mutex);

    return 0;
}

//
// Local prototypes
//

static int lockFile(FileLockListData_t* fileLockListData, FileId_t fileId, LockFunction_t lockFunction)
{
    FileEntry_t* firstFreeEntry = NULL;
    FileEntry_t* existingEntry = NULL;

    pthread_mutex_lock(&fileLockListData->mutex);

    for(size_t i = 0; i < kv_size(fileLockListData->entries); ++i)
    {
        if(kv_A(fileLockListData->entries, i).fileId == FILEID_MAX && firstFreeEntry == NULL)
        {
            firstFreeEntry = &kv_A(fileLockListData->entries, i);
        }
        else if(kv_A(fileLockListData->entries, i).fileId == fileId)
        {
            kv_A(fileLockListData->entries, i).useCount++;
            existingEntry = &kv_A(fileLockListData->entries, i);
            break;
        }
    }

    if(existingEntry == NULL)
    {
        if(firstFreeEntry == NULL)
        {
            pthread_mutex_unlock(&fileLockListData->mutex);
            return 1;
        }

        firstFreeEntry->fileId = fileId;
        firstFreeEntry->useCount = 1;
        existingEntry = firstFreeEntry;
    }

    pthread_mutex_unlock(&fileLockListData->mutex);

    lockFunction(&existingEntry->rwlock);

    return 0;
}
