#include <daos.h>
#include <file_table.h>
#include <files.h>
#include <logger.h>
#include <definitions.h>
#include <macro_utils.h>
#include <app_errors.h>
#include <request_response_definitions.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

//
// Constants
//

#define DAOS_DIRECTORY_LENGTH       2048
#define DAOS_FILE_PREFIX_LENGTH     64
#define DAOS_FILE_EXTENSION_LENGTH  8
#define DAOS_MAX_DIRECTORY_DEPTH    100
#define DAOS_MAX_DIRECTORY_COUNT    10000

//
// Types
//

typedef
struct
{
    char directory[DAOS_DIRECTORY_LENGTH + 1];
    char filePrefix[DAOS_FILE_PREFIX_LENGTH + 1];
    char fileExtension[DAOS_FILE_EXTENSION_LENGTH + 1];
    uint8_t numIdDigits;
    DaosCount_t numItemsPerFile;
    uint32_t dirCount;
    uint32_t dirDepth;

    DaosItemFunctions_t itemFunctions;
} DaosData_t;

//
// Local prototypes
//

static int saveItemToNewFile(const char* filename, DaosData_t* daosData, const void* item);
static int saveItemToExistingFile(const char* filename, FILE* existingFile,
    DaosData_t* daosData, const void* item, bool* newItem);

static void generateFilename(DaosData_t* data, DaosId_t id, char* filename, size_t filenameLength);
static FILE* createTempFile(const char* directory, const char* filePrefix,
    char* filename, size_t filenameLength);

//
// External interface
//

Daos_t daos_init(const char* directory, uint32_t dirDepth, uint32_t dirCount, const char* filePrefix, const char* fileExtension,
    uint8_t numIdDigits, DaosCount_t numItemsPerFile, DaosItemFunctions_t* itemFunctions)
{
    Daos_t daos = Daos_NULL;

    if (directory == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for directory.");
        return daos;
    }

    if (filePrefix == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for filePrefix.");
        return daos;
    }

    if (fileExtension == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for fileExtension.");
        return daos;
    }

    if (itemFunctions == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for itemFunctions.");
        return daos;
    }

    if (strnlen(directory, MAX_STRING_LEN) > DAOS_DIRECTORY_LENGTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Directory's (\"%s\") length is greater"
            " than maximum (%zu).", directory, DAOS_DIRECTORY_LENGTH);
        return daos;
    }

    if (strnlen(filePrefix, MAX_STRING_LEN) > DAOS_FILE_PREFIX_LENGTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: filePrefix's (\"%s\") length is"
            " greater than maximum (%zu).", filePrefix, DAOS_FILE_PREFIX_LENGTH);
        return daos;
    }

    if (strnlen(fileExtension, MAX_STRING_LEN) > DAOS_FILE_EXTENSION_LENGTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: fileExtension's (\"%s\") length is "
            "greater than maximum (%zu).", fileExtension, DAOS_FILE_EXTENSION_LENGTH);
        return daos;
    }

    if (numIdDigits == 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for numIdDigits.");
        return daos;
    }

    if (numItemsPerFile == 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for numItemsPerFile.");
        return daos;
    }

    if (dirDepth > DAOS_MAX_DIRECTORY_DEPTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for dirDepth.");
        return daos;
    }

    if (dirCount > DAOS_MAX_DIRECTORY_COUNT)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for dirCount.");
        return daos;
    }

    DIR* dir = opendir(directory);
    if (dir == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: opendir(%s) failed. errno = %d"
            " (\"%s\").", directory, errno, strerror(errno));
        return daos;
    }

    closedir(dir);

    DaosData_t* data = (DaosData_t*)malloc(sizeof(DaosData_t));
    if (data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(size = %zu) failed.",
            sizeof(DaosData_t));
        return daos;
    }

    strcpy(data->directory, directory);
    strcpy(data->filePrefix, filePrefix);
    strcpy(data->fileExtension, fileExtension);
    data->numIdDigits = numIdDigits;
    data->numItemsPerFile = numItemsPerFile;
    data->itemFunctions = *itemFunctions;
    data->dirCount = dirCount;
    data->dirDepth = dirDepth;

    daos.d = data;

    return daos;
}

void daos_free(Daos_t daos)
{
    DaosData_t* data = (DaosData_t*)daos.d;
    if (data == NULL) { return; }

    free(data);
}

bool daos_isNull(Daos_t daos)
{
    return daos.d == NULL;
}

// create a file for "item" without header or any additional data.
static int daos_getItemFlatFile(Daos_t daos, DaosId_t id, void** itemBinary, void** item, uint32_t* itemDataSize)
{
    DaosData_t* data = (DaosData_t*)daos.d;
    char filename[PATH_MAX];
    generateFilename(data, id, filename, ARRAY_COUNT(filename));

    FILE* fp;
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fopen(%s) failed. errno = %d (\"%s\").",
            id, filename, errno, strerror(errno));
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    // get the file size
    fseek(fp, 0, SEEK_END);
    *itemDataSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // allocate storage for the file content
    *itemBinary = malloc(*itemDataSize);
    if (*itemBinary == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(size = %zu) failed.",
            *itemDataSize);
        fclose(fp);
        return ERR_FS_MEMORY_ALLOC_FAILED;
    }

    // read the whole file
    size_t bytesRead = fread(*itemBinary, 1, *itemDataSize, fp);
    fclose(fp);
    if (bytesRead != *itemDataSize)
    {
        LOG_DEFAULT_APP_ERROR(ERR_FS_FILE_READ_FAILED);
        return ERR_FS_FILE_READ_FAILED;
    }

    // item that will be used by caller
    *item = *itemBinary;

    return ERR_FS_NO_ERROR;
}

// save the buffer as is into a file
static int daos_saveItemFlatFile(Daos_t daos, int id, const void* item, uint32_t itemSize)
{
    DaosData_t* data = (DaosData_t*)daos.d;
    char filename[PATH_MAX];
    generateFilename(data, id, filename, ARRAY_COUNT(filename));

    FILE* file = fopen(filename, "wb");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - opendir(%s) failed. errno = %d"
            " (\"%s\").", id, filename, errno, strerror(errno));
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_WRITE;
    }

    size_t bytesWritten = fwrite(item, 1, itemSize, file);
    fclose(file);
    if(bytesWritten != itemSize)
    {
        LOG_DEFAULT_APP_ERROR(ERR_FS_FILE_WRITE_NOT_COMPLETED);
        return ERR_FS_FILE_WRITE_NOT_COMPLETED;
    }

    return ERR_FS_NO_ERROR;
}

int daos_getItem(Daos_t daos, DaosId_t id, void** itemBinary, void** item, uint32_t* itemDataSize)
{
    DaosData_t* data = (DaosData_t*)daos.d;
    if (data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return ERR_FS_PARAM_DAO_NULL;
    }

    if (data->numItemsPerFile <= 1)
    {
        return daos_getItemFlatFile(daos, id, itemBinary, item, itemDataSize);
    }

    char filename[PATH_MAX] = { 0 };

    generateFilename(data, id, filename, ARRAY_COUNT(filename));

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fopen(%s) failed. errno = %d (\"%s\").",
            id, filename, errno, strerror(errno));
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    DaosOffset_t itemOffset = 0;
    DaosSize_t itemSize = 0;

    int ret = getFileTableEntry(file, id, &itemOffset, &itemSize);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - getFileTableEntry(file = %s)"
            " returned %d.", id, filename, ret);
        fclose(file);
        return ret;
    }

    *itemBinary = (uint8_t*)malloc(itemSize);
    if (*itemBinary == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(size = %zu) failed.",
            itemSize);
        fclose(file);
        return ERR_FS_MEMORY_ALLOC_FAILED;
    }

    ret = readItemFromFile(file, itemOffset, itemSize, *itemBinary);
    fclose(file);

    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - readItemFromFile(file = %s)"
            " returned %d.", id, filename, ret);
        return 4;
    }

    ret = processItemRead(file, *itemBinary, itemSize, NULL, item, itemDataSize);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - processItemRead(file = %s)"
            " returned %d.", id, filename, ret);
        return ret;
    }

    return ERR_FS_NO_ERROR;
}

int daos_saveItem(Daos_t daos, const void* item, uint32_t itemSize)
{
    DaosData_t* data = (DaosData_t*)daos.d;
    if (data == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return 1;
    }

    DaosId_t id = data->itemFunctions.getPersistentItemId(item);

    if (data->numItemsPerFile <= 1)
    {
        return daos_saveItemFlatFile(daos, id, item, itemSize);
    }

    char filename[PATH_MAX] = { 0 };

    generateFilename(data, id, filename, ARRAY_COUNT(filename));

    FILE* file = fopen(filename, "r+b");
    if (file == NULL)
    {
        if (errno != ENOENT)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - opendir(%s) failed. errno = %d"
                " (\"%s\").", id, filename, errno, strerror(errno));
            return ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ;
        }

        int ret = saveItemToNewFile(filename, data, item);
        if (ret != ERR_FS_NO_ERROR)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - saveItemToNewFile(file = %s)"
                " returned %d.", id, filename, ret);
            return ret;
        }
    }
    else
    {
        bool newItem;
        int ret = saveItemToExistingFile(filename, file, data, item, &newItem);
        if (ret != ERR_FS_NO_ERROR)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - saveItemToExistingFile(file = %s)"
                " returned %d.", id, filename, ret);
            return ret;
        }
    }

    return ERR_FS_NO_ERROR;
}

int daos_getFileVersion(Daos_t daos, const char* filename, DaosFileVersion_t* libraryVersion,
    DaosFileVersion_t* fileVersion)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    int ret = getFileTableVersion(file, libraryVersion, fileVersion);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTableVersion(file = %s) returned %d.",
            filename, ret);
        fclose(file);
        return ret;
    }

    fclose(file);

    return ERR_FS_NO_ERROR;
}

int daos_getFileTable(Daos_t daos, const char* filename, DaosFileVersion_t* fileVersion,
    DaosCount_t* capacity, DaosCount_t* numItemsInFile, FileTableEntry_t** fileTable)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ;
    }

    int ret = getFileTable(file, fileVersion, capacity, numItemsInFile, fileTable);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (file = %s) returned %d.", filename, ret);
        fclose(file);
        return ret;
    }

    fclose(file);

    return ERR_FS_NO_ERROR;
}

//
// Local functions
//

static int saveItemToNewFile(const char* filename, DaosData_t* daosData, const void* item)
{
    FILE* file = fopen(filename, "w+b");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_WRITE;
    }

    int ret = writeInitialFileTableToFile(file, daosData->numItemsPerFile);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeInitialFileTableToFile(file = %s, capacity = %lu)"
            " returned %d.", filename, daosData->numItemsPerFile, ret);
        fclose(file);
        return ret;
    }

    DaosId_t id = daosData->itemFunctions.getPersistentItemId(item);
    DaosSize_t itemSize = 0;

    ret = writeItemToNewFile(file, daosData->directory, daosData->numItemsPerFile,
        &daosData->itemFunctions, item, &itemSize);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeItemToNewFile(file = %s, directory = %s, capacity = %lu,"
            " id = %lu) returned %d.", filename, daosData->directory, daosData->numItemsPerFile, id, ret);
        fclose(file);
        return ret;
    }

    ret = updateFileTableInFile(file, id, itemSize);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: updateFileTableInFile(file = %s, id = %lu,"
            " item size = %lu) returned %d.", filename, id, itemSize, ret);
        fclose(file);
        return ret;
    }

    fclose(file);

    return ERR_FS_NO_ERROR;
}

static int saveItemToExistingFile(const char* filename, FILE* existingFile,
    DaosData_t* daosData, const void* item, bool* newItem)
{
    char tempFilename[PATH_MAX] = { 0 };

    FILE* tempFile = createTempFile(daosData->directory, daosData->filePrefix, tempFilename,
        ARRAY_COUNT(tempFilename));
    if (tempFile == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: createTempFile() failed.");
        fclose(existingFile);
        return ERR_FS_FAILED_TO_OPEN_FILE_FOR_WRITE;
    }

    DaosId_t id = daosData->itemFunctions.getPersistentItemId(item);
    DaosOffset_t existingItemOffset = 0;
    DaosSize_t existingItemSize = 0;

    int ret = getFileTableEntry(existingFile, id, &existingItemOffset, &existingItemSize);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTableEntry(file = %s, id = %lu) returned %d.",
            filename, id, ret);
        fclose(existingFile);
        fclose(tempFile);
        remove(tempFilename);
        return ret;
    }

    DaosSize_t itemSize = 0;
    ret = writeItemToExistingFile(tempFile, existingFile, existingItemOffset,
        existingItemSize, &daosData->itemFunctions, item, &itemSize, newItem);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeItemToExistingFile(tempFile = %s, existingFile"
            " = %s, id = %lu) returned %d.", tempFilename, filename, id, ret);
        fclose(existingFile);
        fclose(tempFile);
        remove(tempFilename);
        return ret;
    }

    fclose(existingFile);

    ret = updateFileTableInFile(tempFile, id, itemSize);
    if (ret != ERR_FS_NO_ERROR)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: updateFileTableInFile(file = %s, id  = %s,"
            " item size = %lu) returned %d.", filename, id, itemSize, ret);
        fclose(tempFile);
        remove(tempFilename);
        return ret;
    }

    fclose(tempFile);

    if (rename(tempFilename, filename) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: rename(old = %s, new = %s) failed. errno = %d"
            " (\"%s\").", tempFilename, filename, errno, strerror(errno));
        remove(tempFilename);
        return ERR_FS_FILE_RENAME_FAILED;
    }

    return ERR_FS_NO_ERROR;
}

static void generateFilename(DaosData_t* data, DaosId_t id, char* filename, size_t filenameLength)
{
    if (data->dirDepth > 1 && data->dirCount > 1)
    {
        int dirNameVect[DAOS_MAX_DIRECTORY_DEPTH];
        int fileName = id / data->numItemsPerFile; // we could store the file in root folder without collision

        id /= data->dirCount; // number of files in the last directory
        for (size_t i = 0; i < data->dirDepth - 1; i++)
        {
            dirNameVect[i] = id % data->dirCount;
            id = id / data->dirCount;
        }
        dirNameVect[data->dirDepth - 1] = id;

        int bytesWritten = 0;
        bytesWritten += snprintf(&filename[bytesWritten], filenameLength - bytesWritten, "%s", data->directory);
        for (ssize_t i = data->dirDepth - 1; i >= 0; i--)
        {
            bytesWritten += snprintf(&filename[bytesWritten], filenameLength - bytesWritten, "/%d", dirNameVect[i]);
            mkdir(filename, 0755);
        }
        snprintf(&filename[bytesWritten], filenameLength - bytesWritten, "/%d", fileName);
    }
    else
    {
        DaosId_t group = id / data->numItemsPerFile;
        snprintf(filename, filenameLength, "%s/%d", data->directory, group);
    }
}

static FILE* createTempFile(const char* directory, const char* filePrefix,
    char* filename, size_t filenameLength)
{
    snprintf(filename, filenameLength, "%s/temp_%sXXXXXX", directory, filePrefix);

    int fd = mkstemp(filename);
    if (fd < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: mkstemp(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return NULL;
    }

    FILE* file = fdopen(fd, "w+b");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fdopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        close(fd);
        return NULL;
    }

    return file;
}
