#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "strstr5bit.h"

//#define _DEBUG_STR_5_BIT_LEVEL_2
//#define _DEBUG_STR_5_BIT
#ifdef _DEBUG_STR_5_BIT
	#define assert_5bit(x,s) if(!(x))printf("%s:%d %s\n",__FILE__,__LINE__,s);
#else
	#define assert_5bit(x,s)
#endif

static char conversionMap[256];
static char reverseconversionMap[256];

/// <summary>
/// Convert selected characters to destination caracters
/// Any non specified character in the list will be converted to 0
/// The new 5bit string does not check for NULL terminator. 0 value is allowed
/// Values that can not be converted, will be skipped
/// 5 bits might be too few :
///		- all numbers : 10 values
///		- punctuation '_', ' ' : 2 values
///		- special 1 : 1 value
///		- characters : 'a' .. 'z' except : 'z', 'q', 'j', 'x', 'k', 'b', 'w', 'y' : 17 values
/// </summary>
void InitConversionMap5Bit()
{
	char replaceChars[256]; // in order to reduce the number of charactes, we need to replace/remove some of the existing chars
	memset(replaceChars, 0, sizeof(replaceChars));
	// lowercase characters
	for (size_t i = 'A'; i <= 'Z'; i++) // 26 chars
	{
		replaceChars[i] = (unsigned char)i + 'a' - 'A';
	}
	for (size_t i = 'a'; i <= 'z'; i++)
	{
		replaceChars[i] = (unsigned char)i;
	}
	for (size_t i = '0'; i <= '9'; i++)
	{
		replaceChars[i] = (unsigned char)i;
	}
	// all sign characters replaced by '_'
	replaceChars[' '] = ' ';
	replaceChars['!'] = '_';
	replaceChars['"'] = '_';
	replaceChars['#'] = '_';
	replaceChars['$'] = '_';
	replaceChars['%'] = '_';
	replaceChars['&'] = '_';
	replaceChars['\''] = '_';
	replaceChars['('] = '_';
	replaceChars[')'] = '_';
	replaceChars['*'] = '_';
	replaceChars['+'] = '_';
	replaceChars[','] = '_';
	replaceChars['-'] = '_';
	replaceChars['.'] = '_';
	replaceChars['/'] = '_';
	replaceChars[':'] = '_';
	replaceChars[';'] = '_';
	replaceChars['<'] = '_';
	replaceChars['='] = '_';
	replaceChars['>'] = '_';
	replaceChars['?'] = '_';
	replaceChars['@'] = '_';
	replaceChars['['] = '_';
	replaceChars['\\'] = '_';
	replaceChars[']'] = '_';
	replaceChars['^'] = '_';
	replaceChars['_'] = '_';
	replaceChars['`'] = '_';
	replaceChars['{'] = '_';
	replaceChars['|'] = '_';
	replaceChars['}'] = '_';
	replaceChars['~'] = '_';
	// in order to get to 5 bits, we need to get rid of 7 characters
	// these are the least used chars in english alphabet
	replaceChars['z'] = '1';
	replaceChars['Z'] = '1';
	replaceChars['q'] = '2';
	replaceChars['Q'] = '2';
	replaceChars['j'] = '3';
	replaceChars['J'] = '3';
	replaceChars['x'] = '4';
	replaceChars['X'] = '4';
	replaceChars['k'] = '5';
	replaceChars['K'] = '5';
	replaceChars['b'] = '6';
	replaceChars['B'] = '6';
	replaceChars['y'] = '7';
	replaceChars['Y'] = '7';
	replaceChars['w'] = '8'; // because we want special chars
	replaceChars['W'] = '8'; // because we want special chars
//	replaceChars['g'] = '9';
//	replaceChars['G'] = '9';
	// at this point we should have reduced from 96 to 31 characters : [1-31]
	char charsUsed[256];
	memset(charsUsed, 0, sizeof(charsUsed));
//	const char* printableAsciiChars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	// reorganized order so some characters would fit into 4 bits
	const char* printableAsciiChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789{|}~ !\"#$%&'()*+,-./:;<=>?@[\\]^_`";
	//create a string that contains all printable ascii chars
#ifdef WINDOWS_BUILD
	strcpy_s(charsUsed, sizeof(charsUsed), printableAsciiChars);
#else
	strcpy(charsUsed, printableAsciiChars);
#endif

	//reduce the number of characters based on our replace map
	for (size_t index = 0; index < strlen(charsUsed); index++)
	{
		if (replaceChars[(size_t)*(unsigned char*)&charsUsed[index]] != 0)
		{
			charsUsed[index] = replaceChars[(size_t) *(unsigned char*)&charsUsed[index]];
		}
	}
	size_t usedCharsCount = 0;
	unsigned char charsUnique[256];
	for (size_t index = 0; index < strlen(charsUsed); index++)
	{
		size_t foundIt = 0;
		for (size_t index2 = 0; index2 < usedCharsCount; index2++)
		{
			if (charsUnique[index2] == charsUsed[index])
			{
				foundIt = 1;
			}
		}
		if (foundIt == 0)
		{
			charsUnique[usedCharsCount] = charsUsed[index];
			usedCharsCount++;
		}
	}

#ifdef _DEBUG_STR_5_BIT
	charsUnique[usedCharsCount] = 0;
	printf("charsUnique : '%s'\n", charsUnique);
	if (usedCharsCount != 30)
	{
		printf("Found %d chars. Can't have more than 30 chars\n", (int)usedCharsCount);
		for (size_t i = 0; i < usedCharsCount; i++)
		{
			printf("%c", charsUnique[i]);
		}
		printf("\n");
	}
#endif

	size_t Code = 0;
	memset(conversionMap, 32, sizeof(conversionMap));
	memset(reverseconversionMap, 0, sizeof(reverseconversionMap));
	for (size_t i = 0; i < usedCharsCount; i++)
	{
		int inputChar = charsUnique[i];
		for (size_t i2 = 0; i2 < 256; i2++)
		{
			if(replaceChars[i2] == inputChar)
			{
				conversionMap[i2] = (unsigned char)Code;
			}
		}
		reverseconversionMap[Code] = inputChar;
		Code++;
	}

#ifdef _DEBUG_STR_5_BIT
	if(Code != 30)printf("Using %d char codes, instead optimal char count 30\n", (int)Code);
#endif

	// bleah, special case for array separators. Want to replace value 1 with 31
	conversionMap[1] = 31;
	reverseconversionMap[31] = 1;
}

void initStr5Bit(str5Bit* str)
{
	str->len = 0;
	str->str = NULL;
	str->str_LH = NULL;
}

void freeStr5Bit(str5Bit* str)
{
	str->len = 0;
	free(str->str);
	str->str = NULL;
	free(str->str_LH);
	str->str_LH = NULL;
}

#define STR5BIT_GET_L_LEN(strlen) ((strlen * 4 + 7) / 8 + 1)
str5Bit* ConvertTo5BitLH(const char* str, str5Bit* out_dst)
{
	size_t len = strlen(str);
	size_t newLen_low = STR5BIT_GET_L_LEN(len);
	size_t newLen_high = (len * 1 + 7) / 8;

	assert_5bit(newLen_low < 0xFFFF, "Can't have more than 0xFFFF chars");

	const size_t padding = 2; // needed because we read outside the size
	unsigned char* resstr_low = (unsigned char*)malloc(newLen_low + newLen_high + padding); // padding added to avoid illegal read
	if (resstr_low == NULL)
	{
		if (out_dst)
		{
			initStr5Bit(out_dst);
		}
		return NULL;
	}
	memset(resstr_low, 0, newLen_low + newLen_high + padding); // we might end up skipping all chars due to utf8
	unsigned char* resstr_high = &resstr_low[newLen_low];

	size_t valuesWritten = 0;
	for (size_t i = 0; i < len; i++)
	{
		unsigned char inputChar = str[i];

		unsigned short inputSymbol = conversionMap[inputChar];
		if (inputSymbol > 31) // only happens to cut out characters like UTF8
		{
			continue;
		}
		size_t byteWriteIndex_low = valuesWritten / 2;
		if ((valuesWritten & 1) == 0)
		{
			unsigned char setSymbol = inputSymbol & 0x0F; // shift out the higher 4 bits. Only need lower 4 bits
			unsigned char prevDestValue = resstr_low[byteWriteIndex_low];
			//prevDestValue = prevDestValue & 0xF0; // make sure we do not mix old unused data with new data
			resstr_low[byteWriteIndex_low] = prevDestValue | setSymbol;
		}
		else
		{
			unsigned char setSymbol = inputSymbol << 4; // shift out the higher 4 bits. Only need lower 4 bits
			unsigned char prevDestValue = resstr_low[byteWriteIndex_low];
//			prevDestValue = prevDestValue & 0x0F; // make sure we do not mix old unused data with new data
			resstr_low[byteWriteIndex_low] = prevDestValue | setSymbol;
		}
		size_t byteWriteIndex = valuesWritten >> 3; // remainder of 8 to get the byte index
		size_t bitWriteIndex2 = valuesWritten & 0x07; // can be anything from [0..7], we might be writing 13 bits
		unsigned char clearMask = ~(0xFFFFFFFF << bitWriteIndex2);
		unsigned char setSymbol = (inputSymbol >> 4) << bitWriteIndex2; // only store the high bits
		unsigned char prevDestValue = resstr_high[byteWriteIndex];
		prevDestValue = prevDestValue & clearMask; // make sure we do not mix old unused data with new data
		resstr_high[byteWriteIndex] = prevDestValue | setSymbol;
		valuesWritten++;
	}
	if (out_dst == NULL)
	{
		out_dst = (str5Bit*)malloc(sizeof(str5Bit));
	}
	out_dst->str_LH = resstr_low;
	out_dst->len = (unsigned short)valuesWritten;

	return out_dst;
}

/// <summary>
/// Based on the conversion table, convert input characters to destination characters
/// Characters that can not be converted, will be skipped. Example UTF8 or '\n'
/// </summary>
/// <param name="str"></param>
/// <param name="out_dst"></param>
/// <returns></returns>
str5Bit* ConvertTo5Bit(const char* str, str5Bit* out_dst)
{
	size_t bitWriteIndex = 0;
	size_t len = strlen(str);
	size_t newLen = (len * 5 + 7) / 8;
	newLen += 7; // Have to add padding to make sure we do not read more than have data

	assert_5bit(newLen < 0xFFFF, "Can't have more than 0xFFFF chars");

	char* resstr = (char*)malloc(newLen);
	if (resstr == NULL)
	{
		if (out_dst)
		{
			initStr5Bit(out_dst);
		}
		return NULL;
	}

	memset(resstr, 0, newLen); // we might end up skipping all chars due to utf8
	for (size_t i = 0; i < len; i++)
	{
		unsigned char inputChar = str[i];

		unsigned short inputSymbol = conversionMap[inputChar];
		if (inputSymbol > 31)
		{
#ifdef _DEBUG_STR_5_BIT_LEVEL_2
			// Does not support UTF8 char set
			if (inputChar < 128 && inputChar != '\n' && inputChar != '\r' && inputChar != '\t')
			{
				printf("unable to convert source char %c=%d at pos %d/%d\n", inputChar, (int)inputChar, (int)i, (int)len);
			}
#endif
			continue;
		}
		size_t byteWriteIndex = bitWriteIndex >> 3; // remainder of 8 to get the byte index
		size_t bitWriteIndex2 = bitWriteIndex & 0x07; // can be anything from [0..7], we might be writing 13 bits
		unsigned short clearMask = ~(0xFFFFFFFF << bitWriteIndex2);
		unsigned short setSymbol = inputSymbol << bitWriteIndex2;
		unsigned short prevDestValue = *(unsigned short*)&resstr[byteWriteIndex];
		prevDestValue = prevDestValue & clearMask; // make sure we do not mix old unused data with new data
		*(unsigned short*)(&resstr[byteWriteIndex]) = prevDestValue | setSymbol;
		bitWriteIndex+=5;
	}
	if (out_dst == NULL)
	{
		out_dst = (str5Bit*)malloc(sizeof(str5Bit));
		initStr5Bit(out_dst);
	}
	out_dst->len = (unsigned short)(bitWriteIndex/5);
	out_dst->str = resstr;
	ConvertTo5BitLH(str, out_dst);

#ifdef _DEBUG_STR_5_BIT_LEVEL_2
	char* oriStr = ConvertFrom5Bit(out_dst);
	if (strlen(str) != strlen(oriStr))
	{
		printf("Reconversion length different %d!=%d=%d\n", (int)strlen(str), (int)strlen(oriStr), out_dst->len);
	}
	if (strcmp(oriStr, str) != 0)
	{
		printf("Separator '1' is converted into %c=%d reversed %c=%d\n", 
			conversionMap[1], (int)conversionMap[1], reverseconversionMap[(int)conversionMap[1]], (int)reverseconversionMap[(int)conversionMap[1]]);
		printf("++++++++++++++++++++++++++++++++++++++++\n");
		printf("ori str : \n'%s' \n, not same : \n'%s'\n", str, oriStr);
		printf("========================================\n");
	}
	free(oriStr);
#endif

	return out_dst;
}

char* ConvertFrom5Bit(const str5Bit* str)
{
	size_t bitReadIndex = 0;
	size_t newLen = str->len;
	char* resstr = (char*)malloc(newLen + 1);
	for (size_t i = 0; i < newLen; i++)
	{
		size_t readMask = 0x1F;
		size_t byteReadIndex = bitReadIndex >> 3;
		size_t bitReadIndex2 = bitReadIndex & 0x07;
		size_t readVal = *(size_t*)(&str->str[byteReadIndex]);
		readVal = readVal >> bitReadIndex2;
		readVal = readVal & readMask;
		assert_5bit(readVal <= 31, "character code is beyond possibility");
		unsigned char outputChar = reverseconversionMap[readVal];
		resstr[i] = outputChar;
		bitReadIndex += 5;
	}
	resstr[newLen] = 0;
	return resstr;
}
#ifdef WINDOWS_BUILD
	#define forceinline__ __forceinline
#else
	#define forceinline__ __attribute__((always_inline))
#endif

/// <summary>
/// The only reason this would be faster is no bounds checking + inlining
/// </summary>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <param name="count"></param>
/// <returns></returns>
#define smallmemcmpNOT(m1, m2, count) \
{ \
	switch (count) \
	{ \
		case 1: return m1[0] == m2[0]; \
		case 2: return *(unsigned short*)m1 == *(unsigned short*)m2; \
		case 3: return ((*(unsigned int*)m1) & 0x00FFFFFF) == ((*(unsigned int*)m2) & 0x00FFFFFF); \
		case 4: return ((*(unsigned int*)m1)) == ((*(unsigned int*)m2)); \
		case 5: return ((*(uint64_t*)m1) & 0xFFFFFFFFFF) == ((*(uint64_t*)m2) & 0xFFFFFFFFFF); \
		case 6: return ((*(uint64_t*)m1) & 0xFFFFFFFFFFFF) == ((*(uint64_t*)m2) & 0xFFFFFFFFFFFF); \
		case 7: return ((*(uint64_t*)m1) & 0xFFFFFFFFFFFFFF) == ((*(uint64_t*)m2) & 0xFFFFFFFFFFFFFF); \
		case 8: return ((*(uint64_t*)m1)) == ((*(uint64_t*)m2)); \
		case 9: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (m1[8] == m2[8])); \
		case 10: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (*(unsigned short*)&m1[8] == *(unsigned short*)&m2[8])); \
		case 11: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (((*(int*)&m1[8]) & 0x00FFFFFF) == (((*(int*)&m2[8]))& 0x00FFFFFF))); \
		case 12: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (((*(int*)&m1[8])) == ((*(int*)&m2[8])))); \
		case 13: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (((*(uint64_t*)&m1[8]) & 0xFFFFFFFFFF) == (((*(uint64_t*)&m2[8]) & 0xFFFFFFFFFF)))); \
		case 14: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (((*(uint64_t*)&m1[8]) & 0xFFFFFFFFFFFF) == (((*(uint64_t*)&m2[8]) & 0xFFFFFFFFFFFF)))); \
		case 15: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (((*(uint64_t*)&m1[8]) & 0xFFFFFFFFFFFFFF) == (((*(uint64_t*)&m2[8]) & 0xFFFFFFFFFFFFFF)))); \
		case 16: return (((*(uint64_t*)m1)) == ((*(uint64_t*)m2)) && (((*(uint64_t*)&m1[8])) == ((*(uint64_t*)&m2[8])))); \
	} \
	return !memcmp(m1, m2, count); \
} \

size_t HasStr5Bit(const str5Bit* largeStr, const str5Bit* subStr)
{
#define REGISTER_BITCOUNT	64						// this is just constant naming, you should not change it
#define COMPARE_MAX_BITLEN	(REGISTER_BITCOUNT-7)	// because 8 bits would make us read 1 byte next
	if (largeStr->len < subStr->len)
	{
		return 0;
	}
	if (largeStr->len == subStr->len)
	{
		smallmemcmpNOT(largeStr->str, subStr->str, (largeStr->len * 5 + 7) / 8);
	}
	uint64_t searchedBitlen2 = subStr->len * 5;
	size_t sourceBits = largeStr->len * 5;
	size_t bitReadIndex = 0;
	uint64_t searchedMask;
	uint64_t searchedValue;
#define SHIFT_AND_COMPARE(shift) if( ((readVal>>shift)&searchedMask)==searchedValue) return 1;
#define PREPARE_SRC_BITS size_t byteReadIndex = bitReadIndex >> 3; \
						 size_t bitReadIndex2 = bitReadIndex & 0x07; \
						 uint64_t readVal = *(uint64_t*)&largeStr->str[byteReadIndex]; \
						 readVal = readVal >> bitReadIndex2;

	switch (subStr->len)
	{
		case 1: // 5 bits are used out of the 64 - 7. bitReadIndex can be : 0,5,2,7,4,1,6,3 !
		{
			searchedMask = 0b11111;
			searchedValue = (*(unsigned char*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 55;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..5]
				SHIFT_AND_COMPARE(5); 
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);
				SHIFT_AND_COMPARE(25);
				SHIFT_AND_COMPARE(30);
				SHIFT_AND_COMPARE(35); 
				SHIFT_AND_COMPARE(40); 
				SHIFT_AND_COMPARE(45); 
				SHIFT_AND_COMPARE(50); // bits [50..55]
				bitReadIndex += bitsPerBatch;
			}

		}break;
		case 2: // 10 bits are used out of the 64 - 7. bitReadIndex can be : 0,2,4,6 !
		{
			searchedMask = 0b1111111111;
			searchedValue = (*(unsigned short*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 50;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..10]
				SHIFT_AND_COMPARE(5); 
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);
				SHIFT_AND_COMPARE(25);
				SHIFT_AND_COMPARE(30);
				SHIFT_AND_COMPARE(35); 
				SHIFT_AND_COMPARE(40); 
				SHIFT_AND_COMPARE(45); // bits [45..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 3: // 15 bits are used out of the 64 - 7. bitReadIndex can be : 0,7,6,5,4,3,2,1 !
		{
			searchedMask = 0b111111111111111;
			searchedValue = (*(unsigned short*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 45;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..15]
				SHIFT_AND_COMPARE(5); 
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);
				SHIFT_AND_COMPARE(25);
				SHIFT_AND_COMPARE(30);
				SHIFT_AND_COMPARE(35); 
				SHIFT_AND_COMPARE(40); // bits [40..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 4: // 20 bits are used out of the 64 - 7. bitReadIndex can be : 0,5,2,7,4,1,6,3 !
		{
			searchedMask = 0b11111111111111111111;
			searchedValue = (*(unsigned int*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 40;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..20]
				SHIFT_AND_COMPARE(5); // bits [5..25]
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);
				SHIFT_AND_COMPARE(25);
				SHIFT_AND_COMPARE(30);
				SHIFT_AND_COMPARE(35); // bits [35..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 5: // 25 bits are used out of the 57. bitReadIndex can be : 0,1,2,3,4,5,6,7 !
		{
			searchedMask = 0b1111111111111111111111111;
			searchedValue = (*(unsigned int*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 35;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..25]
				SHIFT_AND_COMPARE(5); // bits [5..30]
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);
				SHIFT_AND_COMPARE(25);
				SHIFT_AND_COMPARE(30); // bits [30..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 6: // 30 bits are used out of the 58. bitReadIndex can be : 0,6,4,2 !
		{
			searchedMask = 0b111111111111111111111111111111;
			searchedValue = (*(unsigned int*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 30;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..30]
				SHIFT_AND_COMPARE(5); 
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);
				SHIFT_AND_COMPARE(25);// bits [25..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 7: // 35 bits are used out of the 57. bitReadIndex can be : 0,3,6,1,4,7,2,5 !
		{
			searchedMask = 0b11111111111111111111111111111111111;
			searchedValue = (*(uint64_t*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 25;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..35]
				SHIFT_AND_COMPARE(5);
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20); // bits [20..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 8: // 40 bits are used out of the 64. bitReadIndex can be : 0 !
		{
			searchedMask = 0b1111111111111111111111111111111111111111;
			searchedValue = (*(uint64_t*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 25;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..40]
				SHIFT_AND_COMPARE(5);
				SHIFT_AND_COMPARE(10);
				SHIFT_AND_COMPARE(15);
				SHIFT_AND_COMPARE(20);// bits [20..60]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 9: // 45 bits are used out of the 57. bitReadIndex can be : 0,5,2,7,4,1,6,3 !
		{
			searchedMask = 0b111111111111111111111111111111111111111111111;
			searchedValue = (*(uint64_t*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 15;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..45]
				SHIFT_AND_COMPARE(5);
				SHIFT_AND_COMPARE(10);// bits [10..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		case 10: // 50 bits are used out of the 58. bitReadIndex can be : 0,2,4,6 !
		{
			searchedMask = 0b11111111111111111111111111111111111111111111111111;
			searchedValue = (*(uint64_t*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 10;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..50]
				SHIFT_AND_COMPARE(5); // bits [5..55]
				bitReadIndex += bitsPerBatch;				
			}
		}break;
		case 11: // 55 bits are used out of the 57
		{
			searchedMask = 0b1111111111111111111111111111111111111111111111111111111;
			searchedValue = (*(uint64_t*)&subStr->str[0]) & searchedMask;

			const size_t bitsPerBatch = 5;
			size_t stepCount2 = (sourceBits / bitsPerBatch);
			for (; stepCount2 > 0; --stepCount2)
			{
				PREPARE_SRC_BITS;
				SHIFT_AND_COMPARE(0); // bits [0..55]
				bitReadIndex += bitsPerBatch;
			}
		}break;
		//for anything larger than 60 bits
		default:
		{
			searchedMask = 0b111111111111111111111111111111111111111111111111111111111;
			uint64_t searchedValue = *(uint64_t*)subStr->str;
			searchedValue = searchedValue & searchedMask;
			int64_t stepCount = largeStr->len - subStr->len;
			// compare 57 bits(11 chars) and repeat until we run out of source to compare
			for (; stepCount >= 0; stepCount--)
			{
				PREPARE_SRC_BITS;
				if ((readVal & searchedMask) == searchedValue)
				{
					// check the remaining values in chunks of 57 bits
					assert_5bit(searchedBitlen2 > COMPARE_MAX_BITLEN + 1, "ValueOverflow");
					size_t checkStartLargeStr = bitReadIndex + COMPARE_MAX_BITLEN + 1;
					size_t checkStartSubStr = COMPARE_MAX_BITLEN + 1;
					for (; checkStartSubStr < searchedBitlen2; checkStartSubStr += 58)
					{
						size_t byteReadIndex = checkStartSubStr >> 3;
						size_t bitReadIndex2 = checkStartSubStr & 0x07;
						uint64_t searchedValue2 = *(uint64_t*)&subStr->str[byteReadIndex];
						searchedValue2 = searchedValue2 >> bitReadIndex2;
						int64_t searchBitcount2 = searchedBitlen2 - checkStartSubStr;
						if (searchBitcount2 > COMPARE_MAX_BITLEN)
						{
							searchBitcount2 = COMPARE_MAX_BITLEN;
						}
						uint64_t searchedMask2 = ~0;
						uint64_t unusedBits2 = (REGISTER_BITCOUNT - searchBitcount2);
						searchedMask2 = searchedMask2 >> unusedBits2;

						byteReadIndex = checkStartLargeStr >> 3;
						bitReadIndex2 = checkStartLargeStr & 0x07;
						readVal = *(uint64_t*)&largeStr->str[byteReadIndex];
						readVal = readVal >> bitReadIndex2;
						if ((readVal & searchedMask2) != (searchedValue2 & searchedMask2))
						{
							goto no_full_match_found;
						}
					}
					// check remaining bits 1 by 1
					return 1;
				}
no_full_match_found:
				bitReadIndex += 5;
			}
			return 0;
		}break;
	}
	// this code block should only execute for substr smaller than 11 chars
	if (bitReadIndex < sourceBits)
	{
		int64_t remainingBits = sourceBits - bitReadIndex;
		switch (remainingBits)
		{
		case 5: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0); // bits [0..5]
		}break;
		case 10: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
		}break;
		case 15: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
		}break;
		case 20: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
		}break;
		case 25: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
		}break;
		case 30: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
			SHIFT_AND_COMPARE(25);
		}break;
		case 35: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
			SHIFT_AND_COMPARE(25);
			SHIFT_AND_COMPARE(30);
		}break;
		case 40: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
			SHIFT_AND_COMPARE(25);
			SHIFT_AND_COMPARE(30);
			SHIFT_AND_COMPARE(35);
		}break;
		case 45: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
			SHIFT_AND_COMPARE(25);
			SHIFT_AND_COMPARE(30);
			SHIFT_AND_COMPARE(35);
			SHIFT_AND_COMPARE(40);
		}break;
		case 50: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
			SHIFT_AND_COMPARE(25);
			SHIFT_AND_COMPARE(30);
			SHIFT_AND_COMPARE(35);
			SHIFT_AND_COMPARE(40);
			SHIFT_AND_COMPARE(45);
		}break;
		case 55: {
			PREPARE_SRC_BITS;
			SHIFT_AND_COMPARE(0);
			SHIFT_AND_COMPARE(5);
			SHIFT_AND_COMPARE(10);
			SHIFT_AND_COMPARE(15);
			SHIFT_AND_COMPARE(20);
			SHIFT_AND_COMPARE(25);
			SHIFT_AND_COMPARE(30);
			SHIFT_AND_COMPARE(35);
			SHIFT_AND_COMPARE(40);
			SHIFT_AND_COMPARE(45);
			SHIFT_AND_COMPARE(55);
		}break;
#ifdef _DEBUG
		case 0: break;
		default: assert_5bit(remainingBits == 0, "unexpected remaining bitcount");
#endif
		}
	}

	return 0;
}

void runstr5BitLHTests();
void RunDebug5BitTests()
{
	InitConversionMap5Bit();
	runstr5BitLHTests();
#ifdef _DEBUG
	return;
	char test[] = "This is A test 1234 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~The quick brown fox jumps over the lazy dog.";
	char sustr[] = "his ";
	str5Bit* str5bit = ConvertTo5Bit(test, NULL);
	str5Bit* substr5bit = ConvertTo5Bit(sustr, NULL);
	char* str8bit = ConvertFrom5Bit(str5bit);
	size_t hasstr = HasStr5Bit(str5bit, substr5bit);
#endif
}

#include "InputGenerator.h"
#include "ProfileTimer.h"

#ifndef _noinline_
	#define _noinline_ 
#endif

_noinline_ void Run_strstr_5Bit()
{
	printf("Starting test %s ...", __FUNCTION__);
	StartTimer();
	size_t searchesMade = 0;
	size_t foundCount = 0; // anti optimization
	for (size_t repeatCount = 0; repeatCount < REPEAT_SAME_TEST_COUNT; repeatCount++)
		for (size_t searchedIndex = 0; searchedIndex < uiSearchedStrCount; searchedIndex++)
	{
		for (size_t inputIndex = 0; inputIndex < uiInputStrCount; inputIndex++)
		{
			size_t testRes = HasStr5Bit(&sInputStrings5Bit[inputIndex], &sSearchedStrings5Bit[searchedIndex]);
#ifdef _DEBUG
			int debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
			if (testRes != debugstrstrRes)
			{
				testRes = HasStr5Bit(&sInputStrings5Bit[inputIndex], &sSearchedStrings5Bit[searchedIndex]);
				testRes = HasStr5Bit(&sInputStrings5Bit[inputIndex], &sSearchedStrings5Bit[searchedIndex]);
			}
#endif
			if (testRes != 0)
			{
				foundCount++;
			}
			searchesMade++;
		}
	}
	double runtimeSec = EndTimer();
	printf(" ... Done\n");
	printf("Searches made %zd. Found the string %d times. Seconds : %f\n\n", searchesMade, (int)foundCount, (float)runtimeSec);
}
