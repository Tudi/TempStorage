#include "InputGenerator.h"
#include <string.h>
#include <stdlib.h>

const char* SeedString = "The quick brown fox jumps over the lazy dog.The quick brown fox jumps over the lazy dog.";
size_t uiInputStrCount = 0;
profiledStringStore* sInputStrings = NULL;

size_t uiSearchedStrCount = 0;
profiledStringStore* sSearchedStrings = NULL;

size_t uiInputNOPStrCount = 0;
noPointerString** sInputNOPStrings = NULL;

char* strPartialDup(const char* str, size_t start, size_t count, char addPadding)
{
	char* ret = (char*)malloc(count + 1 + addPadding * 3);
	memcpy(ret, &str[start], count);
	ret[count] = 0;
	if (addPadding)
	{
		ret[count+1] = 0;
		ret[count+2] = 0;
		ret[count+3] = 0;
	}
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

void GenerateInputStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding)
{
	GenerateStrings_(memorySizeUsed, minLen, maxLen, addPadding, &sInputStrings, &uiInputStrCount);
}

void GenerateSearchedStrings(size_t memorySizeUsed, size_t minLen, size_t maxLen, char addPadding)
{
	GenerateStrings_(memorySizeUsed, minLen, maxLen, addPadding, &sSearchedStrings, &uiSearchedStrCount);
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
		sInputNOPStrings[index] = (noPointerString*)malloc(sizeof(noPointerString) + strLen + 1);
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
