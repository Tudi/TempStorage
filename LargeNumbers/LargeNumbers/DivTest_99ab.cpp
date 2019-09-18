#include "StdAfx.h"

int CheckCandidateMatch2( LargeNumber *tN, LargeNumber *mul, LargeNumber *a1, LargeNumber *b1, int pos, LargeNumber *TempRes )
{
    //99 * ( a2 + b2 )
    if( MAX( a1->Len, b1->Len ) + mul->Len - 1 > tN->Len )
        return 0;
    //a2 * b2
    if( a1->Len + b1->Len - 1 > tN->Len )
        return 0;
    //99 * ( a2 + b2 ) + a2 * b2 = N - 99 * 99
    LargeNumber apb,mulapb, amb;

    AddLN( a1, b1, &apb );

    MulLN( mul, &apb, &mulapb );
    
    MulLN( a1, b1, &amb );
    
    AddLN( &mulapb, &amb, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

void DivTest2( int iA, int iB, int imul)
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
    int ia = iA - imul;
    int ib = iB - imul;
    printf("Expecting solution a = %d, b = %d. SQRT(N) = %d. Bruteforce trycount %d\n", ia, ib, isqrt( iN ), isqrt( iN ) / 2 );

    LargeNumber a1, b1, mul;

    SetLN( mul, imul );
    SetLN( tN, iN - imul * imul );

    LargeNumber EndSignal;
    InitLN( EndSignal );
#define ParamCount 3
    LargeNumber *vLN[ParamCount];
    vLN[0] = &a1;
    vLN[1] = &b1;
    vLN[2] = &EndSignal;

    for( int i = 0; i < ParamCount; i++ )
        SetLN( vLN[i], 1 );
    InitLN( vLN[ ParamCount - 1 ] );

    //start generating combinations and check if it's a feasable candidate
    int AtPos = 0;
    int SolutionFound = 0;
    int SolutionsFound = 0;
    int CandidatesFound = 0;
    int StepsTaken = 0;
    char DEBUG_Combinations_generated[ParamCount][99+1];
    memset( DEBUG_Combinations_generated, 0, sizeof( DEBUG_Combinations_generated ) );
    do
    {
        LargeNumber TempRes;
        int GenNextCandidate = 0;
        StepsTaken++;
/*
SetLN( a1, ia );
SetLN( b1, ib );
/**/
for( int i = 0; i < ParamCount - 1; i++ )
{
	__int64 Combo;
    ToIntLN( vLN[i], &Combo );
    DEBUG_Combinations_generated[i][ Combo % 100 ] = 1;
}
        int Match = CheckCandidateMatch2( &tN, &mul, &a1, &b1, AtPos, &TempRes );
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
            {
                int chars[4]={'\\','|','/','-'};
                printf("\r%c", chars[ ( CandidatesFound / 100 ) % 4 ] );
            }
            SolutionFound = CheckSolution( tN, TempRes ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d )sol : \t a:", SolutionsFound, CandidatesFound );
                PrintLN( a1 );
                printf("\t b:");
                PrintLN( b1 );
                printf("\t tN:");
                PrintLN( tN );
                printf("\n"); 
                GenNextCandidate = 1;
            }
            AtPos++;
            ResetCandidateAtPos( vLN, ParamCount, AtPos, 1 );
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
//    }while( SolutionFound == 0 && vLN[ParamCount-1]->Digits[0] == 0 );
    }while( vLN[ParamCount-1]->Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "\rNo Luck finding a solution\n");
    else
        printf( "\rDone testing all possible solutions\n");

    printf("Steps taken %d\n\n", StepsTaken );

/*
    for( int i = 0; i < ParamCount - 1; i++ )
        for( int j = 0; j < 100; j++ )
            if(  DEBUG_Combinations_generated[i][ j ] != 1 )
                printf("missing combination i, j => %d %d \n", i, j );
*/
}

void DivTest99ab()
{
//    DivTest2( 349, 751, 234 ); // N = 262099 SN = 511

//    DivTest2( 6871, 7673, 6034 ); // N = 52721183 , SN = 7260
    // 960k tries
    DivTest2( 26729, 31793, isqrt( 26729 * 31793 ) / 2 ); // N = 849795097 , SN = 29151
    // 702k tries
    DivTest2( 26729, 31793, 26700 ); // N = 849795097 , SN = 29151
    // 808k tries
    DivTest2( 26729, 31793, 20000 ); // N = 849795097 , SN = 29151
    // 1.1M tries
    DivTest2( 26729, 31793, 10000 ); // N = 849795097 , SN = 29151
}
