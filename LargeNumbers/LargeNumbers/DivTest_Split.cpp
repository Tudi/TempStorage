#include "StdAfx.h"

/*
N = A * B = (d * a + ma) * ( d * b + mb ) = d * d * a * b + d * ( mb * a + ma * b ) + ma * mb
if d is larger than sqrt(N), it means a = 0 and ma = A ... we should not be able to generate a combo ?
*/
namespace SplitInHalf {
	LargeNumber N;
	LargeNumber Divider;
	LargeNumber NDivided;
#define	ma_i 0
#define mb_i 1
#define a_i 2
#define b_i 3

	int CheckCandidateMatch_SplitHalf(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		//truth test 1 is that		ma * mb == N mod Divider
		//truth test 2 is that		d * a * b + mb * a + ma * b == N / Divider

		int RealLengths[mb_i + 1];
		for (int nr = 0; nr < mb_i + 1; nr++)
		{
			RealLengths[nr] = 0;
			for (int tp = pos; tp >= 0; tp--)
				if (vLN[nr]->Digits[tp] != 0)
				{
					RealLengths[nr] = tp + 1;
					break;
				}
		}

		//we picked the divider so that this will be at least 1
		if (RealLengths[b_i] == 0)
			return 0;

		// d * a * b + mb * a + ma * b
		// d * a * b
//		if (RealLengths[a_i] + RealLengths[b_i] + Divider.Len > NDivided.Len + 2)
//			return 0;

		// mb * a
//		if (RealLengths[mb_i] + RealLengths[a_i] > NDivided.Len + 1)
//			return 0;

		// ma * b
//		if (RealLengths[ma_i] + RealLengths[b_i] > NDivided.Len + 1)
//			return 0;

		// this is the last resort. Should happen when a = 0
		// ma * mb
//		if (RealLengths[ma_i] + RealLengths[mb_i] > tN->Len + 1)
//			return 0;

		// should not get here
//		if (pos * 2 > tN->Len)
//			return 0;

		MulLN(vLN[ma_i], vLN[mb_i], TempRes1);
		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != tN->Digits[i])
				return 0;

		//can only use ma, mb after we reach the second half
		EatLeadingZeros(TempRes1);
		if (TempRes1->Len < Divider.Len)
			return 1;

		LargeNumber ab,dab,mba,mab,mbamab, dabmbamab, ddabmbamab;
		MulLN(vLN[a_i], vLN[b_i], &ab);
		MulLN(&Divider, &ab, &dab);
		MulLN(vLN[mb_i], vLN[a_i], &mba);
		MulLN(vLN[ma_i], vLN[b_i], &mab);
		AddLN(&mba, &mab, &mbamab);

		AddLN(&dab, &mbamab, &dabmbamab);
		MulLN(&dabmbamab, &Divider, &ddabmbamab);
		AddLN(&ddabmbamab, TempRes1, TempRes2);

		EatLeadingZeros(TempRes2);
		if (TempRes2->Len > NDivided.Len)
			return 0;

		for (int i = 0; i <= pos; i++)
			if (TempRes2->Digits[i] != N.Digits[i])
				return 0;

		return 1;
	}

	int CheckSolution2(LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		if (TempRes2->Len == N.Len)
		{
			return 1;
		}
		return 0;
	}

	void DivTest_SplitHalf(__int64 iA, __int64 iB)
	{
//return; // because this is no other than A * B, just with 4 values instead of 2
		__int64 iN = iA * iB;
		SetLN(N, iN);
		__int64 iSN = isqrt(iN);
		__int64 iDivider = GetMaskDecimal(iSN) / 10; // should be smaller than sqrt(N) to avoid a==b==0
		SetLN(Divider, iDivider);
		__int64 iNDivided = iN / iDivider;
		SetLN(NDivided, iNDivided);

		printf("Expecting solution a = %lld, b = %lld. ma = %lld mb = %lld. N=%lld. Bruteforce trycount %lld\n", iA / iDivider, iB / iDivider, iA % iDivider, iB % iDivider, iN, iSN/2);

		LargeNumber a, b, ma, mb;
		LargeNumber EndSignal;

#define ParamCount 5
		LargeNumber *vLN[ParamCount];
		vLN[a_i] = &a;
		vLN[b_i] = &b;
		vLN[ma_i] = &ma;
		vLN[mb_i] = &mb;
		vLN[4] = &EndSignal;

		for (int i = 0; i < ParamCount; i++)
			SetLN(vLN[i], (__int64)0);
		InitLN(vLN[ParamCount - 1]);

			//start generating combinations and check if it's a feasable candidate
		int AtPos = 0;
		int SolutionsFound = 0;
		int CandidatesFound = 0;
		int StepsTaken = 0;
		do
		{
			LargeNumber TempRes1, TempRes2;
			int GenNextCandidate = 0;
			StepsTaken++;
			int Match = CheckCandidateMatch_SplitHalf(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
			if (Match == 1)
			{
				CandidatesFound++;
				if (CandidatesFound % 100 == 0)
				{
					int chars[4] = { '\\','|','/','-' };
					printf("\r%c", chars[(CandidatesFound / 100) % 4]);
				}
				int SolutionFound = CheckSolution2(&TempRes1, &TempRes2);
				if (SolutionFound == 1)
				{
					SolutionsFound++;
					printf("\r%d / %d )sol : \t a:", SolutionsFound, CandidatesFound);
					PrintLN(a);
					printf("\t b:");
					PrintLN(b);
					printf("\t ma:");
					PrintLN(ma);
					printf("\t mb:");
					PrintLN(mb);
					printf("\t t1:");
					PrintLN(TempRes1);
					printf("\t t2:");
					PrintLN(TempRes2);
					printf("\n");
					GenNextCandidate = 1;
				}
				AtPos++;
				ResetCandidateAtPos(vLN, ParamCount, AtPos, 1);
			}
			else
				GenNextCandidate = 1;
			if (AtPos > 0 && vLN[ParamCount - 1]->Digits[AtPos] > 0)
			{
				ResetCandidateAtPos(vLN, ParamCount, AtPos);
				AtPos--;
				GenNextCandidate = 1;
			}
			if (GenNextCandidate == 1)
				GenerateNextCandidateAtPos(vLN, ParamCount, AtPos);
			//    }while( SolutionsFound == 0 && vLN[ParamCount-1]->Digits[0] == 0 );
		} while (vLN[ParamCount - 1]->Digits[0] == 0);

		if (SolutionsFound == 0)
			printf("\rNo Luck finding a solution\n");
		else
			printf("\rDone testing all possible solutions\n");

		printf("Steps taken %d\n\n", StepsTaken);

	}
};

void DivTestSplitHalf()
{
	SplitInHalf::DivTest_SplitHalf(23, 41);
//	SplitInHalf::DivTest_SplitHalf(349, 751); // N = 262099 SN = 511
//	SplitInHalf::DivTest_SplitHalf(6871, 7673); // N = 52721183 , SN = 7260
//	SplitInHalf::DivTest_SplitHalf(26729, 31793); // N = 849795097 , SN = 29151
//	SplitInHalf::DivTest_SplitHalf(784727, 918839); // N = 721037771953
//	SplitInHalf::DivTest_SplitHalf(6117633, 7219973);
//	SplitInHalf::DivTest_SplitHalf(26729, 61781);
//	SplitInHalf::DivTest_SplitHalf(11789, 61781);
}