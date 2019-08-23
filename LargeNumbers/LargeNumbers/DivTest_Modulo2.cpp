#include "StdAfx.h"

/*
idea is to propose a remainder for A and B and try to "proove" we guessed it correctly or not
(a1 * d1 + ma1)*(b1 * d1 + mb1) = N
(a1 * d1 + ma1) = A = ( a2 * d2 + ma2 )
if d is 2, we know the remainder
if d is 3, ma, mb can be 1 or 2 => generates 4 possible formulas where only 1 should be valid

After some testing, i could not find a way to eliminate badly guessed ma,mb combinations
Every possible input a,b will find a ma,mb combination that will hint that a,b is usable and not bad
*/
namespace ReminderTest {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 Mask;

	#define HighestDivider 55
	struct RemainderFilterStore
	{
		__int64 Divider; // value we fivided A and B with
		int InvalidatedCount; // if we invalidated this filter with some digit test
		int ShouldBeValid; // while testing, we mark the ma and mb that should be always valid
		int ValidatedCount;
		__int64 ma, mb;	// initialized as a guessed value
	};

	std::list<RemainderFilterStore*> Filters[HighestDivider + 1];

	int ValidateFilter(RemainderFilterStore *f, __int64 N, __int64 a, __int64 b, __int64 Mask)
	{
		if ((a % Mask) == ((A / f->Divider) % Mask) && (b % Mask) == ((B / f->Divider) % Mask))
			A = A;
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

		f->ValidatedCount++;
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
		if (A % Divider == ma && B % Divider == mb)
			f->ShouldBeValid = 1;
		else
			f->ShouldBeValid = 0;
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

	int CanFilterUsetAtB(RemainderFilterStore *f, __int64 tA, __int64 tB)
	{
		//let's check if we can generate a tA2,tB2 so that we would obtain the same tA, Tb
		int ret = 0;
		for (__int64 tA2 = 0; tA2 < Mask; tA2++)
			for (__int64 tB2 = 0; tB2 < Mask; tB2++)
			{
				//is this tA and tB combination good based on this selected filter ?
				int FilterPased = ValidateFilter(f, N, tA2, tB2, Mask);
				if (FilterPased == 0)
					continue;
				__int64 AGeneratedFromtAAndFilter = tA2 * f->Divider + f->ma;
				__int64 BGeneratedFromtBAndFilter = tB2 * f->Divider + f->mb;
				if (AGeneratedFromtAAndFilter % Mask == tA % Mask && BGeneratedFromtBAndFilter % Mask == tB % Mask)
				{
					f->ValidatedCount++;
					ret = 1;
//					return ret;
				}
			}
		if(ret == 0)
			f->InvalidatedCount++;
if (f->ShouldBeValid == 0)			return 0;
		return ret;
	}

	int CanAllFilterGroupsUsetAtB(__int64 tA, __int64 tB)
	{
		int SmallFilterCountThatConfrimedtAtB = 0;
		int SmallFilterGroupsUsedForConfirm = 0;
		for (int Divider = 1; Divider <= HighestDivider; Divider++)
		{
			if (Filters[Divider].empty() == false)
				SmallFilterGroupsUsedForConfirm++;

			int GroupValidated = 0;
			for (auto ConfirmFilter : Filters[Divider]) // group of formulas that use the same divider
			{
				if (CanFilterUsetAtB(ConfirmFilter, tA, tB) == 1)
				{
					GroupValidated = 1;
//					break;
				}
			}
			if(GroupValidated == 1)
				SmallFilterCountThatConfrimedtAtB++;
		}
		if (SmallFilterCountThatConfrimedtAtB >= SmallFilterGroupsUsedForConfirm)
		{
//			printf(" \tpossible digit tA=%lld tB=%lld\n", tA, tB);
			return 1;
		}
		return 0;
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

	void DivTestModuloFilters(__int64 iA, __int64 iB)
	{
		N = iA * iB;
		ResultSet.clear();
		//	CreateFiltersForDivider(N,1);
//		CreateFiltersForDivider(N, 2);
//		CreateFiltersForDivider(N, 3);
//		CreateFiltersForDivider(N, 4);
		CreateFiltersForDivider(N, 5);
//		Filters[5].push_back(CreateFilter(5, iA % 5, iB % 5));
//		CreateFiltersForDivider(N, 6);
//		CreateFiltersForDivider(N, 7);
//		CreateFiltersForDivider(N, 8);
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

		int FiltersValid_Initial = CountValidFilters();

		Mask = 1000;
		for (__int64 tA = 0; tA < Mask; tA++)
			for (__int64 tB = 0; tB < Mask; tB++)
			{
				if ((tA * tB) % Mask != N % Mask)
					continue;
				int abMightBeValid = CanAllFilterGroupsUsetAtB(tA, tB);
				if (abMightBeValid >= 1)
				{
					if (IsMirroredResult(tA, tB))
						continue;
					static int CountPossibleGoodDigits = 0;
					printf("%d)possible digit a=%lld b=%lld\n", CountPossibleGoodDigits++, tA, tB);
				}
				else 
				{
					printf(" \tsmall filters invalidated digit tA=%lld tB=%lld\n", tA, tB);
				}
			}
		int FiltersValidated_EndTrain = CountValidatedFilters();
		int FiltersValid_EndTrain = CountValidFilters();
		printf("Started with %d filters. Validated %d filters. Valid filters %d \n", FiltersValid_Initial, FiltersValidated_EndTrain, FiltersValid_EndTrain);
	}
};

void DivTestModulo2(__int64 iA, __int64 iB)
{
	ReminderTest::A = iA;
	ReminderTest::B = iB;
	ReminderTest::DivTestModuloFilters(iA, iB);
}

void DivTestModulo2()
{
	//	DivTestModulo(23, 41);
//	DivTestModulo2(349, 751); // N = 262099 SN = 511
	DivTestModulo2(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo2(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo2(784727, 918839);
//	DivTestModulo2(6117633, 7219973);
//	DivTestModulo2(26729, 61781);
//	DivTestModulo2(11789, 61781);
}