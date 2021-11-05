#include <string.h>
#include <stdio.h>
#ifdef WINDOWS_BUILD
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif
#include "InputGenerator.h"
#include "ProfileTimer.h"

size_t strstr_AVX2_V2_len2(const char* s, size_t n, const char* needle, size_t k)
{
	const __m256i first = _mm256_set1_epi16(*(unsigned short*)needle);
	for (size_t i = 0; i < n; i += 32)
	{
		__m256i block = _mm256_loadu_si256((__m256i*)(s+0));
		__m256i eq_block = _mm256_cmpeq_epi16(first, block);
		size_t bits_non_zero = _mm256_movemask_epi8(eq_block);
		if (bits_non_zero != 0)
		{
			return 1;
		}
		block = _mm256_loadu_si256((__m256i*)(s + 1));
		eq_block = _mm256_cmpeq_epi16(first, block);
		bits_non_zero = _mm256_movemask_epi8(eq_block);
		if (bits_non_zero != 0)
		{
			return 1;
		}
		s+=32;
	}
	return 0;
}

size_t strstr_AVX2_V2_len4(const char* s, size_t n, const char* needle, size_t k)
{
	const __m256i first = _mm256_set1_epi32(*(int*)needle);
	for (size_t i = 0; i < n; i += 32)
	{
		__m256i block = _mm256_loadu_si256((__m256i*)(s + 0));
		__m256i eq_block = _mm256_cmpeq_epi32(first, block);
		size_t bits_non_zero = _mm256_movemask_epi8(eq_block);
		if (bits_non_zero != 0)
		{
			return 1;
		}
		block = _mm256_loadu_si256((__m256i*)(s + 1));
		eq_block = _mm256_cmpeq_epi32(first, block);
		bits_non_zero = _mm256_movemask_epi8(eq_block);
		if (bits_non_zero != 0)
		{
			return 1;
		}
		block = _mm256_loadu_si256((__m256i*)(s + 2));
		eq_block = _mm256_cmpeq_epi32(first, block);
		bits_non_zero = _mm256_movemask_epi8(eq_block);
		if (bits_non_zero != 0)
		{
			return 1;
		}
		block = _mm256_loadu_si256((__m256i*)(s + 3));
		eq_block = _mm256_cmpeq_epi32(first, block);
		bits_non_zero = _mm256_movemask_epi8(eq_block);
		if (bits_non_zero != 0)
		{
			return 1;
		}
		s+=32;
	}
	return 0;
}

size_t strstr_AVX2_V2_len8(const char* s, size_t n, const char* needle, size_t k)
{
	__m256i first = _mm256_set1_epi64x(*(size_t*)needle);
	for (size_t i = 0; i < n; i += 32)
	{
		for (size_t j = 0; j < 8; j++)
		{
			__m256i block = _mm256_loadu_si256((__m256i*)(s + j));
			__m256i eq_block = _mm256_cmpeq_epi64(first, block);
			size_t bits_non_zero = _mm256_movemask_epi8(eq_block);
			if (bits_non_zero != 0)
			{
				return 1;
			}
		}
		s += 32;
	}
	return 0;
}
/*
size_t strstr_AVX2_V2_LenGreater4(const char* s, size_t n, const char* needle, size_t k)
{
	const __m256i first = _mm256_set1_epi32(*(int*)needle);

	const char* needle_p_5 = needle + 4;
	size_t km2 = k - 4;
	for (size_t i = 0; i < n; i += 32)
	{
		for (size_t j = 0; j < 4; j++)
		{
			const __m256i block = _mm256_loadu_si256((__m256i*)(s + j));
			const __m256i eq_block = _mm256_cmpeq_epi32(first, block);
			size_t bits_non_zero = _mm256_movemask_epi8(eq_block);
			for (size_t k = 0; k < 8; k++)
			{
				if (bits_non_zero & (1<<(k*4)))
				{
					if (memcmp(s + j + k * 4 + 4, needle_p_5, km2) == 0)
					{
						if (i + j + k * 4 >= n) // check if we are reading outside the boundary
						{
							return 0;
						}
						return 1;
					}
				}
			}
		}
		s += 32;
	}
	return 0;
}*/

size_t strstr_AVX_v2(char const* tgt, size_t tgt_len, char const* pat, const size_t pat_len)
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
		case  2: return strstr_AVX2_V2_len2(tgt, tgt_len, pat, 2);
		case  3: return strstr(tgt, pat) != NULL;
		case  4: return strstr_AVX2_V2_len4(tgt, tgt_len, pat, 4);
		case  5: return strstr(tgt, pat) != NULL;
		case  6: return strstr(tgt, pat) != NULL;
		case  7: return strstr(tgt, pat) != NULL;
		case  8: return strstr_AVX2_V2_len8(tgt, tgt_len, pat, 8);
		default: return strstr(tgt, pat) != NULL;
	}
}

_noinline_ void Run_strstr_AVX2_V2_test()
{
	printf("Starting test %s ...", __FUNCTION__);
	StartTimer();
	size_t searchesMade = 0;
	size_t foundCount = 0; // anti optimization
	for (size_t searchedIndex = 0; searchedIndex < uiSearchedStrCount; searchedIndex++)
	{
		for (size_t inputIndex = 0; inputIndex < uiInputStrCount; inputIndex++)
		{
			size_t testRes = strstr_AVX_v2(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);

#ifdef _DEBUG
			size_t debugstrstrRes = strstr(sInputStrings[inputIndex].str, sSearchedStrings[searchedIndex].str) != NULL;
			if (testRes != debugstrstrRes)
			{
				testRes = strstr_AVX_v2(sInputStrings[inputIndex].str, sInputStrings[inputIndex].len, sSearchedStrings[searchedIndex].str, sSearchedStrings[searchedIndex].len);
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
