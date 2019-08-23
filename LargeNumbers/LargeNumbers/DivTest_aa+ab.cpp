#include "StdAfx.h"

int CheckCandidateMatch_aa_ab(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1)
{
	// N = A * B = A * A + A * a
	// a = B - A		B = A + a		N = A * ( A + a )
	if (vLN[0]->Len > (tN->Len + 1) / 2 || vLN[1]->Len > (tN->Len + 1) / 2)
		return 0;

	LargeNumber aA;
	AddLN(vLN[0], vLN[1], &aA);
	MulLN(vLN[0], &aA, TempRes1);

	return 1;
}

void DivTest_aa_ab(__int64 iA, __int64 iB)
{
	// N = A * B = A * A + A * a
	// a = B - A		B = A + a		N = A * ( A + a )
	LargeNumber tN;
	unsigned int iN = (unsigned int)(iA * iB);
	int isqn = isqrt(iN);
	int ia = iB - iA;
	LargeNumber A,a;

	SetLN(tN, iN);
	SetLN(A, iA);
	SetLN(a, ia);

	//init the coefficients
	LargeNumber EndSignal;
#define ParamCount 3
	LargeNumber *vLN[ParamCount];
	vLN[0] = &A;
	vLN[1] = &a;
	vLN[4] = &EndSignal;

	for (int i = 0; i < ParamCount; i++)
		SetLN(vLN[i], 1);
	InitLN(vLN[ParamCount - 1]);

	//start generating combinations and check if it's a feasable candidate
	int AtPos = 0;
	int SolutionFound = 0;
	int SolutionsFound = 0;
	int CandidatesFound = 0;
	int CrossChecks = 0;
	int StepsTaken = 0;
	char DEBUG_Combinations_generated[ParamCount][99 + 1];
	memset(DEBUG_Combinations_generated, 0, sizeof(DEBUG_Combinations_generated));
	do
	{
		LargeNumber TempRes;
		int GenNextCandidate = 0;
		StepsTaken++;
		for (int i = 0; i < ParamCount; i++)
		{
			int Combo;
			ToIntLN(vLN[i], &Combo);
			DEBUG_Combinations_generated[i][Combo % 100] = 1;
		}

		int Match = CheckCandidateMatch_aa_ab(&tN, vLN, ParamCount, AtPos, &TempRes);
		if (Match == 1)
		{
			CandidatesFound++;
			if (CandidatesFound % 100 == 0)
			{
				int chars[4] = { '\\','|','/','-' };
				printf("\r%c", chars[(CandidatesFound / 100) % 4]);
			}
			SolutionFound = CheckSolution(tN, TempRes);
			if (SolutionFound == 1)
			{
				SolutionsFound++;
				printf("\r%d / %d / %d)sol : \t A:", SolutionsFound, CandidatesFound, CrossChecks);
				PrintLN(A);
				printf("\t a:");
				PrintLN(a);
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
		printf("\rNo Luck finding a solution\n");
	else
		printf("\rDone testing all possible solutions\n");
	printf("Steps taken %d\n\n", StepsTaken);

	for (int i = 0; i < ParamCount - 1; i++)
		for (int j = 0; j < 100; j++)
			if (DEBUG_Combinations_generated[i][j] != 1)
				printf("missing combination i, j => %d %d \n", i, j);
}

void DivTestaa_ab()
{
//    DivTest_RecDiv( 23, 41 ); // N = 943 , SN = 30
//    DivTest_RecDiv( 349, 751 ); // N = 262099 , SN = 511
//	DivTest_aa_ab(6871, 7673); // N = 52721183 , SN = 7260
//    DivTest_RecDiv( 26729, 31793 ); // N = 849795097 , SN = 29151
}
