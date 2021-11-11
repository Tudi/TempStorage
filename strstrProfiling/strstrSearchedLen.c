#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

/// <summary>
/// Copy pasted function from gcc 
/// </summary>
/// <param name="s1"></param>
/// <param name="s2"></param>
/// <returns></returns>
size_t strstr_s2Len(const char* s1, const char* s2, size_t s2_len)
{
	while (*s1)
	{
		if (memcmp(s1, s2, s2_len) == 0) // 2 string matched
		{
			return 1;
		}
		++s1;
	}
	return (0);
}

_noinline_ void Run_strstr_know_searched_length_test()
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
			if (strstr_s2Len(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len) != 0)
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
