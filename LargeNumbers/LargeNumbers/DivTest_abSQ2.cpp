#include "StdAfx.h"

LargeNumber m9;
LargeNumber six, two;
LargeNumber SQRTN2;


int CheckCandidateMatch_abSQ2(LargeNumber *tN, LargeNumber **vLN, int Params, int pos, LargeNumber *TempRes1, LargeNumber *TempRes2)
{
	// Len(a) <= Len(N) / 2		Len(b) <= ( Len(N) + 1 ) / 2
	if (vLN[0]->Len > (tN->Len + 1) / 2 || vLN[1]->Len > (tN->Len + 1) / 2)
		return 0;

	// a = 3 + x	b = SQN + y
	// for any already tested a * a we can also write : a * a + 2 * x * a + x * x + m = 2 * SQN * y + y * y
	// simplified version is : a * a + 2 * x * a + x * x + N = b * b
	// if ( a = ( B - A ) / 2 ) and a > 3 than : 9 + 6 * x + m = 2 * SQN * y + y * y
	LargeNumber x6;
	MulLN(vLN[0], &six, &x6);
	LargeNumber xx;
	MulLN(vLN[0], vLN[0], &xx);
	LargeNumber m9x6;
	AddLN(&m9, &x6, &m9x6);
	AddLN(&xx, &m9x6, TempRes1);

	LargeNumber y2srtn;
	MulLN(vLN[1], &SQRTN2, &y2srtn);

	LargeNumber yy;
	MulLN(vLN[1], vLN[1], &yy);
	AddLN(&y2srtn, &yy, TempRes2);

	for (int i = 0; i <= pos; i++)
		if (TempRes1->Digits[i] != TempRes2->Digits[i])
			return 0;

	// a = 3 + x	b = SQN + y
	// A = a - b	B = a + b
	return 1;
}

int CheckSolution2(LargeNumber &TempRes1, LargeNumber &TempRes2)
{
	EatLeadingZeros(TempRes1);
	EatLeadingZeros(TempRes2);
	if (TempRes1.Len != TempRes2.Len)
		return 0;
	for (int i = 0; i <= TempRes1.Len; i++)
		if (TempRes1.Digits[i] != TempRes2.Digits[i])
			return 0;
	return 1;
}

void DivTest_abSQ2( int iA, int iB)
{
    // ( a - b ) * ( a + b ) = N
    // a * a - b * b = N	
    LargeNumber N;
    unsigned int iN = iA * iB;
    SetLN( N, iN ); 
	int ia = (iB - iA) / 2;
	int ib = (iA + iB) / 2;
	int iSQRTN = isqrt(iN);
	int im = iA * iB - iSQRTN * iSQRTN;
    printf("Expecting solution a = %d, b = %d. N = %d, SQRT(N) = %d. Bruteforce trycount %d\n", ia, ib, iN, iSQRTN, isqrt( iN ) / 2 );

    LargeNumber a, b;
	LargeNumber EndSignal;

#define ParamCount 3
    LargeNumber *vLN[ParamCount];
    vLN[0] = &a;
    vLN[1] = &b;
	vLN[2] = &EndSignal;

    for( int i = 0; i < ParamCount; i++ )
        SetLN( vLN[i], 0 );
    InitLN( vLN[ ParamCount - 1 ] );

	SetLN(&m9, im + 9);
	SetLN(&six, 6);
	SetLN(&two, 2);
	SetLN(&SQRTN2, iSQRTN * 2);

//	SetLN(&a, 198);
//	SetLN(&b, 39);

    //start generating combinations and check if it's a feasable candidate
    int AtPos = 0;
    int SolutionsFound = 0;
    int CandidatesFound = 0;
    int StepsTaken = 0;
    char DEBUG_Combinations_generated[ParamCount][99+1];
    memset( DEBUG_Combinations_generated, 0, sizeof( DEBUG_Combinations_generated ) );
    do
    {
        LargeNumber TempRes1, TempRes2;
        int GenNextCandidate = 0;
        StepsTaken++;
#ifdef _DEBUG
        for( int i = 0; i < ParamCount - 1; i++ )
            DEBUG_Combinations_generated[i][ ToIntLN(vLN[i]) % 100 ] = 1;
#endif
		int Match = CheckCandidateMatch_abSQ2(&N, vLN, ParamCount, AtPos, &TempRes1, &TempRes2);
        if( Match == 1 )
        {
            CandidatesFound++;
            if( CandidatesFound % 100 == 0 )
            {
                int chars[4]={'\\','|','/','-'};
                printf("\r%c", chars[ ( CandidatesFound / 100 ) % 4 ] );
            }
            int SolutionFound = CheckSolution2( TempRes1, TempRes2 ); 
            if( SolutionFound == 1 )
            {
                SolutionsFound++;
                printf("\r%d / %d )sol : \t a:", SolutionsFound, CandidatesFound );
                PrintLN( a );
                printf("\t b:");
                PrintLN( b );
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
//    }while( SolutionsFound == 0 && vLN[ParamCount-1]->Digits[0] == 0 );
    }while( vLN[ParamCount-1]->Digits[0] == 0 );

    if( SolutionsFound == 0 )
        printf( "\rNo Luck finding a solution\n");
    else
        printf( "\rDone testing all possible solutions\n");

    printf("Steps taken %d\n\n", StepsTaken );

#ifdef _DEBUG
/*
    for( int i = 0; i < ParamCount - 1; i++ )
        for( int j = 0; j < 100; j++ )
            if(  DEBUG_Combinations_generated[i][ j ] != 1 )
                printf("missing combination i, j => %d %d \n", i, j );
/**/
#endif
}

void DivTestabSQ2()
{
    // 121k
    DivTest_abSQ2( 349, 751 ); // N = 262099 SN = 511
    // 918k
    DivTest_abSQ2( 6871, 7673 ); // N = 52721183 , SN = 7260
    // 9M tries
    DivTest_abSQ2( 26729, 31793 ); // N = 849795097 , SN = 29151
}
