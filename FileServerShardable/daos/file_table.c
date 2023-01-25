#include <file_table.h>
#include <logger.h>
#include <definitions.h>
#include <macro_utils.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//
// Types
//

typedef int64_t DaosCalculation_t;

//
// Constants
//

#define FILE_TABLE_BLOCK_COUNT 1024
#define FILE_TABLE_ENTRY_SIZE (sizeof(DaosOffset_t) + sizeof(DaosSize_t))

//
// Variables
//

static DaosFileVersion_t currentFileVersion = 0x6;

//
// Local prototypes
//

static int updateFileTableBlock(FILE* file, uint8_t* fileTableBlock,
    DaosCount_t numEntries, DaosCalculation_t sizeDifference);
static DaosOffset_t writeFileTableBlockToFile(FILE* file, uint8_t* fileTableBlock,
    DaosCount_t numEntries, DaosOffset_t initialOffset);

//
// External interface
//

int getFileTableEntry(FILE* file, DaosId_t id, DaosOffset_t* itemOffsetInFile, DaosSize_t* itemSize)
{
    rewind(file);

    // Read file version.

    DaosFileVersion_t fileVersion = 0;

    size_t numBytesRead = fread(&fileVersion, 1, sizeof(fileVersion), file);
    if(numBytesRead != sizeof(fileVersion))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(fileVersion) failed. "
            "numBytesRead (%zu) != fileVersion size (%zu).", id, numBytesRead, sizeof(fileVersion));
        return 1;
    }

    if(fileVersion != currentFileVersion)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - file version (%hu) != current version"
            " (%hu).", id, fileVersion, currentFileVersion);
        return 2;
    }

    // Read capacity.

    DaosCount_t capacity = 0;

    numBytesRead = fread(&capacity, 1, sizeof(capacity), file);
    if(numBytesRead != sizeof(capacity))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(capacity) failed. numBytesRead"
            " (%zu) != capacity size (%zu).", id, numBytesRead, sizeof(capacity));
        return 3;
    }

    // Skip number of items in file.

    if(fseek(file, sizeof(DaosCount_t), SEEK_CUR) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(numItems, offset = %zu,"
            " SEEK_CUR) failed.", id, sizeof(DaosCount_t));
        return 4;
    }

    // Read item offset and size in file.

    DaosCount_t itemPositionInTable = id % capacity;

    int64_t itemOffsetInTable = itemPositionInTable * FILE_TABLE_ENTRY_SIZE;

    if(fseek(file, itemOffsetInTable, SEEK_CUR) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(item entry in table, offset ="
            " %zu, SEEK_CUR) failed.", id, itemOffsetInTable);
        return 5;
    }

    numBytesRead = fread(itemOffsetInFile, 1, sizeof(*itemOffsetInFile), file);
    if(numBytesRead != sizeof(*itemOffsetInFile))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(itemOffsetInFile) failed. "
            "numBytesRead (%zu) != itemOffsetInFile size (%zu).", id, numBytesRead,
            sizeof(*itemOffsetInFile));
        return 6;
    }

    numBytesRead = fread(itemSize, 1, sizeof(*itemSize), file);
    if(numBytesRead != sizeof(*itemSize))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(itemSize) failed. numBytesRead"
            " (%zu) != itemSize's size (%zu).", id, numBytesRead, sizeof(*itemSize));
        return 7;
    }

    return 0;
}

int writeInitialFileTableToFile(FILE* file, DaosCount_t capacity)
{
    // Write file version.

    size_t numBytesWritten = fwrite(&currentFileVersion, 1, sizeof(currentFileVersion), file);
    if(numBytesWritten != sizeof(currentFileVersion))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(currentFileVersion) failed. numBytesWritten"
            " (%zu) != currentFileVersion size (%zu).", numBytesWritten,
            sizeof(currentFileVersion));
        return 1;
    }

    // Write capacity.

    numBytesWritten = fwrite(&capacity, 1, sizeof(capacity), file);
    if(numBytesWritten != sizeof(capacity))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(capacity) failed. numBytesWritten (%zu) != "
            "capacity size (%zu).", numBytesWritten, sizeof(capacity));
        return 2;
    }

    // Write number of items.

    DaosCount_t numItemsInFile = 0;

    numBytesWritten = fwrite(&numItemsInFile, 1, sizeof(numItemsInFile), file);
    if(numBytesWritten != sizeof(numItemsInFile))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(numItemsInFile) failed. numBytesWritten (%zu)"
            " != numItemsInFile's size (%zu).", numBytesWritten, sizeof(numItemsInFile));
        return 3;
    }

    // Write file table.

    DaosOffset_t nextOffset = sizeof(currentFileVersion) + sizeof(capacity) + sizeof(numItemsInFile)
        + capacity * FILE_TABLE_ENTRY_SIZE;

    uint8_t fileTableBlock[FILE_TABLE_BLOCK_COUNT * FILE_TABLE_ENTRY_SIZE];

    DaosCount_t numBlocks = capacity / FILE_TABLE_BLOCK_COUNT;

    for(DaosCount_t i = 0; i < numBlocks; ++i)
    {
        nextOffset = writeFileTableBlockToFile(file, fileTableBlock,
            FILE_TABLE_BLOCK_COUNT, nextOffset);
        if(nextOffset == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeFileTableBlockToFile(block %lu / %lu,"
                " numEntries = %lu) failed.", FILE_TABLE_BLOCK_COUNT, i, numBlocks);
            return 4;
        }
    }

    DaosCount_t numRemainingEntries = capacity % FILE_TABLE_BLOCK_COUNT;
    if(numRemainingEntries != 0)
    {
        if(writeFileTableBlockToFile(file, fileTableBlock, numRemainingEntries, nextOffset) == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writefileTableBlockToFile(last block,"
                " numEntries = %lu) failed.", numRemainingEntries);
            return 5;
        }
    }

    return 0;
}

int updateFileTableInFile(FILE* file, DaosId_t id, DaosSize_t itemSize)
{
    rewind(file);

    // Read file version.

    DaosFileVersion_t fileVersion = 0;

    size_t numBytesRead = fread(&fileVersion, 1, sizeof(fileVersion), file);
    if(numBytesRead != sizeof(fileVersion))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(fileVersion) failed. "
            "numBytesRead (%zu) != fileVersion size (%zu).", id, numBytesRead, sizeof(fileVersion));
        return 1;
    }

    if(fileVersion != currentFileVersion)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - file version (%hu) != current"
            " version (%hu).", id, fileVersion, currentFileVersion);
        return 2;
    }

    // Read capacity.

    DaosCount_t capacity = 0;

    numBytesRead = fread(&capacity, 1, sizeof(capacity), file);
    if(numBytesRead != sizeof(capacity))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(capacity) failed. numBytesRead"
            " (%zu) != capacity size (%zu).", id, numBytesRead, sizeof(capacity));
        return 3;
    }

    // Read number of items in file.

    int64_t numItemsInFileOffset = ftell(file);
    DaosCount_t numItemsInFile = 0;

    numBytesRead = fread(&numItemsInFile, 1, sizeof(numItemsInFile), file);
    if(numBytesRead != sizeof(numItemsInFile))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(numItemsInFile) failed. "
            "numBytesRead (%zu) != numItemsInFile size (%zu).", id, numBytesRead,
            sizeof(numItemsInFile));
        return 4;
    }

    // Skip item offset in file.

    DaosCount_t itemPositionInTable = id % capacity;
    int64_t itemOffsetInTable = itemPositionInTable * FILE_TABLE_ENTRY_SIZE;

    if(fseek(file, itemOffsetInTable, SEEK_CUR) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(itemOffsetInTable, offset"
            " = %zu, SEEK_CUR) failed.", id, itemOffsetInTable);
        return 5;
    }

    if(fseek(file, sizeof(DaosOffset_t), SEEK_CUR) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(itemOffset, offset = %zu,"
            " SEEK_CUR) failed.", id, sizeof(DaosOffset_t));
        return 6;
    }

    // Read item size from file.

    int64_t itemSizeInFileOffset = ftell(file);
    DaosSize_t originalItemSize = 0;

    numBytesRead = fread(&originalItemSize, 1, sizeof(originalItemSize), file);
    if(numBytesRead != sizeof(originalItemSize))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fread(originalItemSize) failed."
            " numBytesRead (%zu) != originalItemSize size (%zu).", id, numBytesRead,
            sizeof(originalItemSize));
        return 7;
    }

    // Update item size.

    if(originalItemSize == itemSize) { return 0; }

    if(fseek(file, itemSizeInFileOffset, SEEK_SET) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(itemSizeInFile, offset"
            " = %zu, SEEK_SET) failed.", id, itemSizeInFileOffset);
        return 8;
    }

    size_t numBytesWritten = fwrite(&itemSize, 1, sizeof(itemSize), file);
    if(numBytesWritten != sizeof(itemSize))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fwrite(itemSize) failed. "
            "numBytesWritten (%zu) != itemSize's size (%zu).", id, numBytesWritten,
            sizeof(itemSize));
        return 9;
    }

    if(fflush(file) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fflush(file table entry) failed.");
        return 10;
    }

    // Update number of items in file.

    if(originalItemSize == EMPTY_RECORD_SIZE)
    {
        int64_t originalOffset = ftell(file);

        if(fseek(file, numItemsInFileOffset, SEEK_SET) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(numItemsInFile, offset"
                " = %zu, SEEK_SET) failed.", id, numItemsInFileOffset);
            return 11;
        }

        ++numItemsInFile;
        numBytesWritten = fwrite(&numItemsInFile, 1, sizeof(numItemsInFile), file);
        if(numBytesWritten != sizeof(numItemsInFile))
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(numItemsInFile) failed. numBytesWritten"
                " (%zu) != numItemsInFile size (%zu).", numBytesWritten, sizeof(numItemsInFile));
            return 12;
        }

        if(fflush(file) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fflush(numItems) failed.");
            return 13;
        }

        if(fseek(file, originalOffset, SEEK_SET) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - fseek(rest of profile table, "
                "offset = %zu, SEEK_SET) failed.", id, originalOffset);
            return 14;
        }
    }

    // Update rest of profile table.

    if(itemPositionInTable == capacity - 1) { return 0; }

    uint8_t fileTable[FILE_TABLE_BLOCK_COUNT * FILE_TABLE_ENTRY_SIZE];

    DaosCalculation_t sizeDifference
        = (DaosCalculation_t) itemSize - (DaosCalculation_t) originalItemSize;
    DaosCount_t numRemainingItemsInFile = capacity - itemPositionInTable - 1;
    DaosCount_t numBlocks = numRemainingItemsInFile / FILE_TABLE_BLOCK_COUNT;

    for(DaosCount_t i = 0; i < numBlocks; ++i)
    {
        if(updateFileTableBlock(file, fileTable, FILE_TABLE_BLOCK_COUNT, sizeDifference) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - updateFileTableBlock(block "
                "%lu / %lu, numEntries = %lu) failed.", id, FILE_TABLE_BLOCK_COUNT, i, numBlocks);
            return 15;
        }
    }

    DaosCount_t numLastItems = numRemainingItemsInFile % FILE_TABLE_BLOCK_COUNT;
    if(numLastItems != 0)
    {
        if(updateFileTableBlock(file, fileTable, numLastItems, sizeDifference) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) - updateFileTableBlock(last block,"
                " numEntries = %lu) failed.", id, numLastItems);
            return 16; 
        }
    }

    return 0;
}

int getFileTableVersion(FILE* file, DaosFileVersion_t* libraryVersion,
    DaosFileVersion_t* fileVersion)
{
    rewind(file);

    // Read file version.

    size_t numBytesRead = fread(fileVersion, 1, sizeof(*fileVersion), file);
    if(numBytesRead != sizeof(*fileVersion))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(fileVersion) failed. numBytesRead (%zu) != "
            "fileVersion size (%zu).", numBytesRead, sizeof(*fileVersion));
        return 1;
    }

    *libraryVersion = currentFileVersion;

    return 0;
}

int getFileTable(FILE* file, DaosFileVersion_t* fileVersion, DaosCount_t* capacity,
    DaosCount_t* numItemsInFile, FileTableEntry_t** fileTable)
{
    rewind(file);

    // Read file version.

    size_t numBytesRead = fread(fileVersion, 1, sizeof(*fileVersion), file);
    if(numBytesRead != sizeof(*fileVersion))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(fileVersion) failed. numBytesRead (%zu) != "
            "fileVersion size (%zu).", numBytesRead, sizeof(*fileVersion));
        return 1;
    }

    if(*fileVersion != currentFileVersion)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: file version (%hu) != current version (%hu).",
            *fileVersion, currentFileVersion);
        return 2;
    }

    // Read capacity.

    numBytesRead = fread(capacity, 1, sizeof(*capacity), file);
    if(numBytesRead != sizeof(*capacity))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(capacity) failed. numBytesRead (%zu) != "
            "capacity size (%zu).", numBytesRead, sizeof(*capacity));
        return 3;
    }

    // Read number of items in file.

    numBytesRead = fread(numItemsInFile, 1, sizeof(*numItemsInFile), file);
    if(numBytesRead != sizeof(*numItemsInFile))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(numItemsInFile) failed. numBytesRead (%zu) != "
            "numItemsInFile size (%zu).", numBytesRead, sizeof(numItemsInFile));
        return 4;
    }

    // Read item offsets and sizes in file.

    if(fileTable != NULL)
    {
        *fileTable = (FileTableEntry_t*) malloc(*numItemsInFile * sizeof(FileTableEntry_t));
        if(*fileTable == NULL)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.",
                *numItemsInFile * sizeof(FileTableEntry_t));
            return 5;
        }

        DaosCount_t resultIndex = 0;

        for(DaosCount_t capacityIndex = 0; capacityIndex < *capacity; ++capacityIndex)
        {
            DaosOffset_t itemOffset = 0;
            DaosSize_t itemSize = 0;

            numBytesRead = fread(&itemOffset, 1, sizeof(itemOffset), file);
            if(numBytesRead != sizeof(itemOffset))
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(itemOffset) failed. table entry "
                    "%lu / %lu. numBytesRead (%zu) != itemOffset size (%zu).", capacityIndex,
                    *capacity, numBytesRead, sizeof(itemOffset));
                return 6;
            }

            numBytesRead = fread(&itemSize, 1, sizeof(itemSize), file);
            if(numBytesRead != sizeof(itemSize))
            {
                LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(itemSize) failed. table entry "
                    "%lu / %lu. numBytesRead (%zu) != itemSize's size (%zu).", capacityIndex,
                    *capacity, numBytesRead, sizeof(itemSize));
                return 7;
            }

            if(itemSize > EMPTY_RECORD_SIZE)
            {
                (*fileTable)[resultIndex].offset = itemOffset;
                (*fileTable)[resultIndex].size = itemSize;
                ++resultIndex;
            }
        }
    }

    return 0;
}

//
// Local functions
//

static DaosOffset_t writeFileTableBlockToFile(FILE* file, uint8_t* fileTableBlock,
    DaosCount_t numEntries, DaosOffset_t initialOffset)
{
    uint8_t* tablePtr = fileTableBlock;

    DaosOffset_t itemOffset = initialOffset;
    const DaosSize_t emptyItemSize = EMPTY_RECORD_SIZE;

    for(DaosCount_t i = 0; i < numEntries; ++i)
    {
        memcpy(tablePtr, &itemOffset, sizeof(itemOffset));
        tablePtr += sizeof(itemOffset);
        itemOffset += emptyItemSize;

        memcpy(tablePtr, &emptyItemSize, sizeof(emptyItemSize));
        tablePtr += sizeof(emptyItemSize);
    }

    size_t numBytesToWrite = numEntries * FILE_TABLE_ENTRY_SIZE;
    size_t numBytesWritten = fwrite(fileTableBlock, 1, numBytesToWrite, file);
    if(numBytesWritten != numBytesToWrite)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite() failed. numBytesWritten (%zu) != "
            "entries block size (%zu).", numBytesWritten, numBytesToWrite);
        return 0;
    }

    if(fflush(file) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fflush() failed.");
        return 0;
    }

    return itemOffset;
}

static int updateFileTableBlock(FILE* file, uint8_t* fileTableBlock,
    DaosCount_t numEntries, DaosCalculation_t sizeDifference)
{
    int64_t fileOffset = ftell(file);

    size_t numBytesToRead = numEntries * FILE_TABLE_ENTRY_SIZE;
    size_t numBytesRead = fread(fileTableBlock, 1, numBytesToRead, file);
    if(numBytesRead != numBytesToRead)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread() failed. numBytesRead (%zu) != "
            "entries block size (%zu).", numBytesRead, numBytesToRead);
        return 1;
    }

    uint8_t* tablePtr = fileTableBlock;

    for(DaosCount_t i = 0; i < numEntries; ++i)
    { 
        DaosOffset_t* itemOffset = (DaosOffset_t*) tablePtr;
        DaosCalculation_t auxItemOffset
            = (DaosCalculation_t) *itemOffset + (DaosCalculation_t) sizeDifference;
        *itemOffset = (DaosOffset_t) auxItemOffset;
        tablePtr += FILE_TABLE_ENTRY_SIZE;
    }

    if(fseek(file, fileOffset, SEEK_SET) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(offset = %zu, SEEK_SET) failed.", fileOffset);
        return 2;
    }

    size_t numBytesToWrite = numEntries * FILE_TABLE_ENTRY_SIZE;
    size_t numBytesWritten = fwrite(fileTableBlock, 1, numBytesToWrite, file);
    if(numBytesWritten != numBytesToWrite)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite() failed. numBytesWritten (%zu) != "
            "entries block size (%zu).", numBytesWritten, numBytesToWrite);
        return 3;
    }

    if(fflush(file) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fflush() failed.");
        return 4;
    }

    return 0;
}
