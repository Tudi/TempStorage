#include "StdAfx.h"

/*
N(0)=A*B=(a+0)*(b+0)
N(1)=(a+10)*(b+10)=ab+10a+10b+100
N(2)=(a+10+10)*(b+10+10)=ab+20a+20b+400
N(n)=(a+n*10)*(b+n*10)=a*b+n*10*a+n*10*b+100*n*n
*/
namespace gen_ab6 {

	void DivTest_gen_ab6(__int64 iA, __int64 iB)
	{
		__int64 iN = iA * iB;
		__int64 iSN = isqrt(iN);
		printf("this method does not work. Leaving it here in the pit of failures\n");
		__int64 StepsTaken = 0;
		for (__int64 n = 0; n <= iSN / 10; n++)
		{
			__int64 tN = iN - 100 * n * n;
			__int64 tenn = 10 * n;
			for (__int64 a = 0; a < 10; a++)
				for (__int64 b = 0; b < 10; b++)
				{
					StepsTaken++;
					__int64 Right = a * b + tenn * (a + b);
					if (tN == Right)
					{
						__int64 tA = a + n * 10;
						__int64 tB = b + n * 10;
						printf("Solution : A=%lld B=%lld", tA, tB);
					}
					if (Right > tN)
					{
//						printf("Maybe we missed our chance to find a solution\n");
					}
					__int64 tA = a + n * 10;
					__int64 tB = b + n * 10;
					if( tA * tB == iN )
						printf("Solution2 : A=%lld B=%lld", tA, tB);
				}
		}
		printf("Steps taken : %lld\n", StepsTaken);
	}
};

void DivTestab6()
{
	gen_ab6::DivTest_gen_ab6(23, 41);
	//	gen_ab6::DivTest_gen_ab6(349, 751); // N = 262099 SN = 511
	//	gen_ab6::DivTest_gen_ab6(6871, 7673); // N = 52721183 , SN = 7260
	//gen_ab6::DivTest_gen_ab6(26729, 31793); // N = 849795097 , SN = 29151
	//	gen_ab6::DivTest_gen_ab6(784727, 918839); // N = 721037771953
	//	gen_ab6::DivTest_gen_ab6(6117633, 7219973);
	//	gen_ab6::DivTest_gen_ab6(26729, 61781);
	//	gen_ab6::DivTest_gen_ab6(11789, 61781);
}