#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "strstr5bit.h"

#define STR5BIT_GET_L_LEN(strlen) ((strlen * 4 + 7) / 8 + 7)

//substr size is 1 character ( 5 bits )
static size_t strstr5BitLHAt_1(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 1 + 1) / 2; // -1 because subStr len is 1
	const unsigned char Searched_Low1 = subStr->str_LH[0]; // 4 bits
	const unsigned char Searched_Low2 = subStr->str_LH[0] << 4; // 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if ((str_low[i] & 0x0F) == Searched_Low1)
		{
			size_t char_index = i << 1;
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
		if ((str_low[i] & 0xF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned char Searched_High1 = str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x01) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 2 character ( 10 bits )
static size_t strstr5BitLHAt_2(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 2 + 1) / 2; // -1 because subStr len is 1
	const unsigned char Searched_Low1 = subStr->str_LH[0]; // 4 + 4 bits
	const unsigned short Searched_Low2 = ((unsigned short)subStr->str_LH[0]) << 4; // 4 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (str_low[i] == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x03) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned short*)&str_low[i]) & 0x0FF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x03) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 3 character ( 15 bits )
static size_t strstr5BitLHAt_3(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 3 + 1) / 2; // -3 because subStr len is 3
	const unsigned short Searched_Low1 = (*(unsigned short*)&subStr->str_LH[0]) & 0x0FFF; // 4 + 4 + 4 bits
	const unsigned short Searched_Low2 = Searched_Low1 << 4; // 4 + 4 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(unsigned short*)&str_low[i]) & 0x0FFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x07) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned short*)&str_low[i]) & 0xFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x07) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 4 character ( 20 bits )
static size_t strstr5BitLHAt_4(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 4 + 1) / 2; // -4 because subStr len is 3
	const unsigned short Searched_Low1 = (*(unsigned short*)&subStr->str_LH[0]) & 0xFFFF; // 4 + 4 + 4 + 4 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 4 + 4 + 4 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(unsigned short*)&str_low[i])) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x0F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[i]) & 0x000FFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x0F) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 5 character ( 25 bits )
static size_t strstr5BitLHAt_5(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 5 + 1) / 2; // -5 because subStr len is 5
	const unsigned int Searched_Low1 = (*(unsigned int*)&subStr->str_LH[0]) & 0xFFFFF; // 20 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 20 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(unsigned int*)&str_low[i]) & 0x000FFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[i]) & 0x00FFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1F) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 6 character ( 30 bits )
static size_t strstr5BitLHAt_6(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 6 + 1) / 2; // -6 because subStr len is 6
	const unsigned int Searched_Low1 = (*(unsigned int*)&subStr->str_LH[0]) & 0x00FFFFFF; // 24 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 24 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(unsigned int*)&str_low[i]) & 0x00FFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[i]) & 0x0FFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3F) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 6 character ( 35 bits )
static size_t strstr5BitLHAt_7(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 7 + 1) / 2; // -7 because subStr len is 7
	const unsigned int Searched_Low1 = (*(unsigned int*)&subStr->str_LH[0]) & 0x0FFFFFFF; // 28 bits
	const unsigned int Searched_Low2 = Searched_Low1 << 4; // 28 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(unsigned int*)&str_low[i]) & 0x0FFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7F) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(unsigned int*)&str_low[i]) & 0xFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7F) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 8 character ( 40 bits )
static size_t strstr5BitLHAt_8(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 8 + 1) / 2; // -8 because subStr len is 8
	const uint32_t Searched_Low1 = (*(uint32_t*)&subStr->str_LH[0]); // 32 bits
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 32 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint32_t*)&str_low[i]) ) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFF) == substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFF) == substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 9 character ( 45 bits )
static size_t strstr5BitLHAt_9(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 9 + 1) / 2; // -9 because subStr len is 9
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFF; // 36 bits
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 36 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned short Searched_High1 = *(unsigned short*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FF) == *(unsigned short*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

//substr size is 10 character ( 45 bits )
static size_t strstr5BitLHAt_10(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 10 + 1) / 2; // -10 because subStr len is 10
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFFF; // 36 bits
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 36 + 4 bits
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

static size_t strstr5BitLHAt_11(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 11 + 1) / 2; 
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFFFF; 
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4; // 
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

static size_t strstr5BitLHAt_12(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 12 + 1) / 2;
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;  
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0xFFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

static size_t strstr5BitLHAt_13(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 13 + 1) / 2;
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x1FFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

static size_t strstr5BitLHAt_14(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 14 + 1) / 2;
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x3FFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

static size_t strstr5BitLHAt_15(const str5Bit* largeStr, const str5Bit* subStr)
{
	const size_t max_byte_searched = (largeStr->len - 15 + 1) / 2;
	const uint64_t Searched_Low1 = (*(uint64_t*)&subStr->str_LH[0]) & 0xFFFFFFFFFFFFFFF;
	const uint64_t Searched_Low2 = (uint64_t)Searched_Low1 << 4;
	const unsigned char* str_low = largeStr->str_LH;
	const unsigned char* str_high = &largeStr->str_LH[STR5BIT_GET_L_LEN(largeStr->len)];
	const unsigned char* substr_high = &subStr->str_LH[STR5BIT_GET_L_LEN(subStr->len)];
	// checks 2 chars every loop
	for (size_t i = 0; i <= max_byte_searched; i++)
	{
		// check value at pos 0 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFFFFF) == Searched_Low1)
		{
			// check the high 1 bit
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // every byte contains 8 chars
			size_t bit_index = char_index & 0x07;
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
		// check value at pos 1 + i
		if (((*(uint64_t*)&str_low[i]) & 0xFFFFFFFFFFFFFFF0) == Searched_Low2)
		{
			size_t char_index = i << 1;
			size_t byte_index = char_index >> 3; // pointless to add 1 since we shift it
			size_t bit_index = (char_index + 1) & 0x07; // add 1 because we are checking pos 'i + 1' not 'i'
			const unsigned int Searched_High1 = *(unsigned int*)&str_high[byte_index];
			if (((Searched_High1 >> bit_index) & 0x7FFF) == *(unsigned int*)&substr_high[0])
			{
				return 1;
			}
		}
	}
	return 0;
}

size_t HasStr5BitLH(const str5Bit* largeStr, const str5Bit* subStr)
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
	default:
		printf("Not yet implemented len %d\n", subStr->len);
		break;
	}
	return 0;
}
