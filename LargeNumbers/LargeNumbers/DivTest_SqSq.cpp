#include "StdAfx.h"

__int64 m1;
int f1 = 0;
int f2 = 0;

__int64 GetMaskForBitCount(int Count)
{
	int ret = 0;
	for (int i = 0; i < Count; i++)
		ret = ret * 2 + 1;
	return ret;
}

int CountBitsMatch(__int64 a, __int64 b)
{
	int ret = 0;
	while ((a & 1) == (b & 1) && a > 0 && b > 0)
	{
		ret++;
		a = a / 2;
		b = b / 2;
	}
	return ret;
}

int TruthTest(__int64 a, __int64 b, __int64 m, __int64 x, __int64 y, int Mask)
{
	__int64 left1 = x * b + m1; // len(x) is known
	__int64 right1 = y * (a - x); // MIN(len(y),len(x)) is known

	if (Mask > 0 && (left1 % Mask != right1 % Mask))
	{
		return 0;
	}

	__int64 left = b * b * (2 * a * x - x * x) + m; //len(x) is known ?
	__int64 right = (2 * b * y + y * y) * (a - x) * (a - x); // len(y)+len(x)+len(x) is known ?

	__int64 Mask2 = Mask;
	if (Mask > 0)
	{
		if (left % Mask2 == right % Mask2)
		{
			return 1;
		}
		else
		{
			f1++;
		}
	}
	else
	{
		if (left == right)
			return 1;
	}
	return 0;
}


int TruthTest2(__int64 N, __int64 x, __int64 y, int Mask)
{
	__int64 left = x * y;
	__int64 right = N;
	if (Mask > 0)
	{
		if (left % Mask == right % Mask)
			return 1;
	}
	else
	{
		if (left == right)
			return 1;
	}
	return 0;
}

void DivTest_SQSQ(__int64 iA, __int64 iB)
{
	__int64 N = iA * iB;
	__int64 SQN = N * N;
	__int64 SN = isqrt(N);
//	__int64 b = 2;
//	__int64 a = isqrt(SQN / (b * b));
	__int64 a = SN;
	__int64 b = N / a;
	__int64 m = SQN - a * a * b * b;
	m1 = N - a * b;

	__int64 LookingForX = a - iA;
	__int64 LookingForY = iB - b;
	if (TruthTest(a, b, m, LookingForX, LookingForY, 0) != 1)
		printf("Failed to calculate proper formula 1\n");
	printf("looking for numbers x=%lld, y=%lld \n", LookingForX, LookingForY);

	if (TruthTest(a, b, m, LookingForX % 10, LookingForY % 10, 10) != 1)
		printf("Failed to calculate proper formula 2\n");

	int CandidatesFound = 0;
	__int64 Range = a;
	//	Range = 100;
	for (__int64 x = 0; x <= Range; x++)
		for (__int64 y = 0; y <= Range; y++)
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
			if (TruthTest(a, b, m, x, y, Mask) == 1)
				printf("%d)Possible candidate : x = %lld, y = %lld, M = %d\n", CandidatesFound++, x, y, Mask);
//			if (TruthTest2(N, x, y, Mask) == 1)
//				printf("%d)Possible candidate : x = %lld, y = %lld \n", CandidatesFound++, x, y);
//			if (TruthTest(a, b, m, x, y, Mask) == 1)
//				if (TruthTest2(N, a - x, b + y, Mask) == 1)
//					printf("%d)Possible candidate : x = %lld, y = %lld \n", CandidatesFound++, x, y);
		}
	printf("f1=%d,f2=%d\n", f1, f2);
}

void DivTestSQSQ()
{
//	DivTest_SQSQ(23, 41);
	DivTest_SQSQ(349, 751); // N = 262099 SN = 511
//	DivTest_SQSQ(6871, 7673); // N = 52721183 , SN = 7260
//	DivTest_SQSQ(26729, 31793); // N = 849795097 , SN = 29151
//	DivTest_SQSQ(784727, 918839);
//	DivTest_SQSQ(6117633, 7219973);
//	DivTest_SQSQ(26729, 61781);
//	DivTest_SQSQ(11789, 61781);
}