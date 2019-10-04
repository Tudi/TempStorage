#include "StdAfx.h"

/*
anything special in using chained filters ? Maybe there is a combo that will have special hit count ?
*/

namespace ReminderTest6 {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 Mask;
	int HighestDividerFound = 0;

#define MergeTwoValuesForKey(a,b)	(a*100000+b)
#define GetFirstValueFromKey(Key)	(Key/100000)
#define GetSecondValueFromKey(Key)	(Key%100000)
#define HighestDivider 150
#define MAXFilterChain	200
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

	struct FilterChainStruct
	{
		int FullFilterHitCount;
		int ShouldWork;
		std::list<RemainderFilterStore*> Filters;
	};

	std::list<RemainderFilterStore*> Filters[HighestDivider + 1];
	FilterChainStruct FilterChain[MAXFilterChain + 1];

	RemainderFilterStore * CreateFilter(__int64 Divider, __int64 ma, __int64 mb)
	{
		RemainderFilterStore *f = new RemainderFilterStore;
		f->Divider = Divider;
		f->InvalidatedCount = 0;
		f->ma = ma;
		f->mb = mb;
		f->ValidatedCount = 0;
		if ((A % Divider == ma && B % Divider == mb) || (B % Divider == ma && A % Divider == mb))
		//if ((A % Divider == ma && B % Divider == mb))
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
/*				int ShoulfBeValid = 0;
				if ((A % Divider == ma && B % Divider == mb))
					ShoulfBeValid = 1;*/
				Filters[Divider].push_back(CreateFilter(Divider, ma, mb));
			}
	}

	int CanFilterGenerateAB(RemainderFilterStore *f, __int64 tA, __int64 tB)
	{
		__int64 Mask = GetMaskDecimal(tA, tB);
		int aGenerated = 0;
		__int64 Start = tA / f->Divider;
		if (Start > 0)
			Start -= 1;
		Start = Start * f->Divider;
		for (__int64 ta = Start; ta <= tA * tA; ta += f->Divider)
		{
			__int64 tta = ta + f->ma;
			if (tta % Mask == tA)
			{
				f->GeneratedA = tta;
				f->ta = ta;
				aGenerated = 1;
				break;
			}
		}
		int bGenerated = 0;
		Start = tB / f->Divider;
		if (Start > 0)
			Start -= 1;
		Start = Start * f->Divider;
		for (__int64 tb = Start; tb <= tB * tB; tb += f->Divider)
		{
			__int64 ttb = tb + f->mb;
			if (ttb % Mask == tB)
			{
				f->GeneratedB = ttb;
				f->tb = tb;
				bGenerated = 1;
				break;
			}
		}

		if (((aGenerated + bGenerated) == 2))
		{
			f->ReconN = f->GeneratedA * f->GeneratedB;
			return 1;
		}

		f->ReconN = -1;
		return 0;
	}

	void CheckFilterChainCanGenerateAB(__int64 tA, __int64 tB)
	{
		for (int i = 0; i < MAXFilterChain; i++)
		{
			if (FilterChain[i].Filters.empty())
				continue;

			int SuccessCount = 0;
			for (auto f : FilterChain[i].Filters)
				SuccessCount += CanFilterGenerateAB(f, tA, tB);
			if (SuccessCount == FilterChain[i].Filters.size())
				FilterChain[i].FullFilterHitCount++;
		}
	}

	void GenerateFilterChains()
	{
		int FilterSelected[MAXFilterChain];
		memset(FilterSelected, 0, sizeof(FilterSelected));
		//select filters based on the FilterSelected index indicator
		int IncreaseNext = 0;
		int ChainIndex = 0;
		while (IncreaseNext == 0 && ChainIndex < MAXFilterChain)
		{
			FilterChain[ChainIndex].FullFilterHitCount = 0;
			int IsFirstFilter = 1;
			for (int fi = 2; fi < HighestDivider; fi++)
			{
				if (Filters[fi].empty())
					continue;
				int MaxFilterCount = Filters[fi].size();
				int SelectFilterAtIndex = FilterSelected[fi] + IncreaseNext;
				if (IncreaseNext != 0)
				{
					FilterSelected[fi]++;
					IncreaseNext = 0;
				}
				if (SelectFilterAtIndex >= MaxFilterCount)
				{
					SelectFilterAtIndex = 0;
					IncreaseNext = 1;
					FilterSelected[fi] = 0;
				}
				//get the filter that uses this divider and is at index
				int FilterCounter = 0;
				RemainderFilterStore *SelectedFilter = NULL;
				for (auto f : Filters[fi])
				{
					if (FilterCounter == SelectFilterAtIndex)
					{
						SelectedFilter = f;
						break;
					}
					FilterCounter++;
				}
				if (IsFirstFilter == 1)
				{
					IsFirstFilter = 0;
					FilterSelected[fi]++;
				}
				//add it to our chain
				assert(SelectedFilter != NULL);
				FilterChain[ChainIndex].Filters.push_back(SelectedFilter);
			}
			ChainIndex++;
		}
		//for debugging sake, mark the chain that should be working
		for (int i = 0; i < MAXFilterChain; i++)
		{
			if (FilterChain[i].Filters.empty())
				continue;

			int SuccessCount = 0;
			for (auto f : FilterChain[i].Filters)
				SuccessCount += f->ShouldBeValid;

			if (SuccessCount == FilterChain[i].Filters.size())
				FilterChain[i].ShouldWork = 1;
			else
				FilterChain[i].ShouldWork = 0;
		}
	}

	void DivTestModuloFilters(__int64 iA, __int64 iB)
	{
		N = iA * iB;

		for (int Divider = 1; Divider <= HighestDivider; Divider++)
			Filters[Divider].clear();


/*		__int64 Divider = 16;
		Filters[Divider].push_back(CreateFilter(Divider, A % Divider, B % Divider));
		Divider = 25;
		Filters[Divider].push_back(CreateFilter(Divider, A % Divider, B % Divider)); /**/

/*		for(int i=2;i<=7;i++)
			CreateFiltersForDivider(N, i); /**/

//		CreateFiltersForDivider(N, 16);
//		CreateFiltersForDivider(N, 25);
		CreateFiltersForDivider(N, 32);

		GenerateFilterChains();

		for (int Divider = HighestDivider; Divider > 1; Divider--)
			if (Filters[Divider].empty() == false)
			{
				HighestDividerFound = Divider;
				break;
			}

		__int64 SN = isqrt(N);
		__int64 Mask = GetMaskDecimal(SN);
		for (__int64 tA = 0; tA < Mask; tA++)
			for (__int64 tB = 0; tB < Mask; tB++)
			{
				__int64 Mask2 = GetMaskDecimal(tA, tB);
				if (N % Mask2 != (tA * tB) % Mask2)
					continue;
				CheckFilterChainCanGenerateAB(tA, tB);
			}
		
		A = A;
	}
};

void DivTestModulo6(__int64 iA, __int64 iB)
{
	ReminderTest6::A = iA;
	ReminderTest6::B = iB;
	ReminderTest6::DivTestModuloFilters(iA, iB);
}

void DivTestModulo6()
{
	//	DivTestModulo6(23, 41);
	DivTestModulo6(349, 751); // N = 262099 SN = 511
//	DivTestModulo6(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo6(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo6(784727, 918839);
//	DivTestModulo6(6117633, 7219973);
//	DivTestModulo6(26729, 61781);
//	DivTestModulo6(11789, 61781);
}