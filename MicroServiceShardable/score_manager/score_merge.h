#ifndef _SCORE_MERGE_H_
#define _SCORE_MERGE_H_

#include <score_file.h>
#include <file_lock_list.h>
#include <request_response_definitions.h>

/// <summary>
/// Merge one or multiple score file content into a single score file
/// Merge operation is MAX between 2 matching IDs
/// Score files are ascending ordered
/// 
/// </summary>
/// <param name="req"></param>
/// <param name="outBuf"></param>
/// <param name="outBytesWritten"></param>
/// <param name="outBytesAllocated"></param>
/// <param name="bytesRead"></param>
/// <param name="inputFiles"></param>
/// <param name="inputFilesCount"></param>
/// <returns></returns>
int mergeScoreFiles_st(const SSPacketDataScoreRequestBlock* __restrict req, uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten, uint32_t* __restrict outBytesAllocated,
    uint32_t* __restrict bytesRead, MappedFileStore* inputFiles, uint32_t inputFilesCount);

/// <summary>
/// Used only when writing a new id-score pair to the resulting network packet
/// makes sure the network packet contains enough free bytes for the write operation to take place
/// It will allocate more bytes than needed to reduce the amount of allocations that will happen through the processing of multiple files
/// </summary>
/// <param name="outBuf"></param>
/// <param name="outBytesWritten"></param>
/// <param name="outBytesAllocated"></param>
/// <param name="valueCount"></param>
/// <returns></returns>
int ensureEnoughBufferAvailableForWrite(uint8_t** __restrict outBuf, uint32_t* __restrict outBytesWritten,
    uint32_t* __restrict outBytesAllocated, const size_t valueCount);

/// <summary>
/// Prepare a single input file to be merged
/// </summary>
/// <param name="fileLockList"></param>
/// <param name="similarityPaths"></param>
/// <param name="fType"></param>
/// <param name="fId"></param>
/// <param name="inputFile"></param>
/// <param name="MTNotPossible"></param>
/// <returns></returns>
int prepareFileToProcess_st(FileLockList_t fileLockList, const char** __restrict similarityPaths,
    int fType, int fId, MappedFileStore* inputFile, int* MTNotPossible);
#endif