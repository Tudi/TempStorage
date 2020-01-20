#include "StdAfx.h"


int CheckCandidateMatch3( LargeNumber *tN, LargeNumber *a, LargeNumber *b, int pos, LargeNumber *TempRes )
{
    //a * b = N
    if( a->Len + b->Len - 1 > tN->Len )
        return 0;

    MulLN( a, b, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

void DivTest3( __int64 iA, __int64 iB)
{
    // sqrt( N )
    // a * b = N
    LargeNumber N, tN;
    unsigned int iN = (unsigned int) ( iA * iB );
    SetLN( N, iN ); 

    LargeNumber a1, b1;

    SetLN( tN, iN );

    //init the coefficients
    InitLN( a1 );
    InitLN( b1 );

    LargeNumber EndSignal;
    InitLN( EndSignal );
    LargeNumber *vLN[4];
    vLN[0] = &a1;
    vLN[1] = &b1;
    vLN[2] = &EndSignal;

    //start generating combinations and check if it's a feasable candidate
    int i = 0;
    int SolutionFound = 0;
    int SolutionsFound = 0;
    int CandidatesFound = 0;
    int StepsTaken = 0;
    do
    {
        LargeNumber TempRes;
        StepsTaken++;
/*
SetLN( a1, iA );
SetLN( b1, iB );
/**/
        int Match = CheckCandidateMatch3( &tN, &a1, &b1, i, &TempRes );
        if( Match == 1 )
        {
            SolutionFound = CheckSolution( tN, TempRes ); 
            if( SolutionFound == 1 )
            {
                printf("%d)possible candidate : \n\t a:", ++CandidatesFound);
                PrintLN(a1);
                printf("\n\t b:");
                PrintLN(b1);
                printf("\n\t tN:");
                PrintLN(tN);
                printf(" == ");
                PrintLN(TempRes);
                printf("\n");
                SolutionsFound++;
                printf( "%d)!!!!!!yolo\n",SolutionsFound);
            }
            i++;
            ResetCandidateAtPos( vLN, 3, i, 1 );
        }
        else
            GenerateNextCandidateAtPos( vLN, 3, i );
        if( i > 0 && EndSignal.Digits[i] > 0 )
        {
            ResetCandidateAtPos( vLN, 3, i );
            i--;
            GenerateNextCandidateAtPos( vLN, 3, i );
        }
    }while( SolutionsFound < 2 && EndSignal.Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "No Luck finding a solution\n");
    else
        printf( "Done testing all possible solutions\n");
    printf("Steps taken %d\n", StepsTaken );
}

void DivTestab()
{
//    int iA = 349;
//    int iB = 751;
//    int imul = 234;
//    DivTest3( iA, iB, imul ); // SN = 511

//    DivTest3( 6871, 7673, 6034 ); // N = 52721183 , SN = 7260
    DivTest3( 26729, 31793 ); // N = 849795097 , SN = 29151
}
