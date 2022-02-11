#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

int strstrv2(char const* tgt, size_t tgt_len, char const* pat, const size_t pat_len);
_noinline_ void Run_strstr_NOPString_test()
{
    printf("Starting test %s ...", __FUNCTION__);
    StartTimer();
    size_t searchesMade = 0;
    size_t foundCount = 0; // anti optimization
    for (size_t repeatCount = 0; repeatCount < REPEAT_SAME_TEST_COUNT; repeatCount++)
        for (size_t searchedIndex = 0; searchedIndex < uiSearchedStrCount; searchedIndex++)
    {
        for (size_t inputIndex = 0; inputIndex < uiInputNOPStrCount; inputIndex++)
        {
            char* NOPString = GetNOPString(sInputNOPStrings[inputIndex]);
            int testRes = strstrv2(NOPString, sInputNOPStrings[inputIndex]->len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
#ifdef _DEBUG
            int debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
            if (testRes != debugstrstrRes)
            {
                testRes = strstrv2(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
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
