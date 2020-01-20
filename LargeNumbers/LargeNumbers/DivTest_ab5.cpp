#include "StdAfx.h"

/*
can we get closer with an x, y so that we are still below the tN and we still generate partially the number ?
In theory, we need to guess an "a" that is at least 1 digit smaller than the real A ?

N=A*B=(a+x)*(b+y)
N-xy=ab+ay+bx
...
a=a1+a2
b=b1+b2
N-xy=(a1+a2)(b1+b2)+(a1+a2)y+(b1+b2)x
N-xy-a2b2-a2y-b2x=a1b1+a2b1+a1b2+a1y+b1x
...
x*y = N % 10
ex1 : 349 * 751 = 262099 = (340+9)*(750+1)=340*750+9*750+1*340+1*9 => 10*(10*34*75+9*75+1*34)= 10*26209
ex2 : 349 * 751 = 262099 = (300+49)*(700+51)=300*700+49*700+51*300+51*49 => 100*(100*3*7+49*7+51*3)= 100*2620
exbad1 : 349 * 751 = 262099 = (346+3)*(748+3)=346*748+3*748+3*346+3*3 => 346*748+3*748+3*346 = 262090
exbad2 : 349 * 751 = 262099 = (340+6+3)*(740+8+3)=340*740+(6+3)*740+(8+3)*340+9*11 => 340*740+9*740+11*340 = 262000
*/
namespace gen_aabb5 {
	LargeNumber N;
	LargeNumber a, b, m;
	LargeNumber aPrev,aGuessed,bPrev,bGuessed, tN;
	int ProgramPhase;
	enum PhaseStages
	{
		GuessA,
		GuessB,
		GuessAB,
	};

#define ParamCount 3
#define	x_i 0
#define y_i 1
	int xi = x_i;
	int yi = y_i;

	int CheckCandidateMatch_gen_aabb5_1(LargeNumber* tN__, LargeNumber** vLN, int Params, int pos, LargeNumber* TempRes1, LargeNumber* TempRes2)
	{
		//we will make many many tests. One of our running condition is that we know the size of a
		if (ProgramPhase == GuessA)
		{
			LargeNumber GuessedA;
			AddLN(&aPrev, vLN[xi], &GuessedA);
			if (IsLarger(&GuessedA, &a))
				return 2;
		}
		else if(ProgramPhase == GuessB)
		{
			LargeNumber GuessedB;
			AddLN(&bPrev, vLN[yi], &GuessedB);
			if (IsLarger(&GuessedB, &b))
				return 2;
		}
		if (vLN[xi]->Len + vLN[yi]->Len - 1 > tN.Len)
			return 0;

/*		if (ProgramPhase == GuessAB)
		{
			SetLN(&aGuessed,21632);
			SetLN(&bGuessed, 21801);
		}*/

		LargeNumber bGuessed_aGuessed,aGuessed_bPrev,bGuessed_aPrev,ab;
		MulLN(vLN[xi], vLN[yi], &bGuessed_aGuessed);
		MulLN(vLN[xi], &bPrev, &aGuessed_bPrev);
		MulLN(vLN[yi], &aPrev, &bGuessed_aPrev);
		AddLN(&bGuessed_aGuessed, &aGuessed_bPrev, &ab);
		AddLN(&bGuessed_aPrev, &ab, TempRes1);

		//we grow too large, time to reset
		if (ProgramPhase == GuessA || ProgramPhase == GuessB)
		{
			if (IsLarger(TempRes1, &tN))
				return 2;
		}

		if (ProgramPhase == GuessAB && IsLarger(TempRes1, &tN))
		{
/*
#ifdef _DEBUG
			NormalizeLN(TempRes1);
			assert(IsLarger(TempRes1, &tN) == 1);
#endif*/
			return 0;
		}/**/

		for (int i = 0; i <= pos; i++)
			if (TempRes1->Digits[i] != tN.Digits[i])
				return 0;

		return 1;
	}

	void InitForFixedTest(__int64 iA, __int64 iB)
	{
		ProgramPhase = GuessAB;
		for (int i = 0; i < a.Len; i++)
		{
			aPrev.Digits[i] = 1;
			bPrev.Digits[i] = 1;
		}
		aPrev.Len = a.Len;
		bPrev.Len = a.Len;
		LargeNumber ttn;
		MulLN(aPrev, bPrev, ttn);
		SubLN(&N, &ttn, &tN);
		NormalizeLN(&tN);

		__int64 x = ToIntLN(&aPrev);
		__int64 y = ToIntLN(&bPrev);
		__int64 itN = ToIntLN(&tN);
		__int64 Expected_a = iA - x;
		__int64 Expected_b = iB - y;
		//					GenNextCandidate = 0; //last digits can be 0 because we actually guessed the value combinations
		printf("Starting with x=%lld,y=%lld,tN=%lld.Expecting a=%lld,b=%lld\n", x, y, itN, Expected_a, Expected_b);
	}

	void DivTest_gen_aabb5(__int64 iA, __int64 iB)
	{
		__int64 iN = iA * iB;
		SetLN(N, iN);
		SetLN(a, iA);
		SetLN(b, iB);
		__int64 iSN = isqrt(iN);

		SetLN(aPrev, 0);
		SetLN(bPrev, 0);
		SetLN(tN, iN);

		LargeNumber EndSignal;

		LargeNumber* vLN[ParamCount];
		vLN[xi] = &aGuessed;
		vLN[yi] = &bGuessed;
		vLN[2] = &EndSignal;

		for (int i = 0; i < ParamCount; i++)
			SetLN(vLN[i], (__int64)0);
		InitLN(vLN[ParamCount - 1]);

		ProgramPhase = GuessA;

		InitForFixedTest(iA,iB);

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
			int Match = CheckCandidateMatch_gen_aabb5_1(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
			if (Match == 2)
			{
				LargeNumber Temp;
				
				//if we got here,  aGuessed or bGuessed is already too large
				ResetCandidateAtPos(vLN, ParamCount, AtPos, 1);

				CopyLN(aPrev, Temp);
				AddLN(&Temp, vLN[xi], &aPrev);

				//have to add 1 to one of the numbers due to messed up system :S
				//should remake this part later
				vLN[yi]->Digits[0] = (vLN[yi]->Digits[0] + 1) % 10;

				CopyLN(bPrev, Temp);
				AddLN(&Temp, vLN[yi], &bPrev);
				//have to add 1 to one of the numbers due to messed up system :S

				LargeNumber ttn;
				MulLN(aPrev, bPrev, ttn);
				SubLN(&N, &ttn, &tN);

				//reset the guessed part
				for (int i = 0; i < ParamCount; i++)
					SetLN(vLN[i], (__int64)0);
				InitLN(vLN[ParamCount - 1]);
				AtPos = 0;

				GenNextCandidate = 1;
				//focus on guessing the other side next
				xi = 1 - xi;
				yi = 1 - yi;
				if(ProgramPhase == GuessA)
					ProgramPhase = GuessB;
				else if (ProgramPhase == GuessB)
				{
					ProgramPhase = GuessAB;

					__int64 x = ToIntLN(&aPrev);
					__int64 y = ToIntLN(&bPrev);
					__int64 itN = ToIntLN(&tN);
					__int64 Expected_a = iA - x;
					__int64 Expected_b = iB - y;
//					GenNextCandidate = 0; //last digits can be 0 because we actually guessed the value combinations
					printf("Starting with x=%lld,y=%lld,tN=%lld.Expecting a=%lld,b=%lld\n", x, y, itN, Expected_a, Expected_b);
				}
			}
			else if (Match == 1)
			{
				CandidatesFound++;
				if (CandidatesFound % 100 == 0)
				{
					int chars[4] = { '\\','|','/','-' };
					printf("\r%c", chars[(CandidatesFound / 100) % 4]);
				}
				int SolutionFound = CheckSolution(TempRes1, tN);
				if (SolutionFound == 1)
				{
					SolutionsFound++;
					printf("\r%d / %d )sol : \t x:", SolutionsFound, CandidatesFound);
					PrintLN(aGuessed);
					printf("\t y:");
					PrintLN(bGuessed);
					printf("\t a:");
					PrintLN(aPrev);
					printf("\t b:");
					PrintLN(bPrev);
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
//	    }while( SolutionsFound == 0 && vLN[ParamCount-1]->Digits[0] == 0 );
		} while (vLN[ParamCount - 1]->Digits[0] == 0);

		if (SolutionsFound == 0)
			printf("\rNo Luck finding a solution\n");
		else
			printf("\rDone testing all possible solutions\n");

		printf("Steps taken %d. Candidates found %d\n\n", StepsTaken, CandidatesFound);

	}
};

void DivTestab5()
{
//	return; // NOT FINISHED
//	gen_aabb5::DivTest_gen_aabb5(23, 41);
//	gen_aabb5::DivTest_gen_aabb5(349, 751); // N = 262099 SN = 511
//	gen_aabb5::DivTest_gen_aabb5(6871, 7673); // N = 52721183 , SN = 7260
	gen_aabb5::DivTest_gen_aabb5(26729, 31793); // N = 849795097 , SN = 29151
//	gen_aabb5::DivTest_gen_aabb5(784727, 918839); // N = 721037771953
//	gen_aabb5::DivTest_gen_aabb5(6117633, 7219973);
//	gen_aabb5::DivTest_gen_aabb5(26729, 61781);
//	gen_aabb5::DivTest_gen_aabb5(11789, 61781);
}