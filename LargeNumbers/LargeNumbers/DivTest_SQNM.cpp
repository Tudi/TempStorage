#include "StdAfx.h"
#include <math.h>

int CheckCandidateMatch5( LargeNumber *SQN, LargeNumber *m, LargeNumber *k, LargeNumber *y, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2 )
{
    // k * SQN + m + y * k = y * SQN
    if( k->Len > SQN->Len || y->Len > SQN->Len )
        return 0;

    LargeNumber kmsqn, ymk, kSQNm;

    MulLN( k, SQN, &kmsqn );
    AddLN( &kmsqn, m, &kSQNm );
    MulLN( y, k, &ymk );
    AddLN( &kSQNm, &ymk, TempRes1 );

    MulLN( y, SQN, TempRes2 );

    EatLeadingZeros( TempRes1 );
    EatLeadingZeros( TempRes2 );
    for( int i = 0; i <= pos; i++ )
        if( TempRes1->Digits[i] != TempRes2->Digits[i] )
            return 0;
    return 1;
}

int CheckCandidateMatch5_1( LargeNumber *SQN, LargeNumber *N, LargeNumber *k, LargeNumber *y, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2 )
{
    // ( SQN - k ) * ( SQN + y ) = N
    // SQN * SQN + SQN * y - k * SQN - k * y = N
    // SQN * ( SQN + y ) = N + k * ( SQN + y )
    if( k->Len > SQN->Len || y->Len > SQN->Len )
        return 0;
//    TempRes1->Len = 0; TempRes2->Len = 1;
//    return 1;

    LargeNumber SQNy, ksqny;

    AddLN( SQN, y, &SQNy );

    MulLN( SQN, &SQNy, TempRes1 );

    MulLN( k, &SQNy, &ksqny );
    AddLN( N, &ksqny, TempRes2 );

    EatLeadingZeros( TempRes1 );
    EatLeadingZeros( TempRes2 );
    for( int i = 0; i <= pos; i++ )
        if( TempRes1->Digits[i] != TempRes2->Digits[i] )
            return 0;
    return 1;
}

void DivTest5( unsigned int iA, unsigned int iB)
{
    // A = SQN - k
    // B = SQN + y
    // k * SQN + m + y * k = y * SQN
    // k = [ 1, SQN - 1 ]
    // y = [ 1, SQN ]
    LargeNumber SQN, m, N;
    unsigned int iN = iA * iB;
    unsigned int iSQN = isqrt( iN );
    unsigned int im = iN - iSQN * iSQN;
        unsigned int ik = iSQN - iA;
        unsigned int iy = iB - iSQN;
        printf( "Searching for k = %d, y = %d\n", ik, iy );
    SetLN( N, iN );
    SetLN( SQN, iSQN );
    SetLN( m, im );

    LargeNumber k, y;

    //init the coefficients
    InitLN( k );
    InitLN( y );

    LargeNumber EndSignal;
    InitLN( EndSignal );
#define ParamCount 3
    LargeNumber *vLN[ParamCount];
    vLN[0] = &k;
    vLN[1] = &y;
    vLN[2] = &EndSignal;

    //start generating combinations and check if it's a feasable candidate
    int i = 0;
    int SolutionsFound = 0;
    int CandidatesFound = 0;
    int CrossChecks = 0;
    int StepsTaken = 0;
    do
    {
        LargeNumber TempRes1, TempRes2;
        int GenNextCandidate = 0;
        StepsTaken++;
/*        
SetLN( y, iy );
SetLN( k, ik );
/**/
/*        
SetLN( y, iy % 10);
SetLN( k, ik % 10);
i = 1;
/**/
/*
        int ty,tk;
        ToIntLN( y, ty );
        if( ty == ik || ty == iy )
            tk = ty;
        ToIntLN( k, tk );
        if( tk == ik || tk == iy )
            ty = tk;
/**/
//        int Match = CheckCandidateMatch5( &SQN, &m, &k, &y, i, &TempRes1, &TempRes2 );
        int Match = CheckCandidateMatch5_1( &SQN, &N, &k, &y, i, &TempRes1, &TempRes2 );
/*        if( Match == 1 )
        {
            Match = CheckCandidateMatch5_1( &SQN, &N, &k, &y, i, &TempRes1, &TempRes2 );
            if( Match == 0 )
                CrossChecks++;
        }/**/
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
                printf("#");
            int SolutionFound = CheckSolution( TempRes1, TempRes2 ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d / %d)sol : \t k:", SolutionsFound, CandidatesFound, CrossChecks );
                PrintLN( k );
                printf("\t y:");
                PrintLN( y );
                printf("\t tN:");
                PrintLN( TempRes1 );
                printf("\n"); 
                GenNextCandidate = 1;
            }
            else
            {
                i++;
                ResetCandidateAtPos( vLN, ParamCount, i, 1 );
            }
        }
        else
            GenNextCandidate = 1;
        if( i > 0 && EndSignal.Digits[i] > 0 )
        {
            ResetCandidateAtPos( vLN, ParamCount, i, 0 );
            i--;
            GenNextCandidate = 1;
        }
        if( GenNextCandidate == 1 )
            GenerateNextCandidateAtPos( vLN, ParamCount, i );
    }while( SolutionsFound == 0 && EndSignal.Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "\rNo Luck finding a solution\n");
    else
        printf( "\rDone testing all possible solutions\n");
    printf("Steps taken %d\n\n", StepsTaken );
}

void DivTest_SQNM()
{
//    DivTest5( 349, 751 ); // SN = 511
//    DivTest5( 6871, 7673 ); // N = 52721183 , SN = 7260
    DivTest5( 26729, 31793 ); // N = 849795097 , SN = 29151
}
