#include "StdAfx.h"

/*
trying to extract known information from the unknown part. This is based on substraction
conclusion : it is 10 times worse than trying y value 1 by 1
*/
namespace LineDraw9 {

	void DivTest_LineDraw9(__int64 iA, __int64 iB)
	{
		__int64 iN = iA * iB;
		__int64 iSN = isqrt(iN);

		__int64 ia = iSN;
		__int64 ib = ia;
		__int64 im = iN - ia * ib;

		__int64 ix1 = ia - iA;
		__int64 iy1 = iB - ib - ix1;
		printf("Expecting solution x1 = %lld, y1 = %lld. N=%lld. Bruteforce trycount %lld\n", ix1, iy1, iN, iSN / 2);

		int FoundSolution = 0;
		int TryCount = 0;
		while (FoundSolution != 1)
		{
			for (__int64 x = 0; x < 10; x++)
			{
				for (__int64 y = 0; y < 10; y++)
				{
					TryCount++;
					__int64 left, right;
					//would be enough to calculate last digit
					if (im > 0)
					{
						left = ib * x + im;
						right = (x + y) * (ia - x);
					}
					else
					{
						left = ib * x;
						right = (x + y) * (ia - x) - im;
					}
					//might be a full y. can we obtain a valid x from this y ?
					if (left % 10 == right % 10)
					{
						__int64 sqb = (ib + y - ia);
						__int64 delta = sqb * sqb - 4 * 1 * (im - y * ia);
						__int64 deltasq = isqrt(delta);
						__int64 x12 = (-sqb + deltasq) / (2 * 1);

						__int64 tA = ia - x12;
						__int64 tB = ib + x12 + y;

						if (tA * tB == iN)
						{
							FoundSolution = 1;
							printf("%d)Found solution : %lld * %lld \n", TryCount, tA, tB);
							x = y = 10;
							break;
						}
					}
				}
			}
			ia -= 10;
			ib += 20;
			ia = iN / ib;
			im = iN - ia * ib;
		}
	}
};

void DivTestLineDraw9()
{
	LineDraw9::DivTest_LineDraw9(23, 41);
	LineDraw9::DivTest_LineDraw9(349, 751); // N = 262099 SN = 511
	LineDraw9::DivTest_LineDraw9(6871, 7673); // N = 52721183 , SN = 7260
	LineDraw9::DivTest_LineDraw9(26729, 31793); // N = 849795097 , SN = 29151
	LineDraw9::DivTest_LineDraw9(784727, 918839); // N = 721037771953
	LineDraw9::DivTest_LineDraw9(6117633, 7219973);
	LineDraw9::DivTest_LineDraw9(26729, 61781);
	LineDraw9::DivTest_LineDraw9(11789, 61781);
}