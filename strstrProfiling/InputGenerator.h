#ifndef _INPUT_GENERATOR_H_
#define _INPUT_GENERATOR_H_

#ifdef _DEBUG
	#define MEMORY_ALLOC_FOR_INPUT		(1024) // 100M 

	#define MIN_SEARCH_LEN				1	// just to test very small values also use 2. Probably not realistic though
	#define MAX_SEARCH_LEN				25	
	#define REPEAT_SAME_TEST_COUNT		1
	#define MIN_INPUT_LEN				1	//1 byte
#else
	#define MEMORY_ALLOC_FOR_INPUT		(10*1024*1024) // 100M 

	#define MIN_SEARCH_LEN				4	// just to test very small values also use 2. Probably not realistic though
	#define MAX_SEARCH_LEN				15	
	#define REPEAT_SAME_TEST_COUNT		20 // make sure tests run for enough time to rule out CPU spikes
	#define MIN_INPUT_LEN				4	//number of bytes
#endif

#define MAX_INPUT_LEN				15	

#define MEMORY_ALLOC_FOR_SEARCH		(1024) // 1K 

#define USE_STRING_PADDING			32 // AVX2 requires 32 bytes padding. Can be avoided with string pooling

#include <stdint.h>
#include <stddef.h>

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
void GenerateInput5BitStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding);

typedef struct profiledStringStore
{
	char* str;
	unsigned short len;
	char* compressedStr; // not yet used. Would be around 4-6bps complementary helper. Requires special library
	unsigned short compressedLen;
	char AntiCacheLineStreamRead[55]; // break cache line to not optimize input for stream reading. Semi realistic case
}profiledStringStore;

/// <summary>
/// The actual string comes after the structure so no pointer is used. This reduces structure size by 6 bytes
/// </summary>
#pragma pack (push,1) // optimize for cache line usage. Only valid for stream read
typedef struct noPointerString
{
	unsigned short len;
	unsigned short loc; // required if structure contains more than 1 string
}noPointerString;
#pragma pack (pop)

#define GetNOPStringSize(len) (sizeof(noPointerString)+len)
//#define GetNOPString(store) ((char*)(store)+(sizeof(noPointerString)))
#define GetNOPString(store) ((char*)(store)+store->loc)

extern size_t uiInputStrCount;
extern profiledStringStore *sInputStrings;

extern size_t uiInputNOPStrCount;
extern noPointerString** sInputNOPStrings;

extern struct str5Bit* sInputStrings5Bit;
extern struct str5Bit* sSearchedStrings5Bit;

extern struct str5BitLH** sInputStrings5BitLH;
extern struct str5BitLH** sSearchedStrings5BitLH;

extern size_t uiSearchedStrCount;
extern profiledStringStore* sSearchedStrings;

#endif