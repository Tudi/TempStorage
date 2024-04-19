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

static void AdvanceX(__int64& SQN, __int64 &m, __int64 N, __int64& yForx, __int64& checksMade)
{
	// cause we know x,y can only be pair or impair
	__int64 xs = 0, ys = 0;
	if (((SQN) & 1) == 0)
	{
		xs = 1;
	}
	if (((SQN + xs + yForx) & 1) == 0)
	{
		ys = 1;
	}

	// 40 checks for 100x100 tries
	const __int64 checkDigitsUntil = 100;
	for (__int64 y = ys; y < checkDigitsUntil; y += 2)
	{
		for (__int64 x = xs; x < checkDigitsUntil; x += 2)
		{
			__int64 ty = yForx + y; // would be enough to set low digits
			__int64 left = x * x + ty * x + m;
			__int64 right = ty * SQN;
			if ((left % checkDigitsUntil) == (right % checkDigitsUntil))
			{
				checksMade++;
				if (isXSolution(x, SQN, m, N)
					|| isYSolution(ty, SQN, m, N)
					)
				{
					SQN = 0;
					return;
				}
			}
		}
	}

	__int64 new_xp;
	{
		__int64 ty = yForx + checkDigitsUntil;
		__int64 x_a = 1;
		__int64 x_b = ty;
		__int64 x_c = m - SQN * ty;
		new_xp = (-x_b + isqrt(x_b * x_b - 4 * x_a * x_c)) / (2 * x_a);
	}
	if (new_xp > checkDigitsUntil)
	{
		yForx += 2 * new_xp + checkDigitsUntil;

		SQN -= new_xp;
		m = N - SQN * SQN;
	}
	else
	{
		// at this point we are looping through the same solution candidates. Only m changes in the eq beacause (SQN mod checkDigitsUntil) remains the same
		// this will start to happen when (y+1)-(y) <= 1  .. ((x+1)^2+m-(x^2+m))/(n-x-1) <= 1
		// (x^2+2*x+1-x^2)/(n-x-1) <= 1  2*x <= n-x-1    x <= n / 3 -> aproximately !
		__int64 tx = checkDigitsUntil;
		__int64 new_yp = (tx * tx + m) / (SQN - tx);
		yForx = new_yp;

		SQN -= checkDigitsUntil;
		m = N - SQN * SQN;
	}
}

void DivTestabxy2_x(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	__int64 stepsMade = 0;
	__int64 checksMade = 0;
	__int64 yforx = 0;

	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);
	while (SQN > 0)
	{
		AdvanceX(SQN, m, N, yforx, checksMade);
		stepsMade++;
		if (SQN != 0)
		{
			printf("\t %lld) SQN now=%lld m now=%lld x=%lld y=%lld, checkssofar=%lld\n", stepsMade, SQN, m, SQN - A, A + B - 2 * SQN - yforx, checksMade);
		}
	}
	printf("Found A,B in %lld steps, %lld checks\n\n", stepsMade, checksMade);
}

static void AdvanceY(__int64& SQN, __int64& m, __int64 N, __int64& xFory, __int64& checksMade)
{
	// cause we know x,y can only be pair or impair
	__int64 xs = 0, ys = 0;
	if (((SQN + xFory) & 1) == 0)
	{
		xs = 1;
	}
	if (((SQN + xs + xFory) & 1) == 0)
	{
		ys = 1;
	}

	// 40 checks for 100x100 tries
	const __int64 checkDigitsUntil = 100;
//#define CHECK_SIMETRY_X_Y_CHECKS
#ifdef CHECK_SIMETRY_X_Y_CHECKS
	char uniqueYs[checkDigitsUntil] = { 0 };
	char uniqueXs[checkDigitsUntil] = { 0 };
#endif
	for (__int64 y = ys; y < checkDigitsUntil; y += 2)
	{
		for (__int64 x = xs; x < checkDigitsUntil; x += 2)
		{
			__int64 ty = y; // would be enough to set low digits
			__int64 tx = x + xFory;
			__int64 left, right;
			if (m > 0)
			{
				left = tx * tx + ty * tx + m;
				right = ty * SQN;
			}
			else
			{
				left = tx * tx + ty * tx;
				right = ty * SQN - m;
			}
			if ((left % checkDigitsUntil) == (right % checkDigitsUntil))
			{
#ifdef CHECK_SIMETRY_X_Y_CHECKS
				uniqueYs[y] = 1;
				uniqueXs[x] = 1;
#endif
				checksMade++;
				if (isXSolution(tx, SQN, m, N)
					|| isYSolution(ty, SQN, m, N)
					)
				{
					SQN = 0;
					return;
				}
			}
		}
	}

#ifdef CHECK_SIMETRY_X_Y_CHECKS
	// looks like only 10 y checks are needed for 100 y values
	size_t unique_tests_x = 0, unique_tests_y = 0;
	for (size_t i = 0; i < checkDigitsUntil; i++)
	{
		if (uniqueXs[i] != 0)unique_tests_x++;
		if (uniqueYs[i] != 0)unique_tests_y++;
	}
#endif

	if (xFory + checkDigitsUntil >= (SQN / 3))
	{
		// one X produces more Y
		// we can only increase SQN by y/2
		__int64 tx = xFory + checkDigitsUntil; // we checked this many so far
		__int64 ty = (tx * tx + m) / (SQN - tx);
		// we are only testing X ( lost track of proper y), we are only allowed to progress on the X axis 
		xFory += checkDigitsUntil * 2;
		SQN += checkDigitsUntil;
	}
	else
	{
		// one y produces more Xes
		// we can only increase SQN by y/2
		__int64 ty = checkDigitsUntil;
		__int64 x_a = 1;
		__int64 x_b = ty;
		__int64 x_c = m - SQN * ty;
		xFory = (-x_b + isqrt(x_b * x_b - 4 * x_a * x_c)) / (2 * x_a);
		SQN += ty / 2;
	}
	m = N - SQN * SQN;
}

void DivTestabxy2_y(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	__int64 stepsMade = 0;
	__int64 checksMade = 0;
	__int64 xfory = 0;

	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);
	while (SQN > 0)
	{
		AdvanceY(SQN, m, N, xfory, checksMade);
		stepsMade++;
		if (SQN != 0)
		{
			printf("\t %lld) SQN now=%lld m now=%lld x=%lld y=%lld, checkssofar=%lld\n", 
				stepsMade, SQN, m, SQN - A - xfory, A + B - 2 * SQN, checksMade);
		}
	}
	printf("Found A,B in %lld steps, %lld checks\n\n", stepsMade, checksMade);
}

void DivTestabxy2_(__int64 A, __int64 B)
{
//	DivTestabxy2_x(A, B);
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
	DivTestabxy2_(349, 918839);
}
