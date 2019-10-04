#include "StdAfx.h"

/*
if we manage to find a number that we can multiply N with, so that the number of 0 will increase, than the non zero numbers from the start should represent A or B at some point
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
				Mul.Len = SelectedIndex;
				for (int Digit = 0; Digit < 10; Digit++)
				{
					Mul.Digits[SelectedIndex] = Digit;
					MulLN(lnN, Mul, NewN);
					//check if this digit is usable
					// - ignore the last (SelectedIndex + 1) digits, these can be anything and we do not really care about them
					// - make sure we have 0 digits generated, these should be increasing in numbers
					if (NewN.Digits[SelectedIndex * 2] == 0)
						break;
				}
				SelectedIndex++;
			}
		}
	}
};

void DivTestInvMul1(__int64 iA, __int64 iB)
{
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