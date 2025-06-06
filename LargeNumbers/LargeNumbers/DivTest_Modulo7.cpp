#include "StdAfx.h"

/*
anything special in using chained filters ? Maybe there is a combo that will have special hit count ?
*/

namespace ReminderTest7 {
	__int64 A;
	__int64 B;
	__int64 N;

	int DivTestModuloFilters_S1_9(__int64 N, __int64 SN, __int64 iA, __int64 iB, __int64 &out_iA, __int64 &out_iB)
	{
		for (__int64 y = 0; y < 10; y++)
		{
			for (__int64 x = 0; x < 10; x++)
			{
				__int64 tN = (SN - x) * (SN + x + y);
				if (tN == N)
				{
					out_iA = SN - x;
					out_iB = SN + x + y;
					return 1;
				}
			}
		}

		// y > 9
		__int64 tx = ( 2 * isqrt((SN + 5) * (SN + 4) - N) - 9 ) / 2;
		out_iA = SN - tx;
		out_iB = N / out_iA;

		return 0;
	}

	int DivTestModuloFilters_SX_9(__int64 N, __int64 SN, __int64 iB, __int64& out_iA, __int64& out_iB)
	{
		for (__int64 y = 0; y < 10; y++)
		{
			for (__int64 x = 0; x < 10; x++)
			{
				for (__int64 z = 0; z < 10; z++)
				{
					__int64 tN = (SN - x) * ((SN + x + y) * iB + z);
					// maybe solution ?
					if (tN < N && 
						(tN % 10) == (N % 10))
					{
						__int64 tz = ((x - N) * (SN * SN + SN * y + x * x) - N) / ((x - N) * (x - N));
						if (tz >= 0 && tz < iB)
						{
							tN = (SN - x) * ((SN + x + y) * iB + tz);
							if (tN == N)
							{
								out_iA = SN - x;
								out_iB = SN + x + y;
								return 1;
							}
						}
					}
				}
			}
		}

		// y > 9
		__int64 a = -iB;
		__int64 b = -9 * iB - 1;
		__int64 c = SN * SN * iB + 9 * SN * iB + SN - N;
		__int64 tx1 = (-b - isqrt(b * b - 4 * a * c)) / (2 * a);
//		__int64 tx2 = (-b + isqrt(b * b - 4 * a * c)) / (2 * a);
		out_iA = SN - tx1;
//		out_iA = out_iA / iB;
		out_iB = N / out_iA;

		return 0;
	}

	void DivTestModuloFilters(__int64 iA, __int64 iB)
	{
		__int64 SN = isqrt(N);

		__int64 tA1, tB1;
		if (DivTestModuloFilters_S1_9(N, SN, iA, iB, tA1, tB1))
		{
			printf("Unexpected early exit %lld * %lld\n", tA1, tB1);
			return;
		}

		while (1)
		{
			__int64 N2 = N * tA1;
			__int64 SN2 = isqrt(N2 / tB1);
			__int64 searchedX = SN2 - A;
			__int64 searchedY = (A * tB1 + B * tA1 - 2 * SN2 * tB1) / tB1;
			DivTestModuloFilters_SX_9(N2, SN2, tB1, tA1, tB1);
			tB1 = N / tA1;
		}

		A = A;
	}
};

void DivTestModulo7(__int64 iA, __int64 iB)
{
	ReminderTest7::A = iA;
	ReminderTest7::B = iB;
	ReminderTest7::N = iA * iB;
	ReminderTest7::DivTestModuloFilters(iA, iB);
}

void DivTestModulo7()
{
//	DivTestModulo7(23, 41);
  DivTestModulo7(349, 751); // N = 262099 SN = 511
//	DivTestModulo7(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo7(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo7(784727, 918839);
//	DivTestModulo7(786983, 984689);
//	DivTestModulo7(6117633, 7219973);
//	DivTestModulo7(26729, 61781);
//	DivTestModulo7(11789, 61781);
}