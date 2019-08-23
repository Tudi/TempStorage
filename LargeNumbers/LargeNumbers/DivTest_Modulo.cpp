#include "StdAfx.h"

/*
idea is that we presume a "large" divider, generate all combinations of m for a and b, we can test with smaller dividers if they all match for a specific guessed digit
for any used (d * a + m1) * (d * b + m2), they should all have a common digit at every location of A and B
*/
__int64 A;
__int64 B;

//	__int64 Divider1 = 2;// needed 575 steps to guess it
//	__int64 Divider1 = 3;// needed 153 steps to guess it
//	__int64 Divider1 = 4;// needed 1138 steps to guess it
//	__int64 Divider1 = 5;// needed 901 steps to guess it
//	__int64 Divider1 = 6;// needed 581 steps to guess it
//	__int64 Divider1 = 7;// needed 135 steps to guess it
//	__int64 Divider1 = 8;// needed 1938 steps to guess it
	__int64 Divider1 = 9;// needed 144 steps to guess it
//	__int64 Divider1 = 10;// needed 3613 steps to guess it
//	__int64 Divider1 = 11;// needed 124 steps to guess it

int TruthTest(__int64 N, __int64 tA, __int64 tB, int Mask)
{
	__int64 a = tA; // we are looking for A / Divider1
	__int64 b = tB;
	__int64 m1 = A % Divider1;
	__int64 m2 = B % Divider1;
	__int64 left = Divider1 * Divider1 * (a * b) + Divider1 * (m2 * a + m1 * b) + m1 * m2;
	__int64 right = N;

	if (Mask > 1)
		if (left % Mask != right % Mask)
			return 0;
//		else
//			return 1; 

	if (left > N)
		return 0;

	__int64 Divider2 = 3; // can't be larger than divider1 or else the below division has no idea where to inherit values from. Or it needs to work on small enough values for the division to be valid

	assert(Divider1 > Divider2);

	a = (tA * Divider1 + m1) / Divider2;
	b = (tB * Divider1 + m2) / Divider2;
	m1 = A % Divider2;
	m2 = B % Divider2;
	left = Divider2 * Divider2 * (a * b) + Divider2 * (m2 * a + m1 * b) + m1 * m2;
	right = N;


	if (Mask > 1)
	{
		if (left % Mask == right % Mask)
		{
			//			__int64 left1 = (2*x+1)*(2*y+1);
			//			__int64 right1 = N;
			//			if (left1 % Mask != right1 % Mask)
			//				return 0;
			return 1;
		}
	}
	else
	{
		if (left == right)
			return 1;
	}
	return 0;
}

void DivTestModulo(__int64 iA, __int64 iB)
{
	A = iA;
	B = iB;
	__int64 N = iA * iB;
	__int64 SN = isqrt(N);

	__int64 LookingForX = iA / Divider1;
	__int64 LookingForY = iB / Divider1;
	printf("looking for numbers x=%lld, y=%lld \n", LookingForX, LookingForY);
	__int64 Mask = 10;
	while(LookingForX / Mask > 0)
	{
		if (TruthTest(N, LookingForX % Mask, LookingForY % Mask, Mask) != 1)
		{
			printf("Truth test failed for mask %lld. Abandon ship\n", Mask);
			TruthTest(N, LookingForX % Mask, LookingForY % Mask, Mask);
			assert(0);
		}
		Mask = Mask * 10;
	}

	int CandidatesFound = 0;
	__int64 RangeEnd = SN;
	__int64 RangeS = 0;
	__int64 RangeE = 10;
	//	Range = 100;
	while (RangeEnd > 0)
	{
		for (__int64 x = RangeS; x < RangeE; x++)
			for (__int64 y = RangeS; y < RangeE; y++)
			{
				int Mask = 0;
				if (x < 10 && y < 10)
					Mask = 10;
				else if (x < 100 && y < 100)
					Mask = 100;
				else if (x < 1000 && y < 1000)
					Mask = 1000;
				else if (x < 10000 && y < 10000)
					Mask = 10000;
				else if (x < 100000 && y < 100000)
					Mask = 100000;
				if (TruthTest(N, x, y, Mask) == 1)
					printf("%d)Possible candidate : x = %lld, y = %lld, M = %d\n", CandidatesFound++, x, y, Mask);
			}
		if (RangeS == 0)
			RangeS = 1;
		RangeS = RangeS * 10;
		RangeE = RangeE * 10;
		RangeEnd /= 10;
	}
}

void DivTestModulo()
{
	//	DivTestModulo(23, 41);
//	DivTestModulo(349, 751); // N = 262099 SN = 511
	DivTestModulo(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo(784727, 918839);
//	DivTestModulo(6117633, 7219973);
//	DivTestModulo(26729, 61781);
//	DivTestModulo(11789, 61781);
}