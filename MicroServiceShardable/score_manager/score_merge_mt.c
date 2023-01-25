#include <logger.h>
#include <score_file.h>
#include <app_errors.h>
#include <score_merge.h>
#include <score_merge_mt.h>
#include <score_manager.h>
#include <request_response_definitions.h>
#include <profiling.h>
#include <mt_queue.h>
#include <string.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// slice size guess is a very nice to have to spread work to workerthreads. If best effort fails, some threads might work harder
// this info does not need to be accurate. It helps to be as accurate as possible
static const ScoreFileTypeInfo scfi[SSFT_MAX_SCORE_FILE_TYPE] = {
    {.type = SSFT_NOT_USED_UNINITIALIZED_VALUE, .maxID = 0, .sliceSize = 0},
    {.type = SSFT_COMPANY_SIMILARITY_SCORE, .maxID = 122798834, .sliceSize = 150000000 / MAX_MT_SLICES},
    {.type = SSFT_INDUSTRY_SIMILARITY_SCORE, .maxID = 3956734, .sliceSize = 5000000 / MAX_MT_SLICES},
    {.type = SSFT_TITLE_SIMILARITY_SCORE, .maxID = 86178386, .sliceSize = 100000000 / MAX_MT_SLICES},
    {.type = SSFT_PROFILE_SIMILARITY_SCORE, .maxID = 250000000, .sliceSize = 250000000 / MAX_MT_SLICES},
};

typedef enum WorkerThreadJobType
{
    WT_JT_SHUTDOWN = 0,
    WT_JT_FIND_SLICE,
    WT_JT_MERGE_SLICES,
    WT_JT_MERGE_2_ARRAYS,
    WT_JT_PREPARE_FILE_CONTENT,
    WT_JT_MAX
}WorkerThreadJobType;

//#define _PRINT_WHAT_IS_HAPPENING_IN_SCORE_MANAGER_MT_
#ifdef _PRINT_WHAT_IS_HAPPENING_IN_SCORE_MANAGER_MT_
    #include <assert.h>
    #define SMMT_LOG_MESSAGE(...)    LOG_MESSAGE(DEBUG_LOG_MSG, __VA_ARGS__)
    #define SMMT_ASSERT(x, ...) do{ if(!(x)) {LOG_MESSAGE(DEBUG_LOG_MSG, __VA_ARGS__); assert(0); } }while(0)
#else
    #define SMMT_LOG_MESSAGE(...) do{ }while(0)
    #define SMMT_ASSERT(x, ...) do{ }while(0)
#endif

void* mergeScoreFileWorkerThread(void* param);

const ScoreFileTypeInfo* getScoreFileInfo(int fType)
{
    for (size_t i = SSFT_NOT_USED_UNINITIALIZED_VALUE + 1; i < SSFT_MAX_SCORE_FILE_TYPE; i++)
    {
        if (scfi[i].type == fType)
        {
            return &scfi[i];
        }
    }
    return NULL;
}

int genMTProcessingInfo(const int fType, const ScoreFileLayout* scoreFileMem, ScoreFileLayoutAddedMTData* out_MTData)
{
    StartInlinedProfiling(PE_GEN_MTDATA);

    // get file related info
    const ScoreFileTypeInfo* fileTypeInfo = getScoreFileInfo(fType);
    if (fileTypeInfo == NULL)
    {
        SMMT_LOG_MESSAGE("Missing file type info %d", fType);
        LOG_DEFAULT_APP_ERROR(ERR_SS_UNKNOWN_SCORE_TYPE);
        EndInlinedProfiling(PE_GEN_MTDATA);
        return ERR_SS_UNKNOWN_SCORE_TYPE;
    }
    SMMT_ASSERT(scoreFileMem != NULL, "Parameter scoreFileMem should never be NULL");
    SMMT_ASSERT(out_MTData != NULL, "Parameter out_MTData should never be NULL");

    // make sure we reset all field values
    memset(out_MTData, 0, sizeof(ScoreFileLayoutAddedMTData));
    out_MTData->version = MT_SCORE_MERGE_FILE_INFO_VERSION;
    out_MTData->sliceSize = fileTypeInfo->sliceSize;
    out_MTData->maxId = scoreFileMem->scores[scoreFileMem->count - 1].id;
    out_MTData->idCount = scoreFileMem->count;
    out_MTData->sliceMinUsed = scoreFileMem->scores->id / fileTypeInfo->sliceSize;
    out_MTData->sliceMaxUsed = out_MTData->maxId / fileTypeInfo->sliceSize;

    // count how many IDs are in each slice
    size_t IDCountSlice[MAX_MT_SLICES_UNPLANNED] = { 0 };
    for (size_t i = 0; i < scoreFileMem->count; i++)
    {
        size_t sliceIndex = scoreFileMem->scores[i].id / fileTypeInfo->sliceSize;
        if (sliceIndex >= MAX_MT_SLICES_UNPLANNED)
        {
            SMMT_LOG_MESSAGE("File contains IDs far larger than anticipated. Max expected %d, got %d",
                fileTypeInfo->sliceSize * MAX_MT_SLICES_UNPLANNED, scoreFileMem->scores[i].id);
            LOG_DEFAULT_APP_ERROR(ERR_SS_SCORE_ID_TOO_LARGE);
            out_MTData->invalidSliceInfo = 1;
            EndInlinedProfiling(PE_GEN_MTDATA);
            return ERR_SS_SCORE_ID_TOO_LARGE;
        }
        // first time init
        if (IDCountSlice[sliceIndex] == 0 && sliceIndex > 0)
        {
            out_MTData->slices[sliceIndex].startIndex = i;
        }
        // from the start, how many IDs still fall into this slice
        IDCountSlice[sliceIndex]++;
    }

    // sum up IDcounts
    out_MTData->slices[out_MTData->sliceMinUsed].IDCountTotal = IDCountSlice[out_MTData->sliceMinUsed];
    for (size_t i = out_MTData->sliceMinUsed + 1; i <= out_MTData->sliceMaxUsed; i++)
    {
        out_MTData->slices[i].IDCountTotal = out_MTData->slices[i - 1].IDCountTotal + IDCountSlice[i];
    }

    EndInlinedProfiling(PE_GEN_MTDATA);

    return ERR_SS_NO_ERROR;
}

static int checkFileHasCompatibleMTInfo(int ftype, char* fileData, int fileSize, ScoreFileLayoutAddedMTData **outHasCompatibleMTData)
{
    SMMT_ASSERT(fileData != NULL, "Parameter fileData should never be NULL");
    SMMT_ASSERT(outHasCompatibleMTData != NULL, "Parameter outHasCompatibleMTData should never be NULL");

    *outHasCompatibleMTData = NULL;

    const ScoreFileLayout* scoreFileMem = (const ScoreFileLayout*)fileData;
    int expectedRawFileSize = SCORE_FILE_SIZE(scoreFileMem);
    if (expectedRawFileSize >= fileSize)
    {
        SMMT_LOG_MESSAGE("File size %d<=%d is too small to contain MTData", fileSize, expectedRawFileSize);
        return ERR_SS_NO_ERROR;
    }

    // it has some sort of MT data. Is it good for us ?
    int expectedMTDataFileSize = expectedRawFileSize + sizeof(ScoreFileLayoutAddedMTData);
    if (expectedMTDataFileSize != fileSize)
    {
        SMMT_LOG_MESSAGE("File size %d!=%d not match MTData size", fileSize, expectedMTDataFileSize);
        return ERR_SS_NO_ERROR;
    }

    ScoreFileLayoutAddedMTData* out_MTData = (ScoreFileLayoutAddedMTData*)&fileData[expectedRawFileSize];
    if(out_MTData->version != MT_SCORE_MERGE_FILE_INFO_VERSION)
    {
        SMMT_LOG_MESSAGE("File MTData has version %d, expected %d", out_MTData->version, MT_SCORE_MERGE_FILE_INFO_VERSION);
        return ERR_SS_NO_ERROR;
    }

    const ScoreFileTypeInfo* fileTypeInfo = getScoreFileInfo(ftype);
    if (fileTypeInfo == NULL)
    {
        SMMT_LOG_MESSAGE("File MTData type %d is not recognized", out_MTData->fileType);
        return ERR_SS_NO_ERROR;
    }

    if(out_MTData->sliceSize != fileTypeInfo->sliceSize)
    {
        SMMT_LOG_MESSAGE("File MTData slice size is unexpected %d != %d", out_MTData->sliceSize, fileTypeInfo->sliceSize);
        return ERR_SS_NO_ERROR;
    }

    *outHasCompatibleMTData = out_MTData;

    return ERR_SS_NO_ERROR;
}

int getSetMTProcessingInfo(const char* fileName, int fType, char* fileData, int fileSize, ScoreFileLayoutAddedMTData** out_MTData, int* deallocMTData)
{
    SMMT_ASSERT(fileName != NULL, "Parameter fileName should never be NULL");
    SMMT_ASSERT(fileData != NULL, "Parameter fileData should never be NULL");
    SMMT_ASSERT(out_MTData != NULL, "Parameter out_MTData should never be NULL");
    SMMT_ASSERT(deallocMTData != NULL, "Parameter deallocMTData should never be NULL");

    deallocMTData[0] = 0;
    int checkErr = checkFileHasCompatibleMTInfo(fType, fileData, fileSize, out_MTData);
    if (checkErr == ERR_SS_NO_ERROR && *out_MTData != NULL)
    {
        SMMT_LOG_MESSAGE("File '%s' contained compatible MTData", fileName);
        return ERR_SS_NO_ERROR;
    }

    // first time the file is getting accessed. Generate MTData for it
    deallocMTData[0] = 1;

    // create a new MTdata section for this file
    *out_MTData = (ScoreFileLayoutAddedMTData*)malloc(sizeof(ScoreFileLayoutAddedMTData));
    if (*out_MTData == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", sizeof(ScoreFileLayoutAddedMTData));
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        return ERR_SS_MEMORY_ALLOC_FAILED;

    }

    // generate MTData info for this score file
    int genErr = genMTProcessingInfo(fType, (ScoreFileLayout*)fileData, *out_MTData);
    if (genErr != ERR_SS_NO_ERROR && genErr != ERR_SS_SCORE_ID_TOO_LARGE)
    {
        SMMT_LOG_MESSAGE("Failed to obtain MTData. Error %d", genErr);
        free(*out_MTData);
        *out_MTData = NULL;
        *deallocMTData = 0;
        return genErr;
    }

    // append the data to the file
    FILE *scoreFile = fopen(fileName, "wb");
    if (scoreFile == NULL)
    {
        SMMT_LOG_MESSAGE("Failed to open file '%s' for reading- errno = % d(\"%s\").", fileName, errno, strerror(errno));
        return ERR_SS_FAILED_TO_OPEN_FILE_FOR_READ;
    }
    // save the real content
    const ScoreFileLayout* scoreFileMem = (const ScoreFileLayout*)fileData;
    size_t expectedRawFileSize = SCORE_FILE_SIZE(scoreFileMem);
    size_t bytesWritten = fwrite(fileData, 1, expectedRawFileSize, scoreFile);
    if (bytesWritten != expectedRawFileSize)
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Only written %zu bytes out of %zu for file : %s - errno = %d (\"%s\")",
            bytesWritten, sizeof(ScoreFileLayoutAddedMTData), fileName, errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FILE_WRITE_NOT_COMPLETED);
        fclose(scoreFile);
        return ERR_SS_FILE_WRITE_NOT_COMPLETED;
    }

    // write the newly generated data
    bytesWritten = fwrite(*out_MTData, 1, sizeof(ScoreFileLayoutAddedMTData), scoreFile);
    if (bytesWritten != sizeof(ScoreFileLayoutAddedMTData))
    {
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: Only written %zu bytes out of %zu for file : %s - errno = %d (\"%s\")",
            bytesWritten, sizeof(ScoreFileLayoutAddedMTData), fileName, errno, strerror(errno));
        LOG_DEFAULT_APP_ERROR(ERR_SS_FILE_WRITE_NOT_COMPLETED);
        fclose(scoreFile);
        return ERR_SS_FILE_WRITE_NOT_COMPLETED;
    }

    fclose(scoreFile);

    return ERR_SS_NO_ERROR;
}

typedef struct WorkerThreadParamFindSlice
{
    int* idCountAllSlicesShared; // shared between multiple worker threads. If one calculates a column, others can use it
    int fistSlice;
    int lastSlice;
    int targetIdCount;
    MappedFileStore* inputFiles;
    uint32_t inputFilesCount;
    int resultEndSlice;
}WorkerThreadParamFindSlice;

typedef struct WorkerThreadParamMergeSlices
{
    int startSlice; // process file IDs starting at this index
    int endSlice; // process file IDs until(including) this index
    MappedFileStore* inputFiles;
    uint32_t inputFilesCount;
    SM_NetworkSimilarityScoreData* resultBuff;
    int resultValueCount;
}WorkerThreadParamMergeSlices;

typedef struct WorkerThreadParamMerge2Arrays
{
    int count1;
    SM_NetworkSimilarityScoreData *scores1;
    int freeInput1;
    int count2;
    SM_NetworkSimilarityScoreData *scores2;
    int freeInput2;
    SM_NetworkSimilarityScoreData* resultBuff;
    int resultValueCount;
}WorkerThreadParamMerge2Arrays;

typedef struct WorkerThreadParamPrepareInputFile
{
    MappedFileStore* inputFile; // will contain the output info also
    int MTNotPossible;
    FileLockList_t fileLockList;
    const char** similarityPaths;
    int fType;
    int fId;
    int resultCode; // if there was an error while preparing the content, it will be present here
}WorkerThreadParamPrepareInputFile;

typedef struct WorkerThreadParam
{
    WorkerThreadJobType jobType;
    int threadIndex; // in an array of threads, this is at index XXX

    // don't use union, values from one stage will be used in next
    WorkerThreadParamFindSlice paramFindSlice;
    WorkerThreadParamMergeSlices paramMergeSlices;
    WorkerThreadParamMerge2Arrays paramMerge2Arrays;
    WorkerThreadParamPrepareInputFile paramPrepareFile;

    int* sharedJobsRemaining; // shared between worker threads to signal if full job has been completed
    MtQueue_t resultQueue;
}WorkerThreadParam;

static uint32_t globalWorkerThreadCount = 0;
static pthread_t globalWorkerThreads[MAX_WORKER_THREADS];
MtQueue_t workerThreadJobQueue;

int workerThreadsInit()
{
    globalWorkerThreadCount = sysconf(_SC_NPROCESSORS_ONLN);
    if (globalWorkerThreadCount > MAX_WORKER_THREADS)
    {
        globalWorkerThreadCount = MAX_WORKER_THREADS;
    }
    LOG_MESSAGE(INFO_LOG_MSG, "Using %d availble hardware threads as worker threads.", globalWorkerThreadCount);

    workerThreadJobQueue = mtQueue_init(globalWorkerThreadCount, NULL);

    memset(globalWorkerThreads, 0, sizeof(globalWorkerThreads));
    for (size_t i = 0; i < globalWorkerThreadCount; ++i)
    {
        if (pthread_create(&globalWorkerThreads[i], NULL, mergeScoreFileWorkerThread, NULL) != 0)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Critical error: pthread_create(searchThread = %hu) "
                "failed. errno = %d (\"%s\").", i, errno, strerror(errno));
            return ERR_SS_THREAD_CREATE_FAILED;
        }
    }

    SMMT_LOG_MESSAGE("Done initializing worker threads.");

    return ERR_SS_NO_ERROR;
}

int workerThreadsShutDown()
{
    // push some jobs to make worker threads exit
    int SharedJobCounter = globalWorkerThreadCount;
    WorkerThreadParam workerThreadParam;
    workerThreadParam.jobType = WT_JT_SHUTDOWN;
    workerThreadParam.sharedJobsRemaining = &SharedJobCounter;
    for (size_t i = 0; i < globalWorkerThreadCount; i++)
    {
        mtQueue_push(workerThreadJobQueue, &workerThreadParam);
    }
    // this will wait only for idle threads. Other threads will be shot down
    sleep(1);

    // kill all threads. Hope they managed to exit gracefully. If not, maybe they are frozen
    for (size_t i = 0; i < globalWorkerThreadCount; ++i)
    {
        pthread_cancel(globalWorkerThreads[i]);
    }
    for (size_t i = 0; i < globalWorkerThreadCount; ++i)
    {
        pthread_join(globalWorkerThreads[i], NULL);
    }

    // destroy our queue
    mtQueue_free(workerThreadJobQueue);

    SMMT_LOG_MESSAGE("Merge threads have been shut down.");

    return ERR_SS_NO_ERROR;
}

// 'idCountAllSlices' might be accessed by multiple threads at the same time. So we only write it once. Same value by any thread
static void ensureValueIsSetForSummedSlice(int atIndex, int* idCountAllSlices, MappedFileStore* inputFiles, size_t inputFilesCount)
{
    if (idCountAllSlices[atIndex] == 0)
    {
        size_t sum = 0;
        for (size_t i = 0; i < inputFilesCount; i++)
        {
            sum += inputFiles[i].mtData->slices[atIndex].IDCountTotal;
        }
        idCountAllSlices[atIndex] = sum;
        SMMT_LOG_MESSAGE("At index %d, sum is %zu.", atIndex, sum);
    }
    {
        SMMT_LOG_MESSAGE("At index %d, sum was already ready %zu.", atIndex, idCountAllSlices[atIndex]);
    }
}

typedef struct BinarySearchParam
{
    int treadIndex;
    int startSlice;
    int endSlice;
    int targetIdCount;
    int* idCountAllSlices;
    MappedFileStore* inputFiles;
    uint32_t inputFilesCount;
}BinarySearchParam;

static void getBestMatchSliceEnd(BinarySearchParam* recParam)
{
    while (1)
    {
        SMMT_LOG_MESSAGE("WT-%d: in range %d-%d", recParam->treadIndex, recParam->startSlice, recParam->endSlice);

        // nothing to search. we have only 1 slice to search
        if (recParam->startSlice == recParam->endSlice)
        {
            SMMT_LOG_MESSAGE("WT-%d: start == end == %d ", recParam->treadIndex, recParam->startSlice);
            break;
        }

        // our next pivot point
        int middle = (recParam->startSlice + recParam->endSlice) / 2;

        // make sure we have data to compare with
        ensureValueIsSetForSummedSlice(middle, recParam->idCountAllSlices, recParam->inputFiles, recParam->inputFilesCount);

        SMMT_LOG_MESSAGE("WT-%d: middle at %d value %d target %d ", recParam->treadIndex, middle, recParam->idCountAllSlices[middle], recParam->targetIdCount);

        // in case start + 1 == end
        if (middle == recParam->startSlice)
        {
            // should we pick the upper index ?
            ensureValueIsSetForSummedSlice(recParam->endSlice, recParam->idCountAllSlices, recParam->inputFiles, recParam->inputFilesCount);
            // even if we would pick the upper value, it's still not good enough
            if (recParam->idCountAllSlices[recParam->endSlice] <= recParam->targetIdCount)
            {
                SMMT_LOG_MESSAGE("WT-%d: Mid+1==End val %d target %d.", recParam->treadIndex, recParam->idCountAllSlices[recParam->endSlice], recParam->targetIdCount);
                break;
            }
            // pick the lower index
            recParam->endSlice = middle;
            break;
        }

        // try to find a better match moving to a higher index
        if (recParam->idCountAllSlices[middle] < recParam->targetIdCount)
        {
            SMMT_LOG_MESSAGE("WT-%d: Mid < target %d < %d", recParam->treadIndex, recParam->idCountAllSlices[middle], recParam->targetIdCount);
            recParam->startSlice = middle;
        }
        // try to find a better match moving to a lower index
        else if (recParam->idCountAllSlices[middle] > recParam->targetIdCount)
        {
            SMMT_LOG_MESSAGE("WT-%d: Mid > target %d > %d", recParam->treadIndex, recParam->idCountAllSlices[middle], recParam->targetIdCount);
            recParam->endSlice = middle;
        }
        // probably will never ever happen : ideal match
        else if (recParam->idCountAllSlices[middle] == recParam->targetIdCount)
        {
            SMMT_LOG_MESSAGE("WT-%d: Mid == target %d == %d", recParam->treadIndex, recParam->idCountAllSlices[middle], recParam->targetIdCount);
            recParam->endSlice = middle;
            break;
        }

        // keep on searching
    }

    return;
}

static int mergeScoreFileWorkerThread_FindEndSlice(WorkerThreadParam* threadParam)
{
    WorkerThreadParamFindSlice* param = &threadParam->paramFindSlice;

    // check which worker thread should process which slices
    BinarySearchParam recParam;
    recParam.treadIndex = threadParam->threadIndex;
    recParam.idCountAllSlices = param->idCountAllSlicesShared;
    recParam.inputFiles = param->inputFiles;
    recParam.inputFilesCount = param->inputFilesCount;
    recParam.startSlice = param->fistSlice;
    recParam.endSlice = param->lastSlice;
    recParam.targetIdCount = param->targetIdCount;

    SMMT_LOG_MESSAGE("WT-%d: Searching end slice in range %d-%d", threadParam->threadIndex, recParam.startSlice, recParam.endSlice);

    // search for best possible location
    getBestMatchSliceEnd(&recParam);

    param->resultEndSlice = recParam.endSlice;
    SMMT_LOG_MESSAGE("WT-%d: end slice is %d", threadParam->threadIndex, param->resultEndSlice);

    // mark this job finished
    __atomic_fetch_sub(threadParam->sharedJobsRemaining, 1, __ATOMIC_SEQ_CST);

    if (*threadParam->sharedJobsRemaining == 0)
    {
        mtQueue_push(threadParam->resultQueue, NULL); // we do not directly care about the result, only that there is a result
    }

    return ERR_SS_NO_ERROR;
}

static int mergeScoreFileWorkerThread_MergeSlices(WorkerThreadParam* threadParam)
{
    WorkerThreadParamMergeSlices* param = &threadParam->paramMergeSlices;

    SMMT_LOG_MESSAGE("WT-%d: Merging slice range %d-%d", threadParam->threadIndex, param->startSlice, param->endSlice);

    // make sure someone is not making interpretations on the results
    param->resultBuff = NULL;
    param->resultValueCount = 0;
    ScoreFileScore** __restrict ScoreDataPointers = NULL;
    ScoreFileScore** __restrict ScoreDataEndPointers = NULL;

    // happens when some worker threads have no slices to process
    if (param->endSlice < param->startSlice)
    {
        goto worker_thread_cleanup;
    }

    // for ech file, get a range of scores we need to process
    ssize_t fileStatusesUsed = 0;
    size_t tempPointersSize = param->inputFilesCount * sizeof(ScoreFileScore*);
    ScoreDataPointers = malloc(tempPointersSize);
    ScoreDataEndPointers = malloc(tempPointersSize);

    if (ScoreDataPointers == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", param->inputFilesCount * sizeof(ScoreFileScore*));
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        goto worker_thread_cleanup;
    }

    int maxIdCountInResultSet = 0;
    for (size_t i = 0; i < param->inputFilesCount; i++)
    {
        // does this file fall within the range of slices this thread should process ?
        if (param->inputFiles[i].mtData->sliceMinUsed > param->endSlice ||
            param->inputFiles[i].mtData->sliceMaxUsed < param->startSlice)
        {
            SMMT_LOG_MESSAGE("WT-%d: File %zu does not have ids in selected slice range. File slices %d-%d, selected slices %d-%d", 
                threadParam->threadIndex, i, param->inputFiles[i].mtData->sliceMinUsed, param->inputFiles[i].mtData->sliceMaxUsed,
                param->startSlice, param->endSlice);
            continue;
        }

        // get a valid range of slices we can scan
        size_t curFileStartSlice = param->startSlice;
        if (curFileStartSlice < param->inputFiles[i].mtData->sliceMinUsed)
        {
            curFileStartSlice = param->inputFiles[i].mtData->sliceMinUsed;
        }
        size_t curFileEndSlice = param->endSlice;
        if (curFileEndSlice > param->inputFiles[i].mtData->sliceMaxUsed)
        {
            curFileEndSlice = param->inputFiles[i].mtData->sliceMaxUsed;
        }
        SMMT_LOG_MESSAGE("WT-%d: File %zu actual slice range %zu-%zu", threadParam->threadIndex, i, curFileStartSlice, curFileEndSlice);

        // transform the slice data into score index data
        ssize_t fileScoreStartIndex = param->inputFiles[i].mtData->slices[curFileStartSlice].startIndex;
        ssize_t fileScoreEndIndex = param->inputFiles[i].mtData->slices[curFileEndSlice].IDCountTotal - 1;
        ssize_t fileScoresToProcess = fileScoreEndIndex - fileScoreStartIndex + 1;

        SMMT_ASSERT(fileScoreStartIndex <= fileScoreEndIndex, "Start index should be <= end index");
        SMMT_ASSERT(param->inputFiles[i].mtData->slices[curFileStartSlice].IDCountTotal <= param->inputFiles[i].mtData->slices[curFileEndSlice].IDCountTotal,
            "score count start should be <= end score count");

        // count the number of results we might have at the end
        maxIdCountInResultSet += fileScoresToProcess;

        SMMT_LOG_MESSAGE("WT-%d: File %zu score index range %zd-%zd. Id count %d", threadParam->threadIndex, 
            i, fileScoreStartIndex, fileScoreEndIndex, fileScoresToProcess);

        // range of scores to process
        ScoreDataPointers[fileStatusesUsed] = &(param->inputFiles[i].fileScores->scores[fileScoreStartIndex]);
        ScoreDataEndPointers[fileStatusesUsed] = &(param->inputFiles[i].fileScores->scores[fileScoreEndIndex]); // Non inclusive

        SMMT_LOG_MESSAGE("WT-%d: File %zu score id range %d-%d", threadParam->threadIndex, i,
            ScoreDataPointers[fileStatusesUsed]->id, ScoreDataEndPointers[fileStatusesUsed]->id);
        SMMT_ASSERT(ScoreDataPointers[fileStatusesUsed]->id <= ScoreDataEndPointers[fileStatusesUsed]->id, "Start id should be <= end id");

        // if this was a valid file to process, add to the queue to process
        fileStatusesUsed++;
    }

    // sanity check, should never happen
    if (maxIdCountInResultSet <= 0 || fileStatusesUsed == 0)
    {
        SMMT_LOG_MESSAGE("WT-%d: MTData merged contained no scores", threadParam->threadIndex);
        goto worker_thread_cleanup;
    }

    // allocate memory for the result set
    param->resultBuff = (SM_NetworkSimilarityScoreData*)malloc(maxIdCountInResultSet * sizeof(SM_NetworkSimilarityScoreData));
    if (param->resultBuff == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", maxIdCountInResultSet * sizeof(SM_NetworkSimilarityScoreData));
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        goto worker_thread_cleanup;
    }
    SM_NetworkSimilarityScoreData* __restrict resultBuff = param->resultBuff; // alliasing

    // do the actual merging
    do
    {
        // special case, copy the remaining values to the result set
        if (fileStatusesUsed == 1)
        {
            SMMT_ASSERT(sizeof(SM_NetworkSimilarityScoreData) == sizeof(ScoreFileScore), "Expected score file size == nework score size");
            size_t ValuesRemain = ScoreDataEndPointers[0] - ScoreDataPointers[0] + 1;
            size_t bytesRemain = ValuesRemain * sizeof(ScoreFileScore);
            memcpy(resultBuff, ScoreDataPointers[0], bytesRemain);
            param->resultValueCount += ValuesRemain;
            SMMT_LOG_MESSAGE("WT-%d: Last file copied to result : bytes %zu, values %zu, total values %d. first id %d",
                threadParam->threadIndex, bytesRemain, ValuesRemain, param->resultValueCount, ScoreDataPointers[0]->id);
            goto worker_thread_cleanup;
        }

        size_t minId = ScoreDataPointers[0]->id;
        for (size_t i = 1; i < fileStatusesUsed; i++)
        {
            const size_t idNow = ScoreDataPointers[i]->id;
            if (idNow < minId)
            {
                minId = idNow;
            }
        }
//        SMMT_LOG_MESSAGE("WT-%d: min id %zu", threadParam->threadIndex, minId);

        // This is the strategy of the merge. Right now it's using max score of all the available files
        // In the future, this might change to an average 
        size_t maxScore = 0;
        for (ssize_t i = fileStatusesUsed - 1; i >= 0; --i)
        {
            if (ScoreDataPointers[i]->id == minId)
            {
                // is this the a new max for the same ID ?
                size_t scoreNow = ScoreDataPointers[i]->score;
                if (scoreNow > maxScore)
                {
                    maxScore = scoreNow;
                }
//                SMMT_LOG_MESSAGE("WT-%d: min id %zu at %zd. score %zu. maxscore %zu", threadParam->threadIndex, minId, i, scoreNow, maxScore);

                // if one of the files get consumed, stop processing it
                if (ScoreDataPointers[i] >= ScoreDataEndPointers[i])
                {
                    // list got shorter
                    fileStatusesUsed--;
                    SMMT_ASSERT(ScoreDataPointers[fileStatusesUsed] <= ScoreDataEndPointers[fileStatusesUsed], "Pointer should be in range");
                    SMMT_ASSERT(ScoreDataPointers[i]->id == ScoreDataEndPointers[i]->id, "Last value does not match last value. Mem corruption.");

                    // copy last state in the array to the index that is getting removed
                    ScoreDataPointers[i] = ScoreDataPointers[fileStatusesUsed];
                    ScoreDataEndPointers[i] = ScoreDataEndPointers[fileStatusesUsed];

                    SMMT_LOG_MESSAGE("WT-%d: File at index %zd, last-id %d got consumed. Removing it from the array. Remaining files %zd",
                        threadParam->threadIndex, i, ScoreDataEndPointers[i]->id, fileStatusesUsed);
                    SMMT_LOG_MESSAGE("WT-%d: after consumption, the id is %d. move from %zd to %zd", threadParam->threadIndex, 
                        ScoreDataPointers[i]->id, fileStatusesUsed, i);
                }
                else
                {
                    ScoreDataPointers[i]++;
                }
            }
        }

        // write it to output
        resultBuff->id = minId;
        resultBuff->score = maxScore;
        resultBuff++;
        param->resultValueCount++;

        SMMT_ASSERT(param->resultValueCount <= maxIdCountInResultSet, "Max result count is smaller than actual result count.");
    } while (fileStatusesUsed > 0);

worker_thread_cleanup:
    // mark this job finished
    __atomic_fetch_sub(threadParam->sharedJobsRemaining, 1, __ATOMIC_SEQ_CST);

    if (*threadParam->sharedJobsRemaining == 0)
    {
        mtQueue_push(threadParam->resultQueue, NULL); // we do not directly care about the result, only that there is a result
    }

    free(ScoreDataPointers);
    free(ScoreDataEndPointers);
    return ERR_SS_NO_ERROR;
}

static int mergeScoreFileWorkerThread_Merge2Arrays(WorkerThreadParam* threadParam)
{
    WorkerThreadParamMerge2Arrays* param = &threadParam->paramMerge2Arrays;

    SMMT_LOG_MESSAGE("WT-%d: Merging arrays with sizes %d-%d", threadParam->threadIndex, param->count1, param->count2);

    // for ech file, get a range of scores we need to process
    const size_t resultBuffByteSizeMax = (param->count1 + param->count2) * sizeof(SM_NetworkSimilarityScoreData);
    SM_NetworkSimilarityScoreData* resultBuff = malloc(resultBuffByteSizeMax);
    if (resultBuff == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", resultBuffByteSizeMax);
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        return ERR_SS_MEMORY_ALLOC_FAILED;
    }
    size_t resultValueCount = 0;

    SM_NetworkSimilarityScoreData* scores1 = param->scores1;
    SM_NetworkSimilarityScoreData* scores1Last = &param->scores1[param->count1];

    SM_NetworkSimilarityScoreData* scores2 = param->scores2;
    SM_NetworkSimilarityScoreData* scores2Last = &param->scores2[param->count2];

    SM_NetworkSimilarityScoreData* resultBuffEnd = resultBuff;

    // do the actual merging as long as there are values in one of the 2 arrays
    while (true)
    {
        if (scores1->id < scores2->id)
        {
            // assign values
            *resultBuffEnd = *scores1;
            // advance load / store locations
            resultBuffEnd++;
            scores1++;
            resultValueCount++;
            // done processing the list ?
            if (scores1 >= scores1Last)
            {
                break;
            }
        }
        else if (scores1->id > scores2->id)
        {
            // assign values
            *resultBuffEnd = *scores2;
            // advance load / store locations
            resultBuffEnd++;
            scores2++;
            resultValueCount++;
            // done processing the list ?
            if (scores2 >= scores2Last)
            {
                break;
            }
        }
        else
        {
            //assign values
            resultBuffEnd->id = scores1->id;
            if (scores1->score > scores2->score)
            {
                resultBuffEnd->score = scores1->score;
            }
            else
            {
                resultBuffEnd->score = scores2->score;
            }
            // advance load / store locations
            resultBuffEnd++;
            scores1++;
            scores2++;
            resultValueCount++;
            // done processing one of the lists ?
            if (scores1 >= scores1Last || scores2 >= scores2Last)
            {
                break;
            }
        }
    }

    //copy remaining if there is any
    size_t valuesToCopy = 0;
    if (scores1 < scores1Last)
    {
        valuesToCopy = scores1Last - scores1;
        memcpy(resultBuffEnd, scores1, valuesToCopy * sizeof(SM_NetworkSimilarityScoreData));
    }
    else if (scores2 < scores2Last)
    {
        valuesToCopy = scores2Last - scores2;
        memcpy(resultBuffEnd, scores2, valuesToCopy * sizeof(SM_NetworkSimilarityScoreData));
    }
    resultValueCount += valuesToCopy;

    // resource cleanup
    if (param->freeInput1 == true)
    {
        free(param->scores1);
        param->scores1 = NULL;
    }
    if (param->freeInput2 == true)
    {
        free(param->scores2);
        param->scores2 = NULL;
    }

    param->resultBuff = resultBuff;
    param->resultValueCount = resultValueCount;
    SMMT_LOG_MESSAGE("Thread %zu generated %zu values from %d %d.", threadParam->threadIndex, resultValueCount, param->count1, param->count2);

    // mark this job finished. Main queue will create a new job as long as it's possible
    param->count1 = 0;
    mtQueue_push(threadParam->resultQueue, NULL); // we do not directly care about the result, only that there is a result

    return ERR_SS_NO_ERROR;
}

static int mergeScoreFileWorkerThread_PrepareInputFile(WorkerThreadParam* threadParam)
{
    WorkerThreadParamPrepareInputFile* param = &threadParam->paramPrepareFile;

    SMMT_LOG_MESSAGE("WT-%d: Preparing input file", threadParam->threadIndex);

    param->resultCode = prepareFileToProcess_st(param->fileLockList, param->similarityPaths,
        param->fType, param->fId, param->inputFile, &param->MTNotPossible);

    __atomic_fetch_sub(threadParam->sharedJobsRemaining, 1, __ATOMIC_SEQ_CST);

    if (*threadParam->sharedJobsRemaining == 0)
    {
        mtQueue_push(threadParam->resultQueue, NULL); // we do not directly care about the result, only that there is a result
    }

    return ERR_SS_NO_ERROR;
}

void* mergeScoreFileWorkerThread(void *arg)
{
//    SMMT_ASSERT(param != NULL, "Parameter threadparam should never be NULL");
    WorkerThreadParam* param = NULL;
    while (1)
    {
        param = mtQueue_pop(workerThreadJobQueue);
        
        // signal to kill this thread
        if (param->jobType == WT_JT_SHUTDOWN)
        {
            break;
        }
        else if (param->jobType == WT_JT_FIND_SLICE)
        {
            mergeScoreFileWorkerThread_FindEndSlice(param);
        }
        else if (param->jobType == WT_JT_MERGE_SLICES)
        {
            mergeScoreFileWorkerThread_MergeSlices(param);
        }
        else if (param->jobType == WT_JT_MERGE_2_ARRAYS)
        {
            mergeScoreFileWorkerThread_Merge2Arrays(param);
        }
        else if (param->jobType == WT_JT_PREPARE_FILE_CONTENT)
        {
            mergeScoreFileWorkerThread_PrepareInputFile(param);
        }
    }

    // let shutdown know one less thread is running
    if (param != NULL)
    {
        __atomic_fetch_sub(param->sharedJobsRemaining, 1, __ATOMIC_SEQ_CST);
    }

    SMMT_LOG_MESSAGE("WT exiting");

    return NULL;
}


static void mergeScoreFiles_mt_FindSliceEnds(WorkerThreadParam *workerThreadParams, MappedFileStore* inputFiles, uint32_t inputFilesCount, MtQueue_t resultQueue)
{
    // get the total amount of IDs that need to be merged. Needed to make a fair split of work
    // also get a range where the IDs are located at so we do not check unused slices later
    int totalIDsToProcess = 0;
    int firtSlice = MAX_MT_SLICES_UNPLANNED;
    int lastSlice = 0;
    for (size_t i = 0; i < inputFilesCount; i++)
    {
        totalIDsToProcess += inputFiles[i].mtData->idCount;
        // so later we would not need to check unused slices
        if (inputFiles[i].mtData->sliceMinUsed < firtSlice)
        {
            firtSlice = inputFiles[i].mtData->sliceMinUsed;
        }
        if (inputFiles[i].mtData->sliceMaxUsed > lastSlice)
        {
            lastSlice = inputFiles[i].mtData->sliceMaxUsed;
        }
    }

    // every thread should process ideally this amount of IDs
    int idealIDCountToProcess = totalIDsToProcess / globalWorkerThreadCount;

    // set up worker thread parameters
    int idCountAllSlices[MAX_MT_SLICES_UNPLANNED] = { 0 };

    int sharedJobsRemaining = 0;
    for (size_t i = 0; i < globalWorkerThreadCount; i++)
    {
        workerThreadParams[i].threadIndex = i;
        workerThreadParams[i].jobType = WT_JT_FIND_SLICE;
        workerThreadParams[i].paramFindSlice.fistSlice = firtSlice;
        workerThreadParams[i].paramFindSlice.lastSlice = lastSlice;
        workerThreadParams[i].paramFindSlice.targetIdCount = (i + 1) * idealIDCountToProcess;
        workerThreadParams[i].paramFindSlice.inputFiles = inputFiles;
        workerThreadParams[i].paramFindSlice.inputFilesCount = inputFilesCount;
        workerThreadParams[i].paramFindSlice.idCountAllSlicesShared = idCountAllSlices;
        workerThreadParams[i].sharedJobsRemaining = &sharedJobsRemaining;
        workerThreadParams[i].resultQueue = resultQueue;
        sharedJobsRemaining++;
    }

    // start worker threads
    for (size_t i = 0; i < globalWorkerThreadCount; i++)
    {
        mtQueue_push(workerThreadJobQueue, &workerThreadParams[i]);
    }

    // wait for all worker threads to finish
    mtQueue_pop(resultQueue);
}

static int mergeScoreFiles_mt_MergeSlices(WorkerThreadParam* workerThreadParams, MappedFileStore* inputFiles, uint32_t inputFilesCount, MtQueue_t resultQueue)
{
    int sharedJobsRemaining = 0;
    // make sure slices do not overlap 
    int prevSliceEnd = workerThreadParams->paramFindSlice.fistSlice - 1; // worst case -1
    for (size_t i = 0; i < globalWorkerThreadCount; i++)
    {
        workerThreadParams[i].threadIndex = i;
        workerThreadParams[i].jobType = WT_JT_MERGE_SLICES;
        workerThreadParams[i].paramMergeSlices.startSlice = prevSliceEnd + 1;
        workerThreadParams[i].paramMergeSlices.endSlice = workerThreadParams[i].paramFindSlice.resultEndSlice;
        workerThreadParams[i].paramMergeSlices.inputFiles = inputFiles;
        workerThreadParams[i].paramMergeSlices.inputFilesCount = inputFilesCount;
        workerThreadParams[i].sharedJobsRemaining = &sharedJobsRemaining;
        workerThreadParams[i].resultQueue = resultQueue;

        // next worker thread should not process overlapping slices
        prevSliceEnd = workerThreadParams[i].paramMergeSlices.endSlice;

        sharedJobsRemaining++;

        // rest of the treads would have no work
        if (prevSliceEnd >= workerThreadParams->paramFindSlice.lastSlice)
        {
            break;
        }
    }

    // if too many threads idle ( not expected scenario ), switch to different merge technique
    if (sharedJobsRemaining * 100 / globalWorkerThreadCount < WORKERTHREAD_USAGE_PCT_USE_BINARY_MERGE)
    {
        return ERR_SS_FAILED_TO_SPLIT_MT_WORK;
    }

    // make sure we are not gathering invalid data as results
    for (size_t i = sharedJobsRemaining; i < globalWorkerThreadCount; i++)
    {
        workerThreadParams[i].threadIndex = i;
        workerThreadParams[i].jobType = WT_JT_MAX;
        workerThreadParams[i].paramMergeSlices.resultBuff = NULL;
        workerThreadParams[i].paramMergeSlices.resultValueCount = 0;
    }

    // start worker threads
    for (size_t i = 0; i < globalWorkerThreadCount; i++)
    {
        mtQueue_push(workerThreadJobQueue, &workerThreadParams[i]);
    }

    // wait for all worker threads to finish
    mtQueue_pop(resultQueue);

    return ERR_SS_NO_ERROR;
}

static void mergeScoreFiles_mt_MergeResults(WorkerThreadParam* workerThreadParams, int fType, uint8_t** __restrict outBuf, 
    uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated)
{
    // gather / merge resulting buffers
    size_t totalValuesForResult = 0;
    for (size_t i = 0; i < MAX_WORKER_THREADS; i++)
    {
        totalValuesForResult += workerThreadParams[i].paramMergeSlices.resultValueCount;
    }
    SMMT_LOG_MESSAGE("Total value count from all threads : %zu.", totalValuesForResult);

    // allocate enough buffer for the results
    if (ensureEnoughBufferAvailableForWrite(outBuf, outBytesWritten, outBytesAllocated, totalValuesForResult) != ERR_SS_NO_ERROR)
    {
        SMMT_LOG_MESSAGE("No results gathered due to mempry allocation error.");
        return;
    }

    // prepare result
    SM_NetworkSimilarityScoreHeader* blockData = (SM_NetworkSimilarityScoreHeader*)&(*outBuf)[*outBytesWritten];
    *outBytesWritten += sizeof(SM_NetworkSimilarityScoreHeader);
    blockData->type = fType;
    blockData->blockCount = totalValuesForResult;
    blockData->size = sizeof(SM_NetworkSimilarityScoreHeader) + totalValuesForResult * sizeof(SM_NetworkSimilarityScoreData);

    // copy partial results into final buffer
    for (size_t i = 0; i < MAX_WORKER_THREADS; i++)
    {
        size_t bytesToCopy = workerThreadParams[i].paramMergeSlices.resultValueCount * sizeof(SM_NetworkSimilarityScoreData);
        // in case this worker thread did not produce any results
        if (bytesToCopy == 0 || workerThreadParams[i].paramMergeSlices.resultBuff == NULL)
        {
            continue;
        }
        SMMT_LOG_MESSAGE("Thread %zu generated %d values = %zu bytes.", i, workerThreadParams[i].paramMergeSlices.resultValueCount, bytesToCopy);

        // copy bytes from this worker thread
        memcpy(&(*outBuf)[*outBytesWritten], workerThreadParams[i].paramMergeSlices.resultBuff, bytesToCopy);
        *outBytesWritten += bytesToCopy;
    }
}

static int mergeScoreFiles_mt_UseSlices(int fType, uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated,
    uint32_t* __restrict bytesRead, MappedFileStore* inputFiles, uint32_t inputFilesCount)
{
    int ret = ERR_SS_NO_ERROR;
    StartInlinedProfiling(PE_SPLIT_MT_WORK);
    WorkerThreadParam workerThreadParams[MAX_WORKER_THREADS] = { 0 };

    MtQueue_t resultQueue = mtQueue_init(MAX_WORKER_THREADS, NULL);

    mergeScoreFiles_mt_FindSliceEnds(workerThreadParams, inputFiles, inputFilesCount, resultQueue);
    EndInlinedProfiling(PE_SPLIT_MT_WORK);

    StartInlinedProfiling(PE_MT_MERGE_SLICES);
    int mergeErr = mergeScoreFiles_mt_MergeSlices(workerThreadParams, inputFiles, inputFilesCount, resultQueue);
    if (mergeErr == ERR_SS_NO_ERROR)
    {
        mergeScoreFiles_mt_MergeResults(workerThreadParams, fType, outBuf, outBytesWritten, outBytesAllocated);
    }
    else
    {
        ret = mergeErr;
    }

    // cleanup allocated resources
    mtQueue_free(resultQueue);
    for (size_t i = 0; i < MAX_WORKER_THREADS; i++)
    {
        if (workerThreadParams[i].paramMergeSlices.resultBuff != NULL)
        {
            free(workerThreadParams[i].paramMergeSlices.resultBuff);
        }
    }
    EndInlinedProfiling(PE_MT_MERGE_SLICES);

    return ret;
}

static int mergeScoreFiles_mt_Merge2Arrays(int fType, uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated,
    uint32_t* __restrict bytesRead, MappedFileStore* inputFiles, uint32_t inputFilesCount)
{
    StartInlinedProfiling(PE_MT_MERGE_2ARRAYS);

    MtQueue_t resultQueue = mtQueue_init(inputFilesCount, NULL);

    WorkerThreadParam *workerThreadParams = calloc(inputFilesCount * sizeof(WorkerThreadParam), 1);
    if (workerThreadParams == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", inputFilesCount * sizeof(WorkerThreadParam));
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        mtQueue_free(resultQueue);
        return ERR_SS_MEMORY_ALLOC_FAILED; // if this got triggered, all is doomed
    }

    // feed original arrays into the array processing queue
    for (size_t i = 0; i < (inputFilesCount & (~1)); i += 2)
    {
        workerThreadParams[i].threadIndex = i;
        workerThreadParams[i].jobType = WT_JT_MERGE_2_ARRAYS;
        workerThreadParams[i].resultQueue = resultQueue;

        workerThreadParams[i].paramMerge2Arrays.count1 = inputFiles[i + 0].fileScores->count;
        workerThreadParams[i].paramMerge2Arrays.freeInput1 = 0;
        workerThreadParams[i].paramMerge2Arrays.scores1 = (SM_NetworkSimilarityScoreData*)(inputFiles[i + 0].fileScores->scores);

        workerThreadParams[i].paramMerge2Arrays.count2 = inputFiles[i + 1].fileScores->count;
        workerThreadParams[i].paramMerge2Arrays.freeInput2 = 0;
        workerThreadParams[i].paramMerge2Arrays.scores2 = (SM_NetworkSimilarityScoreData*)(inputFiles[i + 1].fileScores->scores);

        SMMT_LOG_MESSAGE("Queue job to merge ids %d-%d with counts %d-%d", 
            inputFiles[i + 0].id, inputFiles[i +1].id, inputFiles[i + 0].fileScores->count, inputFiles[i + 1].fileScores->count);

        mtQueue_push(workerThreadJobQueue, &workerThreadParams[i]);
    }

    size_t resultsReady = 0;
    // if there is an input array that does not have a matching pair to be queued up to be processed
    // create it as a dummy result. later it will be matched with another result
    if (inputFilesCount & 1)
    {
        workerThreadParams[inputFilesCount - 1].threadIndex = inputFilesCount - 1;
        workerThreadParams[inputFilesCount - 1].jobType = WT_JT_MERGE_2_ARRAYS;

        workerThreadParams[inputFilesCount - 1].paramMerge2Arrays.count1 = 0; // cheat a finished result
        workerThreadParams[inputFilesCount - 1].resultQueue = resultQueue;
        workerThreadParams[inputFilesCount - 1].paramMerge2Arrays.resultValueCount = inputFiles[inputFilesCount - 1].fileScores->count;
        size_t bytesToAlloc = inputFiles[inputFilesCount - 1].fileScores->count * sizeof(SM_NetworkSimilarityScoreData);
        workerThreadParams[inputFilesCount - 1].paramMerge2Arrays.resultBuff = malloc(bytesToAlloc);
        if (workerThreadParams[inputFilesCount - 1].paramMerge2Arrays.resultBuff == NULL)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", bytesToAlloc);
            LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
            mtQueue_free(resultQueue);
            free(workerThreadParams);
            return ERR_SS_MEMORY_ALLOC_FAILED; // if this got triggered, all is doomed
        }
        memcpy(workerThreadParams[inputFilesCount - 1].paramMerge2Arrays.resultBuff, inputFiles[inputFilesCount - 1].fileScores->scores, bytesToAlloc);
        resultsReady++;

        SMMT_LOG_MESSAGE("Queue result, values %d, to merged with other results",
            workerThreadParams[inputFilesCount - 1].paramMerge2Arrays.resultValueCount);
    }

    // loop though results until only 1 remains
    ssize_t pendingJobs = inputFilesCount;
    while(pendingJobs > 1)
    {
        // wait for at least 1 worker thread to finish
        mtQueue_pop(resultQueue);
        pendingJobs -= 2; // 2 lists are down XXXX to remain
        resultsReady++;

        // if we have enough results to queue up a new job
        if (resultsReady == 2)
        {
            // check status. We will only find a match every second loop !
            size_t readyIndex1 = inputFilesCount;
            size_t readyIndex2 = inputFilesCount;
            for (size_t i = 0; i < inputFilesCount; i++)
            {
                // this worker thread is ready
                if (workerThreadParams[i].paramMerge2Arrays.count1 == 0 &&
                    workerThreadParams[i].paramMerge2Arrays.resultBuff != NULL)
                {
                    if (readyIndex1 == inputFilesCount)
                    {
                        readyIndex1 = i;
                    }
                    else if (readyIndex2 == inputFilesCount)
                    {
                        readyIndex2 = i;
                        break;
                    }
                }
            }

            // do we have 2 arrays to process ?
            if (readyIndex1 != inputFilesCount && readyIndex2 != inputFilesCount)
            {
                workerThreadParams[readyIndex1].paramMerge2Arrays.count1 = workerThreadParams[readyIndex1].paramMerge2Arrays.resultValueCount;
                workerThreadParams[readyIndex1].paramMerge2Arrays.freeInput1 = 1;
                workerThreadParams[readyIndex1].paramMerge2Arrays.scores1 = workerThreadParams[readyIndex1].paramMerge2Arrays.resultBuff;
                workerThreadParams[readyIndex1].paramMerge2Arrays.resultBuff = NULL; // do not use it again

                workerThreadParams[readyIndex1].paramMerge2Arrays.count2 = workerThreadParams[readyIndex2].paramMerge2Arrays.resultValueCount;
                workerThreadParams[readyIndex1].paramMerge2Arrays.freeInput2 = 1;
                workerThreadParams[readyIndex1].paramMerge2Arrays.scores2 = workerThreadParams[readyIndex2].paramMerge2Arrays.resultBuff;
                workerThreadParams[readyIndex2].paramMerge2Arrays.resultBuff = NULL; // do not use it again

                pendingJobs += 2; // queue up 2 more lists to be processed

                SMMT_LOG_MESSAGE("Queue job to merge indexes %zu-%zu with counts %d-%d", readyIndex1, readyIndex2, 
                    workerThreadParams[readyIndex1].paramMerge2Arrays.count1, workerThreadParams[readyIndex1].paramMerge2Arrays.count2);

                mtQueue_push(workerThreadJobQueue, &workerThreadParams[readyIndex1]);

                resultsReady -= 2; // used up 2 reaults that should generate a new result later on
            }
            else
            {
                SMMT_LOG_MESSAGE("We were supposed to find 2 results, but only found %d",
                    (readyIndex1 != inputFilesCount) + (readyIndex2 != inputFilesCount));
            }
        }
    }

    SMMT_LOG_MESSAGE("Finished merging. Should have 1 result. Have %d", resultsReady);

    // the one and only remaining array is the real result
    for (size_t i = 0; i < inputFilesCount; i++)
    {
        // this worker thread is ready
        if (workerThreadParams[i].paramMerge2Arrays.count1 == 0 &&
            workerThreadParams[i].paramMerge2Arrays.resultBuff != NULL)
        {
            SMMT_LOG_MESSAGE("Found final result at index %d. Has %d values", i, workerThreadParams[i].paramMerge2Arrays.resultValueCount);
            if (ensureEnoughBufferAvailableForWrite(outBuf, outBytesWritten, outBytesAllocated, workerThreadParams[i].paramMerge2Arrays.resultValueCount) != ERR_SS_NO_ERROR)
            {
                SMMT_LOG_MESSAGE("No results gathered due to mempry allocation error.");
                return ERR_SS_MEMORY_ALLOC_FAILED;
            }
            SM_NetworkSimilarityScoreHeader* blockData = (SM_NetworkSimilarityScoreHeader*)&(*outBuf)[*outBytesWritten];
            *outBytesWritten += sizeof(SM_NetworkSimilarityScoreHeader);
            blockData->type = fType;
            blockData->blockCount = workerThreadParams[i].paramMerge2Arrays.resultValueCount;
            size_t bytesToCopy = workerThreadParams[i].paramMerge2Arrays.resultValueCount * sizeof(SM_NetworkSimilarityScoreData);
            blockData->size = sizeof(SM_NetworkSimilarityScoreHeader) + bytesToCopy;
            // copy bytes from this worker thread
            memcpy(&(*outBuf)[*outBytesWritten], workerThreadParams[i].paramMerge2Arrays.resultBuff, bytesToCopy);
            *outBytesWritten += bytesToCopy;

            free(workerThreadParams[i].paramMerge2Arrays.resultBuff);
            break;
        }
    }

    mtQueue_free(resultQueue);
    free(workerThreadParams);

    EndInlinedProfiling(PE_MT_MERGE_2ARRAYS);

    return ERR_SS_NO_ERROR;
}

int mergeScoreFiles_mt(int fType, uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated,
    uint32_t* __restrict bytesRead, MappedFileStore* inputFiles, uint32_t inputFilesCount)
{
    int mergeRet = mergeScoreFiles_mt_UseSlices(fType, outBuf, outBytesWritten, outBytesAllocated, bytesRead, inputFiles, inputFilesCount);
    if (mergeRet == ERR_SS_FAILED_TO_SPLIT_MT_WORK)
    {
        mergeScoreFiles_mt_Merge2Arrays(fType, outBuf, outBytesWritten, outBytesAllocated, bytesRead, inputFiles, inputFilesCount);
    }
    return ERR_SS_NO_ERROR;
}

int prepareFilesToProcess_mt(FileLockList_t fileLockList, int req_count_capped, const char** __restrict similarityPaths,
    const SSPacketDataScoreRequestBlock* __restrict req, MappedFileStore* inputFiles, uint32_t* outInputFiles,
    uint32_t* MTNotPossible)
{
    MtQueue_t resultQueue = mtQueue_init(req_count_capped, NULL);

    *MTNotPossible = 0;

    WorkerThreadParam* workerThreadParams = calloc(req_count_capped * sizeof(WorkerThreadParam), 1);
    if (workerThreadParams == NULL)
    {
        LOG_MESSAGE(CRITICAL_LOG_MSG, "Error: Failed to allocate %zu bytes ", req_count_capped * sizeof(WorkerThreadParam));
        LOG_DEFAULT_APP_ERROR(ERR_SS_MEMORY_ALLOC_FAILED);
        mtQueue_free(resultQueue);
        return ERR_SS_MEMORY_ALLOC_FAILED; // if this got triggered, all is doomed
    }

    // feed original arrays into the array processing queue
    int sharedJobsRemaining = req_count_capped;
    for (size_t i = 0; i < req_count_capped; i++)
    {
        workerThreadParams[i].threadIndex = i;
        workerThreadParams[i].jobType = WT_JT_PREPARE_FILE_CONTENT;
        workerThreadParams[i].resultQueue = resultQueue;
        workerThreadParams[i].sharedJobsRemaining = &sharedJobsRemaining;

        workerThreadParams[i].paramPrepareFile.fileLockList = fileLockList;
        workerThreadParams[i].paramPrepareFile.inputFile = &inputFiles[i];
        workerThreadParams[i].paramPrepareFile.similarityPaths = similarityPaths;
        workerThreadParams[i].paramPrepareFile.fType = req->type;
        workerThreadParams[i].paramPrepareFile.fId = req->id[i];

        SMMT_LOG_MESSAGE("Queue job to prepare input file index %d, id %d, type %d",
            i, workerThreadParams[i].paramPrepareFile.fId, workerThreadParams[i].paramPrepareFile.fType);

        mtQueue_push(workerThreadJobQueue, &workerThreadParams[i]);
    }

    // wait for all threads to finish work
    mtQueue_pop(resultQueue);

    // in case there were files that did not generate any input, remove them from the array
    *outInputFiles = req_count_capped;
    for (ssize_t i = req_count_capped - 1; i >= 0; --i)
    {
        // make the result array smaller. Move the last element to this new index
        if (workerThreadParams[i].paramPrepareFile.resultCode != ERR_SS_NO_ERROR)
        {
            *outInputFiles = *outInputFiles - 1;
            inputFiles[i] = inputFiles[*outInputFiles];
        }
        else
        {
            if (workerThreadParams[i].paramPrepareFile.MTNotPossible != 0)
            {
                *MTNotPossible = 1;
            }
        }
    }

    SMMT_LOG_MESSAGE("Finished preparing input files. Have %d files", *outInputFiles);

    mtQueue_free(resultQueue);
    free(workerThreadParams);

    return ERR_SS_NO_ERROR;
}