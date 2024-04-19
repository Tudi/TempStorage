#include "StdAfx.h"

// this approach allows us to work on lower bits without needing to do many bit multiplications
// this approach is NOT faster than iterating through x or y 1 by 1
// ex : you can repeatedly use the lower 64 bits and only use all bits to check if x,y is solution and advance to next 64 bits
// x^2 + x*y + m = y*n
// (x+xp)^2+(x+xp)*(y+yp)+m=(y+yp)*n
// x^2 + (2*xp+yp+y)*x+xp*y=y*n+(yp*n-xp^2+xp*yp+m)

// Probably could use a lookup table to quickly check what xy might lead to a solution and only check those instead iterating through 2500 checks
// 100 SQN and 100 m combinations that each have a list of possible x,y to check for full SQN and m
// if you really want to stretch it, you could use only certain SQN values and push the rest into m. Ex : 30*30+43==29*29+102
// you can move y into SQN as SQN2 = SQN + y/2. You can move x into SQN as SQN2=SQN-x ... always obtain SQN % 100000 = 0 ... only need the lookup table for m

// by having a custom chosen SQN + m, with a lookup table you can repeatedly recheck the same x-y combinations while only XP-YP changes in the formula
// that is like counting from 1 to SQN/2
// Ex : SQN = 84913900000 m = 73063256789 until x<SQN/2, we : do the y-x 40000 checks ( if y solution), increase yp by 100000
//												x>SQN/2, we : do the x-y 40000 checks ( if x solution), increase xp by 100000


static int isXSolution(__int64 x, __int64 SQN, __int64 m, __int64 N)
{
	if (x >= SQN)
	{
		return 0;
	}
	__int64 y = (x * x + m) / (SQN - x);
	__int64 tN = (SQN - x) * (SQN + x + y);
	return tN == N;
}

static int isYSolution(__int64 y, __int64 SQN, __int64 m, __int64 N)
{
	__int64 x_a = 1;
	__int64 x_b = y;
	__int64 x_c = m - SQN * y;
	__int64 x = (-x_b + isqrt(x_b * x_b - 4 * x_a * x_c)) / (2 * x_a);
	__int64 tN = (SQN - x) * (SQN + x + y);
	return tN == N;
}

static void AdvanceXY(__int64& xp, __int64& yp, __int64& SQN, __int64 m, __int64 N, __int64 &checksMade)
{
	__int64 left_c1 = 2 * xp + yp;
	__int64 right_c1 = (yp * SQN - (xp * xp + xp * yp + m));

	// cause we know x,y can only be pair or impair
	__int64 xs = 0, ys = 0;
	if (((SQN + xp) & 1) == 0)
	{
		xs = 1;
	}
	if (((SQN + xp + xs + yp) & 1) == 0)
	{
		ys = 1;
	}

	// 4 checks for 10, 40 checks for 100, 400 checks for 1000
	const __int64 checkDigitsUntil = 100;
	for (__int64 y = ys; y < checkDigitsUntil; y += 2)
	{
		for (__int64 x = xs; x < checkDigitsUntil; x += 2)
		{
//			__int64 left = x * x + (left_c1 + y) * x + xp * y;
//			__int64 right = y * SQN + right_c1;
			__int64 tx = x + xp, ty = y + yp; // could skip this step altogether. We only care to set the lower digits properly
											  // and we only care about either x(after tx>n/3) or y(at the begining) from these 2 numbers
			__int64 left = tx * tx + ty * tx + m;
			__int64 right = ty * SQN;
			if ((left % checkDigitsUntil) == (right % checkDigitsUntil))
			{
				checksMade++;
				// we only need to check for solution based on the one we are actually testing 
				// at the beginning, while x/y > 1, we only check if y might provide a solution
				// after about x>n/3, we only need to check if x might provide a solution
				if (isXSolution(x + xp, SQN, m, N) || isYSolution(y + yp, SQN, m, N))
				{
					xp += x;
					yp += y;
					SQN = 0;
					return;
				}
			}
		}
	}
	__int64 new_xp, new_yp;
	{
		__int64 y = checkDigitsUntil;
		__int64 ty = yp + y;
		__int64 x_a = 1;
		__int64 x_b = ty;
		__int64 x_c = m - SQN * ty;
		new_xp = (-x_b + isqrt(x_b * x_b - 4 * x_a * x_c)) / (2 * x_a);
	}
	// should start happening around tx>n/3
	{
		__int64 x = checkDigitsUntil;
		__int64 tx = xp + x;
		new_yp = (tx * tx + m) / (SQN - tx);
	}
	// should be same as (xp+checkDigitsUntil)<SQN/3
	if (new_xp - xp - checkDigitsUntil > new_yp - yp - checkDigitsUntil)
	{
		xp = new_xp;
		yp += checkDigitsUntil;
	}
	else
	{
		xp += checkDigitsUntil;
		yp = new_yp;
	}
}

void DivTestabxy_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	__int64 xp = 0;
	__int64 yp = 0;

	__int64 stepsMade = 0;
	__int64 checksMade = 0;

	printf("N = %lld. SQN = %lld. m = %lld SQNSQN = %d \n", N, SQN, m, isqrt(SQN));
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);
	while (SQN != 0)
	{
		AdvanceXY(xp, yp, SQN, m, N, checksMade);
		stepsMade++;
		if (SQN != 0)
		{
			printf("\t %lld) xnow=%lld ynow=%lld checkssofar=%lld\n", stepsMade, xp, yp, checksMade);
		}
	}
	printf("Found A,B in %lld steps, %lld checks using x=%lld, y=%lld\n\n", stepsMade, checksMade, xp, yp);
}

void DivTestabxy()
{
	//DivTestabxy_(5, 7);
	DivTestabxy_(23, 41);
	DivTestabxy_(349, 751); // N = 262099 , SN = 511
	DivTestabxy_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestabxy_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestabxy_(784727, 918839);
	DivTestabxy_(3, 918839);
	DivTestabxy_(349, 918839);
}
