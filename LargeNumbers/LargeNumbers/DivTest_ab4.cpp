#include "StdAfx.h"

/*
bx + m = (x+y)*(a-x)
if y = 0, a = b .... x * x = m
m0 = N - SN * SN
m1 = m0 - ( 1 * 2 * SN + 1*1)
mz = m0 - ( 2 * z * SN + z * z )
Is this faster than guesing a*b ? or b*b-a*a=N
*/
namespace gen_aabb4 {
	LargeNumber N;
	LargeNumber x, y, a, m;

#define ParamCount 3
#define	x1_i 0
#define y1_i 1

	int CheckCandidateMatch_gen_aabb4(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		//m + z * ( 2 * a + z ) = x * x
		if (vLN[x1_i]->Len > a.Len+1)
			return 0;
		if (vLN[y1_i]->Len > a.Len+1)
			return 0;

		LargeNumber two;
		SetLN(two, 2);
		LargeNumber a2, a2z, a2zz;
		MulLN(&a, &two, &a2);
		AddLN(&a2, vLN[y1_i], &a2z);
		MulLN(&a2z, vLN[y1_i], &a2zz);
		AddLN(&a2zz, &m, TempRes1);

		MulLN(vLN[x1_i], vLN[x1_i], TempRes2);

		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != TempRes2->Digits[i])
				return 0;

		//would a secondary test help ?
/*		{
			LargeNumber bb,bbbb,aa,aaN;
			AddLN(&a, vLN[y1_i], &bb);
			MulLN(&bb, &bb, &bbbb);
			MulLN(vLN[x1_i], vLN[x1_i], &aa);
			AddLN(&aa, &N, &aaN);
			for (int i = 0; i <= pos; i++)
				if (bbbb.Digits[i] != aaN.Digits[i])
					return 0;
		}/**/
		return 1;
	}

	void DivTest_gen_aabb4(__int64 iA, __int64 iB)
	{
		__int64 iN = iA * iB;
		SetLN(N, iN);
		__int64 iSN = isqrt(iN);

		__int64 ia = iSN + 1;
		__int64 im = ia * ia - iN;

		SetLN(a, ia);
//		SetLN(b, ib);
		SetLN(m, im);

		__int64 ix1 = (iB - iA) / 2;
		__int64 iy1 = (iA + iB) / 2 - ia;
		__int64 left = im + iy1 * (2 * ia + iy1);
		__int64 right = ix1 * ix1;
		printf("Starting gen from a = %lld, m = %lld, %lld == %lld\n", ia, im, left, right);
		printf("Expecting solution x = %lld, z = %lld. N=%lld. Bruteforce trycount %lld\n", ix1, iy1, iN, iSN / 2);

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
			int Match = CheckCandidateMatch_gen_aabb4(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
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
					printf("\r%d / %d )sol : \t x:", SolutionsFound, CandidatesFound);
					PrintLN(x1);
					printf("\t z:");
					PrintLN(y1);
					printf("\t t1:");
					PrintLN(TempRes1);
					printf("\t t2:");
					PrintLN(TempRes2);
					printf("\n");
					GenNextCandidate = 1;
//					break;
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

void DivTestab4()
{
//	gen_aabb4::DivTest_gen_aabb4(23, 41);
//	gen_aabb4::DivTest_gen_aabb4(349, 751); // N = 262099 SN = 511
//	gen_aabb4::DivTest_gen_aabb4(6871, 7673); // N = 52721183 , SN = 7260
	gen_aabb4::DivTest_gen_aabb4(26729, 31793); // N = 849795097 , SN = 29151
//	gen_aabb4::DivTest_gen_aabb4(784727, 918839); // N = 721037771953
//	gen_aabb4::DivTest_gen_aabb4(6117633, 7219973);
//	gen_aabb4::DivTest_gen_aabb4(26729, 61781);
//	gen_aabb4::DivTest_gen_aabb4(11789, 61781);
}