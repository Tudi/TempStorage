#include "StdAfx.h"

int CheckCandidateMatch1( LargeNumber *tN, LargeNumber *d, LargeNumber *e, LargeNumber *f, LargeNumber *g, int pos, LargeNumber *TempRes )
{
    LargeNumber de, defg, fg, tres, two, twodefg;
    MulLN( d, e, &de );
    
    MulLN( f, g, &fg );
    
    MulLN( de, fg, defg );

    SetLN( two, 2 );
    MulLN( two, defg, twodefg );

    AddLN( de, fg, tres );
    AddLN( &twodefg, &tres, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

int CheckCandidateMatch1( LargeNumber &tN, LargeNumber &d, LargeNumber &e, LargeNumber &f, LargeNumber &g, int pos, LargeNumber &TempRes )
{
    return CheckCandidateMatch1( &tN, &d, &e, &f, &g, pos, &TempRes );
}

void DivTest1()
{
    //( 2 * d * e + 1 ) * ( 2 * e * f + 1 ) = N
    //2 * ( d * e + 2 * d * e * f * g + f * g ) + 1 = N
    //( d * e + 2 * d * e * f * g + f * g ) = N - 1 / 2
    // 2 * 87 + 2 * 2 * 87 * 5 * 75 + 75 * 5 = 131049
    LargeNumber N, tN;
    int iA = 349;
    int iB = 751;
    int iN = iA * iB;
    SetLN( N, iN ); // 262099

    // would be great to avoid cases when d,e has values { 0, 1, 2 } or f,g has values { 0, 1, 2 }
    LargeNumber d,e,f,g;
    CopyLN( N, tN );

    //substract 1
    SubLN( tN, 1 );
    HalfLN( tN );   // 131049

    //init the coefficients
    InitLN( d );
    InitLN( e );
    InitLN( f );
    InitLN( g );

    LargeNumber EndSignal;
    InitLN( EndSignal );
    LargeNumber *vLN[4];
    vLN[0] = &d;
    vLN[1] = &e;
    vLN[2] = &f;
    vLN[3] = &g;
    vLN[4] = &EndSignal;

    //start generating combinations and check if it's a feasable candidate
    int i = 0;
    int SolutionFound = 0;
    int SolutionsFound = 0;
    int CandidatesFound = 0;
    do
    {
        LargeNumber TempRes;
        GenerateNextCandidateAtPos( vLN, 5, i );
/*
SetLN( d, 2 );d.Len = 2;
SetLN( e, 87 );
SetLN( f, 5 );f.Len = 2;
SetLN( g, 75 );
/**/
/*
        if( d.Digits[0] == 2 && e.Digits[0] == 7 && f.Digits[0]==5 && g.Digits[0]==5 )
        {
            printf("maybe");
            if( d.Digits[1] == 0 && e.Digits[1] == 8 && f.Digits[1]==0 && g.Digits[1]==7 )
                printf("baybe");
        }
/**/
        int Match = CheckCandidateMatch1( tN, d, e, f, g, i, TempRes );
        if( Match == 1 )
        {
            printf("%d)possible candidate : \n\t d:", ++CandidatesFound );
            PrintLN( d );
            printf("\n\t e:");
            PrintLN( e );
            printf("\n\t f:");
            PrintLN( f );
            printf("\n\t g:");
            PrintLN( g );
            printf("\n\t tN:");
            PrintLN( tN );
            printf(" == ");
            PrintLN( TempRes );
            printf("\n");
            SolutionFound = CheckSolution( tN, TempRes ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf( "%d)!!!!!!yolo\n",SolutionsFound);
            }
            i++;
        }
        if( i > 0 && EndSignal.Digits[i] > 0 )
        {
            ResetCandidateAtPos( vLN, 5, i );
            i--;
        }
    }while( SolutionFound == 0 );

    printf( "Done testing all possible solutions\n");
}
