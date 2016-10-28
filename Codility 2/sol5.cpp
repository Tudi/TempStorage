#include "StdAfx.h"
#include <assert.h>
#if 1
#define MAX_LENGTH          300007
#define MAX_LENGTH_SIZE     (sizeof(int) * MAX_LENGTH)
#define NON_ISLAND_VAL      (-10)

int IsIsland_ref[MAX_LENGTH];

void MarkIslands_ref( char *S, int *IsIsland_ref )
{
    memset( IsIsland_ref, NON_ISLAND_VAL, MAX_LENGTH_SIZE );

    int Start = 0;
    int End = 1;
    while( S[ End ] != 0 )
    {
        //extend island
        if( S[ Start ] == '(' && S[ End ] == ')' && IsIsland_ref[ Start ] != 1 && IsIsland_ref[ End ] != 1 )
        {
            IsIsland_ref[ Start ] = 1;
            IsIsland_ref[ End ] = 1;
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
        else
        {
            Start = End;
            End++;
        }

        //merge with previous
        while( Start >= 0 && IsIsland_ref[ Start ] >= 0 )
            Start--;

        if( Start < 0 )
        {
            Start = End;
            End += 1;
        }
    }

    //hmmm, is this even safe or needed ?
    S[ End + 1 ] = 0;
}

int CountRemainUnflipped_ref( int RightCount, int LeftCount, int *FlipsLeft )
{
    if( FlipsLeft == 0 )
        return RightCount + LeftCount;

    int SameCount = ( LeftCount & ( ~1 ) ) + ( RightCount & ( ~1 ) );
    int ImpairLeftCount = LeftCount & 1;
    int ImpairRightCount = RightCount & 1;
    int ConsumedCount = 0;
    if( SameCount >= *FlipsLeft * 2 )
    {
        ConsumedCount = *FlipsLeft * 2;
        *FlipsLeft = 0;
    }
    else
    {
        ConsumedCount = SameCount;
        *FlipsLeft -= SameCount / 2;
    }
    if( *FlipsLeft >= 2 && ImpairLeftCount == ImpairRightCount && ImpairRightCount == 1 )
    {
        ConsumedCount += 2;
        *FlipsLeft -= 2;
    }
    
    assert( RightCount + LeftCount - ConsumedCount >= 0 );

    return RightCount + LeftCount - ConsumedCount;
}

int IsStartIsland_ref( char *S, int Pos, int *IsIsland_ref )
{
    if( IsIsland_ref[ Pos ] >= 0 && ( Pos == 0 || IsIsland_ref[ Pos - 1 ] < 0 ) )
        return 1;
    return 0;
}

int SkipIfFullIsland_ref( char *S, int *Pos, int *IsIsland_ref, int StepOutOfIsland )
{
    if( IsStartIsland_ref( S, *Pos, IsIsland_ref ) == 1 )
    {
        while( IsIsland_ref[ *Pos + 1 ] >= 0 )
           *Pos += 1;
        if( StepOutOfIsland != 0 )
            *Pos += 1;
        return 1;
    }
    return 0;
}

void AdvanceToMaxLen_ref( char *S, int K, int *StartEnd, int *IsIsland_ref )
{
    int End = *StartEnd;
    int LeftCount = 0;
    int RightCount = 0;
    int j;

    //we are at the begginning of an island ? Swallow it all
    if( SkipIfFullIsland_ref( S, &End, IsIsland_ref, 0 ) ) //this will cover largest island possible case
        j = End + 1;
    else
        j = End;

    for(; S[ j ] != 0; )
    {
        int Cur = j;
        //island skips are free
        if( SkipIfFullIsland_ref( S, &Cur, IsIsland_ref, 1 ) && S[ Cur ] == 0 )
        {
            End = Cur - 1; //mark the end of the island
            break;
        }

        int Next = Cur + 1;
        //island skips are free
        SkipIfFullIsland_ref( S, &Next, IsIsland_ref, 1 );
        if( S[ Next ] == 0 )
        {
            //if we could not find a pair for "cur", than try to bite out of the last island
            if( S[ Cur + 1 ] != 0 )
                Next = Cur + 1;
            else
                break;
        }

        //should never trigger this
//        if( !( S[Cur] == '(' || S[Cur] == ')' ) || !( S[Next] == '(' || S[Next] == ')' ) )
//            break;

        //check what kinda symbols are we adding to our Len
        if( S[Cur] == '(' )
            LeftCount++;
        else 
            RightCount++;

        if( S[Next] == '(' )
            LeftCount++;
        else 
            RightCount++;

        //can we increase our max len ?
        int FlipsLeft = K;
        int RemainUnflipped = CountRemainUnflipped_ref( RightCount, LeftCount, &FlipsLeft );

        //we can't continue anymore
        if( RemainUnflipped > 0 )
            break;

        //writeback only if find this a possible solution
        j = Next + 1;

        //memorize last good combo
        End = Next;
    }

    if( *StartEnd != End )
    {
        //maybe an island comes right after we finsihed searching
        int TEnd = End + 1;
        SkipIfFullIsland_ref( S, &TEnd, IsIsland_ref, 0 );
        if( TEnd > End + 1 )
            End = TEnd;

        *StartEnd = End;
    }
}

int solution_ref( char *S, int K )
{
    //no need to work on these
    MarkIslands_ref( S, IsIsland_ref );

    // from every position try to swallow as much as possible, islands get swallowed without cost
    int BestLen = 0;
    for( int i = 0; S[ i ] != 0 && S[ i + 1 ] != 0 ; i++ )
    {
        int Start = i;
        int End = i;

        AdvanceToMaxLen_ref( S, K, &End, IsIsland_ref );

        if( End != Start )
        {
            int CurLen = End + 1 - Start;
            if( CurLen > BestLen )
                BestLen = CurLen;

            //no need to continue if we already have max len
            if( S[ End + 1 ] == 0 )
                break;
        }
    }

    return BestLen;
}
#endif