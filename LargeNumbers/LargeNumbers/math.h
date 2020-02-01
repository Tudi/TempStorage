#pragma once

#define MAX_DIGIT_COUNT 500
#define USE_BASE        10

#ifndef MAX
    #define MAX( a, b ) ((a)>(b))?(a):(b)
    #define MAX3( a, b, c ) MAX( a, MAX( b, c ) )
#endif

struct LargeNumber
{
	LargeNumber()
	{
		Len = Pos = 0;
		memset(Digits, 0, sizeof(Digits));
	}
    int Digits[MAX_DIGIT_COUNT];
    int Len;    // not used yet ?
	int Pos;    // position of digit we are working on
};

void InitLN( LargeNumber *N );
void InitLN( LargeNumber &N );
void SetLN( LargeNumber *N, __int64 n );
void SetLN( LargeNumber &N, __int64 n );
//void SetLN(LargeNumber *dst, LargeNumber *src);
void NormalizeLN( LargeNumber *N, int Start = 0 );
void NormalizeLN( LargeNumber &N, int Start = 0 );
void AddLN( LargeNumber *A, LargeNumber *B, LargeNumber *C );
void AddLN( LargeNumber &A, LargeNumber &B, LargeNumber &C );
//void AddLN(LargeNumber *A, int B, LargeNumber *C);
void MulLN( LargeNumber *A, LargeNumber *B, LargeNumber *C );
void MulLN( LargeNumber &A, LargeNumber &B, LargeNumber &C );
//void MulLN( LargeNumber *A, int B, LargeNumber *C);
void ToIntLN( LargeNumber *N, __int64 *n );
void ToIntLN( LargeNumber &N, __int64 &n );
__int64 ToIntLN( LargeNumber *N );
void CopyLN( LargeNumber *From, LargeNumber *To );
void CopyLN( LargeNumber &From, LargeNumber &To );
void HalfLN( LargeNumber *N );
void HalfLN( LargeNumber &N );
void SubLN( LargeNumber *N, int n );
void SubLN( LargeNumber &N, int n );
void SubLN(LargeNumber *A, LargeNumber *B, LargeNumber *C);

void EatLeadingZeros( LargeNumber *N );
void EatLeadingZeros( LargeNumber &N );

void PrintLN( LargeNumber *N, int EndPos = -1 );
void PrintLN( LargeNumber &N, int EndPos = -1 );

unsigned int isqrt(__int64 n);
__int64 isqrt2(__int64 n);
__int64 isqrt3(__int64 n);
__int64 isqrt4(__int64 n);
__int64 isqrtNegative(__int64 n);
int IsLarger(LargeNumber *Larger, LargeNumber *Smaller);
void ReCheckSize(LargeNumber *Larger);