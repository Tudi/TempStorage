#include "StdAfx.h"

/*
a=(B-A)/2
b=(B+A)/2
x*(2*at+y)+m=y*(2*bt+y)
*/
namespace gen_sqsq3 {

    int CheckHasSolutionLessThan10(__int64 at, __int64 bt, __int64 m, __int64 a, __int64 b)
    {
        for (__int64 x = 0; x < 10;x++)
            for (__int64 y = 0; y < 10; y++)
            {
                __int64 Left = x * (2 * at + x) + m;
                __int64 Right = y * (2 * bt + y);
//printf("%lld=%lld for x=%lld and y=%lld\n", Left, Right, x, y);
                if (Left % 10 == Right % 10)
                {
                    //full calculation to see if one of the at/bt is indeed a solution
                    if (at + x == a || bt + y == b)
                        return 1;
                }
            }
        return 0;
    }

	void DivTest_gen_sqsq3(__int64 iA, __int64 iB)
	{
		__int64 iN = iA * iB;
		__int64 iSN = isqrt(iN);
        __int64 at = 1; // could start with a smaller number
        __int64 bt = isqrt(iN + at*at);
        __int64 m = iN + at * at - bt * bt;
        __int64 a = (iB - iA) / 2;
        __int64 b = (iB + iA) / 2;
        __int64 x = a - at;
        __int64 y = b - bt;
        printf("Trying to find a(%lld),b(%lld) for A=%lld, B=%lld, N=%lld\n", a, b, iA, iB, iN);

		__int64 StepsTaken = 0;
//		for (__int64 n = 0; n <= iSN / 10; n++)
        while (1)
		{
            int IsResult = CheckHasSolutionLessThan10(at, bt, m, a, b);
            if (IsResult == 1)
            {
                printf("Maybe found a result at = %lld bt = %lld\n",at,bt);
                break;
            }
/*            {
                //            at = at + 1; // used to verify if what we are doing is ok
                //            __int64 z = (isqrt(4 * at * at + 4 * (2 * 10 * bt + 100 + m) - 2 * at) / 2; // convert the previous 2 * 10 * bt + 10 * 10 into extending at
                __int64 z = (isqrt(at * at + 2 * 10 * bt + 100 + m) - at) - 1; // convert the previous 2 * 10 * bt + 10 * 10 into extending at
                //            __int64 z = (isqrt(at * at + 2 * 9 * bt + 4*9*9) - at); // convert the previous 2 * 10 * bt + 10 * 10 into extending at
                at = at + z;
                //            at = at + at / 2; // sadly this is not working
                bt = isqrt(iN + at*at);
                m = iN + at * at - bt * bt;
            }/**/
            {
                bt += 10;
                at = isqrt(bt * bt - iN);
                m = iN + at * at - bt * bt;
            }
            __int64 x = a - at;
            __int64 y = b - bt;
            if (x < 0 || y < 0)
            {
                printf("Failed to generate a result. x = %lld, y = %lld\n",x,y);
                break;
            }
            StepsTaken++;
		}
		printf("Steps taken : %lld\n\n", StepsTaken * 100);
	}
};

void DivTestsqsq3()
{
        gen_sqsq3::DivTest_gen_sqsq3(23, 41);
		gen_sqsq3::DivTest_gen_sqsq3(349, 751); // N = 262099 SN = 511
		gen_sqsq3::DivTest_gen_sqsq3(6871, 7673); // N = 52721183 , SN = 7260
	    gen_sqsq3::DivTest_gen_sqsq3(26729, 31793); // N = 849795097 , SN = 29151
		gen_sqsq3::DivTest_gen_sqsq3(784727, 918839); // N = 721037771953
		gen_sqsq3::DivTest_gen_sqsq3(6117633, 7219973); // N = 44169145083909
		gen_sqsq3::DivTest_gen_sqsq3(26729, 61781);
		gen_sqsq3::DivTest_gen_sqsq3(11789, 61781);
}