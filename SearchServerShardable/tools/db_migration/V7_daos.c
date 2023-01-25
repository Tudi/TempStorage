#include <item_functions.h>
#include <logger.h>
#include <macro_utils.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>

#define DAOS_ID_TO_NETWORK htonl
#define NETWORK_TO_DAOS_ID ntohl

#define DAOS_COUNT_TO_NETWORK htonl
#define NETWORK_TO_DAOS_COUNT ntohl

typedef uint16_t DaosFileVersion_V7_t;
typedef int32_t DaosId_V7_t;
typedef uint32_t DaosOffset_V7_t;
typedef uint32_t DaosSize_V7_t;
typedef uint32_t DaosCount_V7_t;

#define RECORD_HEADER_SIZE 3
#define RECORD_TRAILER_SIZE 3

#define RECORD_HEADER_BINARY  0xcc, 0xaa, 0x99
#define RECORD_TRAILER_BINARY 0x99, 0xaa, 0xcc

#define BINARY_ITEM_AND_EXTRAS_MAX_SIZE (256 * 1024)
#define BINARY_ITEM_MAX_SIZE \
    (BINARY_ITEM_AND_EXTRAS_MAX_SIZE - RECORD_HEADER_SIZE - RECORD_TRAILER_SIZE)

#define DAOS_FILE_BUFFER_SIZE (4 * BINARY_ITEM_AND_EXTRAS_MAX_SIZE)

#define DAOS_DIRECTORY_LENGTH      2048
#define DAOS_FILE_PREFIX_LENGTH    64
#define DAOS_FILE_EXTENSION_LENGTH 8

typedef struct
{
    void* d;
} Daos_t;

typedef struct
{
    DaosOffset_V7_t offset;
    DaosSize_V7_t size;
} FileTableEntry_V7_t;

#define FILE_TABLE_BLOCK_COUNT 1024
#define FILE_TABLE_ENTRY_SIZE (sizeof(DaosOffset_V7_t) + sizeof(DaosSize_V7_t))

static const uint8_t recordHeader[RECORD_HEADER_SIZE] = { RECORD_HEADER_BINARY };
static const uint8_t recordTrailer[RECORD_TRAILER_SIZE] = { RECORD_TRAILER_BINARY };

static int loadFileBuffer_V7(FILE* file, DaosOffset_V7_t offset, DaosSize_V7_t size, uint8_t* buffer,
    DaosSize_V7_t* numBytesRead)
{
    if (fseek(file, offset, SEEK_SET) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(%lu, SEEK_SET) failed.", offset);
        return 1;
    }

    // Read buffer.

    size_t auxNumBytesRead = fread(buffer, 1, size, file);
    if ((auxNumBytesRead != size) && (feof(file) == 0))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread() failed. auxNumBytesRead (%zu) !="
            " requested size (%lu).", auxNumBytesRead, size);
        return 2;
    }

    *numBytesRead = auxNumBytesRead;

    return 0;
}

static int V7_getFileTable(FILE* file, DaosFileVersion_V7_t* fileVersion, DaosCount_V7_t* capacity,
    DaosCount_V7_t* numItemsInFile, FileTableEntry_V7_t** fileTable, DaosFileVersion_V7_t expectedVersion)
{
    rewind(file);

    // Read file version.

    size_t numBytesRead = fread(fileVersion, 1, sizeof(*fileVersion), file);
    if (numBytesRead != sizeof(*fileVersion))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(fileVersion) failed. numBytesRead (%zu) != "
            "fileVersion size (%zu).", numBytesRead, sizeof(*fileVersion));
        return 1;
    }

    if (*fileVersion != expectedVersion)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: file version (%hu) != current version (%hu).",
            *fileVersion, expectedVersion);
        return 2;
    }

    // Read capacity.

    numBytesRead = fread(capacity, 1, sizeof(*capacity), file);
    if (numBytesRead != sizeof(*capacity))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(capacity) failed. numBytesRead (%zu) != "
            "capacity size (%zu).", numBytesRead, sizeof(*capacity));
        return 3;
    }

    // Read number of items in file.

    numBytesRead = fread(numItemsInFile, 1, sizeof(*numItemsInFile), file);
    if (numBytesRead != sizeof(*numItemsInFile))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(numItemsInFile) failed. numBytesRead (%zu) != "
            "numItemsInFile size (%zu).", numBytesRead, sizeof(numItemsInFile));
        return 4;
    }

    // Read item offsets and sizes in file.

    if (fileTable != NULL)
    {
        *fileTable = (FileTableEntry_V7_t*)malloc(*numItemsInFile * sizeof(FileTableEntry_V7_t));
        if (*fileTable == NULL)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.",
                *numItemsInFile * sizeof(FileTableEntry_V7_t));
            return 5;
        }

        DaosCount_V7_t resultIndex = 0;

        for (DaosCount_V7_t capacityIndex = 0; capacityIndex < *capacity; ++capacityIndex)
        {
            DaosOffset_V7_t itemOffset = 0;
            DaosSize_V7_t itemSize = 0;

            numBytesRead = fread(&itemOffset, 1, sizeof(itemOffset), file);
            if (numBytesRead != sizeof(itemOffset))
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(itemOffset) failed. table entry "
                    "%lu / %lu. numBytesRead (%zu) != itemOffset size (%zu).", capacityIndex,
                    *capacity, numBytesRead, sizeof(itemOffset));
                return 6;
            }

            numBytesRead = fread(&itemSize, 1, sizeof(itemSize), file);
            if (numBytesRead != sizeof(itemSize))
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(itemSize) failed. table entry "
                    "%lu / %lu. numBytesRead (%zu) != itemSize's size (%zu).", capacityIndex,
                    *capacity, numBytesRead, sizeof(itemSize));
                return 7;
            }

            if (itemSize > RECORD_HEADER_SIZE + RECORD_TRAILER_SIZE)
            {
                (*fileTable)[resultIndex].offset = itemOffset;
                (*fileTable)[resultIndex].size = itemSize;
                ++resultIndex;
            }
        }
    }

    return 0;
}

static int V7_processItemRead(FILE* file, const uint8_t* buffer, DaosSize_V7_t itemSize,
    ItemFunctions_t* itemFunctions, void* item, int fileVersion)
{
    // Header

    const uint8_t* offset = buffer;

    if (memcmp(recordHeader, offset, sizeof(recordHeader)) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Incorrect record header.");
        return 1;
    }

    offset += sizeof(recordHeader);

    // Item

    if (itemSize <= (sizeof(recordHeader) + sizeof(recordTrailer)))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Item size too small (%lu), minimum (%zu).", itemSize,
            (sizeof(recordHeader) + sizeof(recordTrailer)));
        return 2;
    }

    DaosSize_V7_t itemDataSize = itemSize - sizeof(recordHeader) - sizeof(recordTrailer);
    int ret = itemFunctions->binaryToPersistentItem(offset, item, fileVersion);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: binaryToPersistentItem() returned %d", ret);
        return 3;
    }

    offset += itemDataSize;

    // Trailer

    if (memcmp(recordTrailer, offset, sizeof(recordTrailer)) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Incorrect record trailer.");
        return 4;
    }

    return 0;
}

int V7_daos_loadAllItemsFromFile(ItemFunctions_t* itemFunctions, const char* filename, void*** items,
    DaosCount_V7_t* numItems, DaosFileVersion_V7_t expectedVersion)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fopen(%s) failed. errno = %d (\"%s\").",
            filename, errno, strerror(errno));
        return 2;
    }

    DaosFileVersion_V7_t fileVersion = 0;
    DaosCount_V7_t capacity = 0;
    FileTableEntry_V7_t* fileTable = NULL;

    int ret = V7_getFileTable(file, &fileVersion, &capacity, numItems, &fileTable, expectedVersion);
    if (ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: V7_getFileTable(file = %s) returned %d.",
            filename, ret);
        fclose(file);
        return 3;
    }

    void** auxItems = (void**)calloc(*numItems, sizeof(void*));
    if (auxItems == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: calloc(items, numItems = %lu) failed.", *numItems);
        free(fileTable);
        fclose(file);
        return 4;
    }

    uint8_t fileBuffer[DAOS_FILE_BUFFER_SIZE];
    DaosOffset_V7_t fileOffsetInBuffer = 0;
    DaosSize_V7_t fileBufferSize = 0;

    for (DaosCount_V7_t i = 0; i < *numItems; ++i)
    {
        if ((fileTable[i].offset + fileTable[i].size) >= (fileOffsetInBuffer + fileBufferSize))
        {
            ret = loadFileBuffer_V7(file, fileTable[i].offset, DAOS_FILE_BUFFER_SIZE, fileBuffer,
                &fileBufferSize);
            if (ret != 0)
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: loadFileBuffer_V7(file = %s, offset = %lu, "
                    "size = %lu)) returned %d.", filename, fileTable[i].offset,
                    DAOS_FILE_BUFFER_SIZE);
                ret = 5;
                break;
            }

            fileOffsetInBuffer = fileTable[i].offset;
        }

        DaosOffset_V7_t adjustedOffset = fileTable[i].offset - fileOffsetInBuffer;

        ret = V7_processItemRead(file, &fileBuffer[adjustedOffset], fileTable[i].size,
            itemFunctions, &auxItems[i], fileVersion);
        if (ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: V7_processItemRead(file = %s, item %lu / %lu)"
                " returned %d.", filename, i, *numItems, ret);
            ret = 6;
            continue;
        }
    }

    free(fileTable);
    fclose(file);
    *items = auxItems;

    return ret;
}
