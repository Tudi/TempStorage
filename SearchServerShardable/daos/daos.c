#include <daos.h>
#include <file_table.h>
#include <files.h>
#include <logger.h>
#include <daos_versioned.h>
#include <definitions.h>
#include <macro_utils.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

//
// Constants
//

#define DAOS_DIRECTORY_LENGTH      2048
#define DAOS_FILE_PREFIX_LENGTH    64
#define DAOS_FILE_EXTENSION_LENGTH 8

//
// Types
//

typedef
struct
{
    DaosFileVersion_t version;
    char directory[DAOS_DIRECTORY_LENGTH + 1];
    char filePrefix[DAOS_FILE_PREFIX_LENGTH + 1];
    char fileExtension[DAOS_FILE_EXTENSION_LENGTH + 1];
    uint8_t numIdDigits;
    DaosCount_t numItemsPerFile;

    ItemFunctions_t itemFunctions;
} DaosData_t;

//
// Local prototypes
//

static int saveItemToNewFile(const char* filename, DaosData_t* daosData, const void* item);
static int saveItemToExistingFile(const char* filename, FILE* existingFile,
    DaosData_t* daosData, const void* item, bool* isNewItem);

static void generateFilename(DaosData_t* daosData, DaosId_t id, char* filename, size_t filenameLength);
static FILE* createTempFile(const char* directory, const char* filePrefix, char* filename, size_t filenameLength);

//
// External interface
//

Daos_t daos_init(DaosFileVersion_t version, const char* directory, const char* filePrefix, const char* fileExtension,
    uint8_t numIdDigits, DaosCount_t numItemsPerFile, const ItemFunctions_t* itemFunctions)
{
    Daos_t daos = Daos_NULL;

    if(version == 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for version.");
        return daos;
    }

    if(directory == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for directory.");
        return daos;
    }

    if(filePrefix == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for filePrefix.");
        return daos;
    }

    if(fileExtension == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for fileExtension.");
        return daos;
    }

    if(itemFunctions == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL value for itemFunctions.");
        return daos;
    }

    if(strlen(directory) > DAOS_DIRECTORY_LENGTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Directory's (\"%s\") length is greater"
            " than maximum (%zu).", directory, DAOS_DIRECTORY_LENGTH);
        return daos;
    }

    if(strlen(filePrefix) > DAOS_FILE_PREFIX_LENGTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: filePrefix's (\"%s\") length is"
            " greater than maximum (%zu).", filePrefix, DAOS_FILE_PREFIX_LENGTH);
        return daos;
    }

    if(strlen(fileExtension) > DAOS_FILE_EXTENSION_LENGTH)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: fileExtension's (\"%s\") length is "
            "greater than maximum (%zu).", fileExtension, DAOS_FILE_EXTENSION_LENGTH);
        return daos;
    }

    if(numIdDigits == 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for numIdDigits.");
        return daos;
    }

    if(numItemsPerFile == 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid 0 value for numItemsPerFile.");
        return daos;
    }

    DIR* dir = opendir(directory);
    if(dir == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: opendir(%s) failed. errno = %d"
            " (\"%s\").", directory, errno, strerror(errno));
        return daos;
    }

    closedir(dir);

    DaosData_t* daosData = (DaosData_t*) malloc(sizeof(DaosData_t));
    if(daosData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: malloc(size = %zu) failed.",
            sizeof(DaosData_t));
        return daos;
    }

    daosData->version = version;
    strcpy(daosData->directory, directory);
    strcpy(daosData->filePrefix, filePrefix);
    strcpy(daosData->fileExtension, fileExtension);
    daosData->numIdDigits = numIdDigits;
    daosData->numItemsPerFile = numItemsPerFile;
    daosData->itemFunctions = *itemFunctions;

    daos.d = daosData;

    return daos;
}

void daos_free(Daos_t daos)
{
    DaosData_t* daosData = (DaosData_t*) daos.d;
    if(daosData == NULL) { return; }

    free(daosData);
}

bool daos_isNull(Daos_t daos)
{
    return daos.d == NULL;
}

int daos_getItem(Daos_t daos, DaosId_t id, void** item)
{
    DaosData_t* daosData = (DaosData_t*) daos.d;
    if(daosData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return 1;
    }

    char filename[PATH_MAX] = { 0 };

    generateFilename(daosData, id, filename, ARRAY_COUNT(filename));

    FILE* file = fopen(filename, "rb");
    if(file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fopen(%s) failed. errno = %d (\"%s\").",
            id, filename, errno, strerror(errno));
        return 2;
    }

    DaosOffset_t itemOffset = 0;
    DaosSize_t itemSize = 0;

    int ret = getFileTableEntry(file, daosData->version, id, &itemOffset, &itemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - getFileTableEntry(file = %s)"
            " returned %d.", id, filename, ret);
        fclose(file);
        return 3;
    }

    uint8_t itemBinary[BINARY_ITEM_AND_EXTRAS_MAX_SIZE];

    ret = readItemFromFile(file, itemOffset, itemSize, itemBinary);
    fclose(file);

    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - readItemFromFile(file = %s)"
            " returned %d.", id, filename, ret);
        return 4;
    }

    ret = processItemRead(file, itemBinary, itemSize, daosData->itemFunctions.binaryToPersistentItem, item, daosData->version);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - processItemRead(file = %s)"
            " returned %d.", id, filename, ret);
        return 5;
    }

    return 0;
}

int daos_saveItem(Daos_t daos, const void* item, bool* isNewItem)
{
    DaosData_t* daosData = (DaosData_t*) daos.d;
    if(daosData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return 1;
    }

    DaosId_t id = daosData->itemFunctions.getPersistentItemId(item);

    char filename[PATH_MAX] = { 0 };

    generateFilename(daosData, id, filename, ARRAY_COUNT(filename));

    FILE* file = fopen(filename, "r+b");
    if(file == NULL)
    {
        if(errno != ENOENT)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - opendir(%s) failed. errno = %d"
                " (\"%s\").", id, filename, errno, strerror(errno));
            return 2;
        }

        int ret = saveItemToNewFile(filename, daosData, item);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - saveItemToNewFile(file = %s)"
                " returned %d.", id, filename, ret);
            return 3;
        }

        *isNewItem = true;
    }
    else
    {
        int ret = saveItemToExistingFile(filename, file, daosData, item, isNewItem);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - saveItemToExistingFile(file = %s)"
                " returned %d.", id, filename, ret);
            return 4;
        }
    }

    return 0;
}

int daos_getFileVersion(Daos_t daos, const char* fullpathFilename, DaosFileVersion_t* libraryVersion,
    DaosFileVersion_t* fileVersion)
{
    DaosData_t* daosData = (DaosData_t*) daos.d;
    if(daosData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return 1;
    }

    FILE* file = fopen(fullpathFilename, "rb");
    if(file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            fullpathFilename, errno, strerror(errno));
        return 2;
    }

    int ret = getFileTableVersion(file, fileVersion);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTableVersion(file = %s) returned %d.", fullpathFilename, ret);
        fclose(file);
        return 3;
    }

    fclose(file);

    *libraryVersion = daosData->version;

    return 0;
}

int daos_getFileTable(Daos_t daos, const char* fullpathFilename, DaosFileVersion_t* fileVersion,
    DaosCount_t* capacity, DaosCount_t* numItemsInFile, FileTableEntry_t** fileTable)
{
    DaosData_t* daosData = (DaosData_t*) daos.d;
    if(daosData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return 1;
    }

    FILE* file = fopen(fullpathFilename, "rb");
    if(file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            fullpathFilename, errno, strerror(errno));
        return 2;
    }

    int ret = getFileTable(file, daosData->version, fileVersion, capacity, numItemsInFile, fileTable);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTable(file = %s) returned %d.", fullpathFilename, ret);
        fclose(file);
        return 3;
    }

    fclose(file);

    return 0;
}

static int ensureFileIsUpToDate(DaosData_t * daosData, const char* fullpathFilename)
{
    FILE *file = NULL;
    DaosFileVersion_t fileVersion = 0;

    file = fopen(fullpathFilename, "rb");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
                    fullpathFilename, errno, strerror(errno));
        return 1;
    }

    int ret = getFileTableVersion(file, &fileVersion);
    fclose(file); // File no longer needed.
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTableVersion(%s) failed.", fullpathFilename);
        return 2;
    }

    if (fileVersion == daosData->version)
    {
        return 0;
    }

    if (fileVersion != daosData->version - 1)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: version (%d) is too old to migrate.", fileVersion);
        return 3;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Attempting to migrate file (%s) from version (%u) to version (%u).", fullpathFilename, fileVersion, daosData->version);

    Daos_t daos;
    daos.d = daosData;
    ret = upgradeFileVersion(daos, &daosData->itemFunctions, fullpathFilename, fileVersion);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "upgradeFileVersion(%s) returned %d.", fullpathFilename, ret);
        return 4;
    }

    LOG_MESSAGE(DEBUG_LOG_MSG, "Successfully migrated file (%s) to version (%u).", fullpathFilename, daosData->version);
    return 0;
}

int daos_loadAllCachedItemsFromFile(Daos_t daos, const char* fullpathFilename, void*** items, DaosCount_t* numItems)
{
    DaosData_t* daosData = (DaosData_t*) daos.d;
    if(daosData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: Invalid NULL argument for daos.");
        return 1;
    }

    int ret = ensureFileIsUpToDate(daosData, fullpathFilename);
    if (ret != 0)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: ensureFileIsUpToDate(%s) returned %d", fullpathFilename, ret);
        return 2;
    }

    FILE* file = fopen(fullpathFilename, "rb");
    if(file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").", fullpathFilename, errno, strerror(errno));
        return 3;
    }

    DaosFileVersion_t fileVersion = 0;
    DaosCount_t capacity          = 0;
    DaosCount_t auxNumItems       = 0;
    FileTableEntry_t* fileTable   = NULL;

    ret = getFileTable(file, daosData->version, &fileVersion, &capacity, &auxNumItems, &fileTable);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTable(file = %s) returned %d.", fullpathFilename, ret);
        fclose(file);
        return 4;
    }

    void** auxItems = (void**) calloc(auxNumItems, sizeof(void*));
    if(auxItems == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: calloc(items, numItems = %lu) failed.", auxNumItems);
        free(fileTable); 
        fclose(file);
        return 5;
    }

    uint8_t fileBuffer[DAOS_FILE_BUFFER_SIZE];
    DaosOffset_t fileOffsetInBuffer = 0;
    DaosSize_t fileBufferSize       = 0;

    for(DaosCount_t i = 0; i < auxNumItems; ++i)
    {
        if((fileTable[i].offset + fileTable[i].size) >= (fileOffsetInBuffer + fileBufferSize))
        {
            ret = loadFileBuffer(file, fileTable[i].offset, DAOS_FILE_BUFFER_SIZE, fileBuffer, &fileBufferSize);
            if(ret != 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: loadFileBuffer(file = %s, offset = %lu, size = %lu)"
                    " returned %d.", fullpathFilename, fileTable[i].offset, DAOS_FILE_BUFFER_SIZE);
                daosData->itemFunctions.freeCachedList(auxItems, auxNumItems);
                free(fileTable); 
                fclose(file);
                return 6;
            }

            fileOffsetInBuffer = fileTable[i].offset;
        }

        DaosOffset_t adjustedOffset = fileTable[i].offset - fileOffsetInBuffer;

        ret = processItemRead(file, &fileBuffer[adjustedOffset], fileTable[i].size,
            daosData->itemFunctions.binaryToCachedItem, &auxItems[i], fileVersion);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: processItemRead(file = %s, item %lu / %lu) returned %d.",
                fullpathFilename, i, auxNumItems, ret);
            daosData->itemFunctions.freeCachedList(auxItems, auxNumItems);
            free(fileTable); 
            fclose(file);
            return 7;
        }
    }

    free(fileTable); 
    fclose(file);
    *items = auxItems;
    *numItems = auxNumItems;

    return 0;
}

//
// Local functions
//

static int saveItemToNewFile(const char* filename, DaosData_t* daosData, const void* item)
{
    FILE* file = fopen(filename, "w+b");
    if(file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return 1;
    }

    int ret = writeInitialFileTableToFile(file, daosData->version, daosData->numItemsPerFile);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeInitialFileTableToFile(file = %s, capacity = %lu) returned %d.",
            filename, daosData->numItemsPerFile, ret);
        fclose(file);
        return 2;
    }

    DaosId_t id = daosData->itemFunctions.getPersistentItemId(item);
    DaosSize_t itemSize = 0;

    ret = writeItemToNewFile(file, daosData->directory, daosData->numItemsPerFile,
        &daosData->itemFunctions, item, &itemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeItemToNewFile(file = %s, directory = %s, capacity = %lu,"
            " id = %lu) returned %d.", filename, daosData->directory, daosData->numItemsPerFile, id, ret);
        fclose(file);
        return 3;
    }

    ret = updateFileTableInFile(file, daosData->version, id, itemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: updateFileTableInFile(file = %s, id = %lu,"
            " item size = %lu) returned %d.", filename, id, itemSize, ret);
        fclose(file);
        return 4;
    }

    fclose(file);

    return 0;
}

static int saveItemToExistingFile(const char* filename, FILE* existingFile,
    DaosData_t* daosData, const void* item, bool* isNewItem)
{
    char tempFilename[PATH_MAX] = { 0 };

    FILE* tempFile = createTempFile(daosData->directory, daosData->filePrefix, tempFilename,
        ARRAY_COUNT(tempFilename));
    if(tempFile == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: createTempFile() failed.");
        fclose(existingFile);
        return 1;
    }

    DaosId_t id = daosData->itemFunctions.getPersistentItemId(item);
    DaosOffset_t existingItemOffset = 0;
    DaosSize_t existingItemSize = 0;

    int ret = getFileTableEntry(existingFile, daosData->version, id, &existingItemOffset, &existingItemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: getFileTableEntry(file = %s, id = %lu) returned %d.", filename, id, ret);
        fclose(existingFile);
        fclose(tempFile);
        remove(tempFilename);
        return 2;
    }

    DaosSize_t itemSize = 0;
    ret = writeItemToExistingFile(tempFile, existingFile, existingItemOffset,
        existingItemSize, &daosData->itemFunctions, item, &itemSize, isNewItem);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeItemToExistingFile(tempFile = %s, existingFile"
            " = %s, id = %lu) returned %d.", tempFilename, filename, id, ret);
        fclose(existingFile);
        fclose(tempFile);
        remove(tempFilename);
        return 3;
    }

    fclose(existingFile);

    ret = updateFileTableInFile(tempFile, daosData->version, id, itemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: updateFileTableInFile(file = %s, id  = %s,"
            " item size = %lu) returned %d.", filename, id, itemSize, ret);
        fclose(tempFile);
        remove(tempFilename);
        return 4;
    }

    fclose(tempFile);

    if(rename(tempFilename, filename) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: rename(old = %s, new = %s) failed. errno = %d"
            " (\"%s\").", tempFilename, filename, errno, strerror(errno));
        remove(tempFilename);
        return 5;
    }

    return 0;
}

static void generateFilename(DaosData_t* daosData, DaosId_t id, char* filename, size_t filenameLength)
{
    DaosId_t group = id / daosData->numItemsPerFile;

    snprintf(filename, filenameLength, "%s/%s%0*u.%s",
        daosData->directory, daosData->filePrefix, daosData->numIdDigits, group, daosData->fileExtension);
}

static FILE* createTempFile(const char* directory, const char* filePrefix, char* filename, size_t filenameLength)
{
    snprintf(filename, filenameLength, "%s/temp_%sXXXXXX", directory, filePrefix);

    int fd = mkstemp(filename);
    if(fd < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: mkstemp(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return NULL;
    }

    FILE* file = fdopen(fd, "w+b");
    if(file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fdopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        close(fd);
        return NULL;
    }

    return file;
}
