#include "StdAfx.h"

/*
considering we have the modulo, how hard would it be to find the divider ? : N = A * B = ( d * a + ma ) * ( d * b + mb )
*/
namespace ReminderTest5 {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 Mask;
	int HighestDividerFound = 0;

#define MergeTwoValuesForKey(a,b) (a*100000+b)
#define GetFirstValueFromKey(Key)	(Key/100000)
#define GetSecondValueFromKey(Key)	(Key%100000)
#define HighestDivider 150
	struct RemainderFilterStore
	{
		__int64 Divider; // value we fivided A and B with
		__int64 ma, mb;	// initialized as a guessed value
		__int64 ta, tb, GeneratedA, GeneratedB, ReconN;
	};

	std::list<RemainderFilterStore*> Filters[HighestDivider + 1];


	RemainderFilterStore * CreateFilter(__int64 Divider, __int64 ma, __int64 mb)
	{
		RemainderFilterStore *f = new RemainderFilterStore;
		f->Divider = Divider;
		f->ma = ma;
		f->mb = mb;
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

	__int64 GetDivider(__int64 tN, __int64 Modulo)
	{
		std::list<__int64> Dividers;
		__int64 tSN = isqrt(tN);
		for (__int64 i = 2; i < tSN; i++)
			if (tN % i == 0)
				Dividers.push_back(i);
		if (Dividers.size() <= 1)
			return 0;
		Dividers.sort();
		// d should be smaller than A. There is no guarantee that any of the d values will actually work
		// d * a * b + mb * a + b = ( N - mb ) / d
		// b is at least 1 because module is smaller than SN
		// since ma is 1, a should be a large value, maybe even close to SN
		// if m = 1 , b > SN , d * a * b + a + b = ( N - 1 ) / d ... d * a * sn + a + sn = ( N - 1 ) / d
		// d as a safe value is the smallest divisor. There might be larger values for him, but it's not a safe value
		__int64 ret = *(Dividers.begin());
//		if (ret <= Modulo)
//			return 0;
		return ret;
	}

	void DivTestModuloFilters(__int64 iA, __int64 iB)
	{
		N = iA * iB;

		__int64 SN = isqrt(N);
		for(int i=3;i< SN;i++)
			if (IsPrime(i) == 1)
			{
				__int64 PossibleDivider = GetDivider(N - i, i);
				if (PossibleDivider == 0)
					continue;
				RemainderFilterStore *f = CreateFilter(PossibleDivider, 1, i);
				f->ta = (iA - f->ma) / f->Divider;
				f->tb = (iB - f->mb) / f->Divider;
				f->GeneratedA = f->ta * f->Divider + f->ma;
				f->GeneratedB = f->tb * f->Divider + f->mb;
				f->ReconN = f->GeneratedA * f->GeneratedB;
				assert(f->ReconN == N);
				Filters[2].push_back(f);
			}
		A = A;
	}
};

void DivTestModulo5(__int64 iA, __int64 iB)
{
	ReminderTest5::A = iA;
	ReminderTest5::B = iB;
	ReminderTest5::DivTestModuloFilters(iA, iB);
}

void DivTestModulo5()
{
	//	DivTestModulo5(23, 41);
	DivTestModulo5(349, 751); // N = 262099 SN = 511
//	DivTestModulo5(6871, 7673); // N = 52721183 , SN = 7260
//	DivTestModulo5(26729, 31793); // N = 849795097 , SN = 29151
//	DivTestModulo5(784727, 918839);
//	DivTestModulo5(6117633, 7219973);
//	DivTestModulo5(26729, 61781);
//	DivTestModulo5(11789, 61781);
}