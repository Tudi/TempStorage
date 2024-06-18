/*
Hope to never recheck again :
	There is an 'a' and a 'b' for whcih you can say that x1=10*x0 and y1=10*y0
		ex : (a-5*x)*(b+5*x+5*y)=N
	This file is made ot check how many such combinations exist : take a generic N=SQN*SQN+m. Increase a and decrease b by max 9, check if we could confirm x0=0 and y0=0
	
	This could boost search speed of x/y by a factor. Not much, but it's something

Conclusion : 
	for any selected 'a', there is a 'b' where the 'm' will be exactly dividable by the SCALER
	there is no way to guess which 'a','b' combo is the actually correct one ?
	you would need to test every x,y values SCALER times because : x*x+(b-a)*x+m=y*(a-x) contains both 'a' and 'b'
		this means SCALER*x*y combinations to test instead of (x*scaler)*(y*scaler)
	a good 'a','b' combo will also have an 'm' that is dividable by the SCALER
		ex : (a-5*x)*(b+5*(x+y))=N
			 a*b+5*(-x*b+5*x*(x+y)+a*(x+y))=N mod 5 + N div 5
			 a*b mod 5 = N mod 5
	not all scalers produce the same amount of combinations. 
			Ex: 2*3 produces on avg 2 combinations = 33%
			Ex: 5*5=25 produces on avg 20 combinations = 80%
			Ex: 2*3*5=30 produces on avg only 8 combinations = 26%
			Ex: 2*3*7=42 produces on avg 12 combinations = 28%
			Ex: 2*3*5*7=210 produces on avg 48 combinations = 22%

*/
#include "StdAfx.h"

void DivTestMod9_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN;
	__int64 b = N / a;	
	__int64 m = N - a * b;

	printf("N = %lld. SQN = %lld. m = %lld SQNSQN = %d \n", N, SQN, m, isqrt(SQN));
	__int64 searchedX = a - A;
	__int64 searchedY = B - b - searchedX;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);

	__int64 variantsGenerated = 0;
//#define SELECTED_SCALER	(2*3*5*5) // 40 out of 150
//#define SELECTED_SCALER	(2*3*5*2*3) // 48 out of 180
//#define SELECTED_SCALER	(2*3*5*7) // 48 out of 210
//#define SELECTED_SCALER	(2*3*5*2*5) // 80 out of 300
#define SELECTED_SCALER	(2*3*5*7*11) // 480 out of 2310
#define SELECTED_SCALER_CHECKED	10000

	a = a - SELECTED_SCALER; // to avoid negative m
	printf("Searching for scaled x=%lld\n", (a - A + SELECTED_SCALER - 1) / SELECTED_SCALER );
	printf("Searching for scaled y=%lld\n", (B - b - searchedX + SELECTED_SCALER - 1) / SELECTED_SCALER);

	for (__int64 inca = 0; inca < SELECTED_SCALER; inca++)
	{
		for (__int64 incb = 0; incb < SELECTED_SCALER; incb++)
		{
			__int64 ta = a + inca;
			__int64 tb = b - incb;
			__int64 xnow = ta - A;
			__int64 ynow = B - xnow - tb;
			__int64 isgoodab = ((xnow % SELECTED_SCALER) == 0) && ((ynow % SELECTED_SCALER) == 0);
			__int64 tm = N - ta * tb;
			__int64 tx = xnow / SELECTED_SCALER, ty = ynow / SELECTED_SCALER;
/*			{
				__int64 left = SELECTED_SCALER * (SELECTED_SCALER * tx * tx + (tb - ta + SELECTED_SCALER * ty) * tx) + tm;
				__int64 right = SELECTED_SCALER * ta * ty;
				left = left % SELECTED_SCALER;
				right = right % SELECTED_SCALER;
				if (left == 0 && right == 0)
				{
					printf("Possible solution a=%lld b=%lld\n", ta, tb);
				}
			}/**/
			if (isgoodab == 1)
			{
				printf("== this a,b combo (%lld,%lld) is expected to check out as good ==\n", ta, tb);
			}
			if ((tm % SELECTED_SCALER) == 0)
			{
				variantsGenerated++;
				printf("%lld)tx and ty value does not matter. Everything will seem to be good due to tm=%lld value\n", variantsGenerated, tm);
				printf("imagine %d rows printed below. Skipping them for a=%lld b=%lld\n", 
					SELECTED_SCALER_CHECKED * SELECTED_SCALER_CHECKED, ta, tb);
				// this code is to demonstrate that tm will not change no matter what x,y we pick
/*				for (__int64 tx = 0; tx < SELECTED_SCALER_CHECKED; tx++)
				{
					for (__int64 ty = 0; ty < SELECTED_SCALER_CHECKED; ty++)
					{
						__int64 tA = ta - tx * SELECTED_SCALER;
						__int64 tB = tb + (tx + ty) * SELECTED_SCALER;
						__int64 tN = tA * tB;
						__int64 tNmod = tN % SELECTED_SCALER;
					}
				}/**/
				continue;
			}
			int FoundAtLeastOneSolution = 0;
/*			for (tx = 0; tx< SELECTED_SCALER_CHECKED; tx++)
			{
				for(ty=0;ty< SELECTED_SCALER_CHECKED;ty++)
				{
					__int64 ttx = SELECTED_SCALER_CHECKED + tx; // to avoid 0 values
					__int64 tty = SELECTED_SCALER_CHECKED + ty; // to avoid 0 values
					__int64 left = SELECTED_SCALER * (SELECTED_SCALER * ttx * ttx + (tb - ta + SELECTED_SCALER * tty) * ttx) + tm;
					__int64 right = SELECTED_SCALER * tty * ta;
					__int64 leftmod = left % SELECTED_SCALER;
					__int64 rightmod = right % SELECTED_SCALER;
					if (leftmod == 0 && rightmod == 0)
					{
						FoundAtLeastOneSolution = 1;
						printf("Possible solution a=%lld b=%lld tx=%lld ty=%lld. ex=%lld ey=%lld\n", 
							ta, tb, tx, ty, xnow % 10, ynow % 10);
					}
				}
			}/**/
			if (FoundAtLeastOneSolution)
			{
				if ((tm % SELECTED_SCALER) != 0)
				{
					printf("Found a case where m is not dividable by scaler!\n");
				}
				variantsGenerated++;
			}
		}
	}
}

void DivTestMod9()
{
	//DivTestMod9_(5, 7);
	//DivTestMod9_(23, 41);
	DivTestMod9_(349, 751); // N = 262099 , SN = 511
	DivTestMod9_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestMod9_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestMod9_(784727, 918839);
	DivTestMod9_(3, 918839);
	DivTestMod9_(349, 918839);
} 