#include <string.h>
#include <stdio.h>
#include "InputGenerator.h"
#include "ProfileTimer.h"

#if defined(__FreeBSD__)
    #include <sys/endian.h>
#elif defined(__linux__) && defined(__USE_BSD)
    #include <byteswap.h>
    #define  bswap16 bswap_16
    #define  bswap32 bswap_32
#elif defined(WINDOWS_BUILD)
    #define  MSBF16(x)   (*((unsigned short const*)x))
    #define  MSBF32(x)   (*((unsigned int const*)x))
#else
    #error "Need a bswap intrinsic!"
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

static char const* scanstr2(char const* tgt, char const pat[2])
{
    unsigned short head = MSBF16(pat), wind = 0, next;

    while ((next = *(unsigned char const*)tgt++)) {
        wind = (wind << 8) + next;
        if (wind == head)
            return tgt - 2;
    }
    return  NULL;
}

// NOTE: MSBF32(pat) will never read beyond pat[] in memory,
//          because pat has a null-terminator.
static char const* scanstr3(char const* tgt, char const pat[3])
{
    unsigned int head = MSBF32(pat), wind = 0, next;

    while ((next = *(unsigned char const*)tgt++)) {
        wind = (wind + next) << 8;
        if (wind == head)
            return tgt - 3;
    }
    return  NULL;
}

static char const* scanstrm(char const* tgt, char const* pat, int len)
{
    unsigned int head = MSBF32(pat), wind = 0, next;

    pat += 4, len -= 4;
    while ((next = *(unsigned char const*)tgt++)) {
        wind = (wind << 8) + next;
        if (wind == head && !memcmp(tgt, pat, len))
            return tgt - 4;
    }
    return  NULL;
}

char const* scanstr(char const* tgt, char const* pat, size_t pat_len)
{
    switch (pat_len) 
    {
        case  0: return tgt;
        case  1: return strchr(tgt, *pat);
        case  2: return scanstr2(tgt, pat);
        case  3: return scanstr3(tgt, pat);
        default: return scanstrm(tgt, pat, (int)pat_len);
    }
}

_noinline_ void Run_strstr_Mischasan_test()
{
	printf("Starting test %s ...", __FUNCTION__);
	StartTimer();
	size_t foundCount = 0; // anti optimization
    for (size_t repeatCount = 0; repeatCount < REPEAT_SAME_TEST_COUNT; repeatCount++)
        for (size_t searchedIndex = 0; searchedIndex < uiSearchedStrCount; searchedIndex++)
	{
		for (size_t inputIndex = 0; inputIndex < uiInputStrCount; inputIndex++)
		{
			if (scanstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len) != 0)
			{
				foundCount++;
			}
		}

	}
	double runtimeSec = EndTimer();
	printf(" ... Done\n");
	printf("Found the string %d times. Seconds : %f\n", (int)foundCount, (float)runtimeSec);
}
