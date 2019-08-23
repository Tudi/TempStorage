#include "StdAfx.h"

/*
just check if we can find a B that generates a special number : b * x + m
*/


int TruthTest(__int64 a, __int64 b, __int64 Mask, __int64 N)
{
	__int64 left1 = a * b;
	__int64 right1 = N;

	if (Mask > 0 && (left1 % Mask != right1 % Mask))
		return 0;

	return 1;
}

int GetCandidateCount(__int64 SN, FILE *f, __int64 ExpectedX, __int64 ExpectedY, __int64 N)
{
	__int64 RangeEnd = SN;
	//RangeEnd = 1;
	__int64 RangeS = 0;
	__int64 RangeE = 10;
	int CandidatesFound = 0;
	while (RangeEnd > 0)
	{
		if (RangeE > SN)
			RangeE = RangeE;
		for (__int64 x = RangeS; x < RangeE; x++)
			for (__int64 y = RangeS; y < RangeE; y++)
			{
				__int64 Mask = 1;
				int DigitCount = MAX(GetDigitCount(x), GetDigitCount(y));
				for (int i = 0; i < DigitCount; i++)
					Mask *= 10;
				assert(Mask == RangeE);
				if (TruthTest(x, y, Mask, N) == 1)
				{
					CandidatesFound++;
					//					printf("\t%d)Possible candidate : x = %lld, y = %lld, M = %lld, l=%lld, r=%lld\n", CandidatesFound, x, y, Mask, left1, right1);
					int Match = 0;
					if ((ExpectedX % Mask) == (x % Mask) && (ExpectedY % Mask) == (y % Mask))
						Match = 1;
					char WriteBuff[500];
					sprintf_s(WriteBuff, sizeof(WriteBuff), "\t%d)Possible candidate : x = %lld, y = %lld, M = %lld, good=%d\n", CandidatesFound, x, y, Mask, Match);
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

void DivTest_ab2(__int64 iA, __int64 iB)
{
	__int64 N = iA * iB;
	__int64 SN = isqrt(N);
	__int64 a = SN;
	__int64 b = N / a;
	__int64 m = SN - a * b;
	printf("a we are looking for. Values below it might return bogus results : %lld\n", iA);
	printf("b we are looking for. Values above it might return bogus results : %lld\n", iB);

	FILE *f;
	errno_t openerr = fopen_s(&f, "Console.txt", "wt");
	__int64 ExpectedX = iA;
	__int64 ExpectedY = iB;
	int CandidatesFound = GetCandidateCount(SN, f, ExpectedX, ExpectedY, N);
	char WriteBuff[500];
	sprintf_s(WriteBuff, sizeof(WriteBuff), "We found %d candidates. Expecting x=%lld, y=%lld\n", CandidatesFound, ExpectedX, ExpectedY);
	fprintf(f, "%s", WriteBuff);
	fclose(f);
}

void DivTestab2()
{
	//	DivTest_ab2(23, 41);
//	DivTest_ab2(349, 751); // N = 262099 SN = 511
	DivTest_ab2(397, 1481); 
//	DivTest_ab2(6871, 7673); // N = 52721183 , SN = 7260
//	DivTest_ab2(26729, 31793); // N = 849795097 , SN = 29151
//	DivTest_ab2(784727, 918839);
//	DivTest_ab2(6117633, 7219973);
//	DivTest_ab2(26729, 61781);
//	DivTest_ab2(11789, 61781);
	printf("Can close now");
}