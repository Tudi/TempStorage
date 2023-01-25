#ifndef _SCORE_FILE_H_
#define _SCORE_FILE_H_

#include <stddef.h>

// values must match from ferrari-c project 'similarity_score.h'
typedef enum SimilarityFileScoreTypes
{
    SSFT_NOT_USED_UNINITIALIZED_VALUE = 0,
    SSFT_COMPANY_SIMILARITY_SCORE = 1,
    SSFT_INDUSTRY_SIMILARITY_SCORE = 2,
    SSFT_TITLE_SIMILARITY_SCORE = 3,
    SSFT_PROFILE_SIMILARITY_SCORE = 4,
    SSFT_MAX_SCORE_FILE_TYPE // always the last value to be able to detect invalid values
}SimilarityFileScoreTypes;

#pragma pack(push, 1) // needs to be mapable to file content no matter the compilation settings 
typedef struct ScoreFileScore
{
    unsigned int id;
    unsigned short score;
}ScoreFileScore;

typedef struct ScoreFileLayout
{
    unsigned int count;
    unsigned int padding;
    ScoreFileScore scores[0];
}ScoreFileLayout;

typedef struct ScoreFileMTSliceData
{
    int startIndex; // in the array of IDs, this slice starts at index XXX
    int IDCountTotal; // number of IDs since the start of the file
}ScoreFileMTSliceData;

#define SCORE_FILE_SIZE(sf) (sf->count * sizeof(ScoreFileScore) + sizeof(ScoreFileLayout))

// each worker thread will work on a group of slices from all the files
// planned for scenario : 100 files to be merged. Each file has 100K IDs => 10M IDs to be checked by all worker threads
// to find the best workload, each thread will need to find best position: bitcount(MAX_MT_SLICES)*fileCount additions => ex: 14*100
#define MAX_MT_SLICES   10000
#define MAX_MT_SLICES_UNPLANNED   (MAX_MT_SLICES*2)

typedef struct ScoreFileLayoutAddedMTData
{
    int version; // version info, in case we wish to regen this struct, we know which one is outdated
    int maxId; // when the info was generated, maxID from db was this number. If new maxID is larger, this data can be outdated
    int idCount;
    int sliceSize; // when info was generated, we thought this slice size would be good for us
    int sliceMinUsed;
    int sliceMaxUsed; // we aim to squeeze the whole file in MAX_MT_SLICES, but maybe it goes beyond ?
    int invalidSliceInfo; // because for some reason we could not slice up this data
    ScoreFileMTSliceData slices[MAX_MT_SLICES_UNPLANNED];
}ScoreFileLayoutAddedMTData;
#pragma pack(pop)

typedef struct MappedFileStore
{
    int fd;
    char* mmappedData;
    size_t mappedSize;
    ScoreFileLayout* __restrict fileScores;
    size_t indexRead;
    int id;
    int fileType;
    int notMappedData; // if ex file read is used
    ScoreFileLayoutAddedMTData *mtData;
    int freeMTData;
}MappedFileStore;

/// <summary>
/// Based on an ID, generate a file name that can be used both for writing and reading
/// Function made to extract redundant code
/// </summary>
/// <param name="dir"></param>
/// <param name="id"></param>
/// <param name="outBuf"></param>
/// <param name="outBufSize"></param>
void genScoreFileName(const char* dir, const unsigned int id, char* outBuf, size_t outBufSize);

/// <summary>
/// Mapping file content will allow the OS to keep frequently access files in cache
/// On my pc secondary file access read time is almost 0
/// </summary>
/// <param name="filename"></param>
/// <param name="fd"></param>
/// <param name="mmappedData"></param>
/// <param name="filesize"></param>
/// <returns></returns>
int getMappedFile(const char* filename, int* fd, char** mmappedData, size_t* filesize);

/// <summary>
/// Tell OS that we no longer intend to access this file
/// </summary>
/// <param name="fd"></param>
/// <param name="mmappedData"></param>
/// <param name="filesize"></param>
/// <returns></returns>
int cleanupMappedFile(int fd, void* mmappedData, size_t filesize);

/// <summary>
/// Get the file content of a score file into a newly allocated buffer
/// </summary>
/// <param name="filename"></param>
/// <param name="fd"></param>
/// <param name="mmappedData"></param>
/// <param name="filesize"></param>
/// <returns></returns>
int getFileContent(const char* filename, int* fd, char** mmappedData, size_t* filesize);

#endif