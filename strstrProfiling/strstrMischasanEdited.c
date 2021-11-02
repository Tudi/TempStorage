#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

#if defined(WINDOWS_BUILD)
	#include <stdlib.h>
	#define  MSBF16(x)   (_byteswap_ushort(*((unsigned short const*)x)))
	#define  MSBF32(x)   (_byteswap_ulong(*((unsigned long const*)x)))
#endif

#ifndef WINDOWS_BUILD
    #if BYTE_ORDER == BIG_ENDIAN
        #define  MSBF16(x)   (*(uint16_t const*__attribute((aligned(1))))x)
        #define  MSBF32(x)   (*(uint32_t const*__attribute((aligned(1))))x)
    #else
        #define  MSBF16(x)   bswap16(*(uint16_t const*__attribute((aligned(1))))x)
        #define  MSBF32(x)   bswap32(*(uint32_t const*__attribute((aligned(1))))x)
    #endif
#endif

static int scanstr2(char const* tgt, const int64_t tgt_len, char const pat[2], int64_t len)
{
    unsigned short head = MSBF16(pat), wind = MSBF16(tgt);

    tgt += 2;
    for (int64_t counter = 0; counter <= tgt_len - len - 2; counter++)
    {
        if (wind == head)
            return 1;
        wind = (wind << 8) | tgt[0];
        tgt++;
    }
    return  0;
}

static int scanstr3(char const* tgt, const int64_t tgt_len, char const pat[3], int64_t len)
{
    unsigned int head = 0, wind = 0;
    ((char*)&head)[0] = pat[2];
    ((char*)&head)[1] = pat[1];
    ((char*)&head)[2] = pat[0];

    ((char*)&wind)[0] = tgt[2];
    ((char*)&wind)[1] = tgt[1];
    ((char*)&wind)[2] = tgt[0];

    tgt += 3;
    for (int64_t counter = 0; counter <= tgt_len - len; counter++)
    {
        if ((wind & 0x00FFFFFF) == head)
            return 1;
        wind = (wind << 8) | tgt[0];
        tgt++;
    }
    return  0;
}

static int scanstr4(char const* tgt, const int64_t tgt_len, char const* pat, int64_t len)
{
    unsigned int head = MSBF32(pat), wind = MSBF32(tgt);

    tgt += 4;
    for (int64_t counter = 0; counter <= tgt_len - len; counter++)
    {
        if (wind == head)
            return 1;
        wind = (wind << 8) | tgt[0];
        tgt++;
    }
    return  0;
}

static int scanstrm(char const* tgt, const int64_t tgt_len, char const* pat, int64_t len)
{
    unsigned int head = MSBF32(pat), wind = MSBF32(tgt);

    tgt += 4;
    pat += 4;
    len -= 4;
    for (int64_t counter = tgt_len - len + 4; counter >= 0; --counter)
    {
        if (wind == head && memcmp(tgt, pat, len) == 0)
            return 1;
        wind = (wind << 8) | tgt[0];
        tgt++;
    }
    return  0;
}

int scanstr_ext(char const* tgt, int64_t tgt_len, char const* pat, const int64_t pat_len)
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
    case  2: return scanstr2(tgt, tgt_len, pat, 2);
    case  3: return scanstr3(tgt, tgt_len, pat, 3);
    case  4: return scanstr4(tgt, tgt_len, pat, 4);
    default: return scanstrm(tgt, tgt_len, pat, pat_len);
    }
}

_noinline_ void Run_strstr_Mischasan_Extended_test()
{
    printf("Starting test %s ...", __FUNCTION__);
    StartTimer();
    size_t searchesMade = 0;
    size_t foundCount = 0; // anti optimization
    for (size_t searchedIndex = 0; searchedIndex < uiSearchedStrCount; searchedIndex++)
    {
        for (size_t inputIndex = 0; inputIndex < uiInputStrCount; inputIndex++)
        {
            int testRes = scanstr_ext(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
#ifdef _DEBUG
            int debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
            if (testRes != debugstrstrRes)
            {
                testRes = scanstr_ext(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
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
