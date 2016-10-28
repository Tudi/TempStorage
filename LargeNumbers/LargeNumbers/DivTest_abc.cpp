#include "StdAfx.h"

int CheckCandidateMatch4( LargeNumber *tN, LargeNumber *a, LargeNumber *b, LargeNumber *c, int pos, LargeNumber *TempRes )
{
    // a * b + a * a + c * ( a + b ) = N
    if( a->Len + b->Len - 1 > tN->Len || a->Len + a->Len - 1 > tN->Len || a->Len + c->Len - 1 > tN->Len || c->Len + b->Len - 1 > tN->Len || MAX3( a->Len, b->Len, c->Len ) >= tN->Len )
        return 0;

    LargeNumber amb, ama, apb, cmapb, ambpama;

    MulLN( a, b, &amb );
    MulLN( a, a, &ama );
    AddLN( a, b, &apb );
    MulLN( &apb, c, &cmapb );
    AddLN( amb, ama, ambpama );
    AddLN( &ambpama, &cmapb, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

int CheckCandidateMatch4_1( LargeNumber *tN, LargeNumber *a, LargeNumber *b, LargeNumber *c, int pos, LargeNumber *TempRes )
{
    // A * B = N
    // a = A - c    b = B - a   A = a + c   B = a + b
    if( a->Len + b->Len - 1 > tN->Len || a->Len + a->Len - 1 > tN->Len || a->Len + c->Len - 1 > tN->Len || c->Len + b->Len - 1 > tN->Len || MAX3( a->Len, b->Len, c->Len ) >= tN->Len )
        return 0;

    LargeNumber A, B;

    AddLN( a, c, &A );
    AddLN( a, b, &B );
    MulLN( &A, &B, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

void DivTest4( __int64 iA, __int64 iB)
{
    // a * b + a * a + c * ( a + b ) = N
    LargeNumber N, tN;
    unsigned int iN = (unsigned int) ( iA * iB );
    SetLN( N, iN ); 

    LargeNumber a, b, c;

    SetLN( tN, iN );

    //init the coefficients
    SetLN( a, 1 );
    SetLN( b, 1 );
    SetLN( c, 1 );

    LargeNumber EndSignal;
    InitLN( EndSignal );
#define ParamCount 4
    LargeNumber *vLN[ParamCount];
    vLN[0] = &a;
    vLN[1] = &b;
    vLN[2] = &c;
    vLN[3] = &EndSignal;

    //start generating combinations and check if it's a feasable candidate
    int i = 0;
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
SetLN( a, iA - 3 );
SetLN( b, iB - ( iA - 3 ) );
SetLN( c, 3 );
/**/
        int Match = CheckCandidateMatch4_1( &tN, &a, &b, &c, i, &TempRes );
/*        if( Match == 1 )
        {
            Match = CheckCandidateMatch4( &tN, &a, &b, &c, i, &TempRes );
            if( Match != 1 )
                CrossChecks++;
        }/**/
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
                printf("#");
            SolutionFound = CheckSolution( tN, TempRes ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d / %d)sol : \t a:", SolutionsFound, CandidatesFound, CrossChecks );
                PrintLN( a );
                printf("\t b:");
                PrintLN( b );
                printf("\t c:");
                PrintLN( c );
                printf("\t tN:");
                PrintLN( tN );
//                printf(" == ");
//                PrintLN( TempRes );
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
            ResetCandidateAtPos( vLN, ParamCount, i );
            i--;
            GenNextCandidate = 1;
        }
        if( GenNextCandidate == 1 )
            GenerateNextCandidateAtPos( vLN, ParamCount, i );
//    }while( SolutionsFound < 1000 && EndSignal.Digits[0] == 0 );
    }while( SolutionsFound == 0 && EndSignal.Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "No Luck finding a solution\n");
    else
        printf( "Done testing all possible solutions\n");
    printf("Steps taken %d\n", StepsTaken );
}

void DivTestabc()
{
    DivTest4( 349, 751 ); // SN = 511
//    DivTest4( 6871, 7673 ); // N = 52721183 , SN = 7260
//    DivTest4( 26729, 31793 ); // N = 849795097 , SN = 29151
}
