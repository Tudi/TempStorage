#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"
#include "strstr5bit.h"

#ifdef _DEBUG
	#define assert(x,s) if(!x)printf("%s:%d %s\n",__FILE__,__LINE__,s);
#else
	#define assert(x,s)
#endif

static unsigned char replaceChars[256]; // in order to reduce the number of charactes, we need to replace/remove some of the existing chars
static unsigned char conversionMap[256];
static unsigned char reverseconversionMap[256];

/// <summary>
/// 4 bits are not enough. 6 looks more feasable. Would mean it will not be able to distinguish 60% of chars
/// </summary>
void InitConversionMap4Bit()
{
	// 15 chars : ' ' + '_' + "0123456789"+'r'+'i'+'o'
	memset(replaceChars, 0, sizeof(replaceChars));

	// lowercase characters
	for (size_t i = 'A'; i <= 'Z'; i++) // 26 chars
	{
		replaceChars[i] = ((unsigned char)i - 'A') % 10 + '0';
	}
	for (size_t i = 'a'; i <= 'z'; i++)
	{
		replaceChars[i] = ((unsigned char)i - 'a') % 10 + '0';
	}
	for (size_t i = '0'; i <= '9'; i++)
	{
		replaceChars[i] = (unsigned char)i;
	}

	// only these could be kept. These are most frequent chars
	replaceChars['r'] = 'r';
	replaceChars['i'] = 'i';
	replaceChars['o'] = 'o';
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
}

/// <summary>
/// 5 bits might be too few. 6 looks more feasable
/// </summary>
void InitConversionMap5Bit()
{
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
	// at this point we should have reduced from 96 to 31 characters : [1-31]
	unsigned char charsUsed[256];
	memset(charsUsed, 0, sizeof(charsUsed));
	const unsigned char* printableAsciiChars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	//create a string that contains all printable ascii chars
	strcpy_s(charsUsed, sizeof(charsUsed), printableAsciiChars);
	//reduce the number of characters based on our replace map
	for (size_t index = 0; index < strlen(charsUsed); index++)
	{
		if (replaceChars[charsUsed[index]] != 0)
		{
			charsUsed[index] = replaceChars[charsUsed[index]];
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
	assert(usedCharsCount < 32, "Can't have more than 31 chars");

	size_t Code = 1;
	memset(conversionMap, 0, sizeof(conversionMap));
	memset(reverseconversionMap, 0, sizeof(reverseconversionMap));
	for (size_t i = 0; i < usedCharsCount; i++)
	{
		for (size_t i2 = 0; i2 < 256; i2++)
		{
			if(replaceChars[i2] == charsUnique[i])
			{
				conversionMap[i2] = (unsigned char)Code;
			}
		}
		reverseconversionMap[Code] = charsUnique[i];
		Code++;
	}
	assert(Code < 32, "Can't have more than 31 chars");

	for (size_t i = 0; i < 256; i++)
	{
		replaceChars[i] = conversionMap[replaceChars[i]];
	}
}

str5Bit* ConvertTo5Bit(const char* str)
{
	size_t bitWriteIndex = 0;
	size_t len = strlen(str);
	size_t newLen = (len * 5 + 7) / 8;
	char* resstr = (char*)malloc(newLen);
	for (size_t i = 0; i < len; i++)
	{
		unsigned char inputChar = str[i];
		unsigned char inputSymbol = conversionMap[inputChar];
		size_t byteWriteIndex = bitWriteIndex >> 3;
		size_t bitWriteIndex2 = bitWriteIndex & 0x07; // can be anything from [0..7], we might be writing 13 bits
		unsigned short clearMask = ~(0xFFFFFFFF << bitWriteIndex2);
		unsigned short setSymbol = inputSymbol << bitWriteIndex2;
		*(unsigned short*)&resstr[byteWriteIndex] = (*(unsigned short*)&resstr[byteWriteIndex] & clearMask) | setSymbol;
		bitWriteIndex+=5;
	}
	str5Bit* res = (str5Bit*)malloc(sizeof(str5Bit));
	res->len = (unsigned short)len;
	res->str = resstr;
	return res;
}

char* ConvertFrom5Bit(const str5Bit* str)
{
	size_t bitReadIndex = 0;
	size_t newLen = str->len;
	char* resstr = (char*)malloc(newLen + 1);
	for (size_t i = 0; i < newLen; i++)
	{
		size_t readMask = 0x1F;
		size_t byteReadIndex = bitReadIndex / 8;
		size_t bitReadIndex2 = bitReadIndex & 0x07;
		size_t readVal = *(size_t*)&str[byteReadIndex];
		readVal = readVal >> bitReadIndex2;
		readVal = readVal & readMask;
		assert(readVal >= 256, "character code is beyond possibility");
		unsigned char outputChar = reverseconversionMap[readVal];
		resstr[i] = outputChar;
		bitReadIndex += 5;
	}
	resstr[newLen] = 0;
	return resstr;
}

int HasStr5Bit(const str5Bit* largeStr, const str5Bit* subStr)
{
#define REGISTER_BITCOUNT	64						// this is just constant naming, you should not change it
#define COMPARE_MAX_BITLEN	(REGISTER_BITCOUNT-7)	// because 8 bits would make us read 1 byte next
	if (largeStr->len < subStr->len)
	{
		return 0;
	}
	if (largeStr->len == subStr->len)
	{
		int ret = memcmp(largeStr->str, subStr->str, (largeStr->len * 5 + 7) / 8);
		return !ret;
	}
	uint64_t searchedValue = *(uint64_t*)subStr->str;
	uint64_t searchedBitlen2 = subStr->len * 5;
	uint64_t searchedBitlen = searchedBitlen2;
	size_t sourceBits = largeStr->len * 5;
	// we need 7 bits free so that we can shift the value
	if (searchedBitlen > COMPARE_MAX_BITLEN)
	{
		searchedBitlen = COMPARE_MAX_BITLEN;
	}

	uint64_t searchedMask = ~0;
	uint64_t unusedBits = (REGISTER_BITCOUNT - searchedBitlen);
	searchedMask = searchedMask >> unusedBits;
	searchedValue = searchedValue & searchedMask;
	int64_t stepCount = largeStr->len - subStr->len;
	size_t bitReadIndex = 0;
#define SHIFT_AND_COMPARE(shift) if( ((readVal>>shift)&searchedMask)==searchedValue) return 1;
#define PREPARE_SRC_BITS size_t byteReadIndex = bitReadIndex >> 3; \
						 size_t bitReadIndex2 = bitReadIndex & 0x07; \
						 uint64_t readVal = *(uint64_t*)&largeStr->str[byteReadIndex]; \
						 readVal = readVal >> bitReadIndex2;

	switch (subStr->len)
	{
		case 1: // 5 bits are used out of the 64 - 7. bitReadIndex can be : 0,5,2,7,4,1,6,3 !
		{
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
			// compare 57 bits(11 chars) and repeat until we run out of source to compare
			for (; stepCount >= 0; stepCount--)
			{
				PREPARE_SRC_BITS;
				if (((readVal >> 0) & searchedMask) == searchedValue)
				{
					// check the remaining values in chunks of 57 bits
					size_t checkStartLargeStr = bitReadIndex + COMPARE_MAX_BITLEN + 1;
					assert(searchedBitlen2 > COMPARE_MAX_BITLEN + 1, "ValueOverflow");
					size_t checkCount = searchedBitlen2 - (COMPARE_MAX_BITLEN + 1);
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
						searchedValue2 = searchedValue2 & searchedMask2;
						{
							size_t byteReadIndex = checkStartLargeStr >> 3;
							size_t bitReadIndex2 = checkStartLargeStr & 0x07;
							uint64_t readVal = *(uint64_t*)&largeStr->str[byteReadIndex];
							readVal = readVal >> bitReadIndex2;
							if (((readVal >> 0) & searchedMask2) != searchedValue2)
							{
								goto no_full_match_found;
							}
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
		default: assert(remainingBits == 0, "unexpected remaining bitcount");
#endif
		}
	}

	return 0;
}

void RunDebug5BitTests()
{
	InitConversionMap5Bit();
	return;
	char test[] = "This is A test 1234 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~The quick brown fox jumps over the lazy dog.";
	char sustr[] = "his ";
	str5Bit* str5bit = ConvertTo5Bit(test);
	str5Bit* substr5bit = ConvertTo5Bit(sustr);
	char* str8bit = ConvertFrom5Bit(str5bit);
	int hasstr = HasStr5Bit(str5bit, substr5bit);
}

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
			int testRes = HasStr5Bit(&sInputStrings5Bit[inputIndex], &sSearchedStrings5Bit[searchedIndex]);
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
	printf("Searches made %d. Found the string %d times. Seconds : %f\n\n", (int)searchesMade, (int)foundCount, (float)runtimeSec);
}
