/*
Checking :
	- A = SQN - x		B = SQN + x + y		x^2 + m1 = y * (SQN-x)
	- A = div + c		B = N/div - e		div * e + m1 = c * (N/div-e)
	- checkers : A*B=N		(x^2+m1)*(div * e + m2)/(y*c)=N
	- wanted to see every digit generates 10 or 100 new versions

Conclusion :
	- 10x per digit
*/

#include "StdAfx.h"
#include <vector>
#include <Windows.h>

struct global_xyce_state
{
	__int64 N, SQN, m1, div, Ndiv, m2;
	__int64 shift, mask;
	__int64 x, y, c, e;
};
struct gen_xyce_state
{
	__int64 x, y, c, e;
};

static void ProgressState(const global_xyce_state &gs, const gen_xyce_state& pre, const __int64 x, const __int64 y, 
	const __int64 c, const __int64 e, gen_xyce_state& out)
{
	out.x = pre.x + x * gs.shift;
	out.y = pre.y + y * gs.shift;
	out.c = pre.c + c * gs.shift;
	out.e = pre.e + e * gs.shift;
}

static __int64 IsSolution(const global_xyce_state& gs, const gen_xyce_state& newState)
{
	// is x a sol
	if (gs.x == newState.x) return 1;
	if (gs.y == newState.y) return 1;
	if (gs.c == newState.c) return 1;
	if (gs.e == newState.e) return 1;
	return 0;
}

static __int64 IsAcceptableNewState_1(const global_xyce_state& gs, const gen_xyce_state& newState)
{
	{
		const __int64 left1 = (gs.SQN - newState.x) % gs.mask; // A
		const __int64 left2 = (gs.SQN + newState.x + newState.y) % gs.mask; // B
		const __int64 left = left1 * left2;
		const __int64 right = gs.N;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 1;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -1;
		}
	}
	{
		const __int64 left1 = (gs.div + newState.c) % gs.mask;
		const __int64 left2 = (gs.Ndiv - newState.e) % gs.mask;
		const __int64 left = left1 * left2;
		const __int64 right = gs.N;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 2;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -2;
		}
	}
/*
	- A = SQN - x		B = SQN + x + y		x^2 + m1 = y * (SQN-x)
	- A = div + c		B = N/div - e		div * e + m1 = c * (N/div-e)
	- checkers : A*B=N		(x^2+m1)*(div * e + m2)/(y*c)=N
*/
	{
		const __int64 left1 = (newState.x * newState.x + gs.m1) % gs.mask;
		const __int64 left2 = (gs.div * newState.e + gs.m2) % gs.mask;
		const __int64 left = left1 * left2;
		const __int64 right1 = gs.N % gs.mask;
		const __int64 right2 = (right1 * newState.y) % gs.mask;
		const __int64 right = right2 * newState.c;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 3;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -3;
		}
	}
	{
		const __int64 left = (gs.div * newState.e + gs.m2);
		const __int64 right1 = (gs.Ndiv - newState.e) % gs.mask;
		const __int64 right = newState.c * right1;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 4;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -4;
		}
	}
	{
		const __int64 left = (newState.x * newState.x + gs.m1);
		const __int64 right1 = (gs.SQN - newState.x) % gs.mask;
		const __int64 right = newState.y * right1;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 5;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -5;
		}
	}
	{
		const __int64 left1 = (gs.SQN - newState.x) % gs.mask;
		const __int64 left2 = (gs.Ndiv - newState.e) % gs.mask;
		const __int64 left = left1 * left2;
		const __int64 right = gs.N;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 6;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -6;
		}
	}
	{
		const __int64 tB = (gs.Ndiv - newState.e) % gs.mask;
		const __int64 left1 = (newState.x * newState.x + gs.m1) % gs.mask;
		const __int64 left = left1 * tB;
		const __int64 right = gs.N * newState.y;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 7;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -7;
		}
	}
	{
		const __int64 tA = (gs.SQN - newState.x) % gs.mask;
		const __int64 left1 = (gs.div * newState.e + gs.m2) % gs.mask;
		const __int64 left = tA * left1;
		const __int64 right = gs.N * newState.c;

		// we are expecting a single solution. Maybe we are wrong
		if (left == right && IsSolution(gs, newState))
		{
			return 8;
		}

		// is this a partial solution ?
		const __int64 leftMasked = left % gs.mask;
		const __int64 rightMasked = right % gs.mask;
		if (leftMasked != rightMasked)
		{
			return -8;
		}
	}
	return 0;
}

void DivTestGen_xyce_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN;
//	__int64 a = MAX(A + 100, SQN * 2/3);
	__int64 b = a; // added to it to make sure m will become negative
	__int64 m = N - a * b;

	global_xyce_state gs;
	gs.N = N;
//	gs.SQN = SQN;
	gs.SQN = a;
	gs.m1 = m;
	gs.div = 2;
	gs.Ndiv = N / gs.div;
	gs.m2 = N - gs.div * gs.Ndiv;
	assert(gs.SQN == a && a == b);

	printf("================\n");
	printf("N = %lld. A = %lld B = %lld SQN = %lld. m = %lld\n", N, A, B, SQN, m);
	__int64 _x = a - A;
	__int64 _y = B - b - _x;
	__int64 _c = A - gs.div;
	__int64 _e = gs.Ndiv - B;
	gs.x = _x;
	gs.y = _y;
	gs.c = _c;
	gs.e = _e;
	printf("x = %lld. y = %lld. c = %lld. e = %lld\n", _x, _y, _c, _e);

	std::vector<gen_xyce_state> vPrevStates;
	vPrevStates.push_back(gen_xyce_state{ 0,0,0,0 });
	std::vector<gen_xyce_state> vCurStates;
	gs.shift = 1;
	while (gs.shift < SQN)
	{
		for (size_t i = 0; i < vPrevStates.size(); i++)
		{
			const auto prevState = vPrevStates[i];
			gs.mask = 10 * gs.shift;
			__int64 NMasked = N % gs.mask;
			for (__int64 x = 0; x < 10; x++)
			{
				for (__int64 y = 0; y < 10; y++)
				{
					for (__int64 c = 0; c < 10; c++)
					{
						for (__int64 e = 0; e < 10; e++)
						{
							gen_xyce_state t;
							ProgressState(gs, prevState, x, y, c, e, t);
#if 1
							if (t.x % gs.mask == _x % gs.mask && 
								t.y % gs.mask == _y % gs.mask && 
								t.c % gs.mask == _c % gs.mask && 
								t.e % gs.mask == _e % gs.mask)
							{
								t.y = t.y;
								__int64 res = IsAcceptableNewState_1(gs, t);
								if (res < 0)
								{
									res = IsAcceptableNewState_1(gs, t);
									assert(res >= 0);
								}
							}
#endif
							__int64 res = IsAcceptableNewState_1(gs, t);
							if (res == 0)
							{
								vCurStates.push_back(t);
							}
							// at this point this will never lead to a solution
							else if (res > 0)
							{
//								printf("%lld Found a solution where x=%lld, y=%lld, c=%lld, e=%lld. PrevStateCount %lld. Cur state count %lld\n",
//									res, t.x, t.y, t.c, t.e, vPrevStates.size(), vCurStates.size());
//								return;
								vCurStates.push_back(t);
							}
						}
					}
				}
			}
		}

		printf("Mask %lld and number of states %lld\n", gs.shift, vCurStates.size());
		// cur state becomes prev state
		vPrevStates.clear();
		vPrevStates = vCurStates;
		vCurStates.clear();
		vCurStates.reserve(vPrevStates.size() * 10);

		// move up 1 digit
		gs.shift *= 10;
	}
}

void DivTestGen_xyce()
{
//	DivTestGen_xyce_(5, 7);
//	DivTestGen_xyce_(23, 41);
//	DivTestGen_xyce_(349, 751); // N = 262099 , SN = 511
//	DivTestGen_xyce_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestGen_xyce_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestGen_xyce_(200057, 784727);
//	DivTestGen_xyce_(784727, 918839); // N = 721037771953 , SN = 849139
	//DivTestGen_xyce_(127966049, 238311263);
	//DivTestGen_xyce_(297311557, 1055374679);
	//DivTestGen_xyce_(26729, 918839);
	//DivTestGen_xyce_(6871, 918839);
	//DivTestGen_xyce_(3, 918839);
	//DivTestGen_xyce_(349, 918839);
}