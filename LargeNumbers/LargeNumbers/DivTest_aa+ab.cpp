#include "StdAfx.h"

//#define TRY_B_INSTEAD_A

#define INDEX_A 0
#define INDEX_a 1

int CheckCandidateMatch_aa_ab(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1)
{
	// N = A * B = A * ( A + a ) = A * A + A * a
	// a = B - A		B = A + a		N = A * ( A + a )

	// if x + 10 = A and y + 10 = a
	// N = (x+10)*(x+10)+(x+10)*(y+10) => N = x * x + 2 * 10 * x + 100 + x * y + 10 * x + 10 * y + 100 -> complexity increases a lot and limit drops 2xtestcount
	if (vLN[INDEX_A]->Len > (tN->Len + 1) / 2 || vLN[INDEX_a]->Len > (tN->Len + 1) / 2)
//	if (pos > (tN->Len + 1) / 2)
		return 0;

	//this versin for some reason is better than any other version. Just need to figure it out why
	{
		//last digit needs to be pair
		if (vLN[INDEX_a]->Digits[0] % 2 != 0)
			return 0;
		LargeNumber AA, aA;
		AddLN(vLN[INDEX_A], vLN[INDEX_a], &aA);
		MulLN(vLN[INDEX_A], &aA, TempRes1);
	}/**/

	/*
	// this version is visibly worse than previous. I was expecting this one to be better actually
	{
		//N = A * (A + 2 * a)
		LargeNumber AA, aA, two, twoa;
		SetLN(&two, 2);
		MulLN(vLN[INDEX_a], &two, &twoa);
		AddLN(vLN[INDEX_A], &twoa, &aA);
		MulLN(vLN[INDEX_A], &aA, TempRes1);
	}/**/

	for (int i = 0; i <= pos; i++)
		if (TempRes1->Digits[i] != tN->Digits[i])
			return 0;

	ReCheckSize(TempRes1);
	if (IsLarger(TempRes1, tN))
		return 0;

/*	{
		LargeNumber tB, tn;
		AddLN(vLN[INDEX_A], vLN[INDEX_a], &tB);
		MulLN(vLN[INDEX_A], &tB, &tn);
		for (int i = 0; i <= pos; i++)
			if (tn.Digits[i] != tN->Digits[i])
			{
				printf("rare event when second check triggered\n");
				return 0;
			}
	}/**/

	return 1;
}

#ifdef TRY_B_INSTEAD_A
int CheckCandidateMatch_Ba_BB(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1)
{
	// N = A * B
	// A = B - a
	// N = B * (B - a) => N + B * a = B * B
	if (vLN[INDEX_A]->Len > (tN->Len + 1) / 2 || vLN[INDEX_a]->Len > (tN->Len + 1) / 2)
		return 0;

	LargeNumber Ba, NBa, BB;
	MulLN(vLN[INDEX_A], vLN[INDEX_a], &Ba);
	AddLN(&Ba, tN, &NBa);
	MulLN(vLN[INDEX_A], vLN[INDEX_A], &BB);

	for (int i = 0; i <= pos; i++)
		if (NBa.Digits[i] != BB.Digits[i])
			return 0;

	//check if solution
	{
		if (NBa.Len == BB.Len)
		{
			int IsSolution = 2;
			for (int i = pos+1; i < NBa.Len; i++)
				if (NBa.Digits[i] != BB.Digits[i])
				{
					IsSolution = 0;
					break;
				}
			if (IsSolution == 2)
				return 2;
		}
	}
	return 1;
}
#endif

void DivTest_aa_ab(__int64 iA, __int64 iB)
{
	// N = A * B = A * A + A * a
	// a = B - A		B = A + a		N = A * ( A + a )
	LargeNumber tN;
	__int64 iN = iA * iB;
	__int64 ia = iB - iA;
	LargeNumber A,a;

	SetLN(tN, iN);

#ifndef TRY_B_INSTEAD_A
	printf("Expected solution A=%lld,a=%lld\n", iA, ia);
#else
	printf("Expected solution B=%lld,b=%lld\n", iB, ia);
#endif

	//init the coefficients
	LargeNumber EndSignal;
#define ParamCount 3
	LargeNumber *vLN[ParamCount];
	vLN[INDEX_A] = &A;
	vLN[INDEX_a] = &a;
	vLN[2] = &EndSignal;

	for (int i = 0; i < ParamCount; i++)
		SetLN(vLN[i], (__int64)0);
	InitLN(vLN[ParamCount - 1]);

	//start generating combinations and check if it's a feasable candidate
	int AtPos = 0;
	int SolutionFound = 0;
	int SolutionsFound = 0;
	int CandidatesFound = 0;
	__int64 StepsTaken = 0;
//	int CrossChecks = 0;
//	char DEBUG_Combinations_generated[ParamCount][99 + 1];
//	memset(DEBUG_Combinations_generated, 0, sizeof(DEBUG_Combinations_generated));
	do
	{
		LargeNumber TempRes;
		int GenNextCandidate = 0;
		StepsTaken++;
/*		for (int i = 0; i < ParamCount; i++)
		{
			__int64 Combo;
			ToIntLN(vLN[i], &Combo);
			DEBUG_Combinations_generated[i][Combo % 100] = 1;
		}*/

#ifndef TRY_B_INSTEAD_A
		int Match = CheckCandidateMatch_aa_ab(&tN, vLN, ParamCount, AtPos, &TempRes);
#else
		int Match = CheckCandidateMatch_Ba_BB(&tN, vLN, ParamCount, AtPos, &TempRes);
#endif
		if (Match != 0)
		{
			CandidatesFound++;
			if (CandidatesFound % 100 == 0)
			{
				int chars[4] = { '\\','|','/','-' };
				printf("\r%c", chars[(CandidatesFound / 100) % 4]);
			}
#ifndef TRY_B_INSTEAD_A
			SolutionFound = CheckSolution(tN, TempRes);
#else
			SolutionFound = Match == 2;
#endif
			if (SolutionFound != 0)
			{
				SolutionsFound++;
				printf("\r%d / %d)sol : \t A:", SolutionsFound, CandidatesFound);
				PrintLN(A);
				printf("\t a:");
				PrintLN(a);
				printf("\n");
				GenNextCandidate = 1;
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
	} while (SolutionsFound < 1000 && vLN[ParamCount - 1]->Digits[0] == 0);
	//    }while( SolutionsFound == 0 && vLN[ParamCount-1]->Digits[0] == 0 );

	if (SolutionsFound == 0)
		printf("No Luck finding a solution\n");
	else
		printf("Done testing all possible solutions\n");
	printf("Steps taken %lld\n\n", StepsTaken);

/*	for (int i = 0; i < ParamCount - 1; i++)
		for (int j = 0; j < 100; j++)
			if (DEBUG_Combinations_generated[i][j] != 1)
				printf("missing combination i, j => %d %d \n", i, j);*/
}

void DivTestaa_ab()
{
	printf("This only works if B<2A. Should add proper checks\n");
//	DivTest_aa_ab(23, 41);
//	DivTest_aa_ab(349, 751); // N = 262099 SN = 511
//	DivTest_aa_ab(6871, 7673); // N = 52721183 , SN = 7260
	DivTest_aa_ab(26729, 31793); // N = 849795097 , SN = 29151
	DivTest_aa_ab(784727, 918839); // N = 721037771953
	DivTest_aa_ab(6117633, 7219973);
	DivTest_aa_ab(26729, 61781);
	DivTest_aa_ab(11789, 61781);
}

/*
Expected solution A=26729,a=5064. N = 849795097, SQRT(N) = 29151. Bruteforce trycount 14.575
1 / 4.465)sol :   A:26729         a:05064
/Done testing all possible solutions
Steps taken 1120559

Expected solution A=784727,a=134.112. N = 721037771953, SQRT(N) = 849139. Bruteforce trycount 424.569
1 / 81.236)sol :          A:784727
/Done testing all possible solutions
Steps taken 32837440

Expected solution A=6117633,a=110.2340. N = 44169145083909, SQRT(N) = 6645987. Bruteforce trycount 3.322.993
1 / 268.056)sol :         A:6117633
-Done testing all possible solutions
Steps taken 421761963

Expected solution A=26729,a=35052. N = 1651344349, SQRT(N) = 40636. Bruteforce trycount 20.318
1 / 9.365)sol :   A:26729         a:35052
2 / 9.801)sol :   A:26729         a:35052
\Done testing all possible solutions
Steps taken 1859972

Expected solution A=11789,a=49992. N = 728336209, SQRT(N) = 26987. Bruteforce trycount 13.493
1 / 8.069)sol :   A:11789         a:49992
-Done testing all possible solutions
Steps taken 1689573
*/