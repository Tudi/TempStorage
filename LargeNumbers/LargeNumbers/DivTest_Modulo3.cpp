#include "StdAfx.h"

/*
idea is to propose a remainder for A and B and try to "proove" we guessed it correctly or not
(a1 * d1 + ma1) * (b1 * d1 + mb1) = N
(a1 * d1 + ma1) = A = ( a2 * d2 + ma2 )
if d is 2, we know the remainder
if d is 3, ma, mb can be 1 or 2 => generates 4 possible formulas where only 1 should be valid

After some testing, i could not find a way to eliminate badly guessed ma,mb combinations
Every possible input a,b will find a ma,mb combination that will hint that a,b is usable and not bad
*/
namespace ReminderTest2 {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 Mask;
	int HighestDividerFound = 0;

	#define HighestDivider 50
	struct RemainderFilterStore
	{
		__int64 Divider; // value we fivided A and B with
		int InvalidatedCount; // if we invalidated this filter with some digit test
		int ShouldBeValid; // while testing, we mark the ma and mb that should be always valid
		int ValidatedCount;
		int ValidatedCountAll;
		__int64 ma, mb;	// initialized as a guessed value
		std::set<__int64> ValidatedFortAtB;
		std::set<__int64> InvalidatedFortAtB;
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
		if ((A % Divider == ma && B % Divider == mb) || (B % Divider == ma && A % Divider == mb))
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

	// tA, tB is always for largest filter, smaller filters should downconvert
	int CanAllFilterGroupsUsetAtB(__int64 tA, __int64 tB)
	{
		__int64 Mask = GetMaskDecimal(tA, tB);

		int ret = 0;
		for (auto HighFilter : Filters[HighestDividerFound])
		{
			__int64 GoodtA = (A / HighFilter->Divider) % Mask;
			__int64 GoodtB = (B / HighFilter->Divider) % Mask;
			__int64 Goodma = (A % HighFilter->Divider) % Mask;
			__int64 Goodmb = (B % HighFilter->Divider) % Mask;

			if (ValidateFilter(HighFilter, N, tA, tB, Mask) == 0)
			{
				assert(tA != GoodtA || tB != GoodtB || HighFilter->ma != Goodma || HighFilter->mb != Goodmb);
				continue;
			}

			int SmallFilterCountThatConfrimedtAtB = 0;
			int SmallFilterGroupsUsedForConfirm = 0;
			for (int Divider = 1; Divider < HighestDividerFound; Divider++)
			{
				if (Filters[Divider].empty() == true)
					continue;
				
				SmallFilterGroupsUsedForConfirm++;
				int GroupValidated = 0;
				for (auto ConfirmFilter : Filters[Divider]) // group of formulas that use the same divider
				{
					__int64 ta2 = HighFilter->Divider * tA + HighFilter->ma - ConfirmFilter->ma;
					__int64 tb2 = HighFilter->Divider * tB + HighFilter->mb - ConfirmFilter->mb;

					__int64 tA2 = ta2 / ConfirmFilter->Divider;
					__int64 tB2 = tb2 / ConfirmFilter->Divider;
					__int64 Mask2 = Mask;
//					__int64 Mask2 = GetMaskDecimal(tA2, tB2);

					__int64 Goodta2 = (A * ConfirmFilter->Divider) % Mask2;
					__int64 Goodtb2 = (B * ConfirmFilter->Divider) % Mask2;
					__int64 GoodtA2 = (A / ConfirmFilter->Divider) % Mask2;
					__int64 GoodtB2 = (B / ConfirmFilter->Divider) % Mask2;
					__int64 Goodma2 = (A % ConfirmFilter->Divider) % Mask2;
					__int64 Goodmb2 = (B % ConfirmFilter->Divider) % Mask2;

					if (ta2 % ConfirmFilter->Divider != 0)
					{
						assert(tA != GoodtA || tB != GoodtB || ConfirmFilter->ma != Goodma2 || ConfirmFilter->mb != Goodmb2 || ta2 % Mask2 != Goodta2 || tb2 % Mask2 != Goodtb2);
						continue;
					}
					if (tb2 % ConfirmFilter->Divider != 0)
					{
						assert(tA != GoodtA || tB != GoodtB || ConfirmFilter->ma != Goodma2 || ConfirmFilter->mb != Goodmb2 || ta2 % Mask2 != Goodta2 || tb2 % Mask2 != Goodtb2);
						continue;
					}

					if (ValidateFilter(ConfirmFilter, N, tA2, tB2, Mask2) == 0)
					{
						assert(tA != GoodtA || tB != GoodtB || ConfirmFilter->ma != Goodma2 || ConfirmFilter->mb != Goodmb2 || ta2 % Mask2 != Goodta2 || tb2 % Mask2 != Goodtb2);
						continue;
					}
					ConfirmFilter->ValidatedCount++;
					GroupValidated = 1;
				}
				if (GroupValidated > 0)
					SmallFilterCountThatConfrimedtAtB++;
			}
			if (SmallFilterCountThatConfrimedtAtB >= SmallFilterGroupsUsedForConfirm)
			{
				//			printf(" \tpossible digit tA=%lld tB=%lld\n", tA, tB);
				ret = 1;
				HighFilter->ValidatedCount++;
				HighFilter->ValidatedFortAtB.insert(tA * 10000 + tB);

				for (int Divider = 1; Divider < HighestDividerFound; Divider++)
				{
					if (Filters[Divider].empty() == true)
						continue;
					for (auto ConfirmFilter : Filters[Divider]) // group of formulas that use the same divider
					{
						__int64 ta2 = HighFilter->Divider * tA + HighFilter->ma - ConfirmFilter->ma;
						__int64 tb2 = HighFilter->Divider * tB + HighFilter->mb - ConfirmFilter->mb;

						__int64 tA2 = ta2 / ConfirmFilter->Divider;
						__int64 tB2 = tb2 / ConfirmFilter->Divider;
						__int64 Mask2 = Mask;
						if (ta2 % ConfirmFilter->Divider != 0)
						{
							ConfirmFilter->InvalidatedCount++;
							continue;
						}
						if (tb2 % ConfirmFilter->Divider != 0)
						{
							ConfirmFilter->InvalidatedCount++;
							continue;
						}
						if (ValidateFilter(ConfirmFilter, N, tA2, tB2, Mask2) == 0)
						{
							ConfirmFilter->InvalidatedCount++;
							continue;
						}
						ConfirmFilter->ValidatedCountAll++;
					}
				}
			}
			else
			{
				HighFilter->InvalidatedCount++;
				HighFilter->InvalidatedFortAtB.insert(tA * 10000 + tB);
			}
		}
		return ret;
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
	
	void AddSmallFilters(int CombinationLimit=1, __int64 DividerLimit = -1)
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
			if(FilterComplexity <= CombinationLimit && FilterComplexity > 0)
				CreateFiltersForDivider(N, Divider);
		}
	}

	void DivTestModuloFilters(__int64 iA, __int64 iB)
	{
		N = iA * iB;
		ResultSet.clear();
		for (int Divider = 1; Divider <= HighestDivider; Divider++)
			Filters[Divider].clear();

		AddSmallFilters(40,15);

//		CreateFiltersForDivider(N, 2);
//		CreateFiltersForDivider(N, 3);
//		CreateFiltersForDivider(N, 4);
//		CreateFiltersForDivider(N, 5);
//		Filters[5].push_back(CreateFilter(5, iA % 5, iB % 5));
//		CreateFiltersForDivider(N, 6);
//		CreateFiltersForDivider(N, 7);
//		CreateFiltersForDivider(N, 8);
//		Filters[8].push_back(CreateFilter(8, 1, 7));
//		Filters[8].push_back(CreateFilter(8, 7, 1));
//		CreateFiltersForDivider(N, 9);
		//CreateFiltersForDivider(N, 10);
//		CreateFiltersForDivider(N, 11);
//		CreateFiltersForDivider(N, 12);
//		CreateFiltersForDivider(N, 13);
//		CreateFiltersForDivider(N, 50);
		//		Filters[50].push_back(CreateFilter(50, iA % 50, iB % 50));
				/*
					Filters[1].push_back(CreateFilter(1, 0, 0));
					Filters[2].push_back(CreateFilter(2, iA % 2, iB % 2));
					Filters[3].push_back(CreateFilter(3, iA % 3, iB % 3));
					*/

					//		printf("Looking for tA = %lld and tB = %lld \n", A / HighestDivider, B / HighestDivider);

		for (int Divider = HighestDivider; Divider > 1; Divider--)
		{
			if (Filters[Divider].empty() == false)
			{
				HighestDividerFound = Divider;
				break;
			}
		}

		int FiltersValid_Initial = CountValidFilters();

		printf("LargestFilter should work with a=%lld,b=%lld,ma=%lld,mb=%lld\n", (A / HighestDividerFound), (B / HighestDividerFound), (A%HighestDividerFound), (B%HighestDividerFound));
		__int64 SN = isqrt(N);
		Mask = GetMaskDecimal(SN);
		for (__int64 tA = 0; tA < Mask; tA++)
			for (__int64 tB = 0; tB < Mask; tB++)
			{
//				__int64 tMask = GetMaskDecimal(tA * tB);
//				if (tMask > Mask)
//					continue;
				int abMightBeValid = CanAllFilterGroupsUsetAtB(tA, tB);
				if (abMightBeValid >= 1)
				{
//					if (IsMirroredResult(tA, tB))continue;
					static int CountPossibleGoodDigits = 0;
					printf("%d)possible digit a=%lld b=%lld\n", CountPossibleGoodDigits++, tA, tB);
				}
				else
				{
//					printf(" \tsmall filters invalidated digit tA=%lld tB=%lld\n", tA, tB);
				}
			}
		int FiltersValidated_EndTrain = CountValidatedFilters();
		int FiltersValid_EndTrain = CountValidFilters();
		
		printf("Started with %d filters. Validated %d filters. Valid filters %d \n", FiltersValid_Initial, FiltersValidated_EndTrain, FiltersValid_EndTrain);

		for (auto f1 : Filters[HighestDividerFound])
		{
			//find the match for this filter
			RemainderFilterStore *f2 = NULL;
			for( auto t:Filters[HighestDividerFound])
				if (t->Divider == f1->Divider && t->ma == f1->mb && t->mb == f1->ma)
				{
					f2 = t;
					break;
				}
			//check that validaters are the same for both cases
			for (auto a : f1->ValidatedFortAtB)
			{
				__int64 tA = a / 10000;
				__int64 tB = a % 10000;
				__int64 NewKey = tB * 10000 + tA;
				//should be contained in both filters
				if (f2->ValidatedFortAtB.find(NewKey) == f2->ValidatedFortAtB.end())
				{
					assert(false);
				}
			}
			assert(f1->ValidatedFortAtB.size() == f2->ValidatedFortAtB.size());
			//same case for the invalidators
			for (auto a : f1->InvalidatedFortAtB)
			{
				__int64 tA = a / 10000;
				__int64 tB = a % 10000;
				__int64 NewKey = tB * 10000 + tA;
				//should be contained in both filters
				if (f2->InvalidatedFortAtB.find(NewKey) == f2->InvalidatedFortAtB.end())
				{
					assert(false);
				}
			}
			assert(f1->InvalidatedFortAtB.size() == f2->InvalidatedFortAtB.size());
		}
	}
};

void DivTestModulo3(__int64 iA, __int64 iB)
{
	ReminderTest2::A = iA;
	ReminderTest2::B = iB;
	ReminderTest2::DivTestModuloFilters(iA, iB);
}

void DivTestModulo3()
{
//	DivTestModulo3(23, 41);
	DivTestModulo3(349, 751); // N = 262099 SN = 511
//	DivTestModulo3(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo3(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo3(784727, 918839);
//	DivTestModulo3(6117633, 7219973);
//	DivTestModulo3(26729, 61781);
//	DivTestModulo3(11789, 61781);
}