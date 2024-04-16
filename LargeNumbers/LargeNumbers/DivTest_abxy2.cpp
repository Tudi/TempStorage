#include "StdAfx.h"

// this approach allows us to work on lower bits without needing to do many bit multiplications
// compared to previous version, this modifies SQN and m in order to approach either x or y. This version races for x

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

static void AdvanceX(__int64& SQN, __int64 &m, __int64 N, __int64& checksMade)
{
	// cause we know x,y can only be pair or impair
	__int64 xs = 0, ys = 0;
	if (((SQN) & 1) == 0)
	{
		xs = 1;
	}
	if (((SQN + xs) & 1) == 0)
	{
		ys = 1;
	}

	// 
	const __int64 checkDigitsUntil = 100;
	for (__int64 y = ys; y < checkDigitsUntil; y += 2)
	{
		for (__int64 x = xs; x < checkDigitsUntil; x += 2)
		{
			__int64 left = x * x + y * x + m;
			__int64 right = y * SQN;
			if ((left % checkDigitsUntil) == (right % checkDigitsUntil))
			{
				checksMade++;
				if (isXSolution(x, SQN, m, N)
					|| isYSolution(y, SQN, m, N) // would only need to check for first iteration. After that y would increase beyond x
					)
				{
					SQN = 0;
					return;
				}
			}
		}
	}
	SQN -= checkDigitsUntil;
	m = N - SQN * SQN;
}

static void AdvanceY(__int64& SQN, __int64& m, __int64 N, __int64& checksMade)
{
	// cause we know x,y can only be pair or impair
	__int64 xs = 0, ys = 0;
	if (((SQN) & 1) == 0)
	{
		xs = 1;
	}
	if (((SQN + xs) & 1) == 0)
	{
		ys = 1;
	}

	// 
	const __int64 checkDigitsUntil = 100;
	for (__int64 y = ys; y < checkDigitsUntil; y += 2)
	{
		for (__int64 x = xs; x < checkDigitsUntil; x += 2)
		{
			__int64 left;
			__int64 right;
			if (m < 0)
			{
				left = x * x + y * x;
				right = y * SQN - m;
			}
			else
			{
				left = x * x + y * x + m;
				right = y * SQN;
			}
			if ((left % checkDigitsUntil) == (right % checkDigitsUntil))
			{
				checksMade++;
				// we are aiming for y. y will always be smaller than x
				if (isYSolution(y, SQN, m, N))
				{
					SQN = 0;
					return;
				}
			}
		}
	}
	SQN += (checkDigitsUntil - 1) / 2;
	m = N - SQN * SQN;
}

void DivTestabxy2_x(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	__int64 stepsMade = 0;
	__int64 checksMade = 0;

	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);
	while (SQN > 0)
	{
		AdvanceX(SQN, m, N, checksMade);
		stepsMade++;
		if (SQN != 0)
		{
			printf("\t %lld) SQN now=%lld m now=%lld checkssofar=%lld\n", stepsMade, SQN, m, checksMade);
		}
	}
	printf("Found A,B in %lld steps, %lld checks using SQN=%lld, m=%lld\n\n", stepsMade, checksMade, SQN, m);
}

void DivTestabxy2_y(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	__int64 stepsMade = 0;
	__int64 checksMade = 0;

	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);
	while (SQN > 0)
	{
		AdvanceY(SQN, m, N, checksMade);
		stepsMade++;
		if (SQN != 0)
		{
			printf("\t %lld) SQNnow=%lld mnow=%lld xnow=%lld ynow=%lld checkssofar=%lld\n",
				stepsMade, SQN, m, SQN - A, A + B - 2 * SQN, checksMade);
		}
	}
	printf("Found A,B in %lld steps, %lld checks using SQN=%lld, m=%lld\n\n", stepsMade, checksMade, SQN, m);
}


void DivTestabxy2_(__int64 A, __int64 B)
{
	DivTestabxy2_x(A, B);
	DivTestabxy2_y(A, B);
}

void DivTestabxy2()
{
	//DivTestabxy2_(5, 7);
	DivTestabxy2_(23, 41);
	DivTestabxy2_(349, 751); // N = 262099 , SN = 511
	DivTestabxy2_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestabxy2_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestabxy2_(784727, 918839);
	DivTestabxy2_(3, 918839);
}
