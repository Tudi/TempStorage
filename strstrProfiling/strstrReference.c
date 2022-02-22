#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

_noinline_ void Run_strstr_reference_test()
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
			if (strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL)
			{
				foundCount++;
			}
			searchesMade++;
		}
	}
	double runtimeSec = EndTimer();
	printf(" ... Done\n");
	printf("Searches made %zu. Found the string %zu times. Seconds : %f\n\n", searchesMade, foundCount, (float)runtimeSec);
}
