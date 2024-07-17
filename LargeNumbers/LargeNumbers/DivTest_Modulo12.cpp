/*
Hope to never recheck again :
	- simplified version of DivTestMod11
	- (mul * a + m1) * B = N
	- as soon as a is greater than the next prime, we will move it into mul
	- this is applied aristotele sieve

Conclusion :
	- not a personalized method for N. This is a generator for ANY number. Probably not the best approach ?
	- POS : the modulo list can be precalculated for any multiplier
	- POS : testing mods is faster than testing all the dividers
	- NEG : the amount of mods needed to be stored gets impractical quickly. 24 bits ?
	- NEG : favored only when A and B are not close to each other

	-> number of tries reduced from X to Y . Ex : instead of 223.092.870 ,do 36.495.360 divisions
	92169/(2*3*5*7*11*13*17)
	92169/510510=0.1805429864253393665158371040724
	1658880/9699690=0.17102402241721127170043578712309
	36495360/223092870=0.16358819535559339032215597029165
*/

#include "StdAfx.h"
#include <vector>
#include <Windows.h>

// list of primes we are allowed to use when generating a multiplier
const __int64 g_Primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 1 };
// mods do not depend on N or A or B
// array of vectors. mods from previous multiplier to current multiplier
std::vector<__int64> g_mods[sizeof(g_Primes)];

struct StateHolderM12
{
	size_t SelectedPrimesUntil_a;
//	size_t SelectedPrimesUntil_b;
	__int64 mul_a;
//	__int64 mul_b;
//	std::vector<ModuloPair> m;
	std::vector<__int64> *mods_a;
};

static size_t HasSharedDivisor(const __int64 num, const size_t primeIndex)
{
	// make sure there is no shared multiplier
	for (size_t ind = 0; ind <= primeIndex; ind++)
	{
		if ((num % g_Primes[ind]) == 0)
		{
			return 1;
		}
	}
	return 0;
}

static void GenIntervalMods(__int64 PrimeFactorsUntil)
{
	__int64 nStart = 1;
	__int64 nEnd = 1;
	// get interval start-end
	for (size_t ind = 0; ind <= (size_t)PrimeFactorsUntil; ind++)
	{
//		nStart = nEnd; // if we have mods only for a specific interval, than we need to update older lists later
		nEnd *= g_Primes[ind];
	}
	// special case
	if (nStart % 2 == 0)
	{
		nStart += 1;
	}
	// gen mods between this interval
	for (__int64 modNow = nStart; modNow < nEnd; modNow += 2)
	{
		// make sure there is no shared multiplier
		if (HasSharedDivisor(modNow, PrimeFactorsUntil))
		{
			continue;
		}
		// looks like a unique mod that can generate ANY number
		g_mods[PrimeFactorsUntil].push_back(modNow);
	}
}

__int64 GetCoveringBitMask(__int64 nNum)
{
	if (nNum == 0)
	{
		return 0;
	}
	__int64 ret = 1;
	while (ret < nNum)
	{
		ret = (ret << 1) | 1;
	}
	return ret;
}

static size_t CanModGenN(__int64 mul, __int64 mod, __int64 N)
{
#ifdef USE_BINARY_MASK
	__int64 digitEnd = GetCoveringBitMask(mul);
	__int64 NeedsMatchingDigits = N & digitEnd;
	for (__int64 digit = 1; digit <= digitEnd; digit++)
	{
		__int64 NLastDigits = mul * digit + mod;
		if ((NLastDigits & digitEnd) == NeedsMatchingDigits)
		{
			return 1;
		}
	}
#else
	__int64 MatchMask;
	if (mul >= 100000) MatchMask = 100000;
	else if (mul >= 10000) MatchMask = 10000;
	else if (mul >= 1000) MatchMask = 1000;
	else if (mul >= 100) MatchMask = 100;
	else if (mul >= 10) MatchMask = 10;
	else return 1; // mul is less than 10
	__int64 NeedsMatchingDigits = N % MatchMask;
	for (__int64 digit = 1; digit <= mul; digit++)
	{
		__int64 A_LastDigits = mul * digit + mod;
		for (__int64 digitB = 1; digitB <= mul; digitB++)
		{
			__int64 B_LastDigits = digitB;
			__int64 NLastDigits = A_LastDigits * B_LastDigits;
			__int64 NLastDigitsMod = (NLastDigits % MatchMask);
			if (NLastDigitsMod == NeedsMatchingDigits)
			{
				return 1;
			}
		}
	}
#endif
	return 0;
}

void IncreaseMultiplierAndExtendMods(__int64 N, StateHolderM12& MulState)
{
	__int64 nextFactor = g_Primes[MulState.SelectedPrimesUntil_a];

	// right now we alsways jump by 1
	MulState.mul_a *= nextFactor;

	MulState.mods_a = &g_mods[MulState.SelectedPrimesUntil_a];

	// add new possible mods
#if 0
	for (size_t i = 0; i < g_mods[MulState.SelectedPrimesUntil_a].size(); i++)
	{
		__int64 modNow = g_mods[MulState.SelectedPrimesUntil_a][i];
#if defined(NOT_YET_A_BELEAVER) || 0
		if (CanModGenN(MulState.mul_a, modNow, N) == 0)
		{
			continue;
		}
#endif
		MulState.m.push_back(ModuloPair{ modNow,0 });
	}
#endif

	// increase the limit for the next loop
	MulState.SelectedPrimesUntil_a++;
}

void DivTestModulo12_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN;
	__int64 b = SQN;
	__int64 m = N - a * b;

	printf("================\n");
	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 x = a - A;
	__int64 y = B - b - x;
	printf("Looking for x=%lld y=%lld\n", x, y);

	StateHolderM12 MulState;
	MulState.mul_a = 1;
	MulState.SelectedPrimesUntil_a = 0;

	size_t ChecksMade = 0;

	// could make a quick check for small numbers. This is a very small optimization
	// if N is not divisible by the primes than we can directly increase our multiplier until that prime
	// Ex : N not divisible by 31, than the multiplier could become directly 30
	for (size_t i = 0; g_Primes[i] != 1; i++)
	{
		ChecksMade++;
		if ((N % g_Primes[i]) == 0)
		{
			printf("Found a solution as small prime %lld\n", g_Primes[i]);
			return;
		}
	}
	// quick forward our multiplier to a number we already tested
	__int64 lastTestedPrime = g_Primes[_countof(g_Primes) - 2];
	__int64 multiplierNow = g_Primes[0];
	for (size_t i = 1; i< _countof(g_Primes) && (multiplierNow < lastTestedPrime); i++)
	{
		multiplierNow = multiplierNow * g_Primes[i];
		IncreaseMultiplierAndExtendMods(N, MulState);
	}

	// now keep searching for a solution with steps almost 1 by 1
	size_t startTick = GetTickCount64();
	while (1)
	{
#ifdef _DEBUG
		__int64 a_solution = A / MulState.mul_a;
		__int64 m1_solution = A % MulState.mul_a;
		size_t solWouldBeFound = 0;
		for (size_t ind = 0; ind < MulState.mods_a->size(); ind++)
		{
			if (MulState.mods_a->at(ind) == m1_solution)
			{
				solWouldBeFound = 1;
				break;
			}
		}
		assert(solWouldBeFound == 1);
#endif

		const __int64 Check_a_Until = g_Primes[MulState.SelectedPrimesUntil_a];
		const __int64 RoundedSQN = (SQN / MulState.mul_a * MulState.mul_a);

		printf("\t mul now %lld. Mods count %lld. Will do %lld checks\n", MulState.mul_a, MulState.mods_a->size(), Check_a_Until * MulState.mods_a->size());

		for (__int64 a = 1; a <= Check_a_Until; a++)
		{
			for (size_t mi = 0; mi < MulState.mods_a->size(); mi++)
			{
				ChecksMade++;
				// this is increasing from 0 to SQN
				__int64 ANow = (MulState.mul_a * a + MulState.mods_a->at(mi));
				__int64 NNow = N % ANow;
				if (NNow == 0)
				{
					printf("1 Found a res. A = %lld in %llu steps. In %lld ms\n", ANow, ChecksMade, GetTickCount64() - startTick);
					return;
				}/**/
				// for lols, this is going downwards from SQN to 0
				// you probably want to use the other formula for this : x^2 + m = y * ( SQN - X )
				__int64 ANow2 = RoundedSQN - MulState.mul_a * a + MulState.mods_a->at(mi);
				__int64 NNow2 = N % ANow2;
				if (NNow2 == 0)
				{
					printf("2 Found a res. A = %lld in %llu steps. In %lld ms\n", ANow2, ChecksMade, GetTickCount64() - startTick);
					return;
				}
			}
		}
		// maybe some other time
		if (g_mods[MulState.SelectedPrimesUntil_a].size() == 0)
		{
			printf("We did not plan for such large numbers. Maybe generate more mods. Checks made %lld \n", ChecksMade);
			return;
		}

		// increase our multiplier, because the higher the multiplier, the less mods we will have
		IncreaseMultiplierAndExtendMods(N, MulState);
	}
}

void DivTestModulo12()
{
	// mods can be global
#ifdef _DEBUG
	for (size_t i = 0; (i < 8) && (i < _countof(g_Primes)) && (g_Primes[i] != 1); i++)
#else
	for (size_t i = 0; (i < 9) && (i < _countof(g_Primes)) && (g_Primes[i] != 1); i++)
#endif
	{
		GenIntervalMods(i);
	}

	DivTestModulo12_(5, 7);
	DivTestModulo12_(23, 41);
	DivTestModulo12_(349, 751); // N = 262099 , SN = 511
	DivTestModulo12_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestModulo12_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestModulo12_(784727, 918839); // N = 721037771953 , SN = 849139
	DivTestModulo12_(127966049, 238311263);
	DivTestModulo12_(297311557, 1055374679);
	DivTestModulo12_(26729, 918839);
	DivTestModulo12_(6871, 918839);
	DivTestModulo12_(3, 918839);
	DivTestModulo12_(349, 918839);
}