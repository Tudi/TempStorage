/*
Testing :
	- base formula : SQN * x + m = y1 * ( SQN - x )
	- step 1 is to get a ratio between x/y
	- using the ratio, define a new y1 = y2 + d1/d2 * x
	- step 2 substitute the old y with new y
	- new y should generate larger x for the same y tries
	- repeat the proces

Conclusion :
	- y1 derived from y2 is just a roundup of the value. Even if y2 is not precise, it's good enough
	- this is working with 2x length numbers. Be prepared to overflow at 32bit numbers
	- might be a bit faster only as long as y/x < 2 after that, iterating x is faster
*/

#include "StdAfx.h"
#include <vector>
#include <Windows.h>
#include <conio.h>

struct state_store {
	__int64 A;
	__int64 B;
	__int64 N;
	__int64 SQN;
	__int64 m;
	__int64 x1_sol;
	__int64 y1_sol;
	// below will change all the time
	__int64 x1;
	__int64 y1;
	__int64 y2;
	__int64 d1, d2;
	__int64 search_loop_count;
	void init(state_store &self)
	{
		memset(&self, 0, sizeof(state_store));
	}
};

// SQN * x + m = y * ( SQN - x )
__int64 get_y1_from_x(__int64 x, state_store& state)
{
	__int64 y = (state.SQN * x + state.m) / (state.SQN - x);
	return y;
}

void GetXForY2(state_store& state, __int64 y2, __int64& out_x_1, __int64& out_x_2)
{
	__int64 a = state.d1;
	if (a <= 0)
	{
		out_x_1 = 0;
		out_x_2 = 0;
		return;
	}
	__int64 b = state.d2 * state.SQN - state.d1 * state.SQN + y2;
	__int64 c = state.d2 * state.m - y2 * state.SQN;
	__int64 delta = b * b - 4 * a * c;
	if (delta <= 0)
	{
		out_x_1 = 0;
		out_x_2 = 0;
		return;
	}
	__int64 square_delta = sqrt(delta);
	out_x_1 = (-b + square_delta) / (2 * a); // we care about this one probably
//	out_x_2 = (-b - square_delta) / (2 * a);
}

void temp_estimate_importance_of_complement(state_store& state, __int64 y2_base_val)
{
	__int64 trd_y2_sol_constructed, trd_y2_sol_initial = (y2_base_val + 0) * state.d2;
	__int64 tru_y2_sol_constructed, tru_y2_sol_initial = (y2_base_val + 1) * state.d2;
	trd_y2_sol_constructed = trd_y2_sol_initial;
	tru_y2_sol_constructed = tru_y2_sol_initial;
	// since we are not using a precise y, how unprecise would x get from it ?
	// expecting a value range here. But how large of a range ?
	__int64 trdx1, trdx2;
	__int64 trux1, trux2;
	GetXForY2(state, trd_y2_sol_constructed, trdx1, trdx2);
	GetXForY2(state, tru_y2_sol_constructed, trux1, trux2);

	// the obtained x would enforce a specific complement on the input y
	// the complement is not precise, but it might improve precision
	__int64 trdx1_gen_complement = state.d2 - (state.d1 * trdx1) % state.d2;
	//	__int64 trdx2_gen_complement = state.d2 - (state.d1 * trdx2) % state.d2;
	__int64 trux1_gen_complement = state.d2 - (state.d1 * trux1) % state.d2;
	//	__int64 trux2_gen_complement = state.d2 - (state.d1 * trux2) % state.d2;

	trd_y2_sol_constructed += trdx1_gen_complement;
	tru_y2_sol_constructed += trux1_gen_complement;

	GetXForY2(state, trd_y2_sol_constructed, trdx1, trdx2);
	GetXForY2(state, tru_y2_sol_constructed, trux1, trux2);

	// (x*sqn+m)/(sqn-x)=y - we presumably guessed x, get y from base formula
	__int64 trd_y1_1 = (trdx1 * state.SQN + state.m) / (state.SQN - trdx1);
//	__int64 trd_y1_2 = (trdx2 * state.SQN + state.m) / (state.SQN - trdx2);
	__int64 tru_y1_1 = (trux1 * state.SQN + state.m) / (state.SQN - trux1);
//	__int64 tru_y1_2 = (trux2 * state.SQN + state.m) / (state.SQN - trux2);
 
	// using the obtained integer y1, generate back the y2 we started from
	// y2 = y1 - d1/d2 * x
	__int64 trd_y2_1 = trd_y1_1 - state.d1 * trdx1 / state.d2;
//	__int64 trd_y2_2 = trd_y1_2 - state.d1 * trdx2 / state.d2;
	__int64 tru_y2_1 = tru_y1_1 - state.d1 * trux1 / state.d2;
//	__int64 tru_y2_2 = tru_y1_2 - state.d1 * trux2 / state.d2;
	__int64 attachtomedebugger = 1;
}

// let's guess y2/d2
//		so y1 = y2/d2 + d1/d2 * x
//		smallest y2 should be 0
//		y2 should increase by about d2
//		maybe there is a way pingpong to guess the precision of y2 based on x
// d2 * SQN * x + d2 * m = (y2 + d1 * x) * ( SQN - x )
// d2 * SQN * x + d2 * m = y2 * SQN + d1 * x * SQN - y2 * x - d1 * x^2
// d1 * x^2 + ( d2 * SQN - d1 * SQN + y2) * x + d2 * m - y2 * SQN = 0
__int64 step2_try_x_y_combinations_int(state_store& state)
{
	__int64 x;
	__int64 y2_sol = state.y1_sol * state.d2 - state.d1 * state.x1_sol;
	__int64 val_needed_to_complement = (state.d1 * state.x1_sol) % state.d2;
	__int64 the_value_that_complements = state.d2 - val_needed_to_complement;
	__int64 y2_base_val = (y2_sol - the_value_that_complements) / state.d2;
	__int64 y2_sol_constructed = y2_base_val * state.d2 + the_value_that_complements;
	assert(y2_sol_constructed == y2_sol);
//	printf("Extracted values size d1/d2*x=%f\n", state.d1 * state.x1_sol / float(state.d2));
	printf("Ratio %f when using d1=%lld and d2=%lld\n", state.d1 / double(state.d2), state.d1, state.d2);
	printf("Looking for y2 = %lld * %lld + %lld = %lld = %lld.\n", 
			y2_base_val, state.d2, the_value_that_complements, y2_sol, y2_sol / state.d2);

	__int64 x_min = state.x1;
	__int64 y2_min = state.y1 - state.d1 * x_min / state.d2; // if calculated d1/d2 correctly, this should be 1 ( or 0 )

	// temp junk code : what y1 would round down and round up generate
	{
//		temp_estimate_importance_of_complement(state, y2_base_val);
//		temp_estimate_importance_of_complement(state, y2_base_val / 2);
	}

	const __int64 y2limit = 100;
	for (__int64 y2_base = 0; y2_base < y2limit; y2_base++) {
		__int64 y2 = y2_base * state.d2;
		__int64 tx1, tx2;
		GetXForY2(state, y2, tx1, tx2);
		//x = max(tx1, tx2);
		x = tx1;
		if (x == state.x1_sol)
		{
			state.search_loop_count += y2_base;
			printf("Found solution for x = %lld using y2 = %lld. Tests %lld. Should exit now\n", x, y2, state.search_loop_count);
			break;
		}
		if (y2_base_val == y2_base) {
			state.search_loop_count += y2_base;
			printf("Chances are we failed to find x solution for y2 base %lld. Got x=%lld expected %lld\n", y2_base, tx1, state.x1_sol);
			return 1;
		}
		if (x > state.x1_sol)
		{
			__int64 temp_y1 = get_y1_from_x(x, state);
			printf("Should stop now. x = %lld is larger than solution. y2 base = %lld \n", x, y2_base);
		}
	}
	state.d1 = get_y1_from_x(x, state); // round down
	state.d2 = x + 1; // we add 1 as roundup
	// for logs only ?
	state.x1 = x;
	state.y1 = state.d1;
	if (state.x1 == state.x1_sol) {
		return 1;
	}
	state.search_loop_count += y2limit;
	if (state.x1 > state.x1_sol) {
		printf("Sadly we missed our solution\n");
		return 1;
	}
	if (state.d1 <= 100) {
		printf("Code broke. Maybe overflow. Exiting.\n");
		return 1;
	}
	return 0;
}

void DivTestGen_y_readjust_(__int64 A, __int64 B)
{
	state_store state;
	state.init(state);
	state.A = A;
	state.B = B;
	state.N = A * B;
	state.SQN = (__int64)isqrt(state.N);
	state.m = state.N - state.SQN * state.SQN;

	printf("================\n");
	printf("N = %lld. SQN = %lld. m = %lld\n", state.N, state.SQN, state.m);
	state.x1_sol = state.SQN - state.A;
	state.y1_sol = state.B - state.SQN;
	printf("x1 = %lld. y1 = %lld ( old way y = %lld) \n", state.x1_sol, state.y1_sol, state.y1_sol - state.x1_sol);

	// can be any number as long as the ratio is 1
	state.d1 = state.d2 = 1001;

	__int64 cycles_tried = 0;
	while (step2_try_x_y_combinations_int(state) == 0) {
		cycles_tried++;
	}
	printf("Tests made before exiting loop : %lld . Old way %lld \n", state.search_loop_count, state.y1_sol - state.x1_sol);
	_getch();
}

void DivTestGen_y_readjust()
{
	DivTestGen_y_readjust_(5, 7);
	DivTestGen_y_readjust_(23, 41);
	DivTestGen_y_readjust_(349, 751); // N = 262099 , SN = 511
	DivTestGen_y_readjust_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestGen_y_readjust_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestGen_y_readjust_(6871, 31793); 
	DivTestGen_y_readjust_(349, 31793);
	//DivTestGen_y_readjust_(784727, 918839); // N = 721037771953 , SN = 849139
	//DivTestGen_y_readjust_(127966049, 238311263);
	//DivTestGen_y_readjust_(297311557, 1055374679);
	//DivTestGen_y_readjust_(26729, 918839);
	//DivTestGen_y_readjust_(6871, 918839);
	//DivTestGen_y_readjust_(3, 918839);
	//DivTestGen_y_readjust_(349, 918839);
}