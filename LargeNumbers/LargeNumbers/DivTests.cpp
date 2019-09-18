#include "StdAfx.h"

void GenerateNextCandidateAtPos( LargeNumber **vLN, int ParamCount, int pos )
{
    int i = 0;
    vLN[i]->Digits[ pos ]++;
    while( i < ParamCount && vLN[i]->Digits[ pos ] >= USE_BASE )
    {
        if( i < ParamCount - 1 )
            vLN[ i + 1 ]->Digits[ pos ]++;
        vLN[ i ]->Digits[ pos ] = 0;
        i++;
    }

    for( int i = 0; i < ParamCount; i++ )
        vLN[i]->Len = pos + 1;
}

int CheckSolution( LargeNumber &tN, LargeNumber &TempRes )
{
    if( tN.Len != TempRes.Len )
        return 0;
    for( int i = 0; i < tN.Len; i++ )
        if( tN.Digits[i] != TempRes.Digits[i] )
            return 0;
    return 1;
}

void ResetCandidateAtPos( LargeNumber **vLN, int ParamCount, int pos, int Forward )
{
    for( int i = 0; i < ParamCount; i++ )
    {
        vLN[i]->Len = pos + Forward;
        vLN[i]->Digits[ pos ] = 0;
    }
}

int GetBitCount(__int64 Nr)
{
	int ret = 0;
	while (Nr > 0)
	{
		ret++;
		Nr = Nr / 2;
	}
	return ret;
}

int GetDigitCount(__int64 Nr)
{
	if (Nr == 0)
		return 1;
	int ret = 0;
	while (Nr > 0)
	{
		ret++;
		Nr = Nr / 10;
	}
	return ret;
}

__int64 GetMaskDecimal(__int64 Nr1, __int64 Nr2)
{
	if (Nr2 < 1)
		Nr2 = Nr1;
	if (Nr1 == Nr2 && Nr1 == 0)
		return 10;
	int Mask = 1;
	while (Mask <= Nr1 || Mask <= Nr2)
		Mask *= 10;
	return Mask;
}

int IsPrime(__int64 Nr)
{
	if (Nr % 2 == 0)
		return 0;
	__int64 SN = isqrt(Nr);
	for (__int64 i = 3; i < SN; i += 2)
		if (Nr % i == 0)
			return 0;
	return 1;
}