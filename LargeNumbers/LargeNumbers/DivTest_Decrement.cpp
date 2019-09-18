#include "StdAfx.h"

int CheckCandidateMatch_DivTestDecrement( LargeNumber *tN, LargeNumber *a, LargeNumber *b, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2 )
{
    // aa * bb + a * b = N + aa * b + a * bb
    int RealLens[10];
    memset( RealLens, 0, sizeof( RealLens ) );
    for( int i = 0; i < Params - 1; i++ )
    {
        RealLens[i] = vLN[i]->Len;
        while( RealLens[i] > 0 && vLN[i]->Digits[ RealLens[i] ] == 0 )
            RealLens[i]--;
    }
    if( RealLens[0] - 1 > tN->Len / 2 || RealLens[1] > tN->Len )
        return 0;

    LargeNumber AB, ab, Ab, aB, NAb;

    MulLN( vLN[0], vLN[1], &AB );
    MulLN( a, b, &ab );

    MulLN( vLN[0], b, &Ab );
    MulLN( vLN[1], a, &aB );

    AddLN( &AB, &ab, TempRes1 );
    AddLN( tN, &Ab, &NAb );
    AddLN( &NAb, &aB, TempRes2 );

    EatLeadingZeros( TempRes1 );
    EatLeadingZeros( TempRes2 );

    for( int i = 0; i <= pos; i++ )
        if( TempRes1->Digits[i] != TempRes2->Digits[i] )
            return 0;
    return 1;
}

void DivTest_DivTestDecrement( int iA, int iB)
{
    // ( aa - a ) * ( bb - b ) = N
    // aa * bb - aa * b - a * bb + a * b = N
    // aa * bb + a * b = N + aa * b + a * bb
    LargeNumber tN;
    unsigned int iN = (unsigned int) ( iA * iB );
    int isqn = isqrt( iN );
    int ia = isqrt( isqn );
    int ib = isqn;
    int iaa = iA + ia;
    int ibb = iB + ib;
    LargeNumber aa,bb,a,b;

    printf("Using a = %d and b = %d. Expecting aa = %d bb = %d\n", ia, ib, iaa, ibb );

    SetLN( tN, iN );
    SetLN( a, ia );
    SetLN( b, ib );

    //init the coefficients
    LargeNumber EndSignal;
#define ParamCount 3
    LargeNumber *vLN[ParamCount];
    vLN[0] = &aa;
    vLN[1] = &bb;
    vLN[2] = &EndSignal;

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
    char DEBUG_Combinations_generated[ParamCount][99+1];
    memset( DEBUG_Combinations_generated, 0, sizeof( DEBUG_Combinations_generated ) );
    do
    {
//        LargeNumber TempRes;
        LargeNumber TempRes1, TempRes2;
        int GenNextCandidate = 0;
        StepsTaken++;
for( int i = 0; i < ParamCount - 1; i++ )
{
	__int64 Combo;
    ToIntLN( vLN[i], &Combo );
    DEBUG_Combinations_generated[i][ Combo % 100 ] = 1;
}
/*
SetLN( aa, iaa );
SetLN( bb, ibb );
/**/
        int Match = CheckCandidateMatch_DivTestDecrement( &tN, &a, &b, vLN, ParamCount, AtPos, &TempRes1, &TempRes2 );
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
            {
                int chars[4]={'\\','|','/','-'};
                printf("\r%c", chars[ ( CandidatesFound / 100 ) % 4 ] );
            }
            SolutionFound = CheckSolution( TempRes1, TempRes2 ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d / %d)sol : \t a:", SolutionsFound, CandidatesFound, CrossChecks );
                PrintLN( aa );
                printf("\t b:");
                PrintLN( bb );
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
        printf( "\rNo Luck finding a solution\n");
    else
        printf( "\rDone testing all possible solutions\n");
    printf("Steps taken %d\n\n", StepsTaken );

for( int i = 0; i < ParamCount - 1; i++ )
    for( int j = 0; j < 100; j++ )
        if(  DEBUG_Combinations_generated[i][ j ] != 1 )
            printf("missing combination i, j => %d %d \n", i, j );
}

void DivTestDecrement()
{
    DivTest_DivTestDecrement( 349, 751 ); // N = 262099 , SN = 511
//    DivTest_DivTestDecrement( 6871, 7673 ); // N = 52721183 , SN = 7260
//    DivTest_DivTestDecrement( 26729, 31793 ); // N = 849795097 , SN = 29151
}
