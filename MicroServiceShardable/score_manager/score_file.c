#include <logger.h>
#include <score_file.h>
#include <score_manager.h>
#include <score_merge_mt.h>
#include <app_errors.h>
#include <request_response_definitions.h>
#include <profiling.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

void genScoreFileName(const char* dir, const unsigned int id, char* outBuf, size_t outBufSize)
{
    snprintf(outBuf, outBufSize, "%s/%u.score", dir, id);
}

static size_t getFilesize(const char* filename)
{
    struct stat st;
    int statErr = stat(filename, &st);
    if (statErr != 0)
    {
        SM_LOG_MESSAGE("Failed to stat file '%s' - errno = %d (\"%s\").", filename,
            errno, strerror(errno));
        return 0;
    }
    return st.st_size;
}

// temporary not used for the sake of investigating an "error" situation between SM and SSS
int getMappedFile(const char* filename, int* fd, char** mmappedData, size_t* filesize)
{
    // get the file size
    *filesize = getFilesize(filename);
    if (*filesize == 0)
    {
        SM_LOG_MESSAGE("File '%s' size is zero, nothing to load", filename);
        return ERR_SS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    // Open file
    *fd = open(filename, O_RDONLY, 0);
    if (*fd == -1)
    {
        SM_LOG_MESSAGE("Failed to open file '%s' - errno = %d (\"%s\").", filename,
            errno, strerror(errno));
        return ERR_SS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    //Execute mmap
    // MAP_SHARED -> OS left the file in virtual memory. In my tests every mapping reused shared memory, thus reopening file seemed faster
    // MAP_PRIVATE -> OS left the file in virtual memory. In my tests every mapping created a new copy, thus increased procesing time
    *mmappedData = mmap(NULL, *filesize, PROT_READ, MAP_SHARED, *fd, 0);
    if (*mmappedData == MAP_FAILED)
    {
        SM_LOG_MESSAGE("Failed to map file '%s' - errno = %d (\"%s\").", filename,
            errno, strerror(errno));
        close(*fd);
        *fd = 0;
        *mmappedData = NULL;
        return ERR_SS_FAILED_TO_MAP_FILE_FOR_READ;
    }

    // try to give OS hints. Hints only work on certain builds and OS versions
    int adviseErr = posix_madvise(*mmappedData, *filesize, POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED);
    if (adviseErr != 0)
    {
        SM_LOG_MESSAGE("Info : Failed to advice map file '%s' - error %d", filename, adviseErr);
    }

    int err_close = close(*fd);
    if (err_close != 0)
    {
        SM_LOG_MESSAGE("Failed to close file '%s' - errno = %d (\"%s\").", filename,
            errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FAILED_TO_CLOSE_FILE);
    }
    else
    {
        *fd = 0;
    }

    SM_LOG_MESSAGE("Successfully opened and mapped file '%s', size %d", filename, *filesize);
    return ERR_SS_NO_ERROR;
}

int cleanupMappedFile(int fd, void* mmappedData, size_t filesize)
{
    if (fd != 0)
    {
        int err_close = close(fd);
        if (err_close != 0)
        {
            SM_LOG_MESSAGE("Failed to close file - errno = %d (\"%s\").",
                errno, strerror(errno));
            LOG_DEFAULT_APP_ERROR(ERR_SS_FAILED_TO_CLOSE_FILE);
        }
    }
    int rc = munmap(mmappedData, filesize);
    if (rc != 0)
    {
        SM_LOG_MESSAGE("Failed to unmap file - errno = %d (\"%s\").",
            errno, strerror(errno));
        return ERR_SS_FAILED_TO_UNMAP_FILE;
    }
    return ERR_SS_NO_ERROR;
}

int getFileContent(const char* filename, int* fd, char** mmappedData, size_t* filesize)
{
    // get the file size
    *filesize = getFilesize(filename);
    if (*filesize == 0)
    {
        SM_LOG_MESSAGE("File '%s' size is zero, nothing to load", filename);
        return ERR_SS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    // Open file
    *fd = open(filename, O_RDONLY, 0);
    if (*fd == -1)
    {
        SM_LOG_MESSAGE("Failed to open file '%s' for reading- errno = % d(\"%s\").", filename, errno, strerror(errno));
        return ERR_SS_FAILED_TO_OPEN_FILE_FOR_READ;
    }
    *mmappedData = malloc(*filesize + sizeof(ScoreFileLayoutAddedMTData));
    if (*mmappedData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes \n", *filesize);
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        close(*fd);
        *fd = 0;
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }

    int bytesRead = read(*fd, *mmappedData, *filesize);
    if (bytesRead < *filesize)
    {
        SM_LOG_MESSAGE("Failed to read from file '%s'. Asked d, got %d - errno = % d(\"%s\").", filename,
            *filesize, bytesRead, errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FILE_READ_FAILED);
        close(*fd);
        *fd = 0;
        free(*mmappedData);
        *mmappedData = NULL;
        return ERR_SS_FILE_READ_FAILED;
    }

    int err_close = close(*fd);
    if (err_close != 0)
    {
        SM_LOG_MESSAGE("Failed to close file '%s' - errno = %d (\"%s\").", filename,
            errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FAILED_TO_CLOSE_FILE);
    }
    else
    {
        *fd = 0;
    }

//    SM_LOG_MESSAGE("Successfully opened and read file '%s', size %d", filename, *filesize);
    return ERR_SS_NO_ERROR;
}

int saveScoreFile(const int fType, const char* dir, const unsigned int id, const uint8_t* buf, const size_t bufSize)
{
    int ret = ERR_SS_NO_ERROR;
    const ScoreFileLayout* scoreFileMem = (ScoreFileLayout*)buf;
    size_t expectedBufferSize = SCORE_FILE_SIZE(scoreFileMem);
    if (expectedBufferSize != bufSize)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Expected network buffer size %zu, but received %zu\n",
            expectedBufferSize, bufSize);
        LOG_DEFAULT_APP_ERROR(ERR_SS_NETWORK_SIZE_FILE_SIZE_MISMATCH);
        ret = ERR_SS_NETWORK_SIZE_FILE_SIZE_MISMATCH;
        goto cleanup;
    }
    if (bufSize == 0)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: File size is 0. Unexpected bad value\n");
        LOG_DEFAULT_APP_ERROR(ERR_SS_NETWORK_SIZE_FILE_SIZE_MISMATCH);
        ret = ERR_SS_NETWORK_SIZE_FILE_SIZE_MISMATCH;
        goto cleanup;
    }

    char fileName[MAX_PATH_LEN];
    genScoreFileName(dir, id, fileName, sizeof(fileName));

    FILE* scoreFile = fopen(fileName, "wb");
    if (scoreFile == NULL)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Could not open file for writing : %s - errno = %d (\"%s\")\n",
            fileName, errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FAILED_TO_OPEN_FILE_FOR_WRITE);
        ret = ERR_SS_FAILED_TO_OPEN_FILE_FOR_WRITE;
        goto cleanup;
    }

    size_t bytesWritten = fwrite(buf, 1, bufSize, scoreFile);
    if (bytesWritten != bufSize)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Only written %zu bytes out of %zu for file : %s - errno = %d (\"%s\")\n",
            bytesWritten, bufSize, fileName, errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FILE_WRITE_NOT_COMPLETED);
        ret = ERR_SS_FILE_WRITE_NOT_COMPLETED;
        goto cleanup_file_close;
    }

    // generate MT related info and write it to the file
    ScoreFileLayoutAddedMTData mtData;
    int infoGenRet = genMTProcessingInfo(fType, scoreFileMem, &mtData);
    if (infoGenRet == ERR_SS_NO_ERROR)
    {
        // save the info
        size_t bytesWritten = fwrite(&mtData, 1, sizeof(ScoreFileLayoutAddedMTData), scoreFile);
        if (bytesWritten != sizeof(ScoreFileLayoutAddedMTData))
        {
            LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Only written %zu bytes out of %zu for file : %s - errno = %d (\"%s\")\n",
                bytesWritten, bufSize, fileName, errno, strerror(errno));
            LOG_DEFAULT_APP_ERROR(ERR_SS_FILE_WRITE_NOT_COMPLETED);
            ret = ERR_SS_FILE_WRITE_NOT_COMPLETED;
            goto cleanup_file_close;
        }
    }
cleanup_file_close:
    fclose(scoreFile);
cleanup:

    return ret;
}