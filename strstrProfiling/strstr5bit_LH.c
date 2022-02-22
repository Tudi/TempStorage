#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "strstr5bit_LH.h"

//#define _DEBUG_STR_5_BIT
#ifdef _DEBUG_STR_5_BIT
	#define assert_5bitLH(x,s) if(!(x))printf("%s:%d %s\n",__FILE__,__LINE__,s);
#else
	#define assert_5bitLH(x,s)
#endif

static unsigned char conversionMap[256] = {
 32, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 29, 28, 28, 28, 28, 28, 28, 28,
 28, 28, 28, 28, 28, 28, 28, 28, 26, 25, 16,  9, 23, 10,  1, 24, 22, 27, 28, 28,
 28, 28, 28, 28, 28,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 28, 28, 28, 28, 28, 28,  0,  1,  2,
  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
 23, 24, 25, 28, 28, 28, 28, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
};

static unsigned char reverseconversionMap[256] = {
  97,  54,  99, 100, 101, 102, 103, 104, 105,  51,  53, 108, 109, 110, 111, 112,  50, 114, 115, 116,
 117, 118,  56,  52,  55,  49,  48,  57,  95,  32,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

#define STR5BIT_GET_L_LEN(str_len) ((str_len + 1 + 1 * 2) / 2)

size_t GetMemRequired5BitLH(size_t str_len)
{
	size_t newLen_low = STR5BIT_GET_L_LEN(str_len);
	size_t newLen_high = (str_len * 1 + 7) / 8;

	assert_5bitLH(newLen_low < 0xFFFF, "Can't have more than 0xFFFF chars");

	const size_t padding = 0; // needed because we read outside the size of the string
	const size_t headerSize = sizeof(str5BitLH);

	return headerSize + newLen_low + newLen_high + padding;
}

str5BitLH* ConvertTo5BitLH(const char* str, str5BitLH *out_dst, size_t bufSize)
{
	size_t str_len = strlen(str);
	size_t totalMemRequired = GetMemRequired5BitLH(str_len);

	// if we received a buffer, check if it's large enough for us
	if (bufSize != 0 && bufSize < totalMemRequired)
	{
		return NULL;
	}
	str5BitLH* ret;

	// do we need to create a new string ?
	if (out_dst == NULL)
	{
		ret = (str5BitLH *)malloc(totalMemRequired); // padding added to avoid illegal read
		if (ret == NULL)
		{
			if (out_dst)
			{
				out_dst->len = 0;
			}
			return NULL;
		}
	}
	else
	{
		ret = out_dst;
	}

	unsigned char* resstr_low = ret->str;
	memset(ret, 0, totalMemRequired); // we might end up skipping all chars due to utf8

	size_t newLen_low = STR5BIT_GET_L_LEN(str_len);
	unsigned char* resstr_high = &resstr_low[newLen_low];

	size_t valuesWritten = 0;
	for (size_t i = 0; i < str_len; i++)
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
	ret->len = (unsigned short)valuesWritten;

	return ret;
}

//substr size is 1 character ( 5 bits )
static size_t inline strstr5BitLHAt_1(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 1); // -1 because subStr len is 1
	const unsigned char Searched_Low1 = subStr->str[0]; // 4 bits
	const unsigned char Searched_Low2 = subStr->str[0] << 4; // 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if ((str_low[0] & 0x0F) == Searched_Low1)
		{
			size_t byte_index = char_index >> 3; // every byte contains 8 chars, but i is not char index but byte index
			size_t bit_index = char_index & 0x07;
			const unsigned char Searched_High1 = str_high[byte_index];
			// check the high 1 bit
			if (((Searched_High1 >> bit_index) & 0x01) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if ((str_low[0] & 0xF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned char Searched_High1 = str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x01) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 2 character ( 10 bits )
static size_t inline strstr5BitLHAt_2(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 2); // -1 because subStr len is 1
	const unsigned char Searched_Low1 = subStr->str[0]; // 4 + 4 bits
	const unsigned short Searched_Low2 = ((unsigned short)subStr->str[0]) << 4; // 4 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (str_low[0] == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x03) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned short*)&str_low[0]) & 0x0FF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x03) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 3 character ( 15 bits )
static size_t inline strstr5BitLHAt_3(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 3); // -3 because subStr len is 3
	const unsigned short Searched_Low1 = (*(unsigned short*)&subStr->str[0]) & 0x0FFF; // 4 + 4 + 4 bits
	const unsigned short Searched_Low2 = Searched_Low1 << 4; // 4 + 4 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(unsigned short*)&str_low[0]) & 0x0FFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x07) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned short*)&str_low[0]) & 0xFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x07) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 4 character ( 20 bits )
static size_t inline strstr5BitLHAt_4(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 4); // -4 because subStr len is 3
	const unsigned short Searched_Low1 = (*(unsigned short*)&subStr->str[0]) & 0xFFFF; // 4 + 4 + 4 + 4 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 4 + 4 + 4 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(unsigned short*)&str_low[0])) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x0F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[0]) & 0x000FFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x0F) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 5 character ( 25 bits )
static size_t inline strstr5BitLHAt_5(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 5); // -5 because subStr len is 5
	const unsigned int Searched_Low1 = (*(unsigned int*)&subStr->str[0]) & 0xFFFFF; // 20 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 20 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(unsigned int*)&str_low[0]) & 0x000FFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[0]) & 0x00FFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1F) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 6 character ( 30 bits )
static size_t inline strstr5BitLHAt_6(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 6); // -6 because subStr len is 6
	const unsigned int Searched_Low1 = (*(unsigned int*)&subStr->str[0]) & 0x00FFFFFF; // 24 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 24 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(unsigned int*)&str_low[0]) & 0x00FFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[0]) & 0x0FFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3F) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 6 character ( 35 bits )
static size_t inline strstr5BitLHAt_7(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 7); // -7 because subStr len is 7
	const unsigned int Searched_Low1 = (*(unsigned int*)&subStr->str[0]) & 0x0FFFFFFF; // 28 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 28 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(unsigned int*)&str_low[0]) & 0x0FFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[0]) & 0xFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7F) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 8 character ( 40 bits )
static size_t inline strstr5BitLHAt_8(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 8); // -8 because subStr len is 8
	const uint32_t Searched_Low1 = (*(uint32_t*)&subStr->str[0]); // 32 bits
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 32 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint32_t*)&str_low[0])) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFF) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFF) == substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 9 character ( 45 bits )
static size_t inline strstr5BitLHAt_9(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 9); // -9 because subStr len is 9
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFF; // 36 bits
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 36 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

//substr size is 10 character ( 45 bits )
static size_t inline strstr5BitLHAt_10(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 10); // -10 because subStr len is 10
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFFF; // 36 bits
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 36 + 4 bits
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

static size_t inline strstr5BitLHAt_11(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 11);
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

static size_t inline strstr5BitLHAt_12(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 12);
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFFF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFFF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

static size_t inline strstr5BitLHAt_13(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 13);
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FFF) == ((*(unsigned int*)&substr_high[0]) & 0x1FFF))
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FFF) == ((*(unsigned int*)&substr_high[0]) & 0x1FFF))
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

static size_t inline strstr5BitLHAt_14(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 14);
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FFF) == ((*(unsigned int*)&substr_high[0]) & 0x3FFF))
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FFF) == ((*(unsigned int*)&substr_high[0]) & 0x3FFF))
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

static size_t inline strstr5BitLHAt_15(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 15);
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str[0]) & 0xFFFFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* __restrict str_low = largeStr->str;
	const unsigned char* __restrict str_high = &largeStr->str[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* __restrict substr_high = &subStr->str[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t char_index = 0; char_index <= max_char_searched; char_index += 2)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FFF) == ((*(unsigned int*)&substr_high[0]) & 0x7FFF))
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[0]) & 0xFFFFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FFF) == ((*(unsigned int*)&substr_high[0]) & 0x7FFF))
			{
				return 1;
			}
		}
		str_low++;
	}
	return 0;
}

#define haszero(v) (((v) - 0x0101010101010101ULL) & ~(v) & 0x8080808080808080ULL)

static size_t inline strstr5BitLHAt_16(const str5BitLH* __restrict largeStr, const str5BitLH* __restrict subStr)
{
	const size_t max_char_searched = (largeStr->len - 15);
	// fill register 1 with first 'char' => 64 bits filled with 4 bits => 16 values
	uint64_t firstChar = subStr->str[0] & 0x0F;
	firstChar = firstChar | (firstChar << 4);
	firstChar = firstChar | (firstChar << 8);
	firstChar = firstChar | (firstChar << 16);
	firstChar = firstChar | (firstChar << 32);
	// fill register 2 with last 'char' => 64 bits filled with 4 bits => 16 values
	uint32_t lastByteIndex = (subStr->len >> 1) - 1;
	uint64_t lastChar;
	if ((subStr->len & 1)==0)
	{
		lastChar = subStr->str[lastByteIndex] & 0x0F;
		lastChar = lastChar | (lastChar << 4);
	}
	else
	{
		lastChar = subStr->str[lastByteIndex] & 0xF0;
		lastChar = lastChar | (lastChar >> 4);
	}
	lastChar = lastChar | (lastChar << 8);
	lastChar = lastChar | (lastChar << 16);
	lastChar = lastChar | (lastChar << 32);

	const unsigned char* __restrict str_low1 = &largeStr->str[0];
	const unsigned char* __restrict str_low2 = &largeStr->str[lastByteIndex];

	// check if any of the bytes are found at respective indexes
	// this loop requires 7 operations to check for 16 positions at once
	uint64_t foundChar1Mask = (*(uint64_t*)str_low1) ^ firstChar; // matching char become 0
	uint64_t foundChar2Mask = (*(uint64_t*)str_low2) ^ lastChar; // matching char become 0
	uint64_t CheckAllPos = foundChar1Mask | foundChar2Mask; // 0 at a specific position if a match was found
	if (haszero(CheckAllPos))
	{
		//check exactly
		CheckAllPos = 1;
	}

	return 0;
}

size_t HasStr5BitLH(const str5BitLH* largeStr, const str5BitLH* subStr)
{
	if (largeStr->len < subStr->len)
	{
		return 0;
	}
	switch (subStr->len)
	{
	case 1:
		return strstr5BitLHAt_1(largeStr, subStr);
	case 2:
		return strstr5BitLHAt_2(largeStr, subStr);
	case 3:
		return strstr5BitLHAt_3(largeStr, subStr);
	case 4:
		return strstr5BitLHAt_4(largeStr, subStr);
	case 5:
		return strstr5BitLHAt_5(largeStr, subStr);
	case 6:
		return strstr5BitLHAt_6(largeStr, subStr);
	case 7:
		return strstr5BitLHAt_7(largeStr, subStr);
	case 8:
		return strstr5BitLHAt_8(largeStr, subStr);
	case 9:
		return strstr5BitLHAt_9(largeStr, subStr);
	case 10:
		return strstr5BitLHAt_10(largeStr, subStr);
	case 11:
		return strstr5BitLHAt_11(largeStr, subStr);
	case 12:
		return strstr5BitLHAt_12(largeStr, subStr);
	case 13:
		return strstr5BitLHAt_13(largeStr, subStr);
	case 14:
		return strstr5BitLHAt_14(largeStr, subStr);
	case 15:
		return strstr5BitLHAt_15(largeStr, subStr);
//	case 16:
//		return strstr5BitLHAt_16(largeStr, subStr);
	default:
		printf("Not yet implemented len %d\n", subStr->len);
		break;
	}
	return 0;
}
