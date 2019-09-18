#include "StdAfx.h"

int CheckCandidateMatch_RecDiv( LargeNumber *tN, LargeNumber *k1, LargeNumber *k2, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes )
{
    // (k1 * a + m1)*(k2 * b + m2) = N
    // k1 * a < SQN      k2 * b < SQN
/*    int RealLens[10];
    memset( RealLens, 0, sizeof( RealLens ) );
    for( int i = 0; i < Params - 1; i++ )
    {
        RealLens[i] = vLN[i]->Len;
        while( RealLens[i] > 0 && vLN[i]->Digits[ RealLens[i] ] == 0 )
            RealLens[i]--;
    } */
    int LenSQN = tN->Len / 2;
    // len( k1 * a ) < SQ( N ) && len( k2 * b ) < SQ( N )
    if( k1->Len + vLN[0]->Len - 1 > LenSQN || k2->Len + vLN[1]->Len - 1 > LenSQN )
        return 0;
    // len( m1 ) < SQN     m2 < SQN
    if( vLN[2]->Len > LenSQN || vLN[3]->Len > LenSQN )
        return 0;

    LargeNumber k1a, k1am1, k2b, k2bm2;

    MulLN( k1, vLN[0], &k1a );
    AddLN( &k1a, vLN[2], &k1am1 );

    MulLN( k2, vLN[1], &k2b );
    AddLN( &k2b, vLN[3], &k2bm2 );

    MulLN( &k1am1, &k2bm2, TempRes );

    EatLeadingZeros( TempRes );
    if( TempRes->Len > tN->Len )
        return 0;
    for( int i = 0; i <= pos; i++ )
        if( TempRes->Digits[i] != tN->Digits[i] )
            return 0;
    return 1;
}

int IsPrime( int p )
{
    if( (p & 1) == 0 )
        return 0;
    for( int i = 3; i < p / 2; i++ )
        if( p % i == 0 )
            return 0;
    return 1;
}

int GetNextPrime( int cur )
{
    int ret = cur + 1;
    while( IsPrime( ret ) == 0 ) 
        ret++;
    return ret;
}

int LargestFact( int Limit )
{
    int ret = 1;
    int next = 1;
    while( ret * next < Limit )
    {
        ret = ret * next;
        next = GetNextPrime( next );
    }
    return ret;
}

void DivTest_RecDiv( int iA, int iB)
{
    // (k1*a + m1)*(k2 * b + m2) = N
    // m1 < k1      m2 < k2     k1 < k2     len(k1)+len(a)-1 < len( SQ(N ) )
    // k1 = [ 1, SQ(N) ] best SQ(SQ(N))
    // k1 * k2 * a * b + m2 * k1 * a + m1 * k2 * b + m1 * m2 = N
    LargeNumber tN;
    unsigned int iN = (unsigned int) ( iA * iB );
    int isqn = isqrt( iN );
//    int ik1 = isqrt( isqn );
//    int ik1 = isqn / 2;
    int ik1 = LargestFact( isqrt( isqn ) ); // makes no sense
    int ik2 = ik1;
    int ia = iA / ik1;
    int ib = iB / ik2;
    int im1 = iA - ( iA / ik1 ) * ik1;
    int im2 = iB - ( iB / ik2 ) * ik2;
    printf("One of the expected results : a=%d b=%d m1=%d m2=%d\n", ia, ib, im1, im2 );
    printf("k1 = %d \t k2 = %d \n", ik1, ik2 );
    LargeNumber k1,k2,a1,b1,m1,m2;

    SetLN( tN, iN );
    SetLN( k1, ik1 );
    SetLN( k2, ik2 );

    //init the coefficients
    LargeNumber EndSignal;
#define ParamCount 5
    LargeNumber *vLN[ParamCount];
    vLN[0] = &a1;
    vLN[1] = &b1;
    vLN[2] = &m1;
    vLN[3] = &m2;
    vLN[4] = &EndSignal;

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
        LargeNumber TempRes;
        int GenNextCandidate = 0;
        StepsTaken++;
for( int i = 0; i < ParamCount; i++ )
{
	__int64 Combo;
    ToIntLN( vLN[i], &Combo );
    DEBUG_Combinations_generated[i][ Combo % 100 ] = 1;
}
/*
SetLN( a1, iA / ik1 );
SetLN( m1, iA - ( iA / ik1 ) * ik1 );
SetLN( b1, iB / ik2 );
SetLN( m2, iB - ( iB / ik2 ) * ik2 );
/**/
        int Match = CheckCandidateMatch_RecDiv( &tN, &k1, &k2, vLN, ParamCount, AtPos, &TempRes );
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
                printf("\r%d / %d / %d)sol : \t a1:", SolutionsFound, CandidatesFound, CrossChecks );
                PrintLN( a1 );
                printf("\t b1:");
                PrintLN( b1 );
                printf("\t m1:");
                PrintLN( m1 );
                printf("\t m2:");
                PrintLN( m2 );
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

void DivTestRecDiv()
{
    // ( a1*a2...*ax + m1)*(b1*b2...*bx + m2) = N
    // m1 < a1,a2...ax
    // m2 < b1,b2...bx
    // len(a1)+..+len(ax) < len( SQN )
    // len(b1)+..+len(bx) < len( SQN )
//    DivTest_RecDiv( 349, 751 ); // N = 262099 , SN = 511
    DivTest_RecDiv( 6871, 7673 ); // N = 52721183 , SN = 7260
//    DivTest_RecDiv( 26729, 31793 ); // N = 849795097 , SN = 29151
}
