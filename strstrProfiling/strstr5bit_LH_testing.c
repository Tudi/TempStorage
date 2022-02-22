#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "strstr5bit_LH.h"

void runstr5BitLHTest(const char* large, const char* small)
{
	str5BitLH *Large, *Small;
	Large = ConvertTo5BitLH(large, NULL, 0);
	Small = ConvertTo5BitLH(small, NULL, 0);
	int expectedResult = strstr(large, small) != NULL;
	if (HasStr5BitLH(Large, Small) != expectedResult)
	{
		printf("b0rken %s - %s\n", large, small);
		HasStr5BitLH(Large, Small);
	}
	free(Large);
	free(Small);
}

void runstr5BitLHTests()
{
	runstr5BitLHTest("dog.the qu", " qui");

	runstr5BitLHTest("9", "9");
	runstr5BitLHTest("9b", "9");
	runstr5BitLHTest("b9", "9");
	runstr5BitLHTest("bb9", "9");
	runstr5BitLHTest("bb9b", "9");
	runstr5BitLHTest("bbbbbbb9", "9");
	runstr5BitLHTest("bbbbbbbb9", "9");

	runstr5BitLHTest("99", "99");
	runstr5BitLHTest("99b", "99");
	runstr5BitLHTest("b99", "99");
	runstr5BitLHTest("bb99", "99");
	runstr5BitLHTest("bb99b", "99");
	runstr5BitLHTest("bbbbbbb99", "99");
	runstr5BitLHTest("bbbbbbbb99", "99");

	runstr5BitLHTest("999", "999");
	runstr5BitLHTest("999b", "999");
	runstr5BitLHTest("b999", "999");
	runstr5BitLHTest("bb999", "999");
	runstr5BitLHTest("bb999b", "999");
	runstr5BitLHTest("bbbbbbb999", "999");
	runstr5BitLHTest("bbbbbbbb999", "999");

	runstr5BitLHTest("9999", "9999");
	runstr5BitLHTest("9999b", "9999");
	runstr5BitLHTest("b9999", "9999");
	runstr5BitLHTest("bb9999", "9999");
	runstr5BitLHTest("bb9999b", "9999");
	runstr5BitLHTest("bbbbbbb9999", "9999");
	runstr5BitLHTest("bbbbbbbb9999", "9999");

	runstr5BitLHTest("99999", "99999");
	runstr5BitLHTest("99999b", "99999");
	runstr5BitLHTest("b99999", "99999");
	runstr5BitLHTest("bb99999", "99999");
	runstr5BitLHTest("bb99999b", "99999");
	runstr5BitLHTest("bbbbbbb99999", "99999");
	runstr5BitLHTest("bbbbbbbb99999", "99999");

	runstr5BitLHTest("999999", "999999");
	runstr5BitLHTest("999999b", "999999");
	runstr5BitLHTest("b999999", "999999");
	runstr5BitLHTest("bb999999", "999999");
	runstr5BitLHTest("bb999999b", "999999");
	runstr5BitLHTest("bbbbbbb999999", "999999");
	runstr5BitLHTest("bbbbbbbb999999", "999999");

	runstr5BitLHTest("9999999", "9999999");
	runstr5BitLHTest("9999999b", "9999999");
	runstr5BitLHTest("b9999999", "9999999");
	runstr5BitLHTest("bb9999999", "9999999");
	runstr5BitLHTest("bb9999999b", "9999999");
	runstr5BitLHTest("bbbbbbb9999999", "9999999");
	runstr5BitLHTest("bbbbbbbb9999999", "9999999");

	runstr5BitLHTest("99999999", "99999999");
	runstr5BitLHTest("99999999b", "99999999");
	runstr5BitLHTest("b99999999", "99999999");
	runstr5BitLHTest("bb99999999", "99999999");
	runstr5BitLHTest("bb99999999b", "99999999");
	runstr5BitLHTest("bbbbbbb99999999", "99999999");
	runstr5BitLHTest("bbbbbbbb99999999", "99999999");

	runstr5BitLHTest("999999999", "999999999");
	runstr5BitLHTest("999999999b", "999999999");
	runstr5BitLHTest("b999999999", "999999999");
	runstr5BitLHTest("bb999999999", "999999999");
	runstr5BitLHTest("bb999999999b", "999999999");
	runstr5BitLHTest("bbbbbbb999999999", "999999999");
	runstr5BitLHTest("bbbbbbbb999999999", "999999999");

	runstr5BitLHTest("9999999999", "9999999999");
	runstr5BitLHTest("9999999999b", "9999999999");
	runstr5BitLHTest("b9999999999", "9999999999");
	runstr5BitLHTest("bb9999999999", "9999999999");
	runstr5BitLHTest("bb9999999999b", "9999999999");
	runstr5BitLHTest("bbbbbbb9999999999", "9999999999");
	runstr5BitLHTest("bbbbbbbb9999999999", "9999999999");
	
/*	runstr5BitLHTest("9999999999999999", "9999999999999999");
	runstr5BitLHTest("9999999999999999b", "9999999999999999");
	runstr5BitLHTest("b9999999999999999", "9999999999999999");
	runstr5BitLHTest("bb9999999999999999", "9999999999999999");
	runstr5BitLHTest("bb9999999999999999b", "9999999999999999");
	runstr5BitLHTest("bbbbbbb9999999999999999", "9999999999999999");
	runstr5BitLHTest("bbbbbbbb9999999999999999", "9999999999999999");*/
}

#include "InputGenerator.h"
#include "ProfileTimer.h"
_noinline_ void Run_strstr_5Bit_LH()
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
				size_t testRes = HasStr5BitLH(sInputStrings5BitLH[inputIndex], sSearchedStrings5BitLH[searchedIndex]);
#ifdef _DEBUG
				int debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
				if (testRes != debugstrstrRes)
				{
					testRes = HasStr5BitLH(sInputStrings5BitLH[inputIndex], sSearchedStrings5BitLH[searchedIndex]);
					testRes = HasStr5BitLH(sInputStrings5BitLH[inputIndex], sSearchedStrings5BitLH[searchedIndex]);
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
	printf("Searches made %zu. Found the string %zu times. Seconds : %f\n\n", searchesMade, foundCount, (float)runtimeSec);
}
