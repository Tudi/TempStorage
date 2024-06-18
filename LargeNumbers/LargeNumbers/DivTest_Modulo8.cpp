/*
this is the Sieve of Eratosthenes
some numbers that have reduced mod count compared to others :
	6=2*3			mods:1,5						2/6=0.33%
		8=2*4			mods:3,5,7					3/8=0.375%
		10=2*5			mods:1,3,7,9
		12=3*4			mods:1,5,7,11				4/12=0.33%
		15=3*5			mods:1,2,4,7,8,11,13,14		-> 8 generators while divides A/15...8/15 difficulty reduction ratio 8/30=0.26%
	30=2*3*5		mods:1,7,11,13,17,19,23,29	-> 8 generators while divides A/30...8/30 difficulty reduction ratio
	210=2*3*5*7		mods:11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211	
											-> 42 generators while divides A/210...42/210 difficulty reduction ratio = 0.2%
	2310=2*3*5*7*11	mods:13...				-> 340 generators while divides A/2310...340/2310 difficulty reduction ratio = 0.14%
	30030=2*3*5*7*11*13
	510510=2*3*5*7*11*13*17
	9699690=2*3*5*7*11*13*17*19
	
	42=2*3*7			mods:1,5,11,13,17,19,23,29
	462=2*3*7*11		mods:1,5,13,17,19,23,29
	6006=2*3*7*11*13	mods:1,5,17,19,23,29

A/2=2*a+1
A/3=3*a+1	3*a+2
A/4=4*a+1	4*a+2=2*(2*a+1)		4*a+3
A/5=5*a+1	5*a+2				5*a+3				5*a+4
A/6=6*a+1	6*a+2=2*(3*a+1)		6*a+3=3*(2*a+1)		6*a+4=2*(3*a+2)		6*a+5	->only has 2 variants divider = 2*3
A/7=7*a+1	7*a+2				7*a+3				7*a+4				7*a+5				7*a+6
A/8=8*a+1	8*a+2=2*(4*a+1)		8*a+3				8*a+4=4*(2*a+1)		8*a+5				8*a+6=2*(4*a+3)		8*a+7
A/9=9*a+1	9*a+2				9*a+3=3*(3*a+1)		9*a+4				9*a+5				9*a+6=3*(3*a+2)		9*a+7	9*a+8
A/10=10*a+1	10*a+2=2*(5*a+1)	10*a+3				10*a+4=2*(5*a+2)	10*a+5=5*(2*a+1)	10*a+6=2*(5*a+3)	10*a+7	10*a+8=2*(5*a+4)
....
A/12=12*a+1	12*a+2=2*(6*a+1)	12*a+3=3*(4*a+1)	12*a+4=4*(3*a+1)	12*a+5				12*a+6=6*(2*a+1)	12*a+7	12*a+8=4*(3*a+2)	12*a+9=3*(4*a+3)	12*a+10=2*(6*a+5)	12*a+11

if we tried to divide N until ex 31 (with all primes), we are allowed to use multipliers like : 6,30 
that means that a will have value at least 1
if we tried all primes until a specific number, it means those numbers should not pop up as mods. Ex : 11*a+29, than a can't be 29. It can be 32, but not the primes we tried

23/6=6*3+5
41/30=30*1+11
23000/6006=6006*3+4982=2*(3003*3+2491)
(23*11)/(2*3*7)

possible approach : no idea what a is, but we want to get amul=1

if N % (2)=1 .. (2*a+1)*(2*b+1)											m e [1]
if N % (3)=1 .. (3*a+1)*(3*b+1) or (3*a+2)*(3*b+2)=(21+2)(39+2)			m e [1,2]
if N % (2*2)=3 .. (4*a+1)*(4*b+3) or (3*a+3)*(3*b+1)=(20+3)(40+1)		m e [1,3]
if N % (2*3)=1 .. (6*a+1)*(6*b+1) or (6*a+5)*(6*b+5)=(18+5)(36+5)		m e [1,5]
if N % (2*2*3)=7 .. (12*a+1)*(12*b+7) or (12*a+7)*(12*b+1)=(12+11)(36+5)	m e [1,5,7,11]
					(12*a+5)*(12*b+11) or (12*a+11)*(12*b+5)
if N % (2*3*3)=7 .. (18*a+1)*(18*b+7) or (18*a+7)*(18*b+1)=(18+5)(36+5)	m e [1,5,7,11,13,17]
					(18*a+5)*(18*b+5) or (18*a+5)*(18*b+5)
					....
if N % (2*3*5)=13 ..(30*a+23)*(30*b+11) or (30*a+?)*(30*b+?)			m e [1,7,11,13,17,19,23,29]
					23*11=253%30=13
					....

reverse approach : if we decide that m=1, we can substract N-m, get the solution for N=(f*a+m1)(f*b+m2)=f*f*a*b+f*a*m2+f*b*m1+m1*m2
					N=f(f*a*b+a*m2+b*m1)+m1*m2	in case we had m1*m2=1 ... (N-1)/f=f*a*b+a*1+b*1
					(N-1) divisors [2,3,157]
					if f=157 (157*a+m1)(157*b+m2)=157*(157*a*b+a*m2+b*m1)+m1*m2 since A,B<157 a=0,b=0 m1=23,m2=41
					if f=6 (6*a+m1)(6*b+m2)=6*(6*a*b+a*m2+b*m1)+m1*m2 a=3,m1=5 b=6,m2=5		157=6*a*b+5*a+5*b
*/

#include "StdAfx.h"

// Function to calculate the product of elements in an array
__int64 product(const __int64 arr[], const __int64 n) {
	__int64 result = 1;
	for (__int64 i = 0; i < n; i++) {
		result *= arr[i];
	}
	return result;
}

// Function to print combinations of primes whose product falls in the range [minA, maxA]
void findCombinations(const __int64 primes[], const __int64 n, __int64 minA, __int64 maxA) {
	assert(n < 500);
	// Iterate through all possible combinations of primes
	for (__int64 i = 0; i < ((__int64)1 << n); i++) {
		__int64 combination[500], size = 0;
		for (__int64 j = 0; j < n; j++) {
			if (i & ((__int64)1 << j)) {
				combination[size++] = primes[j];
			}
		}
		// Calculate product of the current combination
		__int64 p = product(combination, size);
		// Check if product falls within the range [minA, maxA]
		if (p >= minA && p <= maxA) {
			// Print the combination
			printf("Combination: ");
			for (int k = 0; k < size; k++) {
				printf("%lld ", combination[k]);
			}
			printf("Product: %lld\n", p);
		}
	}
}

void DivTestRestrictModulo_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	printf("N = %lld. SQN = %lld. m = %lld SQNSQN = %d \n", N, SQN, m, isqrt(SQN));

//	const static __int64 primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227 };
	const static __int64 primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };
	int primeCount = sizeof(primes) / sizeof(primes[0]);

	// let's presume A size to be between [SQN/2,SQN]
	__int64 searchedAUpperLimit = SQN;
	while (1)
	{
		__int64 A_low = searchedAUpperLimit / 2;
		__int64 A_high = searchedAUpperLimit;
		searchedAUpperLimit = searchedAUpperLimit / 2;
		// get multiple combinations of multipliers that produce a number between [A_low, A_high]
		__int64 divider1 = A_low + 1; // should generate all modulos, except the ones that divide this number
		// we are trying to generate A in the form of : divider[x] * a + mod
		// since dividers are less than 2 times, a should have the same value ( maybe +1 )
		
//		findCombinations(primes, primeCount, A_low, A_high);
	}
}

void DivTestRestrictModulo()
{
	//DivTestRestrictModulo_(5, 7);
	DivTestRestrictModulo_(23, 41);
	DivTestRestrictModulo_(349, 751); // N = 262099 , SN = 511
	DivTestRestrictModulo_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestRestrictModulo_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestRestrictModulo_(784727, 918839);
	DivTestRestrictModulo_(3, 918839);
	DivTestRestrictModulo_(349, 918839);
}
