#include "StdAfx.h"

/*
cross check if negative version produces same result as positive
spoiler : it does
*/
namespace LineDraw8 {
	LargeNumber N;
	LargeNumber x, y, a, b, m, a1, b1, m1;
	LargeNumber yd, ym;

#define ParamCount 3
#define	x1_i 0
#define y1_i 1
#define a1aDistance 50

	int CheckCandidateMatch_LineDraw8_pos(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		//b*x + m = (x+y)*(a-x)// least operations( 5 compared to 7), same result as x^2 + x*(b-a) + m = y*(a-x)
		if (vLN[x1_i]->Len > a.Len)
			return 0;
		if (vLN[y1_i]->Len > a.Len)
			return 0;

		LargeNumber bx, bxm, xy, ax, xyax;
		MulLN(&b, vLN[x1_i], &bx);
		AddLN(&bx, &m, TempRes1);

		AddLN(vLN[x1_i], vLN[y1_i], &xy);
		SubLN(&a, vLN[x1_i], &ax);
		MulLN(&xy, &ax, TempRes2);

		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != TempRes2->Digits[i])
				return 0;

		return 1;
	}

	int CheckCandidateMatch_LineDraw8_negative_m(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		//x1 = x + (a1 - a)
		//y1 = y - (b1 - b)
		//b1*x1 = (x1+y1)*(a1-x1) + m1// least operations( 5 compared to 7), same result as x^2 + x*(b-a) + m = y*(a-x)
		//x1 > m/b1
		//chances are, y will become negative
		if (vLN[x1_i]->Len > a.Len)
			return 0;
		if (vLN[y1_i]->Len > a.Len)
			return 0;

		LargeNumber x1, y1, ten;
		SetLN(&ten, a1aDistance);
		AddLN(vLN[x1_i], &ten, &x1);
		SubLN(vLN[y1_i], &ten, &y1);

		LargeNumber bx, bxm, xy, ax, xyax;
		MulLN(&b1, &x1, TempRes1);

		AddLN(&x1, &y1, &xy);
		SubLN(&a1, &x1, &ax);
		MulLN(&xy, &ax, &xyax);
		AddLN(&xyax, &m1, TempRes2);

		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != TempRes2->Digits[i])
				return 0;

		return 1;
	}

	int CheckCandidateMatch_LineDraw8(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
//		return CheckCandidateMatch_LineDraw8_pos(tN, vLN, Params, pos, TempRes1, TempRes2);
		if (CheckCandidateMatch_LineDraw8_pos(tN, vLN, Params, pos, TempRes1, TempRes2))
		{
			if (CheckCandidateMatch_LineDraw8_pos(tN, vLN, Params, pos, TempRes1, TempRes2))
			{
//				NormalizeLN(TempRes1);
//				NormalizeLN(TempRes2);
				return 1;
			}
			else
				printf("managed to filter\n");
		}
		return 0;
	}

/*	void Increase_ab_until_y_positive(__int64 &a, __int64 &b, __int64 iA, __int64 iB)
	{
		__int64 N = iA * iB;
		__int64 x, y, m;
//		a+=20;
//		b+=20;
		do {
			a++;
			b++;
			m = a * b - N;
			//x > m/b
			x = m/b;
			y = ((b*x) - m) / (a - x) - x;
		} while (y > 0);
		a--;
		b--;
	}/**/

	void DivTest_LineDraw8(__int64 iA, __int64 iB)
	{
		__int64 iN = iA * iB;
		SetLN(N, iN);
		__int64 iSN = isqrt(iN);

		__int64 ia = iSN;
		__int64 ib = ia;
		__int64 im = iN - ia * ib;

		//increase a and b until last digits are 0
		//need to make sure b < B also b + x < B also b + x + y < B
/*		__int64 c = 1;
		while (ia > c * 10 && ib > c * 10)
			c = c * 10;
		{
			__int64 t;
			t = c - (ia % c);
			ia += t;
			t = c - (ib % c);
			ib += t;
			im = ia * ib - iN;
		}/**/
	//	Increase_ab_until_y_positive(ia, ib, iA, iB);

		SetLN(a, ia);
		SetLN(b, ib);
		SetLN(m, im);

		SetLN(a1, ia + a1aDistance);
		SetLN(b1, ib + a1aDistance);
		SetLN(m1, (ia + a1aDistance) * (ib + a1aDistance) - iN);

		__int64 ix1 = ia - iA;
		__int64 iy1 = iB - ib - ix1;
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
			int Match = CheckCandidateMatch_LineDraw8(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
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

void DivTestLineDraw8()
{
	// LineDraw8::DivTest_LineDraw8(23, 41);
	//	LineDraw8::DivTest_LineDraw8(349, 751); // N = 262099 SN = 511
	LineDraw8::DivTest_LineDraw8(6871, 7673); // N = 52721183 , SN = 7260
//	LineDraw8::DivTest_LineDraw8(26729, 31793); // N = 849795097 , SN = 29151
//	LineDraw8::DivTest_LineDraw8(784727, 918839); // N = 721037771953
//	LineDraw8::DivTest_LineDraw8(6117633, 7219973);
//	LineDraw8::DivTest_LineDraw8(26729, 61781);
//	LineDraw8::DivTest_LineDraw8(11789, 61781);
}