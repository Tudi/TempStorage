#include "StdAfx.h"

/*
idea is to propose a remainder for A and B and try to "proove" we guessed it correctly or not
(a1 * d1 + ma1) * (b1 * d1 + mb1) = N
(a1 * d1 + ma1) = A = ( a2 * d2 + ma2 )
if d is 2, we know the remainder
if d is 3, ma, mb can be 1 or 2 => generates 4 possible formulas where only 1 should be valid

Just looking for any magical filter values if they exist. We suppose we know last digit of A and B
Compared to previous version, start out with final A,B and see if filters could obtain that
*/
namespace ReminderTest3 {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 Mask;
	int HighestDividerFound = 0;

#define MergeTwoValuesForKey(a,b) (a*100000+b)
#define GetFirstValueFromKey(Key)	(Key/100000)
#define GetSecondValueFromKey(Key)	(Key%100000)
#define HighestDivider 1150
	struct RemainderFilterStore
	{
		__int64 Divider; // value we fivided A and B with
		int ShouldBeValid; // while testing, we mark the ma and mb that should be always valid
		__int64 ma, mb;	// initialized as a guessed value
		__int64 ta, tb, GeneratedA, GeneratedB, ReconN;
		int InvalidatedCount; // if we invalidated this filter with some digit test
		int ValidatedCount;
		int ValidatedCountAll;
//		std::set<__int64> ValidatedFortAtB;
//		std::set<__int64> InvalidatedFortAtB;
//		std::set<__int64> ValuesThatGeneratedAB;
	};

	std::list<RemainderFilterStore*> Filters[HighestDivider + 1];

	int ValidateFilter(RemainderFilterStore *f, __int64 N, __int64 a, __int64 b, __int64 Mask)
	{
		//		if ((a % Mask) == ((A / f->Divider) % Mask) && (b % Mask) == ((B / f->Divider) % Mask))
		//			A = A;
		__int64 PartialA = a * f->Divider + f->ma;
		__int64 PartialB = b * f->Divider + f->mb;
		__int64 left1 = PartialA * PartialB;
		if ((left1 % Mask) != (N % Mask))
		{
			assert((PartialA % Mask) != (A % Mask) || (PartialB % Mask) != (B % Mask));
			assert((a % Mask) != ((A / f->Divider) % Mask) || (b % Mask) != ((B / f->Divider) % Mask) || (f->ma != (A % f->Divider)) || (f->mb != (B % f->Divider)));
			//		f->InvalidatedCount = 1;
			return 0;
		}

		__int64 left = f->Divider * f->Divider * (a * b) + f->Divider * (f->mb * a + f->ma * b) + f->ma * f->mb;
		__int64 right = N;

		if ((left % Mask) != (right % Mask))
		{
			assert((PartialA % Mask) != (A % Mask) || (PartialB % Mask) != (B % Mask));
			assert((a % Mask) != ((A / f->Divider) % Mask) || (b % Mask) != ((B / f->Divider) % Mask) || (f->ma != (A % f->Divider)) || (f->mb != (B % f->Divider)));
			//		f->InvalidatedCount = 1;
			return 0;
		}

		//		f->ValidatedCount++;
		return 1;
	}

	RemainderFilterStore * CreateFilter(__int64 Divider, __int64 ma, __int64 mb)
	{
		RemainderFilterStore *f = new RemainderFilterStore;
		f->Divider = Divider;
		f->InvalidatedCount = 0;
		f->ma = ma;
		f->mb = mb;
		f->ValidatedCount = 0;
//		if ((A % Divider == ma && B % Divider == mb) || (B % Divider == ma && A % Divider == mb))
		if ((A % Divider == ma && B % Divider == mb))
			f->ShouldBeValid = 1;
		else
			f->ShouldBeValid = 0;
		f->ValidatedCountAll = 0;
		return f;
	}

	void CreateFiltersForDivider(__int64 N, __int64 Divider)
	{
		if (Divider == 1)
			Filters[Divider].push_back(CreateFilter(1, 0, 0));
		for (int ma = 1; ma < Divider; ma++)
			for (int mb = 1; mb < Divider; mb++)
			{
				if ((N - ma * mb) % Divider != 0)
					continue;
				Filters[Divider].push_back(CreateFilter(Divider, ma, mb));
			}
	}

	int CountValidFilters()
	{
		int C = 0;
		for (int Divider = 1; Divider <= HighestDivider; Divider++)
			for (auto a : Filters[Divider])
				if (a->InvalidatedCount == 0)
					C++;
		//	printf("Numer of valid filters remaining : %d \n", C);
		return C;
	}

	int CountValidatedFilters()
	{
		int C = 0;
		for (int Divider = 1; Divider <= HighestDivider; Divider++)
			for (auto a : Filters[Divider])
				if (a->ValidatedCount > 0)
					C++;
		//	printf("Numer of valid filters remaining : %d \n", C);
		return C;
	}

	__int64 TryGenerateWithFineSteps(__int64 Divider, __int64 m, __int64 pA, __int64 MaskAB)
	{
		pA -= m;
		__int64 Mask = GetMaskDecimal(Divider); // the length of the divider will influence the numbers below it
		for (__int64 i = Mask * pA; i > 0; i--)
		{
			__int64 GeneratedA = i * Divider;
			if (pA == (GeneratedA % MaskAB))
				return i;
		}
		return 0;
	}

	//input is partial A and B
	int CanFilterGenerate(RemainderFilterStore *f, __int64 pA, __int64 pB)
	{
		int ret = 0;
		// pA = f->ta * f->divider + f->ma
		__int64 Mask = GetMaskDecimal(pA / f->Divider, pB / f->Divider);
		__int64 MaskAB = GetMaskDecimal(pA, pB);
		__int64 tA, tB;
		__int64 GeneratedA, GeneratedB;
		for (__int64 i = 0; i < MaskAB; i++)
		{
			tA = (i * MaskAB + pA - f->ma) / f->Divider;
			GeneratedA = tA * f->Divider + f->ma;
			if (pA == (GeneratedA % MaskAB))
			{
				ret++;
				break;
			}
		}
		for (__int64 i = 0; i < MaskAB; i++)
		{
			tB = (i * MaskAB + pB - f->mb) / f->Divider;
			GeneratedB = tB * f->Divider + f->mb;
			if (pB == (GeneratedB % MaskAB))
			{
				ret++;
				break;
			}
		}
		if (pA != (GeneratedA % MaskAB))
		{
			tA = TryGenerateWithFineSteps(f->Divider, f->ma, pA, MaskAB);
			GeneratedA = tA * f->Divider + f->ma;
			if (pA == (GeneratedA % MaskAB))
				ret++;
		}
		if (pB != (GeneratedB % MaskAB))
		{
			tB = TryGenerateWithFineSteps(f->Divider, f->mb, pB, MaskAB);
			GeneratedB = tB * f->Divider + f->mb;
			if (pB == (GeneratedB % MaskAB))
				ret++;
		}
		f->ta = tA;
		f->tb = tB;
		f->GeneratedA = GeneratedA;
		f->GeneratedB = GeneratedB;
		f->ReconN = GeneratedA * GeneratedB;
		return ret;
	}

	// tA, tB is always for largest filter, smaller filters should downconvert
	int CanAllFilterGroupsGeneratetAtB(__int64 tA, __int64 tB)
	{
		int FiltersConfirmedFromSameGroup[HighestDivider];
		memset(FiltersConfirmedFromSameGroup, 0, sizeof(FiltersConfirmedFromSameGroup));

		int FilterGroupsTested = 0;
		int FilterGroupsConfirmed = 0;
		for (int Divider = 2; Divider <= HighestDivider; Divider++)
		{
			if (Filters[Divider].empty() == true)
				continue;
			FilterGroupsTested++;
			int GroupGenerated = 0;
			for (auto a : Filters[Divider])
			{
				if (CanFilterGenerate(a, tA, tB) == 2)
				{
					GroupGenerated++;
#ifndef _DEBUG
					break;
#endif
				}
			}
			if (GroupGenerated > 0)
			{
				FilterGroupsConfirmed++;
				FiltersConfirmedFromSameGroup[Divider]++;
			}
		}

#ifdef _DEBUG
//		assert(FilterGroupsTested == FilterGroupsConfirmed);
		__int64 MaskAB = GetMaskDecimal(tA, tB);
//		assert((*Filters[2].begin())->ReconN % MaskAB == N % MaskAB);
		for (int Divider = 2; Divider <= HighestDivider && Divider < tA && Divider < tB; Divider++)
		{
			if (Filters[Divider].empty() == true)
				continue;
			for (auto a : Filters[Divider])
			{
				assert(a->ShouldBeValid == 0 || ((a->ReconN % MaskAB) == (N % MaskAB)) || (tA != A % MaskAB) || (tB != B % MaskAB));
				if ((a->ReconN % MaskAB) != (N % MaskAB))
					a->InvalidatedCount++;
				else
					a->ValidatedCount++;
			}
		}
#endif

		return FilterGroupsTested <= FilterGroupsConfirmed;
	}

	std::map<__int64, __int64> ResultSet;
	int IsMirroredResult(__int64 iA, __int64 iB)
	{
		int ret = 1;
		if (ResultSet.find(iB) == ResultSet.end())
			ret = 0;
		else if (ResultSet[iB] != iA)
			ret = 0;
		ResultSet[iA] = iB;
		return ret;
	}

	int GetFactorCount(__int64 Nr)
	{
		int ret = 0;
		__int64 SNR = isqrt(Nr);
		for (int i = 3; i <= SNR; i++)
			if (Nr % i == 0)
				ret++;
		return ret;
	}

	void AddSmallFilters(int CombinationLimit = 1, __int64 DividerLimit = -1)
	{
		// N = d * d * a * b + d * ( mb * a + ma * b ) + ma * mb
		// we are searching for a 'd' that has very few factors as reminder. Best would be if ma = mb . Note that ma is the reminder of A and not N
		__int64 SN = isqrt(N);
		if (DividerLimit == -1)
			DividerLimit = SN;
		if (HighestDivider < DividerLimit)
			DividerLimit = HighestDivider;
		for (__int64 Divider = 2; Divider < DividerLimit; Divider++)
		{
			if (Divider == A)
				continue;
			if (Divider == B)
				continue;
			int FilterComplexity = 0;
			for (int ma = 1; ma < Divider; ma++)
				for (int mb = ma; mb < Divider; mb++)
				{
					if ((N - ma * mb) % Divider != 0)
						continue;
					FilterComplexity++;
				}
			if (FilterComplexity <= CombinationLimit && FilterComplexity > 0)
				CreateFiltersForDivider(N, Divider);
		}
	}
	void AddGoodFilters(__int64 DividerLimit = -1)
	{
		// N = d * d * a * b + d * ( mb * a + ma * b ) + ma * mb
		// we are searching for a 'd' that has very few factors as reminder. Best would be if ma = mb . Note that ma is the reminder of A and not N
		__int64 SN = isqrt(N);
		if (DividerLimit == -1)
			DividerLimit = SN;
		if (HighestDivider < DividerLimit)
			DividerLimit = HighestDivider;
		for (__int64 Divider = 2; Divider <= DividerLimit; Divider++)
		{
			if (Divider == A)
				continue;
			if (Divider == B)
				continue;
			Filters[Divider].push_back(CreateFilter(Divider, A % Divider, B % Divider));
		}
	}

	void DivTestModuloFiltersTryFilter(int Divider)
	{
		Filters[2].push_back(CreateFilter(Divider, A % Divider, B % Divider));

		for (int Divider = HighestDivider; Divider > 1; Divider--)
		{
			if (Filters[Divider].empty() == false)
			{
				HighestDividerFound = Divider;
				break;
			}
		}

		// to avoid mirrored values....
		__int64 LastDigitA = A % 10;
		__int64 LastDigitB = B % 10;

		//using the validated filters only, try to guess the real tA, tB
		__int64 SN = isqrt(N);
		Mask = GetMaskDecimal(SN) / 10;
		int CountPossibleGoodDigits = 0;
		for (__int64 ttA = 0; ttA < Mask; ttA++)
			for (__int64 ttB = 0; ttB < Mask; ttB++)
			{
				__int64 tA = ttA * 10 + LastDigitA;
				__int64 tB = ttB * 10 + LastDigitB;
				__int64 Mask2 = GetMaskDecimal(tA, tB);
				if ((tA * tB) % Mask2 != N % Mask2)
					continue;
				int abMightBeValid = CanAllFilterGroupsGeneratetAtB(tA, tB);
				if (abMightBeValid >= 1)
					CountPossibleGoodDigits++;
			}
		CreateFiltersForDivider(N, Divider);
		printf("For divider %d, we found %d possible values, for %d filters\n", Divider, CountPossibleGoodDigits, (int)(Filters[Divider].size()));
	}

	void DivTestModuloFilters(__int64 iA, __int64 iB)
	{
		N = iA * iB;

		//		AddSmallFilters(400, 25);
		//		AddGoodFilters(25);

		//		Filters[16].push_back(CreateFilter(16, A % 16, B % 16));
		//		Filters[25].push_back(CreateFilter(25, A % 25, B % 25));
		
//		__int64 Divider = 2;
		//Filters[Divider].push_back(CreateFilter(Divider, A % Divider, B % Divider));

		//		CreateFiltersForDivider(N, x * 16); // 1st best
		//		CreateFiltersForDivider(N, x * 25); // 2nd best ?

/*
		CanAllFilterGroupsGeneratetAtB(A % 10, B % 10);
		CanAllFilterGroupsGeneratetAtB(A % 100, B % 100);
		CanAllFilterGroupsGeneratetAtB(A % 1000, B % 1000);
/**/

//		CreateFiltersForDivider(N, 64);
//		CreateFiltersForDivider(N, 125);
		// 125 - 444 -> first time he takes the lead, rest any x * 16 leads
		// 128 - 774
		printf("Looking for a=%lld,b=%lld\n", iA, iB);
		for (__int64 Divider = 454; Divider <= iA; Divider++)
		{
			ResultSet.clear();
			for (int Divider = 1; Divider <= HighestDivider; Divider++)
				Filters[Divider].clear();

			for (int Divider = HighestDivider; Divider > 1; Divider--)
			{
				if (Filters[Divider].empty() == false)
				{
					HighestDividerFound = Divider;
					break;
				}
			}

			DivTestModuloFiltersTryFilter(Divider);
		}
/*
		__int64 SN = isqrt(N);

		// to avoid mirrored values....
		__int64 LastDigitA = A % 10;
		__int64 LastDigitB = B % 10;

		//using the validated filters only, try to guess the real tA, tB
		Mask = GetMaskDecimal(SN) / 10;
		for (__int64 ttA = 0; ttA < Mask; ttA++)
			for (__int64 ttB = 0; ttB < Mask; ttB++)
			{
				__int64 tA = ttA * 10 + LastDigitA;
				__int64 tB = ttB * 10 + LastDigitB;
				__int64 Mask2 = GetMaskDecimal(tA, tB);
				if ((tA * tB) % Mask2 != N % Mask2)
					continue;
				int abMightBeValid = CanAllFilterGroupsGeneratetAtB(tA, tB);
				if (abMightBeValid >= 1)
				{
					//					if (IsMirroredResult(tA, tB))continue;
					static int CountPossibleGoodDigits = 0;
					printf("%d)possible digit a=%lld b=%lld\n", CountPossibleGoodDigits++, tA, tB);
				}
				else
				{
					printf(" \tsmall filters invalidated digit tA=%lld tB=%lld\n", tA, tB);
				}
			}
*/
		A = A;
	}

	void DivTestModuloFiltersCheckFailureRatesOn1Known(__int64 iA, __int64 iB)
	{
		N = iA * iB;
		printf("Looking for a=%lld,b=%lld\n", iA, iB);
		ResultSet.clear();
		for (int Divider = 1; Divider <= HighestDivider; Divider++)
			Filters[Divider].clear();

		__int64 Divider = 16;
//		CreateFiltersForDivider(N, 16);
//		CreateFiltersForDivider(N, 32);
//		Filters[Divider].push_back(CreateFilter(Divider, A % Divider, B % Divider));
		Filters[2].push_back(CreateFilter(2, 1, 347));

		for (int Divider = HighestDivider; Divider > 1; Divider--)
		{
			if (Filters[Divider].empty() == false)
			{
				HighestDividerFound = Divider;
				break;
			}
		}

		__int64 SN = isqrt(N);

		// to avoid mirrored values....
		__int64 LastDigitA = A % 10;
		__int64 LastDigitB = B % 10;

		//using the validated filters only, try to guess the real tA, tB
		Mask = GetMaskDecimal(SN) / 10;
		for (__int64 ttA = 0; ttA < Mask; ttA++)
			for (__int64 ttB = 0; ttB < Mask; ttB++)
			{
				__int64 tA = ttA * 10 + LastDigitA;
				__int64 tB = ttB * 10 + LastDigitB;
				__int64 Mask2 = GetMaskDecimal(tA, tB);
				if ((tA * tB) % Mask2 != N % Mask2)
					continue;
				int abMightBeValid = CanAllFilterGroupsGeneratetAtB(tA, tB);
				if (abMightBeValid >= 1)
				{
					//					if (IsMirroredResult(tA, tB))continue;
					static int CountPossibleGoodDigits = 0;
					printf("%d)possible digit a=%lld b=%lld\n", CountPossibleGoodDigits++, tA, tB);
				}
				else
				{
					printf(" \tsmall filters invalidated digit tA=%lld tB=%lld\n", tA, tB);
				}
			}
		A = A;
	}
};

void DivTestModulo4(__int64 iA, __int64 iB)
{
	ReminderTest3::A = iA;
	ReminderTest3::B = iB;
	ReminderTest3::DivTestModuloFilters(iA, iB);
//	ReminderTest3::DivTestModuloFiltersCheckFailureRatesOn1Known(iA, iB);
}

void DivTestModulo4()
{
/*	for (int i = 348; i > 0; i--)
		if (IsPrime(i))
		{
			printf("%d", i);
			break;
		}*/
	//	DivTestModulo4(23, 41);
//	DivTestModulo4(349, 751); // N = 262099 SN = 511
	DivTestModulo4(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo4(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo4(784727, 918839); // N = 721037771953
//	DivTestModulo4(6117633, 7219973);
//	DivTestModulo4(26729, 61781);
//	DivTestModulo4(11789, 61781);
}