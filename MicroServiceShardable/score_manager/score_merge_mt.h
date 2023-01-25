#ifndef _SCORE_MERGE_MT_H_
#define _SCORE_MERGE_MT_H_

#include <score_file.h>
#include <request_response_definitions.h>
#include <stddef.h>
#include <stdint.h> 

// if you change any info regarding ScoreFileLayoutAddedMTData, increment this number
#define MT_SCORE_MERGE_FILE_INFO_VERSION 1

// could make it dynamic, but one less allocation is one less headache
#define MAX_WORKER_THREADS 200
#define WORKERTHREAD_USAGE_PCT_USE_BINARY_MERGE 50  // if half of the worker threads failed to find work for slice merge, use arraymerge

// used as hint to describe a specific score file type
typedef struct ScoreFileTypeInfo
{
    int type;
    int maxID;
    int sliceSize;
}ScoreFileTypeInfo;

int workerThreadsInit();
int workerThreadsShutDown();

/// <summary>
/// Get hints on a score file
/// </summary>
/// <param name="fType"></param>
/// <returns></returns>
const ScoreFileTypeInfo* getScoreFileInfo(int fType);

/// <summary>
/// Generate helper data for processing this array by multiple threads
/// Used by File save or file access
/// </summary>
/// <param name="fType"></param>
/// <param name="scoreFileMem"></param>
/// <param name="outMTData"></param>
/// <returns></returns>
int genMTProcessingInfo(const int fType, const ScoreFileLayout* scoreFileMem, ScoreFileLayoutAddedMTData *outMTData);

/// <summary>
/// Checks if the file data contains the MTData block. If not, it will generate one and append it to the file
/// </summary>
/// <param name="fileName">Name of the file to be checked or updated</param>
/// <param name="fType">Score file type</param>
/// <param name="fileData">Content of the file that has already been read</param>
/// <param name="fileSize">Size of the file content</param>
/// <param name="outMTData">Pointer to the MTData info. Can be inside the file content or allocated separately if not present</param>
/// <param name="deallocMTData">If MTData was not present in the file, the newly allocated block needs to be fred after usage</param>
/// <returns></returns>
int getSetMTProcessingInfo(const char *fileName, int fType, char* fileData, int fileSize,
    ScoreFileLayoutAddedMTData** outMTData, int *deallocMTData);

/// <summary>
/// Do all the steps required to merge multiple score files into a single score file using worker threads
/// Expects input files to be loaded into memory and contain MTData block
/// </summary>
/// <param name="fType"></param>
/// <param name="outBuf"></param>
/// <param name="outBytesWritten"></param>
/// <param name="outBytesAllocated"></param>
/// <param name="bytesRead"></param>
/// <param name="inputFiles"></param>
/// <param name="inputFilesCount"></param>
/// <returns></returns>
int mergeScoreFiles_mt(int fType, uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated,
    uint32_t* __restrict bytesRead, MappedFileStore* inputFiles, uint32_t inputFilesCount);

/// <summary>
/// Prepare an array of files to be processed by single or multiple threads
/// </summary>
/// <param name="fileLockList"></param>
/// <param name="req_count_capped"></param>
/// <param name="similarityPaths"></param>
/// <param name="req"></param>
/// <param name="inputFiles"></param>
/// <param name="maxInputFiles"></param>
/// <param name="outInputFiles"></param>
/// <param name="MTNotPossible"></param>
/// <returns></returns>
int prepareFilesToProcess_mt(FileLockList_t fileLockList, int req_count_capped, const char** __restrict similarityPaths,
    const SSPacketDataScoreRequestBlock* __restrict req, MappedFileStore* inputFiles, uint32_t* outInputFiles,
    uint32_t* MTNotPossible);
#endif