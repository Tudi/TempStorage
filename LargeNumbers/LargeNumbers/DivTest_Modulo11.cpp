/*
Hope to never recheck again :
	- based on modulo10.cpp : we can extract a scaler that represents Aristoteles sieve =  we will check less values
	N=A*B  if we checked primes as non good candidates
	N=(a-x)*(b+x+y)
	x*x+(b-a)*x+m=y*(a-x)
	after we apply SCALER =  we adjust a based on m1 and we adjust b based on m2
	N=(a*SCALER+m1-x)*(b*SCALER+m2+x+y)
	x=a/SCALER-A/SCALER=(a-A)/SCALER
	y=B/SCALER-x/SCALER-b/SCALER=(B-b-x)/SCALER

	(m1+a*2*3*5*..)*(m2+b*2*3*5...)
	we know m1 and m2 will not be divisable by either of the primes
	N=m1*m2+m1*b*SCALER+m2*a*SCALER+a*b*SCALER*SCALER
	N=m1*m2+SCALER*(m1*b+m2*a+a*b*SCALER)
	(N-m1*m2)/SCALER=m1*b+m2*a+a*b*SCALER

Conclusion :
	using a scaler will reduce the number of tests because it skips testing the values where m1*m2 is dividable by scaler factors
	for a large enough scaler, it has the chance to reduce number of tests to SQN/5 ( or more )
	explanation : 
		if scaler=2, you need to only test if A in (3,5,7,9...)
		if scaler=3, you need to only test if A in (2,4,5,7,8,10..)
		if scaler=6, you need to only test if A in (5,7,11,13..)
*/

#include "StdAfx.h"
#include <vector>

struct MudoluPair {
	__int64 m1, m2;
};
void GenPossible_m_ForScaler(const __int64 N, const __int64 *SCALER_FACTORS, std::vector<MudoluPair> &out_pairs)
{
	__int64 SCALER = 1;
	for (__int64 i = 0; SCALER_FACTORS[i] != 1; i++)
	{
		SCALER *= SCALER_FACTORS[i];
	}
//	printf("Will use scaler %lld\n", SCALER);

	__int64 m1m2CombosFound = 1;
	__int64 m1m2CombosMirroredFound = 0;
	for (__int64 m1 = 1; m1 < SCALER; m1++)
	{
		// make sure m is not dividable by scaler factors
		__int64 isDivisor = 0;
		for (__int64 i = 0; SCALER_FACTORS[i] != 1; i++)
		{
			if ((m1 % SCALER_FACTORS[i]) == 0)
			{
				isDivisor = 1;
				break;
			}
		}
		if (isDivisor)
		{
			continue;
		}
		for (__int64 m2 = 1; m2 < SCALER; m2++)
		{
			// make sure m is not dividable by scaler factors
			__int64 isDivisor = 0;
			for (__int64 i = 0; SCALER_FACTORS[i] != 1; i++)
			{
				if ((m2 % SCALER_FACTORS[i]) == 0)
				{
					isDivisor = 1;
					break;
				}
			}
			if (isDivisor)
			{
				continue;
			}
			if ((N - m1 * m2) % SCALER != 0)
			{
				//				printf("Found m1 m2 to be fake\n");
				continue;
			}
			if (m2 <= m1)
			{
				m1m2CombosMirroredFound++;
			}
	//		printf("%lld)Nomirror %lld. Possible m1=%lld m2=%lld\n",
//				m1m2CombosFound, m1m2CombosFound - m1m2CombosMirroredFound, m1, m2);
			out_pairs.push_back(MudoluPair{ m1,m2 });
			m1m2CombosFound++;
		}
	}
//	printf("Would reduce number of tries to %f if could use scaler %lld\n",
//		(float)(m1m2CombosFound - m1m2CombosMirroredFound) / (float)SCALER, SCALER);
}

void GetPrimesUntil(__int64 upperLimit, std::vector<__int64>& out_primes)
{
	if (2 < upperLimit)
	{
		out_primes.push_back(2);
	}
	for (__int64 now = 3; now < upperLimit; now++)
	{
		if (IsPrime(now))
		{
			out_primes.push_back(now);
		}
	}
}

void DivTestMod11_(__int64 A, __int64 B)
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
	printf("Unscaled x=%lld y=%lld\n", x, y);
	printf("Estimated loops using normal aproach %lld\n", y / 2);

	// prepare our scaler. In ideal conditions we would do this step by step
	const __int64 PRIMES_TO_PICK_FROM[] = { 2,3,5,7,11,13,17,19,23,29, 1 };
	__int64 SCALER_FACTORS[30];
	__int64 SCALER = 1;
	for (size_t i = 0; PRIMES_TO_PICK_FROM[i] != 1; i++)
	{
		__int64 SCALER_NEXT = SCALER * PRIMES_TO_PICK_FROM[i];
		if (SCALER_NEXT > SQN / 3) {
			break;
		}
		// in real life, we would be increasing scaler based on y tested so far
		// if we scale down y to 0, our formula breaks
		if (SCALER_NEXT > y) {
			break;
		}
		// in real life we would be using huge numbers
		if (SCALER_NEXT > A) {
			break;
		}
		SCALER_FACTORS[i] = PRIMES_TO_PICK_FROM[i];
		SCALER_FACTORS[i + 1] = 1;
		SCALER = SCALER_NEXT;
	}
	printf("Will use scaler %lld\n", SCALER);

	// need to make sure that no prime until scaler is an actual solution
	std::vector<__int64> mprimes;
	GetPrimesUntil(SCALER, mprimes);
	for (__int64 i = 0; i < mprimes.size(); i++)
	{
		if (N % mprimes[i] == 0)
		{
			printf("Prime %lld is actually A. Can't use scaler. Exiting\n", mprimes[i]);
			return;
		}
	}

	// get all the m1,m2 combinations when we can use this scaler
	std::vector<MudoluPair> mpairs;
	GenPossible_m_ForScaler(N, SCALER_FACTORS, mpairs);

	// check if unscaled y is a solution
	for (__int64 y = 1; y < SCALER; y++)
	{
		__int64 eq_b = (b - a + y);
		__int64 eq_delta = eq_b * eq_b - 4 * (m - a * y);
		__int64 xnow = (sqrt(eq_delta) - eq_b) / 2;
		__int64 tA = a - xnow;
		__int64 tB = b + xnow + y;
		if (tA * tB == N)
		{
			printf("Found the solution to y before using scaler. y=%lld\n", y);
			return;
		}
	}

	// now loop until we find x or y
	__int64 xnow = 0, ynow = 1;
	__int64 iterationsMade = 1;
	while (1)
	{
		__int64 foundTheProperm1m2Combo = 0;

		for (MudoluPair& mpair : mpairs)
		{
			// for the sake of debugging
			if ((A - mpair.m1) % SCALER == 0 && (B - mpair.m2) % SCALER == 0)
			{
				foundTheProperm1m2Combo = foundTheProperm1m2Combo;
			}

			// (A - m1) % scaler = 0
			__int64 ta = (a / SCALER + 0) * SCALER + mpair.m1; // round down and add modulo
			__int64 tb = (b / SCALER - 1) * SCALER + mpair.m2; // round down and add modulo
			__int64 tm = N - ta * tb;
			
			// from all the options, one should be good. Just 1
			__int64 xgood = ta - A;
			__int64 ygood = B - tb - xgood;
			__int64 xgoodScaled = xgood / SCALER; 
			__int64 ygoodScaled = ygood / SCALER; // the actual value we are searching for
			if ((xgood % SCALER == 0) && (ygood % SCALER == 0))
			{
				foundTheProperm1m2Combo = 1;
			}

			//x*x+(b-a)*x+m=y*(a-x)
			//x^2+(b-a+y)*x+m-a*y
			// 
			// keeping the full version just in case wondering in the future
			//SACLER^2*x^2+(b-a+y*SCALER)*SCALER*x+m-a*y*SCALER
//			__int64 eq_b = (tb - ta + ynow * SCALER)*SCALER;
//			__int64 eq_delta = eq_b * eq_b - 4 * SCALER * SCALER * (tm - ta * ynow * SCALER);
//			xnow = (sqrt(eq_delta) - eq_b) / (SCALER * 2);
// 
			// simplified with scaler
			__int64 eq_b = (tb - ta + ynow * SCALER);
			__int64 eq_delta = eq_b * eq_b - 4 * (tm - ta * ynow * SCALER);
			xnow = (sqrt(eq_delta) - eq_b) / 2;
			__int64 tA = ta - xnow;
			__int64 tB = tb + xnow + ynow * SCALER;
			if (tA * tB == N)
			{
				printf("Found the solution after %lld loops\n", iterationsMade);
				return;
			}
			iterationsMade++;
		}

		assert(foundTheProperm1m2Combo == 1);
		ynow++;
	}
}

void DivTestMod11()
{
	// This is only partial implementation. Todo :
	// 1) pick scaler only based on already tested y values : scaler=1, scaler=2, scaler=2*3 ...
	// 2) at some point need to switch to calculate y from x (instead x from y)

	//DivTestMod11_(5, 7);
	//DivTestMod11_(23, 41);
	DivTestMod11_(349, 751); // N = 262099 , SN = 511
	DivTestMod11_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestMod11_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestMod11_(784727, 918839); // N = 721037771953 , SN = 849139
	DivTestMod11_(26729, 918839); 
	DivTestMod11_(6871, 918839);
//	DivTestMod11_(3, 918839);
	DivTestMod11_(349, 918839);
}