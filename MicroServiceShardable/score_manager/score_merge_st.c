#include <logger.h>
#include <score_manager.h>
#include <score_file.h>
#include <score_merge_mt.h>
#include <score_merge.h>
#include <app_errors.h>
#include <request_response_definitions.h>
#include <profiling.h>
#include <string.h>
#include <stdlib.h>

int ensureEnoughBufferAvailableForWrite(uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten,
    uint32_t* __restrict outBytesAllocated, const size_t valueCount)
{
    if (*outBytesAllocated < *outBytesWritten + sizeof(SM_NetworkSimilarityScoreHeader)
        + sizeof(SM_NetworkSimilarityScoreData) * valueCount + sizeof(SSPacketHeader))
    {
        size_t new_size = *outBytesAllocated + sizeof(SM_NetworkSimilarityScoreHeader) +
            sizeof(SM_NetworkSimilarityScoreData) * valueCount + sizeof(SSPacketHeader);
        // if we got a valid reallocation request, chances are we will get a lot more
        // preallocate more buffer than actually requested. It's cheaper than reallocating thousands of times
        if (*outBytesAllocated + BUFFER_EXTEND_MIN_SIZE > new_size)
        {
            new_size = *outBytesAllocated + BUFFER_EXTEND_MIN_SIZE;
        }
        uint8_t* newBuf = realloc(*outBuf, new_size);
        if (newBuf == NULL)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes \n", new_size);
            LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
            return ERR_SS_MEMORY_ALLOC_FAILED;
        }
        SM_LOG_MESSAGE("Reallocated to %zu bytes to store scores. Old size %u", new_size, *outBytesAllocated);
        *outBytesAllocated = new_size;
        *outBuf = newBuf;
    }
    return ERR_SS_NO_ERROR;
}

int mergeScoreFiles_st(const SSPacketDataScoreRequestBlock* __restrict req, uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated,
    uint32_t* __restrict bytesRead, MappedFileStore* inputFiles, uint32_t inputFilesCount)
{
    // allocate some buffer to store the scores. We can only guess the required memory
    if (ensureEnoughBufferAvailableForWrite(outBuf, outBytesWritten, outBytesAllocated, inputFiles[0].fileScores->count) != ERR_SS_NO_ERROR)
    {
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }

    SM_NetworkSimilarityScoreHeader* blockData = (SM_NetworkSimilarityScoreHeader*)&(*outBuf)[*outBytesWritten];
    blockData->type = req->type;

    // gen output data
    if (inputFilesCount == 1)
    {
        SM_ASSERT(sizeof(SM_NetworkSimilarityScoreData) == sizeof(ScoreFileScore), "Structure size mismatch !");
        blockData->size = sizeof(SM_NetworkSimilarityScoreHeader) + inputFiles[0].fileScores->count * sizeof(SM_NetworkSimilarityScoreData);
        blockData->blockCount = inputFiles[0].fileScores->count;
        memcpy(blockData->data, inputFiles[0].fileScores->scores, inputFiles[0].fileScores->count * sizeof(SM_NetworkSimilarityScoreData));
        SM_LOG_MESSAGE("Only one similarity score file was found. Attaching it as is. Bytes read %u, bytes written %u",
            *bytesRead, blockData->size);
    }
    else
    {
        size_t maxIdCountTotal = 0;
        for (size_t i = 0; i < inputFilesCount; i++)
        {
            maxIdCountTotal += inputFiles[i].fileScores->count;
        }

        // make sure we have enough memory to write output
        if (ensureEnoughBufferAvailableForWrite(outBuf, outBytesWritten, outBytesAllocated, maxIdCountTotal) != ERR_SS_NO_ERROR)
        {
            return ERR_SS_MEMORY_ALLOC_FAILED;
        }

        // This code block will get executed millions of times
        // Try to make sure to reduce operation count
        size_t blocksWritten = 0;
        size_t inputFilesCountDecrementing = inputFilesCount;
        while (inputFilesCountDecrementing > 0)
        {
            size_t minId = UINT32_MAX;
            for (size_t i = 0; i < inputFilesCountDecrementing; i++)
            {
                size_t indexRead = inputFiles[i].indexRead;
                ScoreFileScore* scores = &inputFiles[i].fileScores->scores[indexRead];
                if (scores->id < minId)
                {
                    minId = scores->id;
                }
            }
            // This is the strategy of the merge. Right now it's using max score of all the available files
            // In the future, this might change to an average 
            size_t maxScore = 0;
            for (ssize_t i = inputFilesCountDecrementing - 1; i >= 0 ; --i)
            {
                size_t indexRead = inputFiles[i].indexRead;
                ScoreFileScore* scores = &inputFiles[i].fileScores->scores[indexRead];
                if (scores->id == minId)
                {
                    if (scores->score > maxScore)
                    {
                        maxScore = scores->score;
                    }
                    if (indexRead + 1 == inputFiles[i].fileScores->count)
                    {
                        // array got shorter
                        inputFilesCountDecrementing--;
                        // move from the end of the array to this index
                        MappedFileStore temp = inputFiles[i];
                        inputFiles[i] = inputFiles[inputFilesCountDecrementing];
                        inputFiles[inputFilesCountDecrementing] = temp;
                        SM_LOG_MESSAGE("File at index %zu got consumed. Removing it from the array. Remaining files %zu", i, inputFilesCountDecrementing);

                        // last file remaining. Copy the remaining values as is
                        if (inputFilesCountDecrementing == 1)
                        {
                            SM_ASSERT(sizeof(SM_NetworkSimilarityScoreData) == sizeof(ScoreFileScore), "Expected score file size == nework score size");
                            size_t valuesRemain = inputFiles[0].fileScores->count - inputFiles[0].indexRead;
                            if (valuesRemain > 0) // should never be 0 ?
                            {
                                size_t bytesRemain = valuesRemain * sizeof(ScoreFileScore);

                                memcpy(&blockData->data[blocksWritten], &inputFiles[0].fileScores->scores[inputFiles[0].indexRead], bytesRemain);
                                blocksWritten += valuesRemain;
                                SM_LOG_MESSAGE("Last file copied to result : bytes %zu, values %zu, total values %d",
                                    bytesRemain, valuesRemain, blocksWritten);
                            }
                            inputFilesCountDecrementing = 0; // sanity, make sure to mark all files used up
                            break;
                        }
                    }
                    else
                    {
                        inputFiles[i].indexRead++;
                    }
                }
            }

            // write it to output
            blockData->data[blocksWritten].id = minId;
            blockData->data[blocksWritten].score = maxScore;
            blocksWritten++;
        };
        blockData->size = sizeof(SM_NetworkSimilarityScoreHeader) + blocksWritten * sizeof(SM_NetworkSimilarityScoreData);
        blockData->blockCount = blocksWritten;
    }
    *outBytesWritten += blockData->size; // includes header size also

    LOG_MESSAGE(DEBUG_LOG_MSG, "Added similarity block type %d, count %d", blockData->type, blockData->blockCount);

    return ERR_SS_NO_ERROR;
}

// in case a similarity file does not exist, self score is always 100% 
static int genSelfMapping(int fType, int selfId, char** mmappedData, size_t* filesize)
{
    SM_ASSERT(mmappedData != NULL, "Parameter mmappedData should never be NULL");
    SM_ASSERT(filesize != NULL, "Parameter filesize should never be NULL");

    SM_LOG_MESSAGE("Generating self mapping for id %d.", selfId);

    *filesize = sizeof(ScoreFileLayout) + sizeof(ScoreFileScore) + sizeof(ScoreFileLayoutAddedMTData);
    *mmappedData = malloc(*filesize);
    if (*mmappedData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes \n", *filesize);
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        *filesize = 0;
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }

    // default init
    memset(*mmappedData, 0, *filesize);

    ScoreFileLayout* selfFile = (ScoreFileLayout*)(*mmappedData);
    selfFile->count = 1;
    selfFile->padding = 0;
    selfFile->scores[0].id = selfId;
    selfFile->scores[0].score = MAX_SCORE_VALUE;

    const ScoreFileTypeInfo* sfi = getScoreFileInfo(fType);
    if (sfi == NULL)
    {
        SM_LOG_MESSAGE("Unknown score type %d when generating MTData for self mapping", fType);
        LOG_DEFAULT_APP_ERROR(ERR_SS_UNKNOWN_SCORE_TYPE);
        return ERR_SS_UNKNOWN_SCORE_TYPE;
    }
    ScoreFileLayoutAddedMTData* selfMTData = (ScoreFileLayoutAddedMTData*)((*mmappedData) + sizeof(ScoreFileLayout) + sizeof(ScoreFileScore));
    selfMTData->version = MT_SCORE_MERGE_FILE_INFO_VERSION;
    selfMTData->idCount = 1;
    selfMTData->maxId = selfId;
    selfMTData->sliceMinUsed = selfId / sfi->sliceSize;
    selfMTData->sliceMaxUsed = selfId / sfi->sliceSize;
    selfMTData->slices[selfMTData->sliceMinUsed].IDCountTotal = 1;
    selfMTData->slices[selfMTData->sliceMinUsed].startIndex = 0;
    selfMTData->sliceSize = sfi->sliceSize;
    selfMTData->invalidSliceInfo = 0;

    return ERR_SS_NO_ERROR;
}

int prepareFileToProcess_st(FileLockList_t fileLockList, const char** __restrict similarityPaths,
    int fType, int fId, MappedFileStore* inputFile, int* MTNotPossible)
{
    char fileName[MAX_PATH_LEN];
    int ret = ERR_SS_NO_ERROR;

    // make sure to reset storage in case it is getting reused after a failed load
    memset(inputFile, 0, sizeof(MappedFileStore));

    // what is the full name for this Type-ID file
    genScoreFileName(similarityPaths[fType], fId, fileName, sizeof(fileName));

    // do not allow "write" threads to change the file content while we are reading it
    fileLockList_lockFileForRead(fileLockList, fId);

    inputFile->notMappedData = 1;
    int errGetFile = getFileContent(fileName, &inputFile->fd,
        &inputFile->mmappedData,
        &inputFile->mappedSize);

    // File might not exist. We always reply with at least 1 ID
    if (errGetFile != ERR_SS_NO_ERROR)
    {
        errGetFile = genSelfMapping(fType, fId, &inputFile->mmappedData,
            &inputFile->mappedSize);
        inputFile->notMappedData = 1;
    }

    // Sanity checks to ensure file content is not corrupted or aything
    if (errGetFile == ERR_SS_NO_ERROR)
    {
        inputFile->fileScores = (ScoreFileLayout*)inputFile->mmappedData;
        // allocated buffer should never be NULL
        if (inputFile->mmappedData == NULL)
        {
            errGetFile = ERR_SS_FAILED_TO_MAP_FILE_FOR_READ;
        }
        // is counter is out of sync with actual file content
        else if (inputFile->fileScores->count < 1 ||
            inputFile->fileScores->count >= MAX_IDS_in_SCORE_FILE)
        {
            errGetFile = ERR_SS_SCORE_FILE_COUNT_UNEXPECTED;
        }
        // file content is truncated : interrupted write
        else if (SCORE_FILE_SIZE(inputFile->fileScores) >
            inputFile->mappedSize)
        {
            errGetFile = ERR_SS_SCORE_FILE_COUNT_BAD;
        }
        // padding is always expected to be 0
        else if (inputFile->fileScores->padding != 0)
        {
            errGetFile = ERR_SS_SCORE_FILE_PADDING_BAD;
        }
        if (errGetFile != ERR_SS_NO_ERROR)
        {
            SM_LOG_MESSAGE("Score file sanity check failed with error %u", errGetFile);
            LOG_DEFAULT_APP_ERROR(errGetFile);
        }
    }

    // set the multi threaded processing info for this file
    int errMTDataGen = getSetMTProcessingInfo(fileName, fType, inputFile->mmappedData,
        inputFile->mappedSize, &inputFile->mtData, &inputFile->freeMTData);
    if (errMTDataGen != ERR_SS_NO_ERROR)
    {
        SM_LOG_MESSAGE("File '%s' can't have MTData. Switching to ST processing", fileName);
        *MTNotPossible = 1;
    }
    if (inputFile->mtData->invalidSliceInfo != 0)
    {
        SM_LOG_MESSAGE("File '%s' can't have MTData. Switching to ST processing", fileName);
        *MTNotPossible = 1;
    }

    if (errGetFile == ERR_SS_NO_ERROR)
    {
        inputFile->fileScores = (ScoreFileLayout*)inputFile->mmappedData;
        inputFile->id = fId;
        SM_LOG_MESSAGE("Opened '%s' file. Size %d. Has %d entries. Min id %d. Max id %d", fileName, inputFile->mappedSize,
            inputFile->fileScores->count, inputFile->fileScores->scores[0].id,
            inputFile->fileScores->scores[inputFile->fileScores->count - 1].id);
    }
    else
    {
        ret = ERR_SS_FAILED_TO_PREPARE_INPUT_FILE;
        // this should already be NULL
        if (inputFile->notMappedData == 1)
        {
            free(inputFile->mmappedData);
            inputFile->mmappedData = NULL;
        }
        inputFile->fileScores = NULL;

        // files that did not have MTData will allocate a separate buffer
        if (inputFile->freeMTData != 0)
        {
            free(inputFile->mtData);
            inputFile->mtData = NULL;
        }
    }

    // only need to hold file lock for mapped files
    if (errGetFile != ERR_SS_NO_ERROR || inputFile->notMappedData == 1)
    {
        fileLockList_releaseFile(fileLockList, fId);
    }

    return ret;
}

int prepareFilesToProcess_st(FileLockList_t fileLockList, int req_count_capped, const char** __restrict similarityPaths,
    const SSPacketDataScoreRequestBlock* __restrict req, MappedFileStore* inputFiles, uint32_t* outInputFiles,
    int* MTNotPossible)
{
    // sanity init
    *MTNotPossible = 0;

    uint32_t inputFilesCount = 0;
    for (size_t i = 0; i < req_count_capped; i++)
    {
        int prepErr = prepareFileToProcess_st(fileLockList, similarityPaths, req->type, req->id[i], &inputFiles[inputFilesCount], MTNotPossible);
        if (prepErr == ERR_SS_NO_ERROR)
        {
            inputFilesCount++;
        }
    }

    *outInputFiles = inputFilesCount;

    return ERR_SS_NO_ERROR;
}