#include "StdAfx.h"

#if 0

#define MAX_LENGTH 30000
#define MAX_LENGTH_SIZE (sizeof(int) * MAX_LENGTH)
int N;

void MarkIslands( char *S, int *IsIsland )
{
    memset( IsIsland, -10, MAX_LENGTH_SIZE );

    int Start = 0;
    int End = 1;
    while( S[ End ] != 0 )
    {
        //extend island
        if( S[ Start ] == '(' && S[ End ] == ')' && IsIsland[ Start ] != 1 && IsIsland[ End ] != 1 )
        {
            IsIsland[ Start ] = 1;
//            IsIsland[ End ] = Start - 1;
            IsIsland[ End ] = 1;
            if( Start > 0 )
                Start--;
            End++;
        }
        else
        {
            Start = End;
            End++;
        }

        //merge with previous
        while( Start >= 0 && IsIsland[ Start ] >= 0 )
        {
//            Start = IsIsland[ Start ];
            Start--;
//            if( IsIsland[ End ] >= 0 )
//                IsIsland[ End ] = Start;
        }
    }
}

int GetNextFlipPos( int CurPos, int N, int *IsIsland )
{
    int i = CurPos + 1;
    //any next valid left ?
    for( ; i < N; i++ )
        if( IsIsland[ i ] < 0 )
            return i;
    //mark it as unusable
    if( i == N )
        return N;
    //remain unusable
    return CurPos;
}

void GenerateNextFlipCombination( int *FlipLocations, int K, int N, int *IsIsland )
{
    //search for last valid flipable location
    int LastValid = K - 1;
    while( LastValid >= 0 && FlipLocations[ LastValid ] >= N )
        LastValid--;

    //from LastValid to Last, reinit the locations
    if( LastValid >= 0 )
    {
        FlipLocations[ LastValid ] = GetNextFlipPos( FlipLocations[ LastValid ], N, IsIsland );
        for( int i = LastValid + 1; i < K; i++ )
        {
            FlipLocations[ i ] = GetNextFlipPos( FlipLocations[ i - 1 ], N, IsIsland );
            //no point continueing
            if( FlipLocations[ i ] >= N )
                break;
        }
    }
}

//Apply our flips on the input string
void ApplyRevertFlips( char *S, int *FlipLocations, int K, int N )
{
    for( int i = 0; i < K; i++ )
    {
        int Loc = FlipLocations[i];
        if( Loc < N )
        {
            if( S[ Loc ] == '(' )
                S[ Loc ] = ')';
            else
                S[ Loc ] = '(';
        }
    }
}

//based on our flips, what is the largest island now ?
int GetBestLenRightNow( char *S )
{
    int BestLen = 0;

    //mark islands
    int IsIsland2[MAX_LENGTH];
    MarkIslands( S, IsIsland2 );

    //measure largest island
    int IslandStart = -1;
    for( int i = 0; S[ i ] != 0; i++ )
    {
        if( IslandStart == -1 && IsIsland2[ i ] >= 0 )
            IslandStart = i;
        if( i > 0 && IsIsland2[ i - 1 ] < 0 && IsIsland2[ i ] >= 0 )
            IslandStart = i;
        if( IsIsland2[ i ] >= 0 && IsIsland2[ i + 1 ] < 0 )
        {
            int CurLen = i - IslandStart + 1;
            if( CurLen > BestLen )
                BestLen = CurLen;
        }
    }

    return BestLen;
}

int solution( char *S, int K )
{
    N = strlen( S );

    //no need to work on these
    int IsIsland[MAX_LENGTH];
    MarkIslands( S, IsIsland );

    //get best len without flips
    int BestLen = GetBestLenRightNow( S );

    //if bestlen is maxlen, or we have no flips, return bestlen
    if( BestLen == N || K == 0 )
        return BestLen;

    printf( "Predicted at least best possible len is %d\n", BestLen + K * 2 > N ? N : BestLen + K * 2 );

    //generate initial FlipLocations. These should not be placed on islands :P
    int FlipLocations[MAX_LENGTH];
    FlipLocations[0] = GetNextFlipPos( -1, N, IsIsland );
    for( int i = 1; i < K; i++ )
        FlipLocations[i] = GetNextFlipPos( FlipLocations[i - 1], N, IsIsland );

    int LoopCounter = 0;
    while( FlipLocations[0] < N )
    {
        //apply flips
        ApplyRevertFlips( S, FlipLocations, K, N );

        //get best len with flips
        int BestNow = GetBestLenRightNow( S );
        if( BestNow > BestLen )
            BestLen = BestNow;

        //best possible solution found. No Need to continue
        if( BestLen == N )
            return BestLen;

        //revert flips
        ApplyRevertFlips( S, FlipLocations, K, N );

         //make sure we are using a valid flip location string
         GenerateNextFlipCombination( FlipLocations, K, N, IsIsland );

        //just for the statistics
        LoopCounter++;
    }

    return BestLen;
}

#endif