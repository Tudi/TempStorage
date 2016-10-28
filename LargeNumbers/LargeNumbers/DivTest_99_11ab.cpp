#include "StdAfx.h"

int CrossCheck( LargeNumber *a1, LargeNumber *b1, int i, int imul2, int imul, int iN )
{
    int imul1_2 = imul - imul2;
    LargeNumber a2, b2, mul2, mul1_2, tN2, TempRes2;

    SetLN( mul2, imul2 );
    SetLN( mul1_2, imul1_2 );
    SetLN( tN2, iN - imul2 * imul2 );

    //init the coefficients
    AddLN( a1, &mul1_2, &a2 );
    AddLN( b1, &mul1_2, &b2 );
    return CheckCandidateMatch2( &tN2, &mul2, &a2, &b2, i, &TempRes2 );
}

void DivTest4( int iA, int iB, int imul)
{
    // sqrt( N )
    // ( 111 + a ) * ( 111 + b ) = N = 111 * 111 + 111 ( a + b ) + a * b
    // 11 * ( a1 + b1 ) + a1 * b1 = N - 11 * 11
    // 99 * ( a2 + b2 ) + a2 * b2 = N - 99 * 99
    // a2 = a1 + 88 
    // b2 = b1 + 88
    LargeNumber N, tN;
    unsigned int iN = iA * iB;
    SetLN( N, iN ); 

    LargeNumber a1, b1, mul;

    SetLN( mul, imul );
    SetLN( tN, iN - imul * imul );

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
    int DoubleCheckHelped = 0;
    int TrippleCheckHelped = 0;
    int StepsTaken = 0;
    do
    {
        LargeNumber TempRes;
        StepsTaken++;
/*
SetLN( a1, iA - imul );
SetLN( b1, iB - imul );
/**/
        int Match = CheckCandidateMatch2( &tN, &mul, &a1, &b1, i, &TempRes );
        if( Match == 1 )
        {
            Match = CrossCheck( &a1, &b1, i, imul / 7, imul, iN );
            if( Match != 1 )
            {
                DoubleCheckHelped++;
//                printf( "%d - %d)It helped 1\n",DoubleCheckHelped,TrippleCheckHelped);
            }
            else
            {
                Match = CrossCheck( &a1, &b1, i, imul / 17, imul, iN );
                if( Match != 1 )
                {
                    TrippleCheckHelped++;
//                    printf( "%d)It helped 2\n",TrippleCheckHelped);
                }
            } /**/
        }
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
                printf("#");
            SolutionFound = CheckSolution( tN, TempRes ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d / %d / %d)sol : \t a:", SolutionsFound, CandidatesFound, DoubleCheckHelped, TrippleCheckHelped );
                PrintLN( a1 );
                printf("\t b:");
                PrintLN( b1 );
                printf("\t tN:");
                PrintLN( tN );
//                printf(" == ");
//                PrintLN( TempRes );
                printf("\n"); 
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
    }while( SolutionFound == 0 && EndSignal.Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "No Luck finding a solution\n");
    else
        printf( "Done testing all possible solutions\n");
    printf("Steps taken %d\n\n", StepsTaken );
}

void DivTest9911ab()
{
//    DivTest4( 349, 751, 234 ); // SN = 511
//    DivTest4( 6871, 7673, 6034 ); // N = 52721183 , SN = 7260
    DivTest4( 26729, 31793, 26700 ); // N = 849795097 , SN = 29151
}
