#include "StdAfx.h"
#include <assert.h>

namespace LineDraw
{
	__int64 A,B;
	__int64 N;
	__int64 a;
	__int64 b;
	__int64 m;
	__int64 z;
	__int64 PrevX = 0;
	__int64 y;
	__int64 ExpectedX;
	__int64 ExpectedY;

	int CheckCandidateMatch_LineDraw()
	{
		// x * ( b - ( a - x ) ) + m = ( z - 1 + y ) * ( a - x )

		// we are searching for an X by guessing the Y. z = b / a
		// At the end	A = a - x		B = b + x * z + y.....
		// a and b can start off from a = sqrt(N) b = N / sqrt(N)
		// !! conclusion : using Z does not bring a benefit ! Maybe Z can be integrated into the formula directly into y ?
		// todo : test y = y + z
		// same as saying x * b = y * ( a - x ) .....		x * ( b + y ) - a * y + m = 0		and y can be z + 1, 2 * z + 2

		__int64 x;
		__int64 sa = 1;
		__int64 sb = (b - a + z - 1 + y);
		__int64 sc = m - z * a + a - y * a;
		x = (-sb + isqrtNegative(sb * sb - 4 * sa * sc)) / 2; // this is the increase on a side and not b

		//check back. since the remaining value does not increase liniarly
		__int64 mx = m + x * (b - (a - x)); // we are expecting this to be y * ( a - x )
		__int64 yax = (z - 1 + y) * (a - x);

		__int64 mxp = m + (x + 1)* (b - (a - (x + 1))); // we are expecting this to be y * ( a - x )
		__int64 yaxp = (z - 1 + y) * (a - (x + 1));

		assert(mx <= yax && mxp > yaxp);

		if (x > 0 && x < a)
		{
			__int64 tB = b + z * x + y;
			__int64 tA = a - x;
			__int64 tN = tA * tB;

			if (x == ExpectedX || tN == N)
			{
				//			printf("Found solution : %lld \n", x);
				return 1;
			}

			//		__int64 CurZ = ( b + z * x + y ) / ( a - x );
			__int64 CurZ = tB / tA;
			if (CurZ > z && CurZ < A)
			{
				//should remake the calculations now based on	N = a * CurZ * a
				// !!!! only valid if we can generate a valid A and B with the Z increments !
				// if the new b is pair, than we should increase
				__int64 iSN = isqrt(N / CurZ);
				a = iSN;
				b = N / a;
				m = N - a * b;
				z = b / a; // must be a whole number. No subdivisions
				ExpectedX = a - A;
				ExpectedY = B - b - z * ExpectedX; // we will remove more rows. we will go below the intended row count by the number of rows filled with the reminder
//				printf("New expected x=%lld y=%lld\n", ExpectedX, ExpectedY);
				y = ( z - 1) + 1;
//				y = 1;
				return 0;
			}/**/
		}

		//failed to calculate a valid x, try a new value
//		y++;
		y += z;

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
		ExpectedX = a - A;
		ExpectedY = B - b - z * ExpectedX; // we will remove more rows. we will go below the intended row count by the number of rows filled with the reminder
		printf("Expecting solution x=%lld y=%lld. N = %lld, SQRT(N) = %lld. Bruteforce steps %lld\n", ExpectedX, ExpectedY, N, iSN, (iSN - A)/2);

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

void DivTestLineDraw()
{
	LineDraw::DivTest_LineDraw(7, 127);
	// 108k
	LineDraw::DivTest_LineDraw(349, 751); // N = 262099 SN = 511
	// 938k
	LineDraw::DivTest_LineDraw(6871, 7673); // N = 52721183 , SN = 7260
	// 9M tries
	LineDraw::DivTest_LineDraw(26729, 31793); // N = 849795097 , SN = 29151
	LineDraw::DivTest_LineDraw(784727, 918839); 
	LineDraw::DivTest_LineDraw(6117633, 7219973);
	LineDraw::DivTest_LineDraw(26729, 61781);
	LineDraw::DivTest_LineDraw(11789, 61781);
}
