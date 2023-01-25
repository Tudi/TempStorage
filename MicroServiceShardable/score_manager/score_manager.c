#include <logger.h>
#include <score_manager.h>
#include <score_merge_mt.h>
#include <app_errors.h>
#include <request_response_definitions.h>
#include <profiling.h>
#include <score_merge.h>
#include <score_file.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

int scoreManagerInit()
{
    workerThreadsInit();
    return 0;
}

int scoreManagerShutdown()
{
    workerThreadsShutDown();
    return 0;
}

static int processScoreRequestBlock(FileLockList_t fileLockList, const uint8_t* __restrict reqPacket, uint32_t * __restrict bytesRead, const size_t inPacketSize,
    uint8_t ** __restrict outBuf, uint32_t * __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated, const char** __restrict similarityPaths)
{
    const SSPacketDataScoreRequestBlock* __restrict req = (SSPacketDataScoreRequestBlock * )&reqPacket[*bytesRead];
    if (req->type == SSFT_NOT_USED_UNINITIALIZED_VALUE || req->type >= SSFT_MAX_SCORE_FILE_TYPE)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Unknown similarity score type %d\n",
            req->type);
        LOG_DEFAULT_APP_ERROR(ERR_SS_UNKNOWN_SCORE_TYPE);
        // consider this block parsed even though we could not serve any scores for it
        *bytesRead += sizeof(SSPacketDataScoreRequestBlock) + req->count * sizeof(req->id[0]);
        return ERR_SS_UNKNOWN_SCORE_TYPE;
    }
    uint32_t req_count_capped = req->count;
    if(req_count_capped == 0)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Similarity array size is 0(zero)\n");
        LOG_DEFAULT_APP_ERROR(ERR_SS_ID_ARRAY_EMPTY);
        // consider this block parsed even though we could not serve any scores for it
        *bytesRead += sizeof(SSPacketDataScoreRequestBlock) + req->count * sizeof(req->id[0]);
        return ERR_SS_ID_ARRAY_EMPTY;
    }

    // generate statistics of average files / request
    ProfilingAddSectionTimeThreadSafe(PE_GET_SIMILARITY_SCORE_COUNT, req->count * MS_TO_NS, 1);

    // no longer an error, just cap the number of IDs we are going to process
    if (req_count_capped >= MAX_ID_ARRAY_SIZE)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Warning: Similarity array size %d too large. Max %d\n",
            req->count, MAX_ID_ARRAY_SIZE);
        req_count_capped = MAX_ID_ARRAY_SIZE;
    }
    if (sizeof(SSPacketDataScoreRequestBlock) + req->count * sizeof(req->id[0]) > inPacketSize - *bytesRead)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Similarity array requires %zu network bytes. Have %d\n",
            sizeof(SSPacketDataScoreRequestBlock) + req->count * sizeof(req->id[0]), inPacketSize - *bytesRead);
        LOG_DEFAULT_APP_ERROR(ERR_SS_MALFORMED_GET_SCORE_PACKET);
        return ERR_SS_MALFORMED_GET_SCORE_PACKET;
    }
    SM_LOG_MESSAGE("Trying to generate scores for type %d, count %d.", req->type, req_count_capped);

    // prepare the input data
    uint32_t inputFilesCount = 0;
    uint32_t MTNotPossible = 0;
    MappedFileStore inputFiles[MAX_ID_ARRAY_SIZE];

    StartInlinedProfiling(PE_LOAD_SIMILARITY_SCORE_FILE);
    int loadContentErr = prepareFilesToProcess_mt(fileLockList, req_count_capped, similarityPaths, req, inputFiles, &inputFilesCount, &MTNotPossible);
    EndInlinedProfiling(PE_LOAD_SIMILARITY_SCORE_FILE);
    if (loadContentErr != ERR_SS_NO_ERROR)
    {
        return loadContentErr;
    }

    // nothing to process or cleanup
    if (inputFilesCount == 0)
    {
        SM_LOG_MESSAGE("No similarity score files were found to be processed");
        // consider this block parsed even though we could not serve any scores for it
        *bytesRead += sizeof(SSPacketDataScoreRequestBlock) + req->count * sizeof(req->id[0]);
        return ERR_SS_NO_INPUT_SIMILARITY_FILES;
    }

    StartInlinedProfiling(PE_MERGE_SIMILARITY_SCORE_FILES);

    int ret = ERR_SS_NO_ERROR;
    if (inputFilesCount < 2 || MTNotPossible != 0)
    {
        ret = mergeScoreFiles_st(req, outBuf, outBytesWritten, outBytesAllocated, bytesRead, inputFiles, inputFilesCount);
    }
    else
    {
        ret = mergeScoreFiles_mt(req->type, outBuf, outBytesWritten, outBytesAllocated, bytesRead, inputFiles, inputFilesCount);
    }

    // whatevery the result, consider this req block parsed
    *bytesRead += sizeof(SSPacketDataScoreRequestBlock) + req->count * sizeof(req->id[0]); // consider the request block "read"

    // cleanup
    for (size_t i = 0; i < inputFilesCount; i++)
    {
        inputFiles[i].fileScores = NULL; // this is the same pointer as mmappedData, No need to free it
        if (inputFiles[i].notMappedData == 1)
        {
//            SM_LOG_MESSAGE("Deallocating file content at index %zu", i);
            free(inputFiles[i].mmappedData);
        }
        else
        {
            SM_LOG_MESSAGE("Unmapping file at index %zu", i);
            cleanupMappedFile(inputFiles[i].fd,
                inputFiles[i].mmappedData,
                inputFiles[i].mappedSize);
            fileLockList_releaseFile(fileLockList, inputFiles[i].id);
        }
        inputFiles[i].mmappedData = NULL;
        inputFiles[i].mappedSize = 0;
        if (inputFiles[i].freeMTData != 0)
        {
//            SM_LOG_MESSAGE("Deallocating MTData at index %zu", i);
            free(inputFiles[i].mtData);
        }
        inputFiles[i].mtData = NULL;
    }

    EndInlinedProfiling(PE_MERGE_SIMILARITY_SCORE_FILES);

    return ret;
}

int generateScorePacket(FileLockList_t fileLockList, const uint8_t* inPacket, const size_t inPacketSize,
    const char** similarityPaths, uint8_t** outPacket, uint32_t* outPacketWritten, uint32_t* outPacketAllocated)
{
    uint32_t bytesRead = 0;
    int errProcessBlock = ERR_SS_NO_ERROR;
    int ret = ERR_SS_NO_ERROR;
    SM_LOG_MESSAGE("Have %zu bytes to process. Minimum request block size %zu", inPacketSize, sizeof(SSPacketDataScoreRequestBlock) + sizeof(((SSPacketDataScoreRequestBlock*)(0))->id[0]));
    while (errProcessBlock == ERR_SS_NO_ERROR && bytesRead + sizeof(SSPacketDataScoreRequestBlock) + sizeof(((SSPacketDataScoreRequestBlock*)(0))->id[0]) <= inPacketSize)
    {
        uint32_t bytesReadNow = bytesRead;
        errProcessBlock = processScoreRequestBlock(fileLockList, inPacket, &bytesRead, inPacketSize, outPacket, outPacketWritten, outPacketAllocated, similarityPaths);
        // if for some reason no bytes have been consumed, consider it a bug and exit
        // this is just safe coding to avoid deadlocks
        if (bytesReadNow == bytesRead)
        {
            ret = ERR_SS_FAILED_TO_PARSE_GET_REQUEST_DATA;
            SM_LOG_MESSAGE("No bytes have been processed. Breaking parse. Parsed %d. Max available %zu", bytesRead, inPacketSize);
            break;
        }
        // we can recover from these error types
        if (errProcessBlock == ERR_SS_UNKNOWN_SCORE_TYPE || 
            errProcessBlock == ERR_SS_ID_ARRAY_TOO_LARGE || 
            errProcessBlock == ERR_SS_ID_ARRAY_EMPTY ||
            errProcessBlock == ERR_SS_NO_INPUT_SIMILARITY_FILES)
        {
            SM_LOG_MESSAGE("Recoverable error %d. Will continue parsing", errProcessBlock);
            errProcessBlock = ERR_SS_NO_ERROR;
        }
        SM_LOG_MESSAGE("Read %zd bytes of data from score request block. Remaining %zd", bytesRead - bytesReadNow, inPacketSize - bytesRead);
    };
    return ret;
}
