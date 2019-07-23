#pragma once

void GenerateNextCandidateAtPos( LargeNumber **vLN, int ParamCount, int pos );
void ResetCandidateAtPos( LargeNumber **vLN, int ParamCount, int pos, int Forward = 0 );
int CheckSolution( LargeNumber &tN, LargeNumber &TempRes );

int CheckCandidateMatch2( LargeNumber *tN, LargeNumber *mul, LargeNumber *a1, LargeNumber *b1, int pos, LargeNumber *TempRes );

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
