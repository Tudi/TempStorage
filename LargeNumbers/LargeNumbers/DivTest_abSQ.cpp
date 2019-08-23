#include "StdAfx.h"

namespace abSQ {
	LargeNumber Ga, Gb, SQN;
	int CheckCandidateMatch_abSQ(LargeNumber *tSQN, LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
	{
		// b * b - a * a = N
		// b * b = N + a * a
		// b [SQN,N]	a [1,SQN]
		if (vLN[0]->Len > (tN->Len + 1) / 2 || vLN[1]->Len > (tN->Len + 1) / 2)
			return 0;

		LargeNumber ta, tb;
//		CopyLN(vLN[0], &ta);
//		CopyLN(vLN[1], &tb);
		AddLN(&Ga, vLN[0], &ta);
		AddLN(&Gb, vLN[1], &tb);

		//b is always larger than SQN
//		if (IsLarger(tSQN, &tb))
//			return 0;

		MulLN(&tb, &tb, TempRes1);

		LargeNumber aa;
		MulLN(&ta, &ta, &aa);
		AddLN(tN, &aa, TempRes2);

		//    EatLeadingZeros( TempRes1 );
		//    EatLeadingZeros( TempRes2 );
		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != TempRes2->Digits[i])
				return 0;

		//'a' needs to be smaller or equal to the value we are looking for : b * b <= N + a * a
//		if (IsLarger(TempRes1, TempRes2) == 1)
//			return 0;

		//funny because these are not required at all
	/*	{
			// A = b - a	B = b + a		A * B = N
			LargeNumber B;
			LargeNumber A;
			AddLN(vLN[0], vLN[1], &B);
			SubLN(vLN[1], vLN[0], &A);
			LargeNumber tn;
			MulLN(&A, &B, &tn);
			for (int i = 0; i <= pos; i++)
				if (tN->Digits[i] != tn.Digits[i])
					return 0;

			// 2 * b = A + B
			LargeNumber b2;
			LargeNumber ApB;
			AddLN(vLN[1], vLN[1], &b2);
			AddLN(&A, &B, &ApB);
			for (int i = 0; i <= pos; i++)
				if (b2.Digits[i] != ApB.Digits[i])
					return 0;

			// 2 * a = B - A
			LargeNumber a2;
			LargeNumber BmA;
			AddLN(vLN[0], vLN[0], &a2);
			SubLN(&B, &A, &BmA);
			for (int i = 0; i <= pos; i++)
				if (a2.Digits[i] != BmA.Digits[i])
					return 0;
		}*/

		return 1;
	}

	void DivTest_abSQ(int iA, int iB)
	{
		// ( b - a ) * ( b + a ) = N
		// b * b - a * a = N
		// A = b - a
		// B = b + a
		LargeNumber N;
		unsigned int iN = iA * iB;
		SetLN(N, iN);
		int ia = (iB - iA) / 2;
		int ib = (iA + iB) / 2;
		__int64 iSN = isqrt(iN);
		printf("Expecting solution a = %d, b = %d. N = %d, SQRT(N) = %d. Bruteforce trycount %d\n", ia, ib, iN, isqrt(iN), (iSN - iA) / 2);

		LargeNumber a, b;

		LargeNumber EndSignal;
#define ParamCount 3
		LargeNumber *vLN[ParamCount];
		vLN[0] = &a;
		vLN[1] = &b;
		vLN[2] = &EndSignal;

		for (int i = 0; i < ParamCount; i++)
			SetLN(vLN[i], 0);
		InitLN(vLN[ParamCount - 1]);

		SetLN(&SQN, iSN);
		SetLN(&Gb, iSN);

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
			/*
			SetLN( a, ia );
			SetLN( b, ib );
			/**/
			for (int i = 0; i < ParamCount - 1; i++)
				DEBUG_Combinations_generated[i][ToIntLN(vLN[i]) % 100] = 1;
#endif
			int Match = CheckCandidateMatch_abSQ(&SQN, &N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
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
					PrintLN(a);
					printf("\t b:");
					PrintLN(b);
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

#ifdef _DEBUG
		/*
			for( int i = 0; i < ParamCount - 1; i++ )
				for( int j = 0; j < 100; j++ )
					if(  DEBUG_Combinations_generated[i][ j ] != 1 )
						printf("missing combination i, j => %d %d \n", i, j );
		/**/
#endif
	}
};

void DivTestabSQ()
{
    // 108k
	abSQ::DivTest_abSQ( 349, 751 ); // N = 262099 SN = 511
    // 938k
	abSQ::DivTest_abSQ( 6871, 7673 ); // N = 52721183 , SN = 7260
    // 9M tries
	abSQ::DivTest_abSQ( 26729, 31793 ); // N = 849795097 , SN = 29151
}
