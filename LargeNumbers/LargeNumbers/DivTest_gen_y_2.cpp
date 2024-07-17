/*
Testing :
	- specific case of DivTest_gen_y_1.cpp
	- testing if we increase m by SQN would jump over c
	- this test keeps increasing C instead decrease it

Conclusion :
	- this approach calculates y from x. It's only worth using after y>SQN/6
*/

#include "StdAfx.h"
#include <vector>
#include <Windows.h>

void DivTestGen_y_v2_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 a = SQN + 2;
	__int64 b = SQN + 2; // added to it to make sure m will become negative
	__int64 m = N - a * b;
	assert(a == b);
	assert(m < 0);
	__int64 mm = -m;

	printf("================\n");
	printf("N = %lld. SQN = %lld. m = %lld\n", N, SQN, m);
	__int64 x = a - A;
	__int64 y = B - b - x;

	__int64 ta = a, tb = b;
	__int64 tm = -(N - ta * tb);
	__int64 min_c = 2 * ta;
	__int64 max_c = 4 * tm;
	while (	B - tb - ta + A != 0 // y can't be 0, there is no c that can produce that
		)
	{
		tm = -(N - ta * tb);

		__int64 tx = ta - A;
		__int64 ty = B - tb - tx;
		assert((tx * tx + tx * ty) == ta * ty + tm);

		assert((ta - tx) * (tb + tx + ty) == N);
		__int64 z = (2 * tx + ty);
		__int64 c = z - ty; // c = 2 * x .. c will increase over time since we increase ta
		__int64 left = (2 * ty + c) * c;
		__int64 right = 4 * (ta * ty + tm);
		assert(left == right);

		// since we expcet c to always b2 positive, it can be achieved in 2 ways : double negative or double positive
		if (c * c < 4 * tm)
		{
			min_c = 2 * ta - 1;		// increases by 2
			max_c = isqrt4(4 * tm);		// increases a lot
			assert(min_c <= c && c <= max_c);
			__int64 min_y = (4 * tm - max_c * max_c) / (2 * max_c - 4 * ta); // decreases a lot
			__int64 max_y = (4 * tm - min_c * min_c) / (2 * min_c - 4 * ta); // decreases by 2
			assert(min_y <= ty && ty <= max_y);
		}
		else
		{
			min_c = isqrt4(4 * tm);		// increases a lot
			max_c = 2 * ta - 1;		// increases by 2
			assert(min_c <= c && c <= max_c);
			__int64 min_y = (4 * tm - min_c * min_c) / (2 * min_c - 4 * ta); // decreases by 2
			__int64 max_y = (4 * tm - max_c * max_c) / (2 * max_c - 4 * ta); // decreases a lot
			assert(min_y <= ty && ty <= max_y);
		}

		// funky
		assert(ty == ((4 * tm - c * c) / (2 * c - 4 * ta)));
		assert(c > isqrt4(4 * tm)); // no magic, if we increase mm with SQN it will still take us SQN steps to reach 2*SQN
		assert(c < 2 * ta); // initially a is SQN

		ta++;
		tb++;
	}
}

void DivTestGen_y_v2()
{
	//DivTestGen_y_v2_(5, 7);
	//DivTestGen_y_v2_(23, 41);
	DivTestGen_y_v2_(349, 751); // N = 262099 , SN = 511
	DivTestGen_y_v2_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestGen_y_v2_(26729, 31793); // N = 849795097 , SN = 29151
	//DivTestGen_y_v2_(784727, 918839); // N = 721037771953 , SN = 849139
	//DivTestGen_y_v2_(127966049, 238311263);
	//DivTestGen_y_v2_(297311557, 1055374679);
	DivTestGen_y_v2_(26729, 918839);
	DivTestGen_y_v2_(6871, 918839);
	//DivTestGen_y_v2_(3, 918839);
	//DivTestGen_y_v2_(349, 918839);
}