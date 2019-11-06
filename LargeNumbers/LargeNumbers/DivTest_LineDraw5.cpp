#include "StdAfx.h"

/*
tried enough to know that unless we reduce the size of A or B there is no point trying to generate values. So, this is probably the last try
*/
namespace LineDraw5 {
	LargeNumber N;
	LargeNumber a1,b1,m1;
	LargeNumber A1,B1;

	static int LogOnlyLen = -1;
	static FILE *f = NULL;
	static __int64 Minxpy = -1;

#define ParamCount 3
#define	x1_i 0
#define y1_i 1


	int CheckCandidateMatch_LineDraw5(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		// truth test 1 is :		- y is the distance to the middle between A and B
		//		b1 * x1 + m1 = ( x1 + y1 ) * ( a1 - x1 )
		//		A = a - x		
		//		B = b + x + y

		if (LogOnlyLen >= 0 && LogOnlyLen < pos)
			return 0;

		if (IsLarger(vLN[x1_i], &a1) != 0)
			return 0;
		if (vLN[y1_i]->Len > N.Len)
			return 0;

		LargeNumber b1x1;
		MulLN(&b1, vLN[x1_i], &b1x1);
		AddLN(&b1x1, &m1, TempRes1);

		LargeNumber x1y1, a1x1;
		AddLN(vLN[x1_i], vLN[y1_i], &x1y1);
		SubLN(&a1, vLN[x1_i], &a1x1);
		MulLN(&x1y1, &a1x1, TempRes2);

		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != TempRes2->Digits[i])
				return 0;

		SubLN(&a1, vLN[x1_i], &A1);
		AddLN(&b1, &x1y1, &B1);

		if (A1.Len + B1.Len > N.Len + 1)
			return 0;

		return 1;
	}

	void DivTest_LineDraw5(__int64 iA, __int64 iB)
	{
		//return; // because this is no other than A * B, just with 4 values instead of 2
		__int64 iN = iA * iB;
		SetLN(N, iN);
		__int64 iSN = isqrt(iN);

		__int64 ia1 = iSN;
//		__int64 ia1 = iSN - (1581 + 94 + 445 + 50 + 233);
		__int64 ib1 = iN / ia1;
//		ia1 = iN / ib1;
		__int64 im1 = iN - ia1 * ib1;

		SetLN(a1, ia1);
		SetLN(b1, ib1);
		SetLN(m1, im1);

		__int64 ix1 = ia1 - iA;
		__int64 iy1 = iB - ib1 - ix1;
		printf("Expecting solution x1 = %lld, y1 = %lld. N=%lld. Bruteforce trycount %lld\n", ix1, iy1, iN, iSN / 2);

		LargeNumber x1, y1;
		LargeNumber EndSignal;

		LargeNumber *vLN[ParamCount];
		vLN[x1_i] = &x1;
		vLN[y1_i] = &y1;
		vLN[2] = &EndSignal;

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
			int Match = CheckCandidateMatch_LineDraw5(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
			if (Match == 1)
			{
				CandidatesFound++;
				if (CandidatesFound % 100 == 0)
				{
					int chars[4] = { '\\','|','/','-' };
					printf("\r%c", chars[(CandidatesFound / 100) % 4]);
				}
				if (LogOnlyLen == -1 || AtPos == LogOnlyLen)
				{
					if (f == NULL)
						errno_t er = fopen_s(&f, "Candidates_draw5.txt", "wt");
					if (f != NULL)
					{
						__int64 x = ToIntLN(&x1);
						__int64 y = ToIntLN(&y1);
						if (Minxpy == -1 || Minxpy > x + y)
							Minxpy = x + y;
						fprintf(f, "%lld\t%lld\t%lld\t%lld\n", ToIntLN(&A1), ToIntLN(&B1), x, y);
					}
					//					fclose(f);
				}
				int SolutionFound = CheckSolution(TempRes1, TempRes2);
				if (SolutionFound == 1)
				{
					SolutionsFound++;
					printf("\r%d / %d )sol : \t x1:", SolutionsFound, CandidatesFound);
					PrintLN(x1);
					printf("\t y1:");
					PrintLN(y1);
					printf("\t t1:");
					PrintLN(TempRes1);
					printf("\t t2:");
					PrintLN(TempRes2);
					printf("\n");
					GenNextCandidate = 1;
					break;
				}
				else
				{
					AtPos++;
					ResetCandidateAtPos(vLN, ParamCount, AtPos, 1);
				}
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
		} while (vLN[ParamCount - 1]->Digits[0] == 0);

		if (SolutionsFound == 0)
			printf("\rNo Luck finding a solution\n");
		else
			printf("\rDone testing all possible solutions\n");

		printf("Steps taken %d. Candidates found %d\n\n", StepsTaken, CandidatesFound);

	}
};

void DivTestLineDraw5()
{
	/*for (int i = 0; i < 5; i++)
	{
		LineDraw5::Minxpy = -1;
		LineDraw5::LogOnlyLen = i;
		LineDraw5::DivTest_LineDraw5(26729, 31793);
		fprintf(LineDraw5::f, "%lld\n\n", LineDraw5::Minxpy);
		printf("%lld\n", LineDraw5::Minxpy);
	}/**/

	//  LineDraw5::DivTest_LineDraw5(23, 41);
	//	LineDraw5::DivTest_LineDraw5(349, 751); // N = 262099 SN = 511
		LineDraw5::DivTest_LineDraw5(6871, 7673); // N = 52721183 , SN = 7260
	//	LineDraw5::DivTest_LineDraw5(26729, 31793); // N = 849795097 , SN = 29151
	//	LineDraw5::DivTest_LineDraw5(784727, 918839); // N = 721037771953
	//	LineDraw5::DivTest_LineDraw5(6117633, 7219973);
	//	LineDraw5::DivTest_LineDraw5(26729, 61781);
	//	LineDraw5::DivTest_LineDraw5(11789, 61781);
}