#include "StdAfx.h"
#include <assert.h>
#include <math.h>

namespace DivTest_xyUpdate
{
	__int64 A, B;
	__int64 N;
	__int64 a;
	__int64 b;
	__int64 m;
	__int64 x = 0;
	__int64 y;

	int CheckCandidateMatch_xyUpdate()
	{
		/*
		b = ceil((double)N / a);
		if (a*b == N)
			return 1;
		a = floor((double)N / b);
		if (a*b == N)
			return 1;
		return 0;
		*/
		// x * a + m = y * ( b - x )
		// y > x
		// A = b - x		b = a + y

		x = b - a + 1;
		double dy = (double)(x * a + m) / (b - x);
		y = (__int64)ceil(dy);

		if (y <= x)
			y = x + 1;

		if ((b - x)*(a + y) == N)
			return 1;

		b = a + y;
		if (b >= B)
			return 1;
		a = N / b;
		if (a <= A)
			return 1;
		m = N - a * b;

		return 0;
	}

	void DivTest_xyUpdate(__int64 iA, __int64 iB)
	{
		// x * b - z * ( x + y ) * ( a - x ) + m - a + x = 0
		// we are searching for an X by guessing the Y. z = b / a
		// At the end a = A		A = a - x	B = b + x + y.....
		// a and b can start off from a = sqrt(N) b = N / sqrt(N)

		A = iA;
		B = iB;
		N = iA * iB;
		__int64 iSN = isqrt(N);
		a = iSN;
		b = N / a;
		m = N - a * b;
		printf("N = %lld, SQRT(N) = %lld. Bruteforce steps %lld\n", N, iSN, (iSN - A) / 2);

		//start generating combinations and check if it's a feasable candidate
		int AtPos = 0;
		int SolutionsFound = 0;
		int CandidatesFound = 0;
		int StepsTaken = 0;
		do
		{
			StepsTaken++;
			int Match = CheckCandidateMatch_xyUpdate();
			if (Match == 1)
			{
				SolutionsFound++;
				printf("\rSolution found after %d steps\n", StepsTaken);
				break;
			}
		} while (1);

		if (SolutionsFound == 0)
			printf("\rNo Luck finding a solution\n");
	}
};

void DivTestxyUpdate()
{
	DivTest_xyUpdate::DivTest_xyUpdate(23, 41);
	// 108k
	DivTest_xyUpdate::DivTest_xyUpdate(349, 751); // N = 262099 SN = 511
	// 938k
	DivTest_xyUpdate::DivTest_xyUpdate(6871, 7673); // N = 52721183 , SN = 7260
	// 9M tries
	DivTest_xyUpdate::DivTest_xyUpdate(26729, 31793); // N = 849795097 , SN = 29151
	DivTest_xyUpdate::DivTest_xyUpdate(784727, 918839);
	DivTest_xyUpdate::DivTest_xyUpdate(6117633, 7219973);
	DivTest_xyUpdate::DivTest_xyUpdate(26729, 61781);
	DivTest_xyUpdate::DivTest_xyUpdate(11789, 61781);
}
