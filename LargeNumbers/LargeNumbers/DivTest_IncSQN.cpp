/*
Hope to never recheck again :
	- since func x*x+(b-a+y)*x+m-y*a=0 mostly depends on y*a, we want to maximize that
	- we will increase 'a' instead decreasing it
	- curious if this is any better than just moving forward with 'a' and 'b' by aplying x,y

Conclusion :
	- only safe increase of 'a' is y..which makes it count 1 by 1 the y
*/

#include "StdAfx.h"
#include <vector>

void DivTestIncSQN_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN;
	__int64 b = SQN;
	__int64 m = N - a * b;

	printf("================\n");
	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 x = a - A;
	__int64 y = B - b - x;
	printf("Looking for x=%lld y=%lld\n", x, y);

	__int64 iterationsMade = 1;
	__int64 xprev = 0, xnow, ynow = 1;
	__int64 ta = a, tb = b, tm = m;
	if (m > tb)
	{
		ta++; tb++;
		tm = N - ta * tb;
	}
	__int64 timetoflip = 0;
	while (1)
	{
		if (timetoflip == 0)
		{
			__int64 isXOdd = 1 - (ta & 1);
			__int64 isYEven = (tb + isXOdd) & 1;
			if (isYEven == 1)
			{
				ynow = 2;
			}
			else
			{
				ynow = 1;
			}
			__int64 eq_b = (tb - ta + ynow);
			__int64 eq_delta = eq_b * eq_b - 4 * (tm - ta * ynow);
			xnow = (isqrt(eq_delta) - eq_b) / 2;
			__int64 tA = ta - xnow;
			__int64 tB = tb + xnow + ynow;
			if (tA * tB == N)
			{
				printf("Found the solution after %lld loops\n", iterationsMade);
				return;
			}
			if (xnow + 2 <= xprev)
			{
//				timetoflip = 1;
			}

			__int64 xwouldbegood = ta - A;
			__int64 ywouldbegood = B - tb - xwouldbegood;

			// this would be the proper limit of ta,tb increase, but then what are we gaining ?
			// we are guessing y 1 by 1
			{
				ta += (ynow + 1) / 2;
				tb += (ynow + 1) / 2;
				tm = N - ta * tb;
//				ynow += 2;
			}
/*			{
				// convert the (x+y)*(a-x) into m
				__int64 floatingpart = (xnow + ynow) * (ta - xnow);
				//			__int64 floatingpart = (xnow / 2 + ynow) * (ta - xnow / 2);
				__int64 fakeN = N + floatingpart;
				__int64 newSQN = isqrt(fakeN);
				ta = tb = newSQN;
				tm = N - ta * tb;
			}/**/

			xprev = xnow;
		}
		iterationsMade++;
	}
}

void DivTestIncSQN()
{
	//DivTestIncSQN_(5, 7);
	//DivTestIncSQN_(23, 41);
	DivTestIncSQN_(349, 751); // N = 262099 , SN = 511
	DivTestIncSQN_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestIncSQN_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestIncSQN_(784727, 918839); // N = 721037771953 , SN = 849139
	DivTestIncSQN_(26729, 918839);
	DivTestIncSQN_(6871, 918839);
	//	DivTestIncSQN_(3, 918839);
	DivTestIncSQN_(349, 918839);
}