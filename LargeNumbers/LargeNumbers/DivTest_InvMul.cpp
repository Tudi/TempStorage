#include "StdAfx.h"

/*
if we manage to find a number that we can multiply N with, so that the number of 0 will increase, than the non zero numbers from the start should represent A or B at some point
N * 0.x = y.000001 where len N > len y, number of 0 in y should be larger than len N
ex : 943 * 0.5
*/

namespace DivTestInvMul_1 {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 SN;
	__int64 Mask;


	void DivTestInvMul(__int64 iA, __int64 iB)
	{
		LargeNumber NewN;
		LargeNumber Mul;
		LargeNumber lnN;

		InitLN(NewN);
		InitLN(Mul);
		SetLN(lnN, N);

		for (int FirstDigit = 1; FirstDigit < 10; FirstDigit++)
		{
			Mul.Digits[0] = FirstDigit;
			int SelectedIndex = 1;
			while (SelectedIndex < lnN.Len)
			{
				Mul.Len = SelectedIndex+1;
				for (int Digit = 0; Digit < 10; Digit++)
				{
					Mul.Digits[SelectedIndex] = Digit;
					MulLN(lnN, Mul, NewN);
					// check if this digit is usable
					// - ignore the last (SelectedIndex + 1) digits, these can be anything and we do not really care about them
					// - make sure we have 0 digits generated, these should be increasing in numbers
//					if (NewN.Digits[SelectedIndex * 2] == 0)
					if (NewN.Digits[SelectedIndex] == 0)
						break;
				}
				//could not generate proper multiplier
				if (NewN.Digits[SelectedIndex] != 0)
					break;
				SelectedIndex++;
			}
		}
	}
};

void DivTestInvMul1(__int64 iA, __int64 iB)
{
/*	int N = 23 * 41;
	// 943 = 2 * 471.5 = 2 * 11.5 * 41 = 2 * 23 * 20.5
	// 943 = 3 * 314.(3)4 = 3 * 7.(6)7 * 41 = 3 * 23 * 13.(6)7
	// 943 = 5 * 188.6 = 5 * 4.6 * 41 = 5 * 23 * 8.2
	// 943 = 7 * 134.(714)
	// 943 = 11 * 85.(72)73 = 11 * 2.(09) * 41 = 11 * 23 * 3.(72)
	for (int t = 1; t < N * 10; t++)
	{
		int N2 = N * t;
		int m1 = N2 % 10; // we ignore this one
		int m2 = (N2 / 10) % 1000;
		int newN = N2 / 10000;
		if (m2 == 0)
			printf("%d * 0.%d = %d\n", N, t, newN);
	}/**/

/*	int N = 23 * 41;
	for (int t = 1; t < N * N; t++)
	{
		if (t % 10 == 0)
			continue;
		int N2 = N * t;
		if (N2 % 10 == 0)
			printf("%d * 0.%d = %d\n", N, t, N2);
	}/**/
	int a1 = 30, b1 = 31, m1 = 13;
	int a2 = 31, b2 = 31, m2 = 18;
	for(int x1=0;x1<10;x1++)
		for (int y1 = 0; y1 < 10; y1++)
//			for (int x2 = 0; x2 < 10; x2++)
//				for (int y2 = 0; y2 < 10; y2++)
				{
					int t1 = b1 * x1 + m1;
					int t2 = y1 * (a1 - x1);
					if (t1 % 10 != t2 % 10)
						continue;

					int y2 = y1;
					int x2 = x1 + 1;
					int t3 = b2 * x2;
					int t4 = y2 * (a2 - x2) + m2;
					if (t3 % 10 != t4 % 10)
						continue;

					printf("%d %d => %d = %d         %d %d => %d = %d\n", x1, y1, t1, t2, x2, y2, t3, t4);
				}

	DivTestInvMul_1::A = iA;
	DivTestInvMul_1::B = iB;
	DivTestInvMul_1::N = iA * iB;
	DivTestInvMul_1::SN = isqrt(DivTestInvMul_1::N);
	DivTestInvMul_1::DivTestInvMul(iA, iB);
}

void DivTestInvMul1()
{
	//	DivTestInvMul1(23, 41);
	DivTestInvMul1(349, 751); // N = 262099 SN = 511
//	DivTestInvMul1(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestInvMul1(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestInvMul1(784727, 918839);
//	DivTestInvMul1(6117633, 7219973);
//	DivTestInvMul1(26729, 61781);
//	DivTestInvMul1(11789, 61781);
}