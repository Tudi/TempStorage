/*
Hope to never recheck again :
	N=A*B  if we checked primes as non good candidates
	(m1+a*2*3*5*..)*(m2+b*2*3*5...)
	we know m1 and m2 will not be divisable by either of the primes
	N=m1*m2+m1*b*SCALER+m2*a*SCALER+a*b*SCALER*SCALER
	N=m1*m2+SCALER*(m1*b+m2*a+a*b*SCALER)
	(N-m1*m2)/SCALER=m1*b+m2*a+a*b*SCALER

Conclusion :
	No miracle
*/
#include "StdAfx.h"

void DivTestMod10_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN;
	__int64 b = N / a;
	__int64 m = N - a * b;

	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);

	const __int64 SCALER_FACTORS[] = {2,3,5,7,11,1};
	__int64 SCALER = 1;
	for (__int64 i = 0; SCALER_FACTORS[i] != 1; i++)
	{
		SCALER *= SCALER_FACTORS[i];
	}
	printf("Will use scaler %lld\n", SCALER);

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
			printf("%lld)Nomirror %lld. Possible m1=%lld m2=%lld\n", 
				m1m2CombosFound, m1m2CombosFound- m1m2CombosMirroredFound, m1, m2);
			m1m2CombosFound++;
		}
	}

	printf("Would reduce number of tries to %f if could use scaler %lld\n", 
		(float)(m1m2CombosFound - m1m2CombosMirroredFound)/(float)SCALER, SCALER);
}

void DivTestMod10()
{
	//DivTestMod10_(5, 7);
	//DivTestMod10_(23, 41);
	DivTestMod10_(349, 751); // N = 262099 , SN = 511
	DivTestMod10_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestMod10_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestMod10_(784727, 918839);
	DivTestMod10_(3, 918839);
	DivTestMod10_(349, 918839);
}