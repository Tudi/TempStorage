#include "StdAfx.h"
#include <assert.h>

namespace LineDraw2
{
	__int64 A, B;
	__int64 N;
	__int64 a;
	__int64 b;
	__int64 m;
	__int64 z;
	__int64 PrevX = 0;
	__int64 y;
	__int64 w;
	__int64 ExpectedX;
	__int64 ExpectedY;

	int CheckCandidateMatch_LineDraw()
	{
		// right now only works for (b+x)/(a-x) < 2
		// m(x) = y * (b + x ) = x * x + x * (b - a - y) + m - y * b 
		//  x * b - z * x * ( a - x ) + m = y * ( b + x * z )
		// z >= 2    =>     x * b - z * x ( a - x ) + m = y * ( b + x * z ) + y * w + w ( a - x )
		// we are searching for an X by guessing the Y
		// At the end A = a - x + y		B = b + x
		// a and b can start off from a = sqrt(N) b = N / sqrt(N)

//		__int64 PrevZ = z;
//		z = (b + PrevX) / (a - PrevX);
//		assert(z == 1);

		//how many rows (len b) do we need to remove and add as columns ( len a ) to be able to fill 'y' number of rows with the remains
		z = b / a;
		__int64 sa = z;
		__int64 sb = (b - z * a - y * z + w * a);
		__int64 sc = m - y * b - y * w - w * a;
		__int64 x = (-sb + isqrt4(sb * sb - 4 * sa * sc)) / 2;

		__int64 tA = a - x + y;

		if (N % tA == 0) 
		{
			//			printf("Found solution : %lld \n", x);
			return 1;
		}
		// because w can be anything below z
		for (int i = 1; i < z; i++)
			if (N % (tA + i) == 0)
				return 1;

/*		PrevX = x;
		//if we stepped into a new zone, we should remake the calculations to abvoid stepping over a solution
		__int64 CurZ = (b + x * z) / (a - x + y);
		if (CurZ != z)
		{
			//should remake the calculations now based on	N = a * CurZ * a
			// !!!! only valid if we can generate a valid A and B with the Z increments !
			// if the new b is pair, than we should increase
			__int64 iSN = isqrt(N / CurZ); 
			a = iSN;
//			assert( a % 2 == 0 ); // because we are looking for a factor that is impar. We decrease a by x
			b = N / a;
			m = N - a * b;
			z = b / a;
			ExpectedX = ( B - b ) / z;
//			assert((B - b) % z == 0);
			ExpectedY = A - a + z * ExpectedX;
			printf("Expecting solution x=%lld y=%lld\n", ExpectedX, ExpectedY);
//			assert( z == CurZ );
			y = 1;
			PrevX = 0;
			return 0;
		}/**/

		//failed to calculate a valid x, try a new value
		y++;

		return 0;
	}

	void DivTest_LineDraw(__int64 iA, __int64 iB)
	{
		// m(x) = i * (b + x ) = x * x + x * (b - a - i) + m - i * b
		// A = a - x + i		B = b + x

		A = iA;
		B = iB;
		N = iA * iB;
		__int64 iSN = isqrt(N);
		a = iSN;
		b = N / a;
		m = N - a * b;
		z = b / a;
		y = 1; // number of rows we intend to fill with the sum of reminders
		w = 0;
		ExpectedX = ( iB - b ) / z; // we keep adding columns to b until we reach B
		ExpectedY = iA - (a - z * ExpectedX); // we will remove more rows. we will go below the intended row count by the number of rows filled with the reminder
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

void DivTestLineDraw2()
{
	//	LineDraw::DivTest_LineDraw(7, 127);
	// 108k
	LineDraw2::DivTest_LineDraw(349, 751); // N = 262099 SN = 511
	// 938k
	LineDraw2::DivTest_LineDraw(6871, 7673); // N = 52721183 , SN = 7260
	// 9M tries
	LineDraw2::DivTest_LineDraw(26729, 31793); // N = 849795097 , SN = 29151
	LineDraw2::DivTest_LineDraw(784727, 918839);
	LineDraw2::DivTest_LineDraw(6117633, 7219973);
	LineDraw2::DivTest_LineDraw(26729, 61781);
	LineDraw2::DivTest_LineDraw(11789, 61781);
}
