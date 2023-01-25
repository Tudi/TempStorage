#include <files.h>
#include <definitions.h>
#include <macro_utils.h>
#include <logger.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//
// Variables
//

static const uint8_t recordHeader[RECORD_HEADER_SIZE] = { RECORD_HEADER_BINARY };
static const uint8_t recordTrailer[RECORD_TRAILER_SIZE] = { RECORD_TRAILER_BINARY };

//
// Local prototypes
//

static int copyFileContents(FILE* destFile, FILE* sourceFile, size_t sourceBeginOffset, size_t sourceEndOffset);
static int writeItemToFile(FILE* file, DaosItemFunctions_t* itemFunctions, const void* item, DaosSize_t* itemSize);

//
// External interface
//

int readItemFromFile(FILE* file, DaosOffset_t itemOffset, DaosSize_t itemSize, uint8_t* buffer)
{
    if(fseek(file, itemOffset, SEEK_SET) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(%lu, SEEK_SET) failed.", itemOffset);
        return 1;
    }

    // Read buffer.

    size_t numBytesRead = fread(buffer, 1, itemSize, file);
    if(numBytesRead != itemSize)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread() failed. numBytesRead (%zu) !="
            " itemSize (%lu).", numBytesRead, itemSize);
        return 2;
    }

    return 0;
}

int loadFileBuffer(FILE* file, DaosOffset_t offset, DaosSize_t size, uint8_t* buffer, DaosSize_t* numBytesRead)
{
    if(fseek(file, offset, SEEK_SET) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(%lu, SEEK_SET) failed.", offset);
        return 1;
    }

    // Read buffer.

    size_t auxNumBytesRead = fread(buffer, 1, size, file);
    if((auxNumBytesRead != size) && (feof(file) == 0))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread() failed. auxNumBytesRead (%zu) !="
            " requested size (%lu).", auxNumBytesRead, size);
        return 2;
    }

    *numBytesRead = auxNumBytesRead;

    return 0;
}

int processItemRead(FILE* file, uint8_t* buffer, DaosSize_t itemSize,
    DaosBinaryToItem_t binaryToItem, void** item, uint32_t *itemDataSize)
{

    if (itemSize <= (sizeof(recordHeader) + sizeof(recordTrailer)))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Item size (%lu) lesser than minimum (%zu).", itemSize,
            (sizeof(recordHeader) + sizeof(recordTrailer) + 1));
        return 2;
    }

    // Header

    uint8_t* offset = buffer;

    if(memcmp(recordHeader, offset, sizeof(recordHeader)) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Incorrect record header.");
        return 1;
    }

    offset += sizeof(recordHeader);

    // Item

    *itemDataSize = itemSize - sizeof(recordHeader) - sizeof(recordTrailer);

    *item = offset;

    offset += *itemDataSize;

    // Trailer

    if(memcmp(recordTrailer, offset, sizeof(recordTrailer)) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Incorrect record trailer.");
        return 4;
    }

    return 0;
}

int writeItemToNewFile(FILE* file, const char* directory, DaosCount_t numItemsPerFile,
    DaosItemFunctions_t* itemFunctions, const void* item, DaosSize_t* itemSize)
{
    DaosId_t id = itemFunctions->getPersistentItemId(item);

    DaosCount_t itemPositionInTable = id % numItemsPerFile;
    DaosSize_t emptyItemSize = 0;
    int ret = 0;

    DaosCount_t i = 0;
    for(; i < itemPositionInTable; ++i)
    {
        ret = writeItemToFile(file, itemFunctions, NULL, &emptyItemSize);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: 1st batch, writeItemToFile(NULL) returned %d.",
                ret);
            return 1;
        }
    }

    ret = writeItemToFile(file, itemFunctions, item, itemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (id = %lu) returned %d.", id, ret);
        return 2;
    }

    for(++i; i < numItemsPerFile; ++i)
    {
        ret = writeItemToFile(file, itemFunctions, NULL, &emptyItemSize);
        if(ret != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: 2nd batch, writeItemToFile(NULL) returned %d.",
                ret);
            return 3;
        }
    }

    if(fflush(file) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fflush() failed.");
        return 4;
    }

    return 0;
}

int writeItemToExistingFile(FILE* tempFile, FILE* existingFile, DaosOffset_t existingItemOffset,
    DaosSize_t existingItemSize, DaosItemFunctions_t* itemFunctions, const void* item,
    DaosSize_t* itemSize, bool* newItem)
{
    int ret = copyFileContents(tempFile, existingFile, 0, existingItemOffset);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: copyFileContents(begin = 0, end = %lu) returned %d.",
            existingItemOffset, ret);
        return 1;
    }

    ret = writeItemToFile(tempFile, itemFunctions, item, itemSize);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: writeItemToFile(id = %lu) failed.",
            itemFunctions->getPersistentItemId(item));
        return 2;
    }

    ret = copyFileContents(tempFile, existingFile, existingItemOffset + existingItemSize, 0);
    if(ret != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: copyFileContents(begin = %lu, end = 0) returned %d.",
            existingItemOffset + existingItemSize, ret);
        return 3;
    }

    *newItem = existingItemSize == EMPTY_RECORD_SIZE;

    return 0;
}

//
// Local functions
//

#define FILE_BUFFER_SIZE (1024*1024)

static int copyFileContents(FILE* destFile, FILE* sourceFile, size_t sourceBeginOffset, size_t sourceEndOffset)
{
    if(sourceBeginOffset == 0)
    {
        rewind(sourceFile);
    } 
    else 
    {
        if(fseek(sourceFile, sourceBeginOffset, SEEK_SET) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(sourceBeginOffset = %zu, SEEK_SET)"
                " failed.", sourceBeginOffset);
            return 1;
        }
    }

    if(sourceEndOffset == 0)
    {
        int64_t originalSourceOffset = ftell(sourceFile);

        if(fseek(sourceFile, 0, SEEK_END) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(0, SEEK_END) failed.");
            return 2;
        }

        sourceEndOffset = ftell(sourceFile);

        if(fseek(sourceFile, originalSourceOffset, SEEK_SET) != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fseek(originalSourceOffset = %ll, SEEK_SET)"
                " failed.", originalSourceOffset);
            return 3;
        }
    }

    size_t contentsSize = sourceEndOffset - sourceBeginOffset;
    
    uint8_t buffer[FILE_BUFFER_SIZE];
    size_t numBlocks = contentsSize / FILE_BUFFER_SIZE;

    size_t i = 0;
    for(; i < numBlocks; ++i)
    {
        size_t numBytesRead = fread(buffer, 1, sizeof(buffer), sourceFile);
        if(numBytesRead != sizeof(buffer))
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(block %zu / %zu) failed. numBytesRead"
                " (%zu) != buffer size (%zu).", i, numBlocks, numBytesRead, sizeof(buffer));
            return 4;
        }

        size_t numBytesWritten = fwrite(buffer, 1, sizeof(buffer), destFile);
        if(numBytesWritten != sizeof(buffer))
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(block %zu / %zu) failed. numBytesWritten"
                " (%zu) != buffer size (%zu).", i, numBlocks, numBytesWritten, sizeof(buffer));
            return 5;
        }
    }

    size_t remainingContentsSize = contentsSize % FILE_BUFFER_SIZE;
    if(remainingContentsSize != 0)
    {
        size_t numBytesRead = fread(buffer, 1, remainingContentsSize, sourceFile);
        if(numBytesRead != remainingContentsSize)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fread(last block) failed. numBytesRead (%zu) != "
                "buffer size (%zu).", numBytesRead, remainingContentsSize);
            return 6;
        }

        size_t numBytesWritten = fwrite(buffer, 1, remainingContentsSize, destFile);
        if(numBytesWritten != remainingContentsSize)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(last block) failed. numBytesWritten"
                " (%zu) != buffer size (%zu).", numBytesWritten, remainingContentsSize);
            return 7;
        }
    }

    if(fflush(destFile) != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fflush() failed.");
        return 8;
    }

    return 0;
}

static int writeItemToFile(FILE* file, DaosItemFunctions_t* itemFunctions, const void* item, DaosSize_t* itemSize)
{
    DaosSize_t auxItemSize = 0;

    // Checking buffer size

    uint32_t itemBinarySize = 0;

    if(item != NULL)
    {
        itemBinarySize = itemFunctions->persistentItemBinarySize(item);
        if(itemBinarySize > BINARY_ITEM_MAX_SIZE)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Item binary size too large. Item (id = %lu) binary size (%lu)"
                " > maximum (%lu).", itemFunctions->getPersistentItemId(item), itemBinarySize, BINARY_ITEM_MAX_SIZE);
            return 1;
        }
    }

    // Header

    size_t numBytesWritten = fwrite(recordHeader, 1, sizeof(recordHeader), file);
    if(numBytesWritten != sizeof(recordHeader))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(record header) failed. numBytesWritten"
            " (%zu) != record header size (%zu).", numBytesWritten, sizeof(recordHeader));
        return 2;
    }

    auxItemSize += (DaosSize_t) numBytesWritten;

    // Item

    if(item != NULL)
    {
        numBytesWritten = fwrite(item, 1, itemBinarySize, file);
        if(numBytesWritten != itemBinarySize)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(id = %lu) failed. numBytesWritten (%zu)"
                " != item size (%zu).", itemFunctions->getPersistentItemId(item), numBytesWritten,
                itemBinarySize);
            return 4;
        }

        auxItemSize += (DaosSize_t) numBytesWritten;
    }

    // Trailer

    numBytesWritten = fwrite(recordTrailer, 1, sizeof(recordTrailer), file);
    if(numBytesWritten != sizeof(recordTrailer))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: fwrite(record trailer) failed. numBytesWritten"
            " (%zu) != record trailer size (%zu).", numBytesWritten, sizeof(recordTrailer));
        return 5;
    }

    auxItemSize += (DaosSize_t) numBytesWritten;
    *itemSize = auxItemSize;

    return 0;
}
