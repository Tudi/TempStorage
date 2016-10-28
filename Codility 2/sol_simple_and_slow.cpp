#include "StdAfx.h"
#if 1
#define MAX_LENGTH          300007
#define MAX_LENGTH_SIZE     (sizeof(int) * MAX_LENGTH)
#define NON_ISLAND_VAL      (-10)

//Apply our flips on the input string
void ApplyRevertFlips_ref2( char *S, int *FlipLocations, int K, int N )
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

void MarkIslands_ref2( char *S, int *IsIsland )
{
    memset( IsIsland, NON_ISLAND_VAL, MAX_LENGTH_SIZE );

    int Start = 0;
    int End = 1;
    int LastIslandEnd = NON_ISLAND_VAL;
    int LastIslandStart = NON_ISLAND_VAL;
    while( S[ End ] != 0 )
    {
        //extend island
        if( S[ Start ] == '(' && S[ End ] == ')' && IsIsland[ Start ] != 1 && IsIsland[ End ] != 1 )
        {
            LastIslandEnd = End;
            LastIslandStart = Start;
            IsIsland[ Start ] = LastIslandEnd;
            IsIsland[ End ] = LastIslandStart;
            if( Start > 0 )
            {
                Start--;
                End++;
            }
            else
            {
                Start = End + 1;
                End += 2;
            }
        }
        //try to detect new islands at the right of what we parsed for now
        else
        {
            Start = End;
            End++;
        }

        //merge with previous island if we bumped into it
        if( Start > 0 && IsIsland[ Start ] >= 0 )
        {
            int PrevIslandStart = IsIsland[ Start ];
            IsIsland[ PrevIslandStart ] = LastIslandEnd;    // the prev island should point to the max possible length
            IsIsland[ LastIslandEnd ] = PrevIslandStart;    // cur island end should point to previous island start
            if( PrevIslandStart > 0 )
                Start = PrevIslandStart - 1;
            else
            {
                Start = End;                                // continue parsing the end
                End++;
            }
        }
    }

    //hmmm, is this even safe or needed ?
    S[ End + 1 ] = 0;
}

int IsIsland2[MAX_LENGTH];
int GetBestLenRightNow_ref2( char *S )
{
    int BestLen = 0;

    //mark islands
    MarkIslands_ref2( S, IsIsland2 );

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


int GenerateNextFlipCombination_ref2( int *FlipLocations, int K, int N )
{
    if( K == 0 )
        return 0;

    FlipLocations[ K - 1 ]++;
    int DidSomething;
    do{
        DidSomething = 0;
        for( int i = 1; i < K; i++ )
        {
            int Curi = K - i;
            int Previ = Curi - 1;
            if( FlipLocations[ Curi ] >= N )
            {
                FlipLocations[ Previ ]++;
                FlipLocations[ Curi ] = FlipLocations[ Previ ] + 1;
                DidSomething = 1;
            }
        }
    }while( DidSomething == 1 && FlipLocations[ 0 ] < N );
    return ( FlipLocations[ 0 ] < N ) ;
}

char S2[ MAX_LENGTH ];
char S3[ MAX_LENGTH ];
int FlipLocations[ MAX_LENGTH ];
int solution_ref2( char *S, int K )
{
    int BestLen = GetBestLenRightNow_ref2( S );

    int N = strlen( S );

    for( int UseFlips = K; UseFlips > 0; UseFlips-- )
    {
        for( int i = 0; i < UseFlips; i++ )
            FlipLocations[ i ] = i;
        FlipLocations[ UseFlips ] = 0;

        do{
            ApplyRevertFlips_ref2( S, FlipLocations, UseFlips, N );

            int CurLen = GetBestLenRightNow_ref2( S );
            if( CurLen > BestLen )
            {
                BestLen = CurLen;
                strcpy( S3, S2 );
            }

            ApplyRevertFlips_ref2( S, FlipLocations, UseFlips, N );

        }while( GenerateNextFlipCombination_ref2( FlipLocations, UseFlips, N ) );
    }

    return BestLen;
}
#endif