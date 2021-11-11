#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

// used for loop unroll
#define CHECK_VALUE_LEN_AT_INDEX(ind, mask, type) if (((*((type*)(tgt+ind))) & mask) == pat_local)  return 1;
#define RETURN_VALUE_LEN_AT_INDEX(ind, mask, type) return (((*((type*)(tgt+ind))) & mask) == pat_local);
#define CHECK_VALUE_LOOP_UNROLL(mask, type)    switch (counter)\
    {\
        case 1: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(1, mask, type);\
        }break;\
        case 2: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(2, mask, type);\
        }break;\
        case 3: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(3, mask, type);\
        }break;\
        case 4: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(4, mask, type);\
        }break;\
        case 5: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(4, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(5, mask, type);\
        }break;\
        case 6: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(4, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(5, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(6, mask, type);\
        }break;\
        case 7: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(4, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(5, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(6, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(7, mask, type);\
        }break;\
        case 8: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(4, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(5, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(6, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(7, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(8, mask, type);\
        }break;\
        case 9: {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(4, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(5, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(6, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(7, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(8, mask, type);\
            RETURN_VALUE_LEN_AT_INDEX(9, mask, type);\
        }break;\
        default:\
        {\
            CHECK_VALUE_LEN_AT_INDEX(0, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(1, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(2, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(3, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(4, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(5, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(6, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(7, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(8, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(9, mask, type);\
            CHECK_VALUE_LEN_AT_INDEX(10, mask, type);\
            counter -= 10;\
            tgt += 10;\
        }\
    }

#define CHECK_VALUE_LEN_AT_INDEX_NOMASK(ind, type) if (((*((type*)(tgt+ind)))) == pat_local)  return 1;
#define RETURN_VALUE_LEN_AT_INDEX_NOMASK(ind, type) return (((*((type*)(tgt+ind)))) == pat_local);
#define CHECK_VALUE_LOOP_UNROLL_NOMASK(type)    switch (counter)\
    {\
        case 1: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
        }break;\
        case 2: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
        }break;\
        case 3: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
        }break;\
        case 4: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
        }break;\
        case 5: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(5, type);\
        }break;\
        case 6: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(5, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(6, type);\
        }break;\
        case 7: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(5, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(6, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(7, type);\
        }break;\
        case 8: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(5, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(6, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(7, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(8, type);\
        }break;\
        case 9: {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(5, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(6, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(7, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(8, type);\
            RETURN_VALUE_LEN_AT_INDEX_NOMASK(9, type);\
        }break;\
        default:\
        {\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(0, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(1, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(2, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(3, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(4, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(5, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(6, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(7, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(8, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(9, type);\
            CHECK_VALUE_LEN_AT_INDEX_NOMASK(10, type);\
            counter -= 10;\
            tgt += 10;\
        }\
    }

static int scanstr2_v2(char const* tgt, const size_t tgt_len, char const pat[2], size_t len)
{
    for (int64_t counter = tgt_len - len; counter >= 0 ; --counter)
    {
        if (*(short*)tgt == *(short*)pat)
            return 1;
        tgt++;
    }
    return  0;
}

static int scanstr3_v2(char const* tgt, const size_t tgt_len, char const pat[3], size_t len)
{
    int pat_local = (*(int*)pat);
    int64_t counter = tgt_len - len;
    CHECK_VALUE_LOOP_UNROLL(0x00FFFFFF, int);
    // Loop unroll
    for (; counter >= 0; --counter)
    {
        if (((*(int*)tgt) & 0x00FFFFFF) == pat_local)
            return 1;
        tgt++;
    }
    return  0;
}

static int scanstr4_v2(char const* tgt, const size_t tgt_len, char const pat[4], size_t len)
{
    int pat_local = (*(int*)pat);
    int64_t counter = tgt_len - len;
    CHECK_VALUE_LOOP_UNROLL_NOMASK(int);
    for (; counter >= 0; --counter)
    {
        if (*(int*)tgt == *(int*)pat)
            return 1;
        tgt++;
    }
    return 0;
}

static int scanstr5_v2(char const* tgt, const size_t tgt_len, char const pat[5], size_t len)
{
    int64_t pat_local = *(int64_t*)pat & 0x000000FFFFFFFFFF;
    int64_t counter = tgt_len - len;
    CHECK_VALUE_LOOP_UNROLL(0x000000FFFFFFFFFF, int64_t);
    for (; counter >= 0; --counter)
    {
        if (((*(int64_t*)tgt) & 0x000000FFFFFFFFFF) == pat_local)
            return 1;
        tgt++;
    }
    return 0;
}

static int scanstr6_v2(char const* tgt, const size_t tgt_len, char const pat[6], size_t len)
{
    int64_t pat_local = *(int64_t*)pat & 0x0000FFFFFFFFFFFF;
    int64_t counter = tgt_len - len;
    CHECK_VALUE_LOOP_UNROLL(0x0000FFFFFFFFFFFF, int64_t);
    for (; counter >= 0; --counter)
    {
        if (((*(int64_t*)tgt) & 0x0000FFFFFFFFFFFF) == pat_local)
            return 1;
        tgt++;
    }
    return 0;
}

static int scanstr7_v2(char const* tgt, const size_t tgt_len, char const pat[7], size_t len)
{
    int64_t pat_local = *(int64_t*)pat & 0x00FFFFFFFFFFFFFF;
    int64_t counter = tgt_len - len;
    CHECK_VALUE_LOOP_UNROLL(0x00FFFFFFFFFFFFFF, int64_t);
    for (; counter >= 0; --counter)
    {
        if (((*(int64_t*)tgt) & 0x00FFFFFFFFFFFFFF) == pat_local)
            return 1;
        tgt++;
    }
    return 0;
}

static int scanstr8_v2(char const* tgt, const size_t tgt_len, char const pat[8], size_t len)
{
    int64_t pat_local = *(int64_t*)pat;
    int64_t counter = tgt_len - len;
    CHECK_VALUE_LOOP_UNROLL_NOMASK(int64_t);
    for (; counter >= 0; --counter)
    {
        if (*(int64_t*)tgt == pat_local)
            return 1;
        tgt++;
    }
    return 0;
}

int strstrv2(char const* tgt, size_t tgt_len, char const* pat, const size_t pat_len)
{
    if (tgt_len < pat_len)
    {
        return 0;
    }
    else if (tgt_len == pat_len)
    {
        return memcmp(tgt, pat, tgt_len) == 0;
    }
    switch (pat_len)
    {
    case  0: return 1;
    case  1: return strchr(tgt, *pat) != NULL;
    case  2: return scanstr2_v2(tgt, tgt_len, pat, 2);
    case  3: return scanstr3_v2(tgt, tgt_len, pat, 3);
    case  4: return scanstr4_v2(tgt, tgt_len, pat, 4);
    case  5: return scanstr5_v2(tgt, tgt_len, pat, 5);
    case  6: return scanstr6_v2(tgt, tgt_len, pat, 6);
    case  7: return scanstr7_v2(tgt, tgt_len, pat, 7);
    case  8: return scanstr8_v2(tgt, tgt_len, pat, 8);
    default: return strstr(tgt, pat) != NULL;
    }
}

_noinline_ void Run_strstr_Both_Len_V2_test()
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
            int testRes = strstrv2(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
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
    printf("Searches made %d. Found the string %d times. Seconds : %f\n\n", (int)searchesMade, (int)foundCount, (float)runtimeSec);
}
