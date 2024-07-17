/*
is generating y^2+4*(m+a*y)=z^2 the same speed as generating for example (2*y+c)*c=4*(a*y+m)

Conclusion :
	- exactly the same speed for bot formulas
	- every digit adds 10x states. Same as generating A*B
	- second version is slightly better as it is searching for a number smaller than y
*/

#include "StdAfx.h"
#include <vector>
#include <Windows.h>

struct gen_y_1_state
{
	__int64 y, z;
};

static size_t IsAcceptableNewState_1(const __int64& tMask, const __int64 a, const __int64 m, const gen_y_1_state& newState)
{
	//y^2+4*(a*y+m)=z^2		where	 z=(2*x+y)
	const __int64 left = newState.y * newState.y + 4 * (a * newState.y + m);
	const __int64 right = newState.z * newState.z;

	// we are expecting a single solution. Maybe we are wrong
	if (left == right)
	{
		return 2;
	}

	// is this a partial solution ?
	const __int64 leftMasked = left % tMask;
	const __int64 rightMasked = right % tMask;
	if (leftMasked != rightMasked)
	{
		return 0;
	}

	return 1;
}

static size_t IsAcceptableNewState_2(const __int64& tMask, const __int64 a, const __int64 m, const gen_y_1_state& newState)
{
	//(2 * y + c)* c = 4 * (a * y + m)
	const __int64 left = (2 * newState.y + newState.z) * newState.z;
	const __int64 right = 4 * (a * newState.y + m);

	// we are expecting a single solution. Maybe we are wrong
	if (left == right)
	{
		return 2;
	}

	// is this a partial solution ?
	const __int64 leftMasked = left % tMask;
	const __int64 rightMasked = right % tMask;
	if (leftMasked != rightMasked)
	{
		return 0;
	}

	return 1;
}

void DivTestGen_y_v1_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN + 1;
	__int64 b = SQN + 1; // added to it to make sure m will become negative
	__int64 m = N - a * b;
	assert(a == b);
	assert(m < 0);
	__int64 mm = -m;

	printf("================\n");
	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 x = a - A;
	__int64 y = B - b - x;

	// testing approach 1 : y^2+4*(m+a*y)=z^2
	{
		assert((a - x) * (b + x + y) == N);
		__int64 z = (2 * x + y);
		__int64 left = z * z;
		__int64 right = y * y + 4 * (a * y + mm);
		assert(left == right);
		printf("Looking for y=%lld and z=%lld\n", y, z);
		std::vector<gen_y_1_state> vPrevStates;
		vPrevStates.push_back(gen_y_1_state{ 0,0 });
		std::vector<gen_y_1_state> vCurStates;
		__int64 shift = 1;
		while (shift < SQN)
		{
			for (size_t i = 0; i < vPrevStates.size(); i++)
			{
				__int64 mask = 10 * shift;
				__int64 NMasked = N % mask;
				for (__int64 y = 0; y < 10; y++)
				{
					for (__int64 z = 0; z < 10; z++)
					{
						gen_y_1_state t;
						t.y = y * shift + vPrevStates[i].y;
						t.z = z * shift + vPrevStates[i].z;
#if 0
						if (t.y == 6 && t.z == 2)
						{
							t.y = t.y;
						}
						if (t.y == 76 && t.z == 2)
						{
							t.y = t.y;
						}
#endif
						size_t res = IsAcceptableNewState_1(mask, a, mm, t);
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
							printf("Found a solution where y=%lld, z=%lld. PrevStateCount %lld. Cur state count %lld\n",
								t.y, t.z, vPrevStates.size(), vCurStates.size());
//							return;
						}
					}
				}
			}

			printf("Mask %lld and number of states %lld\n", shift, vCurStates.size());
			// cur state becomes prev state
			vPrevStates.clear();
			vPrevStates = vCurStates;
			vCurStates.clear();

			// move up 1 digit
			shift *= 10;
		}
	}
	printf("\nSame thing, just using a non sqaure formula\n");
	// testing approach 2 : (2*y+c)*c=4*(a*y+m)
	{
		assert((a - x) * (b + x + y) == N);
		__int64 z = (2 * x + y);
		__int64 c = z - y; // c = 2 * x
		__int64 left = (2 * y + c) * c;
		__int64 right = 4 * (a * y + mm);
		assert(left == right);

		// funky
		assert(y == ((4*mm - c*c)/(2*c-4*a)));
		assert(c > isqrt(4 * mm)); // no magic, if we increase mm with SQN it will still take us SQN steps to reach 2*SQN
		assert(c < 2 * a); // initially a is SQN

		printf("Looking for y=%lld and c=%lld\n", y, c);
		std::vector<gen_y_1_state> vPrevStates;
		vPrevStates.push_back(gen_y_1_state{ 0,0 });
		std::vector<gen_y_1_state> vCurStates;
		__int64 shift = 1;
		while (shift < SQN)
		{
			for (size_t i = 0; i < vPrevStates.size(); i++)
			{
				__int64 mask = 10 * shift;
				__int64 NMasked = N % mask;
				for (__int64 y = 0; y < 10; y++)
				{
					for (__int64 z = 0; z < 10; z++)
					{
						gen_y_1_state t;
						t.y = y * shift + vPrevStates[i].y;
						t.z = z * shift + vPrevStates[i].z;
#if 0
						if (t.y == 6 && t.z == 6)
						{
							t.y = t.y;
						}
						if (t.y == 76 && t.z == 26)
						{
							t.y = t.y;
						}
#endif
						size_t res = IsAcceptableNewState_2(mask, a, mm, t);
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
							printf("Found a solution where y=%lld, z=%lld. PrevStateCount %lld. Cur state count %lld\n",
								t.y, t.z, vPrevStates.size(), vCurStates.size());
							//							return;
						}
					}
				}
			}

			printf("Mask %lld and number of states %lld\n", shift, vCurStates.size());
			// cur state becomes prev state
			vPrevStates.clear();
			vPrevStates = vCurStates;
			vCurStates.clear();

			// move up 1 digit
			shift *= 10;
		}
	}
}

void DivTestGen_y_v1()
{
	//DivTestGen_y_v1_(5, 7);
	//DivTestGen_y_v1_(23, 41);
	DivTestGen_y_v1_(349, 751); // N = 262099 , SN = 511
	DivTestGen_y_v1_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestGen_y_v1_(26729, 31793); // N = 849795097 , SN = 29151
	//DivTestGen_y_v1_(784727, 918839); // N = 721037771953 , SN = 849139
	//DivTestGen_y_v1_(127966049, 238311263);
	//DivTestGen_y_v1_(297311557, 1055374679);
	DivTestGen_y_v1_(26729, 918839);
	DivTestGen_y_v1_(6871, 918839);
	//DivTestGen_y_v1_(3, 918839);
	//DivTestGen_y_v1_(349, 918839);
}