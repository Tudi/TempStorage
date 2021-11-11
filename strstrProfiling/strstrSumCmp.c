#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

static inline int strstrSumWord_(unsigned char* tgt, const size_t tgt_len, unsigned char *pat, size_t len)
{
    size_t needleSum = 0;
    size_t tgtSum = 0;
    for (size_t needleLen = 0; needleLen < len; needleLen++)
    {
        needleSum += pat[needleLen];
        tgtSum += tgt[needleLen];
    }
    unsigned char* tgtl = &tgt[len];
    for (int64_t counter = tgt_len - len; counter >= 0; --counter)
    {
        if (needleSum == tgtSum)
        {
            if (memcmp(tgt, pat, len) == 0)
            {
                return 1;
            }
        }
        tgtSum -= (*tgt);
        tgtSum += (*tgtl);
        tgt++;
        tgtl++;
    }
    return 0;
}

int strstrSumWord(char * tgt, size_t tgt_len, char * pat, const size_t pat_len)
{
    if (tgt_len < pat_len)
    {
        return 0;
    }
    else if (tgt_len == pat_len)
    {
        return memcmp(tgt, pat, tgt_len) == 0;
    }
    if (pat_len > 32)
    {
        return strstrSumWord_((unsigned char*)tgt, tgt_len, (unsigned char*)pat, pat_len);
    }
    switch (pat_len)
    {
        case  0: return 0;
        case  1: return strchr(tgt, *pat) != NULL;
        case  8: return strstr(tgt, pat) != NULL;
        default: return strstr(tgt, pat) != NULL;
    }

    return 0;
}

_noinline_ void Run_strstr_Sum_Word_test()
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
            int testRes = strstrSumWord(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
#ifdef _DEBUG
            int debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
            if (testRes != debugstrstrRes)
            {
                testRes = strstrSumWord(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
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
