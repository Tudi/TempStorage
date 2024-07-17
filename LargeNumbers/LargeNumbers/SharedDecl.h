#pragma once

#define Gen_COMBO_MOD		50

struct iNumberCombo
{
	__int64 a, b; // used to store only partial numbers 
};

struct NumberComboDouble
{
	double x, y, xpy; // used to store only partial numbers 
	__int64 ix, iy; // talk about lasy
};

struct IterationStateHolder {
	__int64 c1;
	__int64 c2; // coeff is c1/c2 and it's always greater than 0
	__int64 AInitial;
	__int64 BInitial;
	__int64 NInitial;
	__int64 SQNInitial;
	__int64 mInitial;
	__int64 ANow;
	__int64 BNow;
	__int64 Nnow;
	__int64 SQNnow;
	__int64 mnow;
	__int64 StopIterating;
	__int64 ChecksMade;
	double cf1, cf2;
	double fNNow, fSQNNow, fmNow;

	iNumberCombo AB[Gen_COMBO_MOD * 10];
	NumberComboDouble xy[Gen_COMBO_MOD];
};

struct ModuloPair {
	__int64 m1, m2;
};

void GenXYCombos(IterationStateHolder& sh, const __int64 SQN, const __int64 m, const __int64 startY);
uint64_t isqrt5(uint64_t n);
__int64 GetCoveringBitMask(__int64 nNum);