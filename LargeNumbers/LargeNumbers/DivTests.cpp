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
