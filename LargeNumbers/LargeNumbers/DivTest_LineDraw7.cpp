#include "StdAfx.h"

/*
tried enough to know that unless we reduce the size of A or B there is no point trying to generate values. So, this is probably the last try
*/
namespace LineDraw7 {
	LargeNumber N;
	LargeNumber a1, b1, m1;
	LargeNumber a2, b2, m2;
	LargeNumber A1, B1, A2, B2;

	//#define ParamCount 3
#define ParamCount 5
#define	x1_i 0
#define y1_i 1
#define x2_i 2
#define y2_i 3

	int CheckCandidateMatch_LineDraw7_1(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		// truth test 1 is :		- y is the distance to the middle between A and B
		//		b1 * x1 + m1 = ( x1 + y1 ) * ( a1 - x1 )
		//		A = a - x		
		//		B = b + x + y

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

	int CheckCandidateMatch_LineDraw7_2(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		// truth test 2 is :		- distance between 2 squares
		//		2 * a2 * x2 + x2 * x2 + m2 = 2 * b2 * y2 + y2 * y2
		//		A = (b2 + y2) - (a2 + x2)
		//		B = (b2 + y2) + (a2 + x2)

		if (vLN[x2_i]->Len > N.Len / 2)
			return 0;
		if (vLN[y2_i]->Len > N.Len / 2)
			return 0;

		if (vLN[x2_i]->Digits[0] == 0 && vLN[y2_i]->Digits[0] == 6)
			if (vLN[x2_i]->Digits[1] == 5 && vLN[y2_i]->Digits[1] == 3)
				if (vLN[x2_i]->Digits[2] == 1 && vLN[y2_i]->Digits[2] == 0)
					vLN[x2_i]->Digits[0] = vLN[x2_i]->Digits[0];

		LargeNumber two;
		SetLN(&two, 2);

		LargeNumber a2x2, a2x22, x2x2, x2x2m2;
		MulLN(&a2, vLN[x2_i], &a2x2);
		MulLN(&a2x2, &two, &a2x22);
		MulLN(vLN[x2_i], vLN[x2_i], &x2x2);
		AddLN(&x2x2, &m2, &x2x2m2);
		AddLN(&a2x22, &x2x2m2, TempRes1);

		LargeNumber b2y2, b2y22, y2y2;
		MulLN(&b2, vLN[y2_i], &b2y2);
		MulLN(&b2y2, &two, &b2y22);
		MulLN(vLN[y2_i], vLN[y2_i], &y2y2);
		AddLN(&b2y22, &y2y2, TempRes2);

		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != TempRes2->Digits[i])
				return 0;

		AddLN(&a2, vLN[x2_i], &a2x2);
		AddLN(&b2, vLN[y2_i], &b2y2);
		SubLN(&b2y2, &a2x2, &A2);
		AddLN(&b2y2, &a2x2, &B2);

		if (A2.Len + B2.Len > N.Len + 1)
			return 0;

		return 1;
	}

	int CheckCandidateMatch_LineDraw7(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{

		if (CheckCandidateMatch_LineDraw7_1(tN, vLN, Params, pos, TempRes1, TempRes2) == 0)
			return 0;

#if(ParamCount==5)
		if (CheckCandidateMatch_LineDraw7_2(tN, vLN, Params, pos, TempRes1, TempRes2) == 0)
			return 0;

		for (int i = 0; i <= pos; i++)
			if (A1.Digits[i] != A2.Digits[i])
				return 0;

		for (int i = 0; i <= pos; i++)
			if (B1.Digits[i] != B2.Digits[i])
				return 0;
		/**/
#endif
		return 1;
	}

	void DivTest_LineDraw7(__int64 iA, __int64 iB)
	{
		//return; // because this is no other than A * B, just with 4 values instead of 2
		__int64 iN = iA * iB;
		SetLN(N, iN);
		__int64 iSN = isqrt(iN);

		__int64 ia1 = iSN;
		__int64 ib1 = iN / ia1;
		__int64 im1 = iN - ia1 * ib1;

		__int64 ia2 = iSN / 20; // this really should be smaller than A while testing
		assert(ia2 < iA);
		__int64 ComplementedLargeSquare = iN + ia2 * ia2; //take the small square and try to bend it over the large square
		__int64 ib2 = isqrt(ComplementedLargeSquare);
		__int64 im2 = ComplementedLargeSquare - ib2 * ib2;

		SetLN(a1, ia1);
		SetLN(b1, ib1);
		SetLN(m1, im1);

		SetLN(a2, ia2);
		SetLN(b2, ib2);
		SetLN(m2, im2);

		__int64 ix1 = ia1 - iA;
		__int64 iy1 = iB - ib1 - ix1;
		printf("Expecting solution x1 = %lld, y1 = %lld. N=%lld. Bruteforce trycount %lld\n", ix1, iy1, iN, iSN / 2);
		__int64 ix2 = (iB - iA) / 2 - ia2;
		__int64 iy2 = (iB + iA) / 2 - ib2;
		printf("Expecting solution x2 = %lld, y2 = %lld\n", ix2, iy2);

		LargeNumber x1, y1, x2, y2;
		LargeNumber EndSignal;

		LargeNumber *vLN[ParamCount];
		vLN[x1_i] = &x1;
		vLN[y1_i] = &y1;
		vLN[2] = &EndSignal;
#if (ParamCount==5)
		vLN[x2_i] = &x2;
		vLN[y2_i] = &y2;
		vLN[4] = &EndSignal;
#endif

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
			int Match = CheckCandidateMatch_LineDraw7(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
			if (Match == 1)
			{
				CandidatesFound++;
				if (CandidatesFound % 100 == 0)
				{
					int chars[4] = { '\\','|','/','-' };
					printf("\r%c", chars[(CandidatesFound / 100) % 4]);
				}
				int SolutionFound = CheckSolution(TempRes1, TempRes2);
				if (SolutionFound == 1)
				{
					SolutionsFound++;
					printf("\r%d / %d )sol : \t x1:", SolutionsFound, CandidatesFound);
					PrintLN(x1);
					printf("\t y1:");
					PrintLN(y1);
					printf("\t x2:");
					PrintLN(x2);
					printf("\t y2:");
					PrintLN(y2);
					printf("\t t1:");
					PrintLN(TempRes1);
					printf("\t t2:");
					PrintLN(TempRes2);
					printf("\n");
					GenNextCandidate = 1;
					break;
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

		printf("Steps taken %d. Candidates found %d\n\n", StepsTaken, CandidatesFound);

	}
};

void DivTestLineDraw7()
{
	// LineDraw7::DivTest_LineDraw7(23, 41);
	//	LineDraw7::DivTest_LineDraw7(349, 751); // N = 262099 SN = 511
	LineDraw7::DivTest_LineDraw7(6871, 7673); // N = 52721183 , SN = 7260
//	LineDraw7::DivTest_LineDraw7(26729, 31793); // N = 849795097 , SN = 29151
//	LineDraw7::DivTest_LineDraw7(784727, 918839); // N = 721037771953
//	LineDraw7::DivTest_LineDraw7(6117633, 7219973);
//	LineDraw7::DivTest_LineDraw7(26729, 61781);
//	LineDraw7::DivTest_LineDraw7(11789, 61781);
}