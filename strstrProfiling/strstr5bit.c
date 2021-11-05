#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

static unsigned char conversionMap[256];

void InitConversionMap()
{
	static unsigned char localConversionMap[256];
	memset(localConversionMap, 0, sizeof(localConversionMap));

	for (size_t i = 'A'; i <= 'Z'; i++) // 26 chars
	{
		localConversionMap[i] = 'a' - 'A';
	}
	for (size_t i = 'a'; i <= 'z'; i++)
	{
		localConversionMap[i] = i;
	}
	for (size_t i = '0'; i <= '9'; i++)
	{
		localConversionMap[i] = i;
	}

	size_t Code = 1;
	memset(conversionMap, 0, sizeof(conversionMap));
	for (size_t i = 0; i < 256; i++)
	{
		if (localConversionMap[i] != 0)
		{
			if(conversionMap[i] == 0)
		}
	}
}

char* ConvertTo5Bit(const char* str)
{

}
