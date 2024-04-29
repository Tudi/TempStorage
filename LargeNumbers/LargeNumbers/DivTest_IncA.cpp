#include "StdAfx.h"
#include "SharedDecl.h"

// I have this file to avoid creating it in the future : no advantage can be gained by multiplying ( or dividing ) N. You can trace it back to x1. Because length of N does not decrease
//				The only advantage can be gained if more info gets known about A or B

// This approach aims to use y=(1,2,3,4...) to test if it's a solution in the eq. y < SQN / 12
// x^2+x*y+m-y*n=0	x=sqrt(y*y-4*(m-y*n))/2 - 1
// if y is not a solution (x is a full square), it recalculates the B/A relation and increases A cased on the coeff
// it continues repeating this process

// In other words : keeps increasing A by a Coef (=c1/c2) until it reaches almost B
// since N gets multiplied, SQN gets bigger, the X we search for gets bigger, but the x jumps also gets bigger SQRT(SQRT(N))
// since A got multiplied by C, but SQN only got increased by SQRT(C), we made the problem more complex by doing more math for same gain

// maybe helps to know that we are searching for x*c instead a simple x.

// same as : N*c1*c2 = A*c1*B*c2 = (n1+c1*x)*(n2+c2*y) = (z*c1-c1*x)*(v*c2+c2*y) = c1*(z-x)*c2*(v+y) ... no difference than n=a*b

// Conclusion : this approach should be exactly useless compared to checking x 1 by 1 unless there is some magical scenario

#define MAX_SEARCHED_RATIO 1000

__int64 gcd(__int64 a, __int64 b) {
	while (b != 0) {
		__int64 temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}

void removeCommonDivisors(__int64* num1, __int64* num2) {
	__int64 common_divisor = gcd(*num1, *num2);
	while (common_divisor != 1) {
		while (*num1 % common_divisor == 0 && *num2 % common_divisor == 0) {
			*num1 /= common_divisor;
			*num2 /= common_divisor;
		}
		common_divisor = gcd(*num1, *num2);
	}
}


void floatToFraction(__int64 &numerator, __int64 & denominator)
{
	__int64 scalerN = 1;
	while (numerator > denominator * 10)
	{
		scalerN *= 10;
		denominator *= 10;
	}
	__int64 scalerD = 1;
	while (numerator * 10 < denominator)
	{
		scalerD *= 10;
		numerator *= 10;
	}

	double value = (double)(numerator) / (double)(denominator);
	assert(value >= 1);
	assert(value < 10);
	double error = value;
	int num = numerator, den = denominator;

	for (int i = 1; i <= MAX_SEARCHED_RATIO; i++) {
		int startVal = i / value;
		for (int j = startVal; j <= i * 10; j++) {
			double newVal = (double)i / j;
			if (newVal <= value) {
				double currentError = value - newVal;
				if (currentError < error) {
					error = currentError;
					num = i;
					den = j;
				}
				else {
					break;
				}
			}
		}
	}

	numerator = num * scalerN;
	denominator = den * scalerD;
}

uint64_t isqrt5(uint64_t n) {
	uint64_t x = n;
	uint64_t y = (x + 1) / 2;
	while (y < x) {
		x = y;
		y = (x + n / x) / 2;
	}
	return x;
}

// even if not good, at least it kinda works
static void MakeProgress1(IterationStateHolder &sh)
{
	if (((double)sh.BNow / sh.ANow) >= MAX_SEARCHED_RATIO)
	{
		printf("Not made to handle this input\n");
		sh.StopIterating = 1;
		return;
	}

	sh.Nnow = sh.AInitial * sh.c1 * sh.BInitial * sh.c2;
	sh.SQNnow = isqrt5(sh.Nnow);
	sh.mnow = sh.Nnow - sh.SQNnow * sh.SQNnow;

	if (sh.mnow == 0)
	{
		printf("Number is a perfect square, both x / y would be 0\n");
		sh.StopIterating = 1;
		return;
	}

	printf("\t %lld)searching for x=%lld y=%lld. c1=%lld, c2=%lld\n", 
		sh.ChecksMade, sh.SQNnow - sh.AInitial * sh.c1, sh.AInitial * sh.c1 + sh.BInitial * sh.c2 - 2 * sh.SQNnow,
		sh.c1, sh.c2);

	size_t bAIsImPair = (sh.AInitial & sh.c1) & 1;
	size_t bBIsImPair = (sh.BInitial & sh.c2) & 1;
	size_t bSQNIsImpair = (sh.SQNnow & 1);
	size_t bXneedsToBeImPair = (bAIsImPair + bSQNIsImpair) & 1;
	size_t bYNeedsToBeImPair = (bBIsImPair + bSQNIsImpair + bXneedsToBeImPair) & 1;
	__int64 yToTest = bYNeedsToBeImPair;
	// set min y : 
	//	needs to be always greater than 0
	//  if m >= SQN, y needs to be greater than 1
	if (yToTest == 0 || sh.mnow > sh.SQNnow)
	{
		yToTest += 2;
	}
	__int64 tA, tB, x;
	// do multiple tests to reduce the overhead of calculating the new N
	// there is no reason to go anything close to SQN/6 -> at this point y increase produce less than 1 x increase
	__int64 xCanBeRoundedUp = 0;
	__int64 maxYToTest = Gen_COMBO_MOD;
	__int64 tSQN = sh.SQNnow;
	__int64 tm = sh.mnow;

	for (; yToTest < maxYToTest; yToTest += 2)
	{
		x = (isqrt5(yToTest * yToTest - 4 * (tm - yToTest * tSQN)) - yToTest) / 2;

		// maybe this is a solution
		tA = tSQN - x;
		tB = tSQN + x + yToTest;
		if (tA * tB == sh.Nnow)
		{
			sh.StopIterating = 1;
			printf("Wow,we found a solution\n\n");
			return;
		}
		sh.ChecksMade++;
	}

	__int64 xDownscaled = x / sh.c1; // should be something dependent on 'SQN' and sligly on 'm'
	sh.ANow = tA;
	sh.BNow = tB;
	sh.c1 = sh.c1 * tB;
	sh.c2 = sh.c2 * tA;
	double trateNow = (double)tB / tA;
	if (((double)sh.c1 / sh.c2) >= MAX_SEARCHED_RATIO)
	{
		printf("Not made to handle this input\n");
		sh.StopIterating = 1;
		return;
	}
	double rateFinal = (double)sh.BInitial / (double)sh.AInitial;
	double rateSumary = (double)sh.c1 / (double)sh.c2;
	// simplify
	printf("%lld)Rate now %f", sh.ChecksMade, (double)sh.c1 / (double)sh.c2);
	floatToFraction(sh.c1, sh.c2);
	printf(" == %f for %lld/%lld. Stop at %f. Downscaled x %lld. Reached y %lld. rateNow %f\n", 
		rateSumary, sh.c1, sh.c2, rateFinal, xDownscaled, yToTest, trateNow);
}

static void GenNumberCombo(IterationStateHolder& sh)
{
	size_t combos_added = 0;
	memset(sh.AB, 0, sizeof(sh.AB));
	for (__int64 a = 0; a < Gen_COMBO_MOD; a++)
	{
		for (__int64 b = 0; b < Gen_COMBO_MOD; b++)
		{
			__int64 ab = a * b;
			if ((ab % Gen_COMBO_MOD) == (sh.NInitial % Gen_COMBO_MOD))
			{
				sh.AB[combos_added].a = a;
				sh.AB[combos_added].b = b;
				combos_added++;
				if (combos_added == _countof(sh.AB))
				{
					return;
				}
			}
		}
	}
}

// to much junk piled up
static void MakeProgress2(IterationStateHolder& sh)
{
	if (((double)sh.BNow / sh.ANow) >= MAX_SEARCHED_RATIO)
	{
		printf("Not made to handle this input\n");
		sh.StopIterating = 1;
		return;
	}

	sh.Nnow = sh.AInitial * sh.c1 * sh.BInitial * sh.c2;
	sh.SQNnow = isqrt5(sh.Nnow);
	sh.mnow = sh.Nnow - sh.SQNnow * sh.SQNnow;

	if (sh.mnow == 0)
	{
		printf("Number is a perfect square, both x / y would be 0\n");
		sh.StopIterating = 1;
		return;
	}

	printf("\t %lld)searching for x=%lld y=%lld. c1=%lld, c2=%lld\n",
		sh.ChecksMade, sh.SQNnow - sh.AInitial * sh.c1, sh.AInitial * sh.c1 + sh.BInitial * sh.c2 - 2 * sh.SQNnow,
		sh.c1, sh.c2);

	size_t bAIsImPair = (sh.AInitial & sh.c1) & 1;
	size_t bBIsImPair = (sh.BInitial & sh.c2) & 1;
	size_t bSQNIsImpair = (sh.SQNnow & 1);
	size_t bXneedsToBeImPair = (bAIsImPair + bSQNIsImpair) & 1;
	size_t bYNeedsToBeImPair = (bBIsImPair + bSQNIsImpair + bXneedsToBeImPair) & 1;
	__int64 yToTest = bYNeedsToBeImPair;
	// set min y : 
	//	needs to be always greater than 0
	//  if m >= SQN, y needs to be greater than 1
	if (yToTest == 0 || sh.mnow > sh.SQNnow)
	{
		yToTest += 2;
	}
	__int64 tA, tB, x;
	// do multiple tests to reduce the overhead of calculating the new N
	// there is no reason to go anything close to SQN/6 -> at this point y increase produce less than 1 x increase
	__int64 xCanBeRoundedUp = 0;
	__int64 maxYToTest = 100;
	__int64 tSQN = sh.SQNnow;
	__int64 tm = sh.mnow;
	// x2 needs to be in the form of : x2 = x1 * c1 + (SQN % c1)
	__int64 xRequired = sh.SQNnow - sh.AInitial * sh.c1;
	__int64 xmod = (tSQN % sh.c1); // number required to be able to round up to good value
	__int64 xdiv = xRequired / sh.c1; // this should be x1. I wonder if there are cases when it's not x1
	assert((xRequired == (xdiv * sh.c1 + xmod)));
	// x2 + y2 should be in the form : (x2 + y2) = z * c2 + (c2 - (SQN % c2))
	//		y2 = z * c2 + (c2 - (SQN % c2)) - (x1 * c1 + (SQN % c1))
	// x2 + y2 = B2 - SQN2
	__int64 x2plusy2Required = sh.BInitial * sh.c2 - sh.SQNnow;
	__int64 ymod = sh.c2 - (sh.SQNnow % sh.c2); // y needs to have this "modulo" so that B can be dividable by c2
	__int64 z = (x2plusy2Required - ymod) / sh.c2; // at the beginning this is greater than y, because we include x that is greater than y
	__int64 yreqcalculated = z * sh.c2 + ymod - (xdiv * sh.c1 + xmod);
	//	__int64 ydiv = (x2plusy2Required - ymod + xdiv * sh.c1 + xmod)) / sh.c2;
	__int64 yRequired = sh.AInitial * sh.c1 + sh.BInitial * sh.c2 - 2 * sh.SQNnow;
	assert(yRequired == yreqcalculated);
	// we also know that y2 = A*c1 + B*c2 - 2 * sqrt(c1*c2) * SQN ... which should be around (c1+c2)/2 * y1 ?
//	__int64 y1 = sh.AInitial + sh.BInitial - 2 * sh.SQNInitial;
//	__int64 aproxy2fromy1 = (sh.c1 + sh.c2) / 2 * y1;

	// useless unless c1/c2 is a big number
/*	if (sh.c1 > 1)
	{
		__int64 xmin = xmod;
		__int64 ymin1 = (xmin * xmin + sh.mnow) / (sh.SQNnow - xmin);
		assert(ymin1 < yRequired);
		if ((bYNeedsToBeImPair && (ymin1 & 1) == 0) || (bYNeedsToBeImPair == 0 && (ymin1 & 1) == 1))
		{
			ymin1++;
		}
		assert(ymin1 <= yRequired);
		if (maxYToTest < ymin1)
		{
			maxYToTest = ymin1 + 1;
		}
		if (ymin1 > yToTest)
		{
			yToTest = ymin1;
		}
	}*/
	// since x needs to be at least 1, a new ymin can be calculated based on : yreqcalculated = z * sh.c2 + ymod - (xdiv * sh.c1 + xmod)
	// z * c2 > xdiv * c1 + xmod	.. if xdiv = 1	.. z > (c1+xmod) / c2
/*	__int64 zmin = (1 * sh.c1 + xmod) / sh.c2; // should be at least 1
	__int64 zroundup = (zmin * sh.c2) != (1 * sh.c1 + xmod);
	__int64 ymin2 = (zmin + zroundup) * sh.c2 + ymod - (1 * sh.c1 + xmod);
	assert(ymin2 < yRequired);
	if ((bYNeedsToBeImPair && (ymin2 & 1) == 0) || (bYNeedsToBeImPair == 0 && (ymin2 & 1) == 1))
	{
		ymin2++;
	}
	assert(ymin2 < yRequired);
	if (maxYToTest < ymin2)
	{
		maxYToTest = ymin2 + 1;
	}
	if (ymin2 > yToTest)
	{
		yToTest = ymin2;
	}
	*/
	for (; yToTest < maxYToTest; yToTest += 2)
	{
		x = (isqrt5(yToTest * yToTest - 4 * (tm - yToTest * tSQN)) - yToTest) / 2;

		// snap to valid x
#ifdef SNAP_TO_NEXT_VALID_X
		if (sh.c1 != 1)
		{
			__int64 xDiv = x / sh.c1;
			// roundup
			if ((xDiv * sh.c1) != x)
			{
				xDiv += 1;
			}
			// snap to next valid value
			x = xDiv * sh.c1 + xm;
			// would this also increase our y ?
			__int64 newYMin = (x * x + tm) / (tSQN - x);
			if ((bYNeedsToBeImPair ^ (newYMin & 1)) == 1)
			{
				newYMin++;
			}
			//			if (yToTest < newYMin)
			if (yToTest < newYMin - 2)
			{
				//				yToTest = newYMin - 2; // -2 because it will be added at the end of the for loop
			}
		}
#endif

		// maybe this is a solution
		tA = tSQN - x;
		tB = tSQN + x + yToTest;
		if (tA * tB == sh.Nnow)
		{
			sh.StopIterating = 1;
			printf("Wow,we found a solution\n\n");
			return;
		}
		sh.ChecksMade++;
#ifdef HAD_TIME_TO_FINISH_Y_JUMP_FORWARD
		// at this point we are allowed to convert the x eq to include c1
		if (sh.c1 != 1 && x >= sh.c1 && xCanBeRoundedUp == 0)
		{
			tSQN = (sh.SQNnow / sh.c1) * sh.c1; // makes SQN smaller and exactly divisible by c1
			tm = sh.Nnow - tSQN * tSQN; // makes m bigger by including values from SQN
			xCanBeRoundedUp = 1;
			// new y min so that our formula would not go negative. 
			// other words : converts the values substracted from SQN into y
			__int64 newYMin = tm / tSQN;
			if ((bYNeedsToBeImPair ^ (newYMin & 1)) == 1)
			{
				newYMin++;
			}
			if (yToTest < newYMin)
			{
				yToTest = newYMin - 2; // -2 because it will be added at the end of the for loop
			}
			if (newYMin > maxYToTest)
			{
				maxYToTest = newYMin + 2;
			}
		}
#endif
	}

	// we know we are searching for a c1 that is divisible by c1
//	if (xCanBeRoundedUp && (x % sh.c1) != 0)
	{
		//		x = (x / sh.c1 + 1) * sh.c1;
	}

	__int64 xDownscaled = x / sh.c1; // should be something dependent on 'SQN' and sligly on 'm'
	sh.ANow = tA;
	sh.BNow = tB;
	sh.c1 = sh.c1 * tB;
	sh.c2 = sh.c2 * tA;
	double trateNow = (double)tB / tA;
	if (((double)sh.c1 / sh.c2) >= MAX_SEARCHED_RATIO)
	{
		printf("Not made to handle this input\n");
		sh.StopIterating = 1;
		return;
	}
	double rateFinal = (double)sh.BInitial / (double)sh.AInitial;
	double rateSumary = (double)sh.c1 / (double)sh.c2;
	// simplify
	printf("%lld)Rate now %f", sh.ChecksMade, (double)sh.c1 / (double)sh.c2);
	floatToFraction(sh.c1, sh.c2);
	printf(" == %f for %lld/%lld. Stop at %f. Downscaled x %lld. Reached y %lld. rateNow %f\n",
		rateSumary, sh.c1, sh.c2, rateFinal, xDownscaled, yToTest, trateNow);
}

static void GetNumberDigits(__int64 num, __int64* digitsStore, __int64 digitCountMax, __int64 &digitCount)
{
	digitCount = 0;
	while (num > 0 && digitCount < digitCountMax)
	{
		*digitsStore = num % 10;
		digitsStore++;
		num = num / 10;
		digitCount++;
	}
}

static void MakeProgress3(IterationStateHolder& sh)
{
	if (((double)sh.BNow / sh.ANow) >= MAX_SEARCHED_RATIO)
	{
		printf("Not made to handle this input\n");
		sh.StopIterating = 1;
		return;
	}

	sh.Nnow = sh.AInitial * sh.c1 * sh.BInitial * sh.c2;
	sh.SQNnow = isqrt5(sh.Nnow);
	sh.mnow = sh.Nnow - sh.SQNnow * sh.SQNnow;

	if (sh.mnow == 0)
	{
		printf("Number is a perfect square, both x / y would be 0\n");
		sh.StopIterating = 1;
		return;
	}

	__int64 xRequired = sh.SQNnow - sh.AInitial * sh.c1;
	__int64 yRequired = sh.AInitial * sh.c1 + sh.BInitial * sh.c2 - 2 * sh.SQNnow;
	printf("\t %lld)searching for x=%lld y=%lld. c1=%lld, c2=%lld\n",
		sh.ChecksMade, xRequired, yRequired, sh.c1, sh.c2);

	size_t bAIsImPair = (sh.AInitial & sh.c1) & 1;
	size_t bBIsImPair = (sh.BInitial & sh.c2) & 1;
	size_t bSQNIsImpair = (sh.SQNnow & 1);
	size_t bXneedsToBeImPair = (bAIsImPair + bSQNIsImpair) & 1;
	size_t bYNeedsToBeImPair = (bBIsImPair + bSQNIsImpair + bXneedsToBeImPair) & 1;
	__int64 yToTest = bYNeedsToBeImPair;
	// set min y : 
	//	needs to be always greater than 0
	//  if m >= SQN, y needs to be greater than 1
	if (yToTest == 0 || sh.mnow > sh.SQNnow)
	{
		yToTest += 2;
	}
	__int64 tA, tB, x;
	// do multiple tests to reduce the overhead of calculating the new N
	// there is no reason to go anything close to SQN/6 -> at this point y increase produce less than 1 x increase
	__int64 xCanBeRoundedUp = 0;
	__int64 maxYToTest = 100;
	__int64 tSQN = sh.SQNnow;
	__int64 tm = sh.mnow;

	// we can do a quick check if y is greater than a specific potential candidate
#if 0
	{
		size_t bYMinDigitCountImpossible[100];
		memset(bYMinDigitCountImpossible, 0, sizeof(bYMinDigitCountImpossible));
		__int64 right = 2 * sh.SQNnow;
		__int64 rightDigits[100];
		__int64 rightDigitCount;
		GetNumberDigits(right, rightDigits, GetDigitCount(Gen_COMBO_MOD), rightDigitCount);
		for (size_t i = 0; i < _countof(sh.AB); i++)
		{
			__int64 left = sh.AB[i].a * sh.c1 + sh.AB[i].b * sh.c2;
			__int64 leftDigits[100];
			__int64 leftDigitCount;
			GetNumberDigits(left, leftDigits, GetDigitCount(Gen_COMBO_MOD), leftDigitCount);
			for (size_t j = GetDigitCount(Gen_COMBO_MOD) - 2; j >= 0; j--)
			{
				if (leftDigits[j] != rightDigits[j])
				{
					break;
				}
				if (j > 0) bYMinDigitCountImpossible[j] = 1;
				else
				{
					bYMinDigitCountImpossible[j] = 1;
					i = _countof(sh.AB);
					break;
				}
			}
		}
		__int64 PossibleMinY = 1;
		for (size_t i = 1; i < GetDigitCount(Gen_COMBO_MOD) - 1 && bYMinDigitCountImpossible[i] == 0; i++)
		{
			PossibleMinY *= 10;
		}
		if (PossibleMinY > yToTest)
		{
			yToTest = PossibleMinY;
		}
		if ((yToTest ^ bYNeedsToBeImPair) & 1)
		{
			yToTest++;
		}
		if (yToTest >= maxYToTest)
		{
			maxYToTest = yToTest + 1;
		}
	}
#endif

	__int64 xmod = (tSQN % sh.c1); // number required to be able to round up to good value

	for (; yToTest < maxYToTest; yToTest += 2)
	{
		x = (isqrt5(yToTest * yToTest - 4 * (tm - yToTest * tSQN)) - yToTest) / 2;

		__int64 xdiv = x / sh.c1;
		__int64 xRounding = (xdiv * sh.c1) != x;
		__int64 xLargest = tSQN - xmod; // x smallest
		__int64 xRounded = xmod + (xdiv + xRounding) * sh.c1; // same as xnow
		__int64 xNext = xmod + (xdiv + xRounding + 1) * sh.c1; // next x
		__int64 nextY = (xNext * xNext + tm) / (tSQN - xNext); // as long as x<n/3, this will be same as yToTest

		// maybe this is a solution
		tA = tSQN - x;
		tB = tSQN + x + yToTest;
		if (tA * tB == sh.Nnow)
		{
			sh.StopIterating = 1;
			printf("Wow,we found a solution\n\n");
			return;
		}
		sh.ChecksMade++;
	}

	__int64 xDownscaled = x / sh.c1; // should be something dependent on 'SQN' and sligly on 'm'
	sh.ANow = tA;
	sh.BNow = tB;
	sh.c1 = sh.c1 * tB;
	sh.c2 = sh.c2 * tA;
	double trateNow = (double)tB / tA;
	if (((double)sh.c1 / sh.c2) >= MAX_SEARCHED_RATIO)
	{
		printf("Not made to handle this input\n");
		sh.StopIterating = 1;
		return;
	}
	double rateFinal = (double)sh.BInitial / (double)sh.AInitial;
	double rateSumary = (double)sh.c1 / (double)sh.c2;
	// simplify
	printf("%lld)Rate now %f", sh.ChecksMade, (double)sh.c1 / (double)sh.c2);
	floatToFraction(sh.c1, sh.c2);
	printf(" == %f for %lld/%lld. Stop at %f. Downscaled x %lld. Reached y %lld. rateNow %f\n",
		rateSumary, sh.c1, sh.c2, rateFinal, xDownscaled, yToTest, trateNow);
}

void GenXYCombos(IterationStateHolder& sh, const __int64 SQN, const __int64 m, const __int64 startY)
{
	size_t combos_added = 0;
	memset(sh.xy, 0, sizeof(sh.xy));
	__int64 xFromy = (isqrt(startY * startY - 4 * (m - startY * SQN)) - startY) / 2;
	if (startY < SQN / 6 && xFromy < SQN / 3)
	{
		// one y increase produces more than 1 x increase
		for (__int64 y = startY; y < startY + Gen_COMBO_MOD * 2 + 10; y += 2)
		{
			__int64 t_a = 1;
			__int64 t_b = y;
			__int64 t_c = sh.mInitial - y * sh.SQNInitial;
			if (t_c >= 0)
			{
				continue;
			}
			__int64 delta = t_b * t_b - 4 * t_a * t_c;
			double x = (-t_b + sqrt(delta)) / (2 * t_a);
			sh.xy[combos_added].y = y;
			sh.xy[combos_added].x = x;
			sh.xy[combos_added].xpy = x + y;
			sh.xy[combos_added].ix = x;
			sh.xy[combos_added].iy = y;
			combos_added++;
			if (combos_added == _countof(sh.xy))
			{
				return;
			}
		}
	}
	else
	{
//		__int64 xmod = (SQN % sh.c1); // number required to be able to round up to good value
		// check if our x form expectation matches : x = w * c1 + xmod
//		assert(((SQN - xmod) % sh.c1) == 0);
//		__int64 xMultMin = xFromy / sh.c1;
		// one x increase produces more than 1 y increase
		if (SQN & 1) xFromy = xFromy & (~1);
		else xFromy = xFromy | 1;
		for (__int64 x = xFromy; x < xFromy + Gen_COMBO_MOD * 2 + 10; x += 2)
//		for (__int64 xMul = xMultMin; xMul < xMultMin + Gen_COMBO_MOD * 2 + 10; xMul += 1)
		{
//			__int64 x = xMul * sh.c1 + xmod;
			double y = (double)(x * x + m) / (SQN - x);
			sh.xy[combos_added].y = y;
			sh.xy[combos_added].x = x;
			sh.xy[combos_added].xpy = x + y;
			sh.xy[combos_added].ix = x;
			sh.xy[combos_added].iy = y;
			combos_added++;
			if (combos_added == _countof(sh.xy))
			{
				return;
			}
		}
	}
}

static void MakeProgress4(IterationStateHolder& sh)
{
	sh.Nnow = sh.AInitial * sh.c1 * sh.BInitial * sh.c2;
	sh.SQNnow = isqrt5(sh.Nnow);
	sh.mnow = sh.Nnow - sh.SQNnow * sh.SQNnow;

	if (sh.mnow == 0)
	{
		printf("Number is a perfect square, both x / y would be 0\n");
		sh.StopIterating = 1;
		return;
	}

	__int64 xRequired = sh.SQNnow - sh.AInitial * sh.c1;
	__int64 yRequired = sh.AInitial * sh.c1 + sh.BInitial * sh.c2 - 2 * sh.SQNnow;
	printf("\t %lld)searching for x=%lld y=%lld. c1=%lld, c2=%lld\n",
		sh.ChecksMade, xRequired, yRequired, sh.c1, sh.c2);

	size_t bAIsImPair = (sh.AInitial & sh.c1) & 1;
	size_t bBIsImPair = (sh.BInitial & sh.c2) & 1;
	size_t bSQNIsImpair = (sh.SQNnow & 1);
	size_t bXneedsToBeImPair = (bAIsImPair + bSQNIsImpair) & 1;
	size_t bYNeedsToBeImPair = (bBIsImPair + bSQNIsImpair + bXneedsToBeImPair) & 1;
	__int64 yToTest = bYNeedsToBeImPair;
	// set min y : 
	//	needs to be always greater than 0
	//  if m >= SQN, y needs to be greater than 1
	if (yToTest == 0 || sh.mnow > sh.SQNnow)
	{
		yToTest += 2;
	}
	__int64 tA, tB, x;
	// do multiple tests to reduce the overhead of calculating the new N
	// there is no reason to go anything close to SQN/6 -> at this point y increase produce less than 1 x increase
	__int64 xCanBeRoundedUp = 0;
	__int64 maxYToTest = 100;
	__int64 tSQN = sh.SQNnow;
	__int64 tm = sh.mnow;

	__int64 xmod = (tSQN % sh.c1); // number required to be able to round up to good value
//	__int64 ymod = sh.c2 - (tSQN % sh.c2); // number required to be able to round up to good value

	// check if our x form expectation matches : x = w * c1 + xmod
	assert(((tSQN - xmod) % sh.c1) == 0);
	// y = c2 * B - x - n = c2 * B - w * c1 - xmod - n

	__int64 x1Required = sh.SQNInitial - sh.AInitial;

	double precisesnq1 = sqrt(sh.NInitial);
	double precisesnq2 = sqrt(sh.Nnow);
	double fullSQNCoef = precisesnq2 / precisesnq1;
	double expectedSQNCoeff = sqrt(sh.c1 * sh.c2);

	// x = sqrt(c1*c2) * n1 - a * c1 = (c1 * n1 - a * c1 ) - missingval
	double missingFromSQNForX = sh.c1 * sh.SQNInitial - expectedSQNCoeff * sh.SQNInitial; // close but not precise
	missingFromSQNForX = sh.c1 * sh.SQNInitial - sh.SQNnow; // flat value of rounding errors
	__int64 i_MissingFromSQNForX = sh.c1 * sh.SQNInitial - sh.SQNnow;
	double xExpectedRecalc = sh.c1 * (sh.SQNInitial - sh.AInitial) - missingFromSQNForX; 
	xExpectedRecalc = sh.c1 * x1Required - missingFromSQNForX;
	double Min_x2_ToProduceSmall_y = missingFromSQNForX / sh.c1; // to compensate for our forced and untested shift
	assert(Min_x2_ToProduceSmall_y <= xRequired);
	// x2 = sh.c1 * x1 - sh.c1 * sh.SQNInitial + sh.SQNnow;
	double Min_x1_ToProducceSmall_y = ( Min_x2_ToProduceSmall_y + sh.c1 * sh.SQNInitial - sh.SQNnow ) / sh.c1;
	double Min_y1_ToProducceSmall_y = (Min_x1_ToProducceSmall_y * Min_x1_ToProducceSmall_y + sh.mInitial) / (sh.SQNInitial - Min_x1_ToProducceSmall_y);
//	double y2ForMinx2 = (Min_x2_ToProduceSmall_y * Min_x2_ToProduceSmall_y + sh.mnow) / (sh.SQNnow - Min_x2_ToProduceSmall_y);
	__int64 genfory = Min_y1_ToProducceSmall_y;
	// x + sqn is always impair, so y always needs to be pair
	genfory = genfory & (~1); //  needs to be pair
	if (genfory == 0 || sh.mInitial > sh.SQNInitial)
	{
		genfory += 2;
	}
	GenXYCombos(sh, sh.SQNInitial, sh.mInitial, genfory);

	__int64 x2fromx1First = sh.c1 * sh.xy[0].x - sh.c1 * sh.SQNInitial + sh.SQNnow;
	__int64 xMultFirst = (x2fromx1First - xmod) / sh.c1;
	__int64 xMultPrev = -1;
	for (size_t i = 0; i < _countof(sh.xy); i++)
	{
		// N = (n-x)*(n+xpy)
		// N*c1*c2 = c1*(n-x)*c2*(n+xpy)
		// we are looking for an integer x and y. when we scale up the y that is always integer, it should produce a valid x sooner
		// the new Y would depend on the partial X-es probably, not necesarelly on the real X
		// x + y = z * c2 + ymod
		// y = c2 * B - c1 * x - n
		// y = c1 * A + c2 * B - 2 * n
		{
			double x2fromx1Now = sh.c1 * (sh.xy[i].x - sh.SQNInitial) + sh.SQNnow;
			__int64 xMultNow = (x2fromx1Now - xmod) / sh.c1; // should be +1 previous
			assert(xMultPrev + 1 <= xMultNow);
			xMultPrev = xMultNow;
		}
		// temp asuming we never hit the result
		if (sh.xy[i].x == sh.xy[i].ix && sh.xy[i].y == sh.xy[i].iy)
		{
			sh.StopIterating = 1;
			printf("Wow,we found a solution after %lld checks. x=%lld\n\n", sh.ChecksMade, sh.xy[i].ix);
			return;
		}
		else
		{
			sh.ChecksMade++;
			continue; // yeah. Since we are just deriving it from x1, we can directly check x1
		}
#ifdef JUST_LOOKING_AROUND_CURIOUSLY
		// assume minx is a real thing
		double x2fromx1ForMin = sh.c1 * sh.xy[i].x - missingFromSQNForX; // allowed to be negative ?
		if (x2fromx1ForMin < Min_x2_ToProduceSmall_y)
		{
			continue; // should not happen
		}

		double x2fromx1 = sh.c1 * sh.xy[i].x - missingFromSQNForX; // allowed to be negative ?
		// y = c2*B - c2*n1 - c2*x1 - x2 - n2 + c2*n1 + c2*x1 = c2 * y1 - x2 - n2 + c2 * n1 + c2 * x1
		double missingFromSQNForY = sh.c2 * sh.xy[i].x - x2fromx1 + sh.c2 * sh.SQNInitial - sh.SQNnow;
		double y2fromy1;
//		y2fromy1 = sh.c2 * sh.xy[i].y + sh.c2 * sh.xy[i].x - x2fromx1 + sh.c2 * sh.SQNInitial - sh.SQNnow;
//		y2fromy1 = sh.c2 * sh.xy[i].y + sh.c2 * sh.xy[i].x - (sh.c1 * sh.xy[i].x - (sh.c1 * sh.SQNInitial - sh.SQNnow))+sh.c2 * sh.SQNInitial - sh.SQNnow;
//		y2fromy1 = sh.c2 * sh.xy[i].y + sh.c2 * sh.xy[i].x - (sh.c1 * sh.xy[i].x - sh.c1 * sh.SQNInitial + sh.SQNnow) + sh.c2 * sh.SQNInitial - sh.SQNnow;
//		y2fromy1 = sh.c2 * sh.xy[i].y + sh.c2 * sh.xy[i].x - sh.c1 * sh.xy[i].x + sh.c1 * sh.SQNInitial - sh.SQNnow + sh.c2 * sh.SQNInitial - sh.SQNnow;
		y2fromy1 = sh.c2 * sh.xy[i].y + (sh.c2 - sh.c1) * sh.xy[i].x + (sh.c1 + sh.c2) * sh.SQNInitial - 2 * sh.SQNnow;
		double yfromx = (x2fromx1 * x2fromx1 + sh.mnow) / (sh.SQNnow - x2fromx1);
		
		// the non liniar behavior happens because we cheated with the missing value
		// Conclusion : The correct value of X will stay on the same index. 
		//				We would be trying out the same amount of values no matter of c1, c2
		// minx will get bigger and bigger, while it will keep producing the same small y due to c1,c2 ratio
		{
			// x will go into negative first, than turn positive
			__int64 i_x2fromx1 = sh.c1 * sh.xy[i].ix - (sh.c1 * sh.SQNInitial - sh.SQNnow);
			// y will decrease first and than start increasing again
			__int64 i_y2fromy1;
//			i_y2fromy1 = sh.c2 * sh.xy[i].iy + sh.c2 * sh.xy[i].ix - i_x2fromx1 + sh.c2 * sh.SQNInitial - sh.SQNnow;
//			i_y2fromy1 = sh.c2 * sh.xy[i].iy + sh.c2 * sh.xy[i].ix - (sh.c1 * sh.xy[i].ix - sh.c1 * sh.SQNInitial + sh.SQNnow) + sh.c2 * sh.SQNInitial - sh.SQNnow;
//			i_y2fromy1 = sh.c2 * sh.xy[i].iy + sh.c2 * sh.xy[i].ix - sh.c1 * sh.xy[i].ix + sh.c1 * sh.SQNInitial - sh.SQNnow + sh.c2 * sh.SQNInitial - sh.SQNnow;
			i_y2fromy1 = sh.c2 * sh.xy[i].iy + (sh.c2 - sh.c1) * sh.xy[i].ix + (sh.c1 + sh.c2) * sh.SQNInitial - 2 * sh.SQNnow;
			// roundup x. We are looking for an integer x, it's ok
			__int64 i_y2fromy1_safe = sh.c2 * sh.xy[i].iy + (sh.c2 - sh.c1) * (sh.xy[i].ix + 1) + (sh.c1 + sh.c2) * sh.SQNInitial - 2 * sh.SQNnow;
			if (i_y2fromy1_safe > yRequired)
			{
				i_y2fromy1 = i_y2fromy1; // this does happen it seems
			}
		}

//		double xfromy = (sqrt(sh.xy[i].y * sh.xy[i].y - 4 * (sh.mnow - sh.xy[i].y * sh.SQNnow)) - sh.xy[i].y) / 2;
		tA = sh.SQNnow - x2fromx1;
		tB = sh.SQNnow + x2fromx1 + y2fromy1;
		sh.ChecksMade++; 
#endif
	}
	size_t lastValidIndex = _countof(sh.xy) - 1;
	double x2fromx1 = sh.c1 * (sh.xy[lastValidIndex].x - sh.SQNInitial) + sh.SQNnow;
	double y2fromy1 = sh.c2 * sh.xy[lastValidIndex].y + (sh.c2 - sh.c1) * sh.xy[lastValidIndex].x + (sh.c1 + sh.c2) * sh.SQNInitial - 2 * sh.SQNnow;
	tA = sh.SQNnow - x2fromx1;
	tB = sh.SQNnow + x2fromx1 + y2fromy1;

	sh.ANow = tA;
	sh.BNow = tB;
	sh.c1 = sh.c1 * tB;
	sh.c2 = sh.c2 * tA;

	double trateNow = (double)tB / tA;
	double rateFinal = (double)sh.BInitial / (double)sh.AInitial;
	double rateSumary = (double)sh.c1 / (double)sh.c2;
	// simplify
	floatToFraction(sh.c1, sh.c2);
	double tratePostSimplified = (double)sh.c1 / (double)sh.c2;

	printf("%lld)Rate sum %f", sh.ChecksMade, rateSumary);
	printf(" == %f for %lld/%lld. Stop at %f. Reached y %lld. rateNow %f\n",
		tratePostSimplified, sh.c1, sh.c2, rateFinal, sh.xy[lastValidIndex].iy, trateNow);
}

void DivTestIncA_(__int64 A, __int64 B)
{
	__int64 N = A * B;
	__int64 SQN = (__int64)isqrt(N);
	__int64 m = N - SQN * SQN;

	printf("N = %lld. SQN = %lld. m = %lld SQNSQN = %d \n", N, SQN, m, isqrt(SQN));
	__int64 searchedX = SQN - A;
	__int64 searchedY = A + B - 2 * SQN;
	printf("Searching for x=%lld for A=%lld\n", searchedX, A);
	printf("Searching for y=%lld for B=%lld\n", searchedY, B);

	IterationStateHolder ish;
	ish.AInitial = A;
	ish.BInitial = B;
	ish.NInitial = N;
	ish.SQNInitial = SQN;
	ish.mInitial = m;
	ish.c1 = 1;
	ish.c2 = 1;
	ish.StopIterating = 0;
	ish.ChecksMade = 0;

	GenNumberCombo(ish);

	while (ish.StopIterating == 0)
	{
//		MakeProgress1(ish);
		MakeProgress4(ish);
	}
}

void DivTestIncA()
{
	//DivTestIncA_(5, 7);
	DivTestIncA_(23, 41);
	DivTestIncA_(349, 751); // N = 262099 , SN = 511
	DivTestIncA_(6871, 7673); // N = 52721183 , SN = 7260
	DivTestIncA_(26729, 31793); // N = 849795097 , SN = 29151
	DivTestIncA_(784727, 918839);
	DivTestIncA_(3, 918839);
	DivTestIncA_(349, 918839);
}
