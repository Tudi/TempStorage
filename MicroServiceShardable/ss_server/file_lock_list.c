#include <file_lock_list.h>
#include <pthread.h>
#include <kvec.h>

//
// Types
//

typedef
struct
{
    uint32_t fileId;
    uint16_t useCount;
    pthread_rwlock_t rwlock;
} FileEntry_t;

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

static void lockFile(FileLockListData_t* data, uint32_t fileId, LockFunction_t lockFunction);

//
// External interface
//

FileLockList_t fileLockList_init(uint16_t numEntries)
{
    FileLockList_t fileList = FileLockList_NULL;

    FileLockListData_t* data = (FileLockListData_t*) malloc(sizeof(FileLockListData_t));
    if(data == NULL) { return fileList; } 

    pthread_mutex_init(&data->mutex, NULL);
    kv_init(data->entries);
    kv_resize(FileEntry_t, data->entries, numEntries);

    uint16_t i = 0;
    for(; i < numEntries; ++i)
    {
        (void) kv_a(FileEntry_t, data->entries, i);

        kv_A(data->entries, i).fileId = FILEID_MAX;
        kv_A(data->entries, i).useCount = 0;
        pthread_rwlock_init(&(kv_A(data->entries, i).rwlock), NULL);
    }

    fileList.f = data;

    return fileList;
}

void fileLockList_free(FileLockList_t fileList)
{
    FileLockListData_t* data = (FileLockListData_t*) fileList.f;
    if(data == NULL) { return; } 

    size_t i = 0;
    for(; i < kv_size(data->entries); ++i) {
        pthread_rwlock_destroy(&kv_A(data->entries, i).rwlock);
    }

    pthread_mutex_destroy(&data->mutex);
    kv_destroy(data->entries);
    kv_init(data->entries);
    free(data);
}

bool fileLockList_isNull(FileLockList_t fileList)
{
    return fileList.f == NULL;
}

void fileLockList_lockFileForRead(FileLockList_t fileList, uint32_t fileId)
{
    lockFile(fileList.f, fileId, pthread_rwlock_rdlock);
}

void fileLockList_lockFileForWrite(FileLockList_t fileList, uint32_t fileId)
{
    lockFile(fileList.f, fileId, pthread_rwlock_wrlock);
}

void fileLockList_releaseFile(FileLockList_t fileList, uint32_t fileId)
{
    FileLockListData_t* data = (FileLockListData_t*) fileList.f;

    pthread_mutex_lock(&data->mutex);

    size_t i = 0;
    for(; i < kv_size(data->entries); ++i)
    {
        if(kv_A(data->entries, i).fileId == fileId)
        {
            if(--(kv_A(data->entries, i).useCount) == 0) {
                kv_A(data->entries, i).fileId = FILEID_MAX;
            }

            pthread_rwlock_unlock(&kv_A(data->entries, i).rwlock);
            break;
        }
    }

    pthread_mutex_unlock(&data->mutex);
}

//
// Local prototypes
//

static void lockFile(FileLockListData_t* data, uint32_t fileId, LockFunction_t lockFunction)
{
    pthread_mutex_lock(&data->mutex);

    FileEntry_t* firstFreeEntry = NULL;
    FileEntry_t* existingEntry = NULL;

    size_t i = 0;
    for(; i < kv_size(data->entries); ++i)
    {
        if(kv_A(data->entries, i).fileId == FILEID_MAX && firstFreeEntry == NULL)
        {
            firstFreeEntry = &kv_A(data->entries, i);
        }
        else if(kv_A(data->entries, i).fileId == fileId)
        {
            kv_A(data->entries, i).useCount++;
            existingEntry = &kv_A(data->entries, i);
            break;
        }
    }

    if(existingEntry == NULL)
    {
        firstFreeEntry->fileId = fileId;
        firstFreeEntry->useCount = 1;
        existingEntry = firstFreeEntry;
    }

    pthread_mutex_unlock(&data->mutex);

    lockFunction(&existingEntry->rwlock);
}
