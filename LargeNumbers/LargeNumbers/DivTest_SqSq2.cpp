#include "StdAfx.h"

/*
just check if we can find a B that generates a special number : b * x + m 
*/


int TruthTest(__int64 a, __int64 b, __int64 m, __int64 x, __int64 y, __int64 Mask, __int64 N)
{
	__int64 left1 = x * b + m; // len(x) is known
	__int64 right1 = y * (a - x); // MIN(len(y),len(x)) is known

	if (Mask > 0 && (left1 % Mask != right1 % Mask))
		return 0;

	__int64 A = a - x; 
	__int64 B = b + y;
	if (A * B % Mask != N % Mask)
		return 0;

	__int64 bb = (A + B) / 2;
	__int64 aa = (B - A) / 2;
	if ((aa * aa + N) % Mask != (bb * bb) % Mask)
		return 0;

	__int64 d1 = 2;
	// d1 = 2	b1 = B / 2
	// B = b + y = d1 * b1 + mb1		b = d1 * b1 + mb1 - y		y = d1 * b1 + mb1 - b
	__int64 b1 = B / 2;
	__int64 mb1 = 1;
//	__int64 left2 = ( d1 * b1 + mb1 - y ) * x + m;
	__int64 left2 = b * x + m;
	// a = d1 * a1 + m1 
	__int64 ma1 = 1;
	__int64 a1 = A / 2;
//	__int64 right2 = y * (d1 * a1 + ma1); 
	__int64 right2 = (d1 * b1 + mb1 - b) * (d1 * a1 + ma1);

	if (Mask > 0 && (left2 % Mask != right2 % Mask))
		return 0;

	return 1;
}

int GetCandidateCount(__int64 SN, __int64 a, __int64 b, __int64 m, FILE *f, __int64 ExpectedX, __int64 ExpectedY, __int64 N)
{
	__int64 RangeEnd = SN;
//RangeEnd = 1;
	__int64 RangeS = 0;
	__int64 RangeE = 10;
	int CandidatesFound = 0;
	while (RangeEnd > 0)
	{
		for (__int64 x = RangeS; x < RangeE; x++)
			for (__int64 y = RangeS; y < RangeE; y++)
			{
				__int64 Mask = 1;
				int DigitCount = MAX(GetDigitCount(x), GetDigitCount(y));
				for (int i = 0; i < DigitCount; i++)
					Mask *= 10;
				assert(Mask == RangeE);
				if (TruthTest(a, b, m, x, y, Mask, N) == 1)
				{
					__int64 left1 = x * b + m; // len(x) is known
					__int64 right1 = y * (a - x); // MIN(len(y),len(x)) is known
					CandidatesFound++;
//					printf("\t%d)Possible candidate : x = %lld, y = %lld, M = %lld, l=%lld, r=%lld\n", CandidatesFound, x, y, Mask, left1, right1);
					int Match = 0;
					if ((ExpectedX % Mask) == (x % Mask) && (ExpectedY % Mask) == (y % Mask))
						Match = 1;
					char WriteBuff[500];
					sprintf_s(WriteBuff, sizeof(WriteBuff), "\t%d)Possible candidate : x = %lld, y = %lld, M = %lld, l=%lld, r=%lld, good=%d\n", CandidatesFound, x, y, Mask, left1, right1, Match);
					fprintf(f, "%s", WriteBuff);
				}
			}
		if (RangeS == 0)
			RangeS = 1;
		RangeS = RangeS * 10;
		RangeE = RangeE * 10;
		RangeEnd /= 10;
	}
	return CandidatesFound;
}

void DivTest_SQSQ2(__int64 iA, __int64 iB)
{
	__int64 N = iA * iB;
	__int64 SN = isqrt(N);
	__int64 a = SN;
	__int64 b = N / a;
	__int64 m = SN - a * b;
	printf("a we are looking for. Values below it might return bogus results : %lld\n", iA);
	printf("b we are looking for. Values above it might return bogus results : %lld\n", iB);

	//increase the first digit of b
	__int64 bp = 1;
	for(int i=1;i<GetDigitCount(b);i++)
		bp *= 10;
	bp += b;

	FILE *f;
	errno_t openerr = fopen_s(&f, "Console.txt", "wt");
	for (__int64 aa = SN; aa > 0; aa-=1)
	{
		__int64 a = aa;
		__int64 b = N / a;
		__int64 m = N - a * b;
		__int64 ExpectedX = a - iA;
		__int64 ExpectedY = iB - b;
		int CandidatesFound = GetCandidateCount(SN, a, b, m, f, ExpectedX, ExpectedY, N);
		char WriteBuff[500];
		sprintf_s(WriteBuff, sizeof(WriteBuff), "For a=%lld we found %d candidates. Expecting x=%lld, y=%lld\n", aa, CandidatesFound, ExpectedX, ExpectedY);
		fprintf(f,"%s", WriteBuff);
	}
	fclose(f);
}

void DivTestSQSQ2()
{
//	DivTest_SQSQ2(23, 41);
	DivTest_SQSQ2(349, 751); // N = 262099 SN = 511
//	DivTest_SQSQ2(397, 1481); 
//	DivTest_SQSQ2(6871, 7673); // N = 52721183 , SN = 7260
//	DivTest_SQSQ2(26729, 31793); // N = 849795097 , SN = 29151
//	DivTest_SQSQ2(784727, 918839);
//	DivTest_SQSQ2(6117633, 7219973);
//	DivTest_SQSQ2(26729, 61781);
//	DivTest_SQSQ2(11789, 61781);
	printf("Can close now");
}