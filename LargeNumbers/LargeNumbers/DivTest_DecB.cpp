#include "StdAfx.h"
#include "SharedDecl.h"

// I have this file to avoid creating it in the future : no advantage can be gained by multiplying ( or dividing ) N. You can trace it back to x1. Because length of N does not decrease
//				The only advantage can be gained if more info gets known about A or B

// N=A*B	N2=A*(0.8*B) .. we aim to reach A with B
//double x2fromx1 = x1 - n1 + n2;
//double y2fromy1 = c2 * y1 + (c2 - 1) * x1 + (1 + c2) * n1 - 2 * n2;
// same as no scale : since y2 increases by some c2 value, the number of increases is the same as for y1, it's just the size of the jump that is smaller

static void MakeProgress1(IterationStateHolder& sh)
{
	sh.fNNow = sh.AInitial * sh.cf1 * sh.BInitial * sh.cf2;
	sh.fSQNNow = sqrt(sh.fNNow);
	sh.Nnow = (__int64)sh.fNNow;
	sh.SQNnow = (__int64)sh.fSQNNow;
	sh.fmNow = sh.fNNow - sh.SQNnow * sh.SQNnow;

	__int64 xRequired = sh.SQNnow - sh.AInitial;
	double yRequired = sh.AInitial + sh.BInitial * sh.cf2 - 2 * sh.SQNnow;
	printf("\t %lld)searching for x=%lld y=%f. c2(sum)=%f. lastr=%f. final=%f nnow=%lld\n",
		sh.ChecksMade, xRequired, yRequired, sh.cf2, (double)sh.ANow / sh.BNow, 
		(double)sh.AInitial / sh.BInitial, sh.SQNnow);

	size_t bAIsImPair = (sh.AInitial & sh.c1) & 1;
	size_t bBIsImPair = (sh.BInitial & sh.c2) & 1;
	size_t bSQNIsImpair = (sh.SQNnow & 1);
	size_t bXneedsToBeImPair = (bAIsImPair + bSQNIsImpair) & 1;
	size_t bYNeedsToBeImPair = (bBIsImPair + bSQNIsImpair + bXneedsToBeImPair) & 1;
	__int64 y1Min = bYNeedsToBeImPair;
	// set min y : 
	//	needs to be always greater than 0
	//  if m >= SQN, y needs to be greater than 1
	if (y1Min == 0 || sh.mnow > sh.SQNnow)
	{
		y1Min += 2;
	}
//	__int64 x1fory1 = (isqrt(y1Min * y1Min - 4 * (sh.mInitial - y1Min * sh.SQNInitial)) - y1Min) / 2;
	
	__int64 x1min = sh.SQNInitial - sh.SQNnow;
	y1Min = (x1min * x1min + sh.mInitial) / (sh.SQNInitial - x1min);
	if (bYNeedsToBeImPair == 1 && (y1Min & 1) == 0)y1Min++;
	if (bYNeedsToBeImPair == 0 && (y1Min & 1) == 1)y1Min++;
	if (y1Min == 0 || (sh.mInitial > sh.SQNInitial && y1Min < 2))
	{
		y1Min += 2;
	}

	GenXYCombos(sh, sh.SQNInitial, sh.mInitial, y1Min);

	for (size_t i = 0; i < _countof(sh.xy); i++)
	{
		__int64 x2fromx1 = sh.xy[i].x - sh.SQNInitial + sh.SQNnow;
		double y2fromy1 = sh.cf2 * sh.xy[i].y + (sh.cf2 - 1) * sh.xy[i].x + (1 + sh.cf2) * sh.SQNInitial - 2 * sh.SQNnow;

		if (sh.xy[i].x == sh.xy[i].ix && sh.xy[i].y == sh.xy[i].iy)
		{
			sh.StopIterating = 1;
			printf("Wow,we found a solution after %lld checks. x=%lld\n\n", sh.ChecksMade, sh.xy[i].ix);
			return;
		}
		else
		{
			sh.ChecksMade++;
			continue; // yeah. Since we are just deriving it from x1, we can directly check x1
		}
	}
	size_t lastValidIndex = _countof(sh.xy) - 1;
	__int64 x2fromx1 = sh.xy[lastValidIndex].x - sh.SQNInitial + sh.SQNnow;
	double y2fromy1 = sh.cf2 * sh.xy[lastValidIndex].y + (sh.cf2 - 1) * sh.xy[lastValidIndex].x + (1 + sh.cf2) * sh.SQNInitial - 2 * sh.SQNnow;
	__int64 tA = sh.SQNnow - x2fromx1;
	double tB = sh.SQNnow + x2fromx1 + y2fromy1;

	sh.ANow = tA;
	sh.BNow = tB;
	sh.cf2 = sh.cf2 * tA / tB;
}

void DivTestDecB_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	printf("N = %lld. SQN = %lld. m = %lld SQNSQN = %d \n", N, SQN, m, isqrt(SQN));
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);

	IterationStateHolder ish;
	ish.AInitial = A;
	ish.BInitial = B;
	ish.NInitial = N;
	ish.SQNInitial = SQN;
	ish.mInitial = m;
	ish.c1 = 1;
	ish.c2 = 1;
	ish.cf1 = 1.0f;
	ish.cf2 = 1.0f;
	ish.StopIterating = 0;
	ish.ChecksMade = 0;

	while (ish.StopIterating == 0)
	{
		MakeProgress1(ish);
	}
}

void DivTestDecB()
{
	//DivTestDecB_(5, 7);
	DivTestDecB_(23, 41);
	DivTestDecB_(349, 751); // N = 262099 , SN = 511
	DivTestDecB_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestDecB_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestDecB_(784727, 918839);
	DivTestDecB_(3, 918839);
	DivTestDecB_(349, 918839);
}
