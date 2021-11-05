#include <string.h>
#include <stdio.h>
#ifdef WINDOWS_BUILD
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif
#include "InputGenerator.h"
#include "ProfileTimer.h"

#ifndef BITSCAN_IS_NOT_AVAILABLE
#ifdef WINDOWS_BUILD
	#define GetfirtsBit(index, mask, res) res = _BitScanForward(index, mask)
#else
	#define GetfirtsBit(index, mask, res) index = __builtin_ffs(mask); if(index==0)res=0;else{res=1;index -=1;}
#endif

#define clear_leftmost_set(value) (value & (value - 1));

size_t strstr_AVX2(const char* s, size_t n, const char* needle, size_t k) 
{
	const __m256i first = _mm256_set1_epi8(needle[0]);
	const __m256i last = _mm256_set1_epi8(needle[k - 1]);

	const char* needlep1= needle + 1;
	const char *skm1 = s + k - 1;
	size_t km2 = k - 2;
	for (size_t i = 0; i < n; i += 32) 
	{
		const __m256i block_first = _mm256_loadu_si256((__m256i*)(s));
		const __m256i block_last = _mm256_loadu_si256((__m256i*)(skm1));

		const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
		const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

		uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

		unsigned long bitpos;
	CHECK_STR_MATCH:
#ifdef WINDOWS_BUILD
		unsigned char bitScanRes;
		bitScanRes = _BitScanForward(&bitpos, mask);
		if (bitScanRes)
		{
			if (memcmp(s + bitpos + 1, needlep1, km2) == 0)
#else
		bitpos = __builtin_ffs(mask);
		if(bitpos>0)
		{
			if (memcmp(s + bitpos + 0, needlep1, km2) == 0)
#endif
			{
				if (i + bitpos >= n) // check if we are reading outside the boundary
				{
					return 0;
				}
				return 1;
			}
			mask = clear_leftmost_set(mask);
			goto CHECK_STR_MATCH;
		}
		s++;
		skm1++;
	}

	return 0;
}
#else
size_t strstr_AVX2(const char* s, size_t n, const char* needle, size_t k)
{
	const __m256i first = _mm256_set1_epi8(needle[0]);
	const __m256i last = _mm256_set1_epi8(needle[k - 1]);

	const char* needlep1 = needle + 1;
	const char* skm1 = s + k - 1;
	size_t km2 = k - 2;
	for (size_t i = 0; i < n; i += 32)
	{
		const __m256i block_first = _mm256_loadu_si256((__m256i*)(s));
		const __m256i block_last = _mm256_loadu_si256((__m256i*)(skm1));

		const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
		const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

		uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

#define CHECK_AT_POS(x) if ((mask & (1 << x)) && memcmp(s + x + 1, needlep1, km2) == 0) { if (i + x >= n) return 0; else return 1; }
		if (mask)
		{
			if (mask & 0x0000FFFF)
			{
				if (mask & 0x000000FF)
				{
					CHECK_AT_POS(0);
					CHECK_AT_POS(1);
					CHECK_AT_POS(2);
					CHECK_AT_POS(3);
					CHECK_AT_POS(4);
					CHECK_AT_POS(5);
					CHECK_AT_POS(6);
					CHECK_AT_POS(7);
				}
				if (mask & 0x0000FF00)
				{
					CHECK_AT_POS(8);
					CHECK_AT_POS(9);
					CHECK_AT_POS(10);
					CHECK_AT_POS(11);
					CHECK_AT_POS(12);
					CHECK_AT_POS(13);
					CHECK_AT_POS(14);
					CHECK_AT_POS(15);
				}
			}
			if (mask & 0xFFFF0000)
			{
				if (mask & 0x00FF0000)
				{
					CHECK_AT_POS(16);
					CHECK_AT_POS(17);
					CHECK_AT_POS(18);
					CHECK_AT_POS(19);
					CHECK_AT_POS(20);
					CHECK_AT_POS(21);
					CHECK_AT_POS(22);
					CHECK_AT_POS(23);
				}
				if (mask & 0xFF000000)
				{
					CHECK_AT_POS(24);
					CHECK_AT_POS(25);
					CHECK_AT_POS(26);
					CHECK_AT_POS(27);
					CHECK_AT_POS(28);
					CHECK_AT_POS(29);
					CHECK_AT_POS(30);
					CHECK_AT_POS(31);
				}
			}
		}
		s++;
		skm1++;
	}
	return 0;
}
#endif

_noinline_ void Run_strstr_AVX2_test()
{
	printf("Starting test %s ...", __FUNCTION__);
	StartTimer();
	size_t searchesMade = 0;
	size_t foundCount = 0; // anti optimization
	for (size_t searchedIndex = 0; searchedIndex < uiSearchedStrCount; searchedIndex++)
	{
		for (size_t inputIndex = 0; inputIndex < uiInputStrCount; inputIndex++)
		{
			size_t testRes = strstr_AVX2(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);

#ifdef _DEBUG
			size_t debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
			if (testRes != debugstrstrRes)
			{
				testRes = strstr_AVX2(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
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
