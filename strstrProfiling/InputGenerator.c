#include "InputGenerator.h"
#include <string.h>
#include <stdlib.h>
#include "strstr5bit.h"

const char* SeedString = "the quick brown fox jumps over the lazy dog.the quick brown fox jumps over the lazy dog.";
size_t uiInputStrCount = 0;
profiledStringStore* sInputStrings = NULL;

size_t uiSearchedStrCount = 0;
profiledStringStore* sSearchedStrings = NULL;

size_t uiInputNOPStrCount = 0;
noPointerString** sInputNOPStrings = NULL;

str5Bit* sInputStrings5Bit = NULL;
str5Bit* sSearchedStrings5Bit = NULL;

char* strPartialDup(const char* str, size_t start, size_t count, char addPadding)
{
	char* ret = (char*)malloc(count + 1 + addPadding);
	memcpy(ret, &str[start], count);
	memset(&ret[count], 0, 1 + addPadding);
	return ret;
}

void GenerateStrings_(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding, profiledStringStore** strStore, size_t *Counter)
{
	size_t avgLen = (minLen + maxLen) / 2;
	size_t stringCount = memorySizeUsed / avgLen;
	*Counter = stringCount;
	size_t seedLen = strlen(SeedString) / 2;
	size_t strStart = 0;
	size_t strLen = minLen;
	(*strStore) = (profiledStringStore *)malloc((stringCount + 1) * sizeof(profiledStringStore));
	for (size_t index = 0; index < stringCount; index++)
	{
		// cut a portion of the seed string
		(*strStore)[index].str = strPartialDup(SeedString, strStart, strLen, addPadding);
		(*strStore)[index].len = (unsigned short)strLen;

		strLen++;
		if (strLen > maxLen)
		{
			strLen = minLen;
		}
		strStart++;
		if (strStart >= seedLen)
		{
			strStart = 0;
		}
	}
}

void GenerateInput5BitStrings_(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding, str5Bit** strStore)
{
	size_t avgLen = (minLen + maxLen) / 2;
	size_t stringCount = memorySizeUsed / avgLen;
	size_t seedLen = strlen(SeedString) / 2;
	size_t strStart = 0;
	size_t strLen = minLen;
	(*strStore) = (str5Bit*)malloc((stringCount + 1) * sizeof(str5Bit));
	for (size_t index = 0; index < stringCount; index++)
	{
		// cut a portion of the seed string
		char* tstr = strPartialDup(SeedString, strStart, strLen, addPadding);
		ConvertTo5Bit(tstr, &(*strStore)[index]);

		free(tstr);

		strLen++;
		if (strLen > maxLen)
		{
			strLen = minLen;
		}
		strStart++;
		if (strStart >= seedLen)
		{
			strStart = 0;
		}
	}
}

void GenerateInputStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding)
{
	GenerateStrings_(memorySizeUsed, minLen, maxLen, addPadding, &sInputStrings, &uiInputStrCount);
	GenerateInput5BitStrings_(memorySizeUsed, minLen, maxLen, addPadding, &sInputStrings5Bit);
}

void GenerateSearchedStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding)
{
	GenerateStrings_(memorySizeUsed, minLen, maxLen, addPadding, &sSearchedStrings, &uiSearchedStrCount);
	GenerateInput5BitStrings_(memorySizeUsed, minLen, maxLen, addPadding, &sSearchedStrings5Bit);
}

void GenerateInputNOPStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding)
{
	size_t avgLen = (minLen + maxLen) / 2;
	size_t stringCount = memorySizeUsed / avgLen;
	uiInputNOPStrCount = stringCount;
	size_t seedLen = strlen(SeedString) / 2;
	size_t strStart = 0;
	size_t strLen = minLen;
	sInputNOPStrings = (noPointerString**)malloc((stringCount + 1) * sizeof(noPointerString*));
	for (size_t index = 0; index < stringCount; index++)
	{
		// cut a portion of the seed string
		size_t allocSize = GetNOPStringSize(strLen + 1); // in case more than 1 string 
		sInputNOPStrings[index] = (noPointerString*)malloc(allocSize + addPadding);
		sInputNOPStrings[index]->loc = sizeof(noPointerString); // write right after the struct. If more than 1 string, a write index should be used
		sInputNOPStrings[index]->len = (unsigned short)strLen;
		char* strStore = GetNOPString(sInputNOPStrings[index]);
		memcpy(strStore, &SeedString[strStart], strLen);
		strStore[strLen] = 0;

		strLen++;
		if (strLen > maxLen)
		{
			strLen = minLen;
		}
		strStart++;
		if (strStart >= seedLen)
		{
			strStart = 0;
		}
	}
}
