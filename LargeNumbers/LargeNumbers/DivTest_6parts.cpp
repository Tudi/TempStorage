#include "StdAfx.h"

int CheckCandidateMatch6( LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes )
{
    // A = a1 + a2 + a3...
    // B = b1 + b2 + b3....
    // N = A * B
    if( vLN[0]->Len > tN->Len )
        return 0;

    LargeNumber SumA1, SumA2, SumB1, SumB2;

    SetLN( SumA1, 0 );
    SetLN( SumB1, 0 );
    for( int i = 0; i < Params - 1; i+=2 )
    {
        AddLN( vLN[i], &SumA1, &SumA2 );
        CopyLN( SumA2, SumA1 );
        AddLN( vLN[i + 1], &SumB1, &SumB2 );
        CopyLN( SumB2, SumB1 );
    }
    MulLN( &SumA1, &SumB1, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

void DivTest6( __int64 iA, __int64 iB)
{
    // (a1 + a2 + a3)*(b1 + b2 + b3) = N
    LargeNumber N, tN;
    unsigned int iN = (unsigned int) ( iA * iB );
    SetLN( N, iN ); 

    LargeNumber a1, a2, a3, b1, b2;

    SetLN( tN, iN );

    //init the coefficients
    LargeNumber EndSignal;
#define ParamCount 5
    LargeNumber *vLN[ParamCount];
    vLN[0] = &a1;
    vLN[1] = &b1;
    vLN[2] = &a2;
    vLN[3] = &b2;
    vLN[4] = &a3;
/*    vLN[5] = &b3;
    vLN[6] = &EndSignal;
*/
    for( int i = 0; i < ParamCount; i++ )
        SetLN( vLN[i], 1 );
    InitLN( vLN[ ParamCount - 1 ] );

    //start generating combinations and check if it's a feasable candidate
    int AtPos = 0;
    int SolutionFound = 0;
    int SolutionsFound = 0;
    int CandidatesFound = 0;
    int CrossChecks = 0;
    int StepsTaken = 0;
    do
    {
        LargeNumber TempRes;
        int GenNextCandidate = 0;
        StepsTaken++;
/*
SetLN( a1, iA / 3 );
SetLN( a2, iA / 3 );
SetLN( a3, iA - iA / 3 * 2 );
SetLN( b1, iB / 3 );
SetLN( b2, iB / 3 );
SetLN( b3, iB - iB / 3 * 2 );
/**/
        int Match = CheckCandidateMatch6( &tN, vLN, ParamCount, AtPos, &TempRes );
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
                printf("#");
            SolutionFound = CheckSolution( tN, TempRes ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d / %d)sol : \t a1:", SolutionsFound, CandidatesFound, CrossChecks );
                PrintLN( a1 );
                printf("\t b1:");
                PrintLN( b1 );
                printf("\t tN:");
                PrintLN( tN );
                printf("\n"); 
                GenNextCandidate = 1;
            }
            else
            {
                AtPos++;
                ResetCandidateAtPos( vLN, ParamCount, AtPos, 1 );
            }
        }
        else
            GenNextCandidate = 1;
        if( AtPos > 0 && vLN[ParamCount-1]->Digits[AtPos] > 0 )
        {
            ResetCandidateAtPos( vLN, ParamCount, AtPos );
            AtPos--;
            GenNextCandidate = 1;
        }
        if( GenNextCandidate == 1 )
            GenerateNextCandidateAtPos( vLN, ParamCount, AtPos );
    }while( SolutionsFound < 1000 && vLN[ParamCount-1]->Digits[0] == 0 );
//    }while( SolutionsFound == 0 && vLN[ParamCount-1]->Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "No Luck finding a solution\n");
    else
        printf( "Done testing all possible solutions\n");
    printf("Steps taken %d\n", StepsTaken );
}

void DivTest6parts()
{
    DivTest6( 349, 751 ); // N = 262099 , SN = 511
//    DivTest4( 6871, 7673 ); // N = 52721183 , SN = 7260
//    DivTest4( 26729, 31793 ); // N = 849795097 , SN = 29151
}
