/*
Fermat's number theorem : IF !! A=a^2+b^2	B=c^2+d^2	N=A*B=(a*c+b*d)^2+(a*d-b*c)^2
Just have to have it in the list of failed tries

conclusion :
	- only works for some numbers. Not all !!
	- good implementation does around (4*sqrt(sqrt(N)))^2 steps max
	- current implementation is horrible : 8 times more steps than required
	- later realized that A=a^2+b^2+c^2+d^2 -> needs 4 variables not 2 as I made it
*/


#include "StdAfx.h"
#include <vector>
#include <Windows.h>

struct FermatGenState
{
	__int64 a, b, c, d;
	__int64 e, f, g, h; // -> TODO : should have implemented these
};

size_t IsAcceptableNewState(const __int64 &NMasked, const __int64 &N, const __int64 &tMask, const FermatGenState &newState)
{
	const __int64 a = newState.a;
	const __int64 b = newState.b;
	const __int64 c = newState.c;
	const __int64 d = newState.d;

	__int64 tN1 = (a * a + b * b) * (c * c + d * d);
	if ((tN1 % tMask) != NMasked)
	{
		return 0;
	}

	if (tN1 > N)
	{
		return -1;
	}
//	if (tN1 == N)
	{
//		return 2;
	}

	__int64 p1 = (a * c + b * d);
	__int64 p2 = (a * d - b * c);
	__int64 tN2 = p1 * p1 + p2 * p2;
	if ((tN2 % tMask) != NMasked)
	{
		return 0;
	}

	if (tN2 == N)
	{
		return 2;
	}

	return 1;
}

size_t IsFermatCompatible(__int64 num)
{
	__int64 SQN = (__int64)isqrt(num);
	for (__int64 a = 1; a < SQN; a++)
	{
		for (__int64 b = a; b < SQN; b++)
		{
			if ((a * a + b * b) == num)
			{
				printf("%lld=%lld^2+%lld^2\n", num, a, b);
				return 1;
			}
		}
	}
	return 0;
}

void DivTestFermatGen1_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN;
	__int64 b = SQN;
	__int64 m = N - a * b;

	printf("================\n");
	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
//	__int64 x = a - A;
//	__int64 y = B - b - x;
//	printf("Looking for x=%lld y=%lld\n", x, y);
	if (IsFermatCompatible(A) == 0 || IsFermatCompatible(B) == 0)
	{
		printf("This can't be solved by fermat\n");
		return;
	}

#if 0
	{
		FermatGenState t;
		t.a = 52;
		t.b = 155;
		t.c = 47;
		t.d = 172;
		size_t res = IsAcceptableNewState(N % 1000, N, 1000, t);
	}
	{
		FermatGenState t;
		t.a = 52;
		t.b = 55;
		t.c = 47;
		t.d = 72;
		size_t res = IsAcceptableNewState(N % 100, N, 100, t);
	}
#endif

	std::vector<FermatGenState> vPrevStates;
	vPrevStates.push_back(FermatGenState{ 0,0,0,0 });
	std::vector<FermatGenState> vCurStates;
	__int64 shift = 1;
	while (1)
	{
		for (size_t i = 0; i < vPrevStates.size(); i++)
		{
			__int64 mask = 10 * shift;
			__int64 NMasked = N % mask;
			for (__int64 a = 0; a < 10; a++)
			{
				for (__int64 b = 0; b < 10; b++)
				{
					for (__int64 c = 0; c < 10; c++)
					{
						for (__int64 d = 0; d < 10; d++)
						{
							FermatGenState t;
							t.a = a * shift + vPrevStates[i].a;
							t.b = b * shift + vPrevStates[i].b;
							t.c = c * shift + vPrevStates[i].c;
							t.d = d * shift + vPrevStates[i].d;
#if 0
							if (t.a == 2 && t.b == 5 && t.c == 7 && t.d == 2)
							{
								t.a = t.a;
							}
							if (t.a == 52 && t.b == 55 && t.c == 47 && t.d == 72)
							{
								t.a = t.a;
							}
							if (t.a == 52 && t.b == 155 && t.c == 47 && t.d == 172)
							{
								t.a = t.a;
							}
#endif
							size_t res = IsAcceptableNewState(NMasked, N, mask, t);
							if (res == 1)
							{
								vCurStates.push_back(t);
							}
							// at this point this will never lead to a solution
							else if (res == -1)
							{
								break;
							}
							else if (res == 2)
							{
								printf("Found a solution where a=%lld, b=%lld, c=%lld, e=%lld. PrevStateCount %lld. Cur state count %lld\n", 
									t.a, t.b, t.c, t.d, vPrevStates.size(), vCurStates.size());
//								return;
							}
						}
					}
				}
			}
		}

		// cur state becomes prev state
		vPrevStates.clear();
		vPrevStates = vCurStates;
		vCurStates.clear();

		// move up 1 digit
		shift *= 10;
	}
}

void DivTestFermatGen1()
{
	//DivTestFermatGen1_(5, 7);
	//DivTestFermatGen1_(23, 41);
	//DivTestFermatGen1_(349, 751); // N = 262099 , SN = 511
	//DivTestFermatGen1_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestFermatGen1_(26729, 31793); // N = 849795097 , SN = 29151
	//DivTestFermatGen1_(784727, 918839); // N = 721037771953 , SN = 849139
	//DivTestFermatGen1_(127966049, 238311263);
	//DivTestFermatGen1_(297311557, 1055374679);
	//DivTestFermatGen1_(26729, 918839);
	//DivTestFermatGen1_(6871, 918839);
	//DivTestFermatGen1_(3, 918839);
	//DivTestFermatGen1_(349, 918839);
}