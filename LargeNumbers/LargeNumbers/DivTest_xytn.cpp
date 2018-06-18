#include "StdAfx.h"
#include <math.h>

int CheckMatch(int pos, LargeNumber *Left, LargeNumber *Right)
{
	// y * n = t * x + m + y * x
	for (int i = 0; i < pos; i++)
		if (Left->Digits[i] != Right->Digits[i])
			return 0;
	return 1;
}

int CheckSolution(LargeNumber *Left, LargeNumber *Right)
{
	if (Left->Len != Right->Len)
		return 0;
	for (int i = 0; i < Left->Len; i++)
		if (Left->Digits[i] != Right->Digits[i])
			return 0;
	return 1;
}

void CalcLeftSide(LargeNumber *n, LargeNumber *y, LargeNumber *Left)
{
	// y * n
	MulLN(n, y, Left);
}

void CalcRightSide(LargeNumber *t, LargeNumber *x, LargeNumber *y, LargeNumber *m, LargeNumber *Right)
{
	// t * x + m + y * x
	LargeNumber tx, yx, txm;

	MulLN(t, x, &tx);
	MulLN(y, x, &yx);

	AddLN(&tx, m, &txm);
	AddLN(&yx, &txm, Right);
}

void InitNewPos(LargeNumber *x, LargeNumber *y, bool IsFirst = false)
{
	x->Len++;
	y->Len++;
	x->Pos = x->Len - 1;
	y->Pos = y->Len - 1;
	x->Digits[x->Pos] = 0;
	if(IsFirst==false)
		y->Digits[y->Pos] = 1;
	else
		y->Digits[y->Pos] = 0;
}

int CanGenPosCombination(LargeNumber *x, LargeNumber *y)
{
	if (x->Digits[x->Pos] == 9 && y->Digits[y->Pos] == 9)
		return 0;
	return 1;
}

void GenPosCombination(LargeNumber *x, LargeNumber *y)
{
	if (x->Digits[x->Pos] < 9)
		x->Digits[x->Pos]++;
	else
	{
		x->Digits[x->Pos] = 0;
		y->Digits[y->Pos]++;
	}
}

int StepBack(LargeNumber *x, LargeNumber *y)
{
	if (x->Len == 0)
		return 1;

	x->Len--;
	y->Len--;
	x->Pos = x->Len - 1;
	y->Pos = y->Len - 1;

	return 0;
}

int CanInitNewPos(LargeNumber *x, LargeNumber *n)
{
	if (x->Len < n->Len)
		return 1;
	if (IsLarger(n, x))
		return 1;
	return 0;
}

void DivTest__Xytn(unsigned int iA, unsigned int iB)
{
	// (n*x+m)/(n-x)=y
	// y * n = t * x + m + y * x
	// x e [0,n] 		y e [0,N-t]		
	// x < y			t / n megadja az alap x / y aranyt. Ennek segitsegevel lehet allitani hogy x nagyjabol == y
	// A = n - x
	// B = t + y
	// max x if x==y 
	LargeNumber SQN, m, N;
	unsigned int iN = iA * iB;
	unsigned int iSQN = isqrt(iN);
	unsigned int im = iN - iSQN * iSQN;
	unsigned int ix = iSQN - iA;
	unsigned int iy = iB - iSQN;
	printf("N = %d\n", iN);
	printf("n = %d\n", iSQN);
	printf("Searching for x = %d, y = %d\n", ix, iy);
//	SetLN(N, iN);
//	SetLN(SQN, iSQN);
	SetLN(m, im);

	LargeNumber x, y;
	LargeNumber Left, Right;
	LargeNumber n, t;
	SetLN(n, iSQN);
	SetLN(t, iSQN);
	InitNewPos(&x, &y, true);

	//start generating combinations and check if it's a feasable candidate
	int SolutionsFound = 0;
	int CandidatesFound = 0;
	int StepsTaken = 0;
	int GeneratedNewCombo = 0;
	do
	{
		GeneratedNewCombo = 0;
		StepsTaken++;

		CalcLeftSide(&n, &y, &Left);
		CalcRightSide(&t, &x, &y, &m, &Right);

		if (CheckMatch(x.Len, &Left, &Right))
		{
			CandidatesFound++;
			if (CandidatesFound % 100 == 0)
				printf("#");
			//check if it is a solution ?
			if (CheckSolution(&Left, &Right))
			{
				//got a solution
				printf("\nGot a solution\n");
				SolutionsFound++;
				break;
			}
			//try to step further in generation
			if (CanInitNewPos(&x, &n))
			{
				InitNewPos(&x, &y, true);
				GeneratedNewCombo = 1;
			}
		}
		//roll a new candidate
		while (GeneratedNewCombo == 0)
		{
			if (CanGenPosCombination(&x, &y))
			{
				GenPosCombination(&x, &y);
				GeneratedNewCombo = 1;
			}
			else if (StepBack(&x, &y) == 1)
			{
				printf("Could not step back. Aborting generation\n");
				break;
			}
		}
	} while (SolutionsFound == 0 && GeneratedNewCombo == 1);

	if (SolutionsFound == 0)
		printf("\rNo luck finding a solution\n");
	else
		printf("\rDone testing all possible solutions\n");
	printf("Steps taken %d\n\n", StepsTaken);
}

void DivTest_xytn()
{
	//DivTest__Xytn( 11, 17 ); // SN = 143
	//DivTest__Xytn( 349, 751 ); // SN = 472
	//DivTest__Xytn( 6871, 7673 ); // SN = 12703
	DivTest__Xytn(26729, 31793); // SN = 162554		n = 29151 -> made 5 times more tries than going 1 by 1 and trying to divide
}
