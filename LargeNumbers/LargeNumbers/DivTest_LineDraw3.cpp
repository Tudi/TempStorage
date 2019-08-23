#include "StdAfx.h"
#include <assert.h>

namespace LineDraw3
{
	__int64 A, B;
	__int64 N;
	__int64 a;
	__int64 b;
	__int64 m;
	__int64 z;
	__int64 x = 0;
	__int64 y;
	__int64 ExpectedX;
	__int64 ExpectedY;

	int CheckCandidateMatch_LineDraw()
	{
		// x * ( b + y ) - a * y + m = 0
		// y >= x * z + 1
		// z = b / a
		
		z = b / a;
		y = x * (z + 1);
		__int64 res = x * (b + y) - a * y + m;

		if (res == 0)
			return 1;

		x++;

		return 0;
	}

	void DivTest_LineDraw(__int64 iA, __int64 iB)
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
		z = b / a;
		y = 1;
		x = 1;
		ExpectedX = a - A;
		ExpectedY = B - b - z * ExpectedX; // we will remove more rows. we will go below the intended row count by the number of rows filled with the reminder
		printf("Expecting solution x=%lld y=%lld. N = %lld, SQRT(N) = %lld. Bruteforce steps %lld\n", ExpectedX, ExpectedY, N, iSN, (iSN - A) / 2);

		//start generating combinations and check if it's a feasable candidate
		int AtPos = 0;
		int SolutionsFound = 0;
		int CandidatesFound = 0;
		int StepsTaken = 0;
		do
		{
			StepsTaken++;
			int Match = CheckCandidateMatch_LineDraw();
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

void DivTestLineDraw3()
{
	LineDraw3::DivTest_LineDraw(23, 41); 
	// 108k
	LineDraw3::DivTest_LineDraw(349, 751); // N = 262099 SN = 511
	// 938k
	LineDraw3::DivTest_LineDraw(6871, 7673); // N = 52721183 , SN = 7260
	// 9M tries
	LineDraw3::DivTest_LineDraw(26729, 31793); // N = 849795097 , SN = 29151
	LineDraw3::DivTest_LineDraw(784727, 918839);
	LineDraw3::DivTest_LineDraw(6117633, 7219973);
	LineDraw3::DivTest_LineDraw(26729, 61781);
	LineDraw3::DivTest_LineDraw(11789, 61781);
}
