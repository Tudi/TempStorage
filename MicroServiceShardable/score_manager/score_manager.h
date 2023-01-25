#ifndef _SCORE_MANAGER_H_
#define _SCORE_MANAGER_H_

/*
Interface for saving / loading score files. Right now there is no caching mechanism, but if in the future there is one, it would be added here
*/

#include <stddef.h>
#include <stdint.h> 
#include <../ss_server/file_lock_list.h>

#define MAX_SCORE_VALUE 10000
#define BUFFER_EXTEND_MIN_SIZE    (10*1024*1024)   // when using realloc, reduce the amount of times we do it by allocating more
                                                    // needs to be larger than 6 bytes 
#pragma pack(push, 1) // needs to be mapable to file content no matter to compile settings 
// packets are from ferrari-c 'similarity_scores.h'
typedef struct SM_NetworkSimilarityScoreData
{
    unsigned int id; // company / industry / title ID
    unsigned short score;
}SM_NetworkSimilarityScoreData;

typedef struct SM_NetworkSimilarityScoreHeader
{
    unsigned int size; // given in bytes. Includes header bytes also !
    unsigned int type; // data block type
    unsigned int blockCount; // list size
    SM_NetworkSimilarityScoreData data[0]; // list of values
}SM_NetworkSimilarityScoreHeader;
#pragma pack(pop)

/// <summary>
/// Initialize internal states
/// </summary>
/// <returns></returns>
int scoreManagerInit();

/// <summary>
/// Internal states are deinitialized
/// </summary>
/// <returns></returns>
int scoreManagerShutdown();

/// <summary>
/// Score files are sent by the ML team. They are saved localy to be fetched by get score requests
/// </summary>
/// <param name="dir"></param>
/// <param name="id"></param>
/// <param name="buf"></param>
/// <param name="bufSize"></param>
/// <returns></returns>
int saveScoreFile(const int fType, const char* dir, const unsigned int id, const uint8_t* buf, const size_t bufSize);

/// <summary>
/// Array of score requests. 
/// Return value is compatible with ferrari-c similarity score input buffer
/// </summary>
/// <param name="dir"></param>
/// <param name="id"></param>
/// <param name="buf"></param>
/// <param name="bufSize"></param>
/// <returns></returns>
int generateScorePacket(FileLockList_t fileLockList, const uint8_t* inPacket, const size_t inPacketSize, const char** similarityPaths,
    uint8_t**outPacket, uint32_t *outPacketWritten, uint32_t* outPacketAllocated);

#define _PRINT_WHAT_IS_HAPPENING_IN_SCORE_MANAGER_
#ifdef _PRINT_WHAT_IS_HAPPENING_IN_SCORE_MANAGER_
    #define SM_LOG_MESSAGE(...)    LOG_MESSAGE(DEBUG_LOG_MSG, __VA_ARGS__)
    #define SM_ASSERT(x, ...) do{ if(!(x)) {LOG_MESSAGE(DEBUG_LOG_MSG, __VA_ARGS__);} }while(0)
#else
    #define SM_LOG_MESSAGE(...) do{ }while(0)
    #define SM_ASSERT(x, ...)   do{ }while(0)
#endif

#endif