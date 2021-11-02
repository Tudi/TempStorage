#ifndef _INPUT_GENERATOR_H_
#define _INPUT_GENERATOR_H_

#ifdef _DEBUG
	#define MEMORY_ALLOC_FOR_INPUT		(1024) // 100M 
	#define MIN_INPUT_LEN				1	//1 byte
	#define MAX_INPUT_LEN				15	
#else
	#define MEMORY_ALLOC_FOR_INPUT		(100*1024*1024) // 100M 
	#define MIN_INPUT_LEN				1	//1 byte
	#define MAX_INPUT_LEN				15	
#endif

#define USE_STRING_PADDING			0

#define MEMORY_ALLOC_FOR_SEARCH		(1024) // 1K 
#define MIN_SEARCH_LEN				3	//3 bytes
#define MAX_SEARCH_LEN				15	

#ifdef WINDOWS_BUILD
    #define int64_t __int64
#else
	#include <stdint.h>
	#include <stddef.h>
#endif

#ifdef WINDOWS_BUILD
	#define _noinline_ __declspec(noinline)
#else
	#define _noinline_ __attribute__((noinline))
#endif
/// <summary>
/// Generate input strings starting from small length to large length
/// </summary>
/// <param name="memorySizeUsed"></param>
/// <param name="minLen"></param>
/// <param name="maxLen"></param>
/// <param name="addPadding">padd strings with bytes to avoid illegal reading</param>
void GenerateInputStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding);
void GenerateSearchedStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding);
void GenerateInputNOPStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding);

typedef struct profiledStringStore
{
	char* str;
	unsigned short len;
	char* compressedStr; // not yet used. Would be around 4-6bps complementary helper. Requires special library
	unsigned short compressedLen;
	char AntiCacheLineStreamRead[55]; // break cache line to not optimize input for stream reading. Semi realistic case
}profiledStringStore;

/// <summary>
/// The actual string comes after the structure. Total size : sizeof(noPointerString)+noPointerString.len+1
/// </summary>
typedef struct noPointerString
{
	unsigned short len;
	char AntiCacheLineStreamRead[75]; // break cache line to not optimize input for stream reading. Semi realistic case
}noPointerString;
#define GetNOPString(store) ((char*)(store)+sizeof(noPointerString))

extern size_t uiInputStrCount;
extern profiledStringStore *sInputStrings;

extern size_t uiInputNOPStrCount;
extern noPointerString** sInputNOPStrings;

extern size_t uiInputStrCount;
extern profiledStringStore* sInputStrings;

extern size_t uiSearchedStrCount;
extern profiledStringStore* sSearchedStrings;

#endif