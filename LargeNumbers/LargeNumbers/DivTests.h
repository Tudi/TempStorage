#pragma once

void GenerateNextCandidateAtPos( LargeNumber **vLN, int ParamCount, int pos );
void ResetCandidateAtPos( LargeNumber **vLN, int ParamCount, int pos, int Forward = 0 );
int CheckSolution( LargeNumber &tN, LargeNumber &TempRes );

int CheckCandidateMatch2( LargeNumber *tN, LargeNumber *mul, LargeNumber *a1, LargeNumber *b1, int pos, LargeNumber *TempRes );
int GetDigitCount(__int64 Nr);
__int64 GetMaskDecimal(__int64 Nr1, __int64 Nr2 =-1);
int IsPrime(__int64 Nr);

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

void DivTest99ab();
void DivTestab();
void DivTest9911ab();
void DivTestabc();
void DivTest_SQNM();
void DivTest6parts();
void DivTestRecDiv();
void DivTestDecrement();
void DivTestabSQ();
void DivTest_xytn();
void DivTestabSQ2();
void DivTestLineDraw();
void DivTestLineDraw2();
void DivTestaaToN();
void DivTestLineDraw3();
void DivTestxyUpdate();
void DivTestSQSQ();
void DivTestModulo();
void DivTestModulo2();
void DivTestSQSQ2();
void DivTestab2();
void DivTestab3();
void DivTestLineDraw4();
void DivTestModulo3();
void DivTestModulo4();
void DivTestModulo5();
void DivTestModulo6();
void DivTestSplitHalf();
void DivTestLineDraw5();
void DivTestLineDraw6();
void DivTestLineDraw7();
void DivTestInvMul1();
void DivTestLineDraw8();
void DivTestLineDraw9();
void DivTestab4();
void DivTestab5();
void DivTestaa_ab();
void DivTestab6();
void DivTestsqsq3();
void DivTestBitByBit();
void DivTestBitByBit2();
void DivTestBitByBit3();
