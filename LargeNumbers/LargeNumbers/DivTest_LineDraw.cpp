#include "StdAfx.h"

int a;
int b;
int m;
int z;
int PrevX = 0;
int y;
int ExpectedX;

int CheckCandidateMatch_LineDraw(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
{
	// x * b - z * ( x + y ) * ( a - x ) + m - a + x = 0
	// we are searching for an X by guessing the Y. z = b / a
	// At the end a = A		A = a - x	B = b + x + y.....
	// a and b can start off from a = sqrt(N) b = N / sqrt(N)

	int PrevZ = z;
	z = ( b - PrevX ) / ( a - PrevX );
	if (PrevZ != z)
		y = 0;
//	int iy = ToIntLN(vLN[0]);

	int t = (b - z * a + y * z + 1);
	int x = (-t + isqrt(t*t - 4 * z*(m - a * (z*y + 1)))) / 2;

	TempRes1->Len = 1;
	TempRes2->Len = 1;
	if (x == ExpectedX)
	{
		printf("Found solution : %d \n", x);
		TempRes1->Digits[0] = 1;
		TempRes2->Digits[0] = 1;
	}
	else
	{
		TempRes1->Digits[0] = 1;
		TempRes2->Digits[0] = 0;
	}

	y++;
	PrevX = x;
		
	return 1;
}

void DivTest_LineDraw(int iA, int iB)
{
	// x * b - z * ( x + y ) * ( a - x ) + m - a + x = 0
	// we are searching for an X by guessing the Y. z = b / a
	// At the end a = A		A = a - x	B = b + x + y.....
	// a and b can start off from a = sqrt(N) b = N / sqrt(N)

	LargeNumber N;
	unsigned int iN = iA * iB;
	SetLN(N, iN);
	a = isqrt(iN);
	b = iN / a;
	m = iN - a * b;
	z = b / a;
	y = 0;
	ExpectedX = a - iA;
	printf("Expecting solution x = %d. N = %d, SQRT(N) = %d. Bruteforce trycount %d\n", a - iA, iN, isqrt(iN), isqrt(iN) / 2);

	LargeNumber t;

	LargeNumber EndSignal;
#define ParamCount 2
	LargeNumber *vLN[ParamCount];
	vLN[0] = &t;
	vLN[1] = &EndSignal;

	for (int i = 0; i < ParamCount; i++)
		SetLN(vLN[i], 0);
	InitLN(vLN[ParamCount - 1]);

	//start generating combinations and check if it's a feasable candidate
	int AtPos = 0;
	int SolutionsFound = 0;
	int CandidatesFound = 0;
	int StepsTaken = 0;
	char DEBUG_Combinations_generated[ParamCount][99 + 1];
	memset(DEBUG_Combinations_generated, 0, sizeof(DEBUG_Combinations_generated));
	do
	{
		LargeNumber TempRes1, TempRes2;
		int GenNextCandidate = 0;
		StepsTaken++;
#ifdef _DEBUG
		for (int i = 0; i < ParamCount - 1; i++)
			DEBUG_Combinations_generated[i][ToIntLN(vLN[i]) % 100] = 1;
#endif
		int Match = CheckCandidateMatch_LineDraw(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
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
				printf("\r%d / %d )sol : \t a:", SolutionsFound, CandidatesFound);
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
		printf("\rDone testing all possble solutions\n");

	printf("Steps taken %d\n\n", StepsTaken);
}

void DivTestLineDraw()
{
	// 108k
	DivTest_LineDraw(349, 751); // N = 262099 SN = 511
	// 938k
	DivTest_LineDraw(6871, 7673); // N = 52721183 , SN = 7260
	// 9M tries
	DivTest_LineDraw(26729, 31793); // N = 849795097 , SN = 29151
}
