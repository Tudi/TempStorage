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
            Start--;
    }
}

int ConvertIslandFormat( char *S, int *IsIsland2, int *IslandStarts, int *IslandEnds, int *IslandCount )
{
    *IslandCount = 0;
    int BestLen = 0;

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
            IslandStarts[ *IslandCount ] = IslandStart;
            IslandEnds[ *IslandCount ] = i;
            *IslandCount = *IslandCount + 1;

            int CurLen = i - IslandStart + 1;
            if( CurLen > BestLen )
                BestLen = CurLen;
        }
    }

    return BestLen;
}

int GetNextValidFlipLoc( char *S, int Cur, int *IsIsland )
{
    // invalid imput ?
    if( Cur < 0 )
        return -1;
    // no more locations
    if( S[ Cur ] == 0 )
        return -1;
    //jump on next
    Cur++;
    //if we jumped on an island, keep searching for a new valid pos
    while( S[ Cur ] != 0 && IsIsland[ Cur ] >= 0 )
        Cur++;
    //no more valid pos
    if( S[ Cur ] == 0 )
        return -1;
    //found one
    return Cur;
}

int GetPrevValidFlipLoc( char *S, int Cur, int *IsIsland )
{
    // invalid imput ?
    if( Cur <= 0 )
        return -1;
    //jump on next
    Cur--;
    //if we jumped on an island, keep searching for a new valid pos
    while( Cur >= 0 && IsIsland[ Cur ] >= 0 )
        Cur--;
    //no more valid pos
    if( Cur < 0 )
        return -1;
    //found one
    return Cur;
}

int TryMatchLocations( char *S, int Pos1, int Pos2, int *FlipsLeft )
{
    //sanity checks
    if( Pos1 < 0 || Pos2 < 0 || S[ Pos1 ] == 0 || S[ Pos2 ] == 0 )
        return 0;

    if( S[Pos1] == S[Pos2] )        // (( or ))
        *FlipsLeft -= 1;
    else if( *FlipsLeft >= 2 )      // )(
        *FlipsLeft -= 2;
    else
        return 0;                   // could be )( but we can only flip 1

    return 1;
}

int ExtendIsland( char *S, int *Start, int *End, int *FlipsLeft, int *IsIsland );

//what is the cost to reach the next island by flipping the next 2 symbols ?
int TryReachNextIsland( char *S, int *Start, int *End, int *IsIsland, int *FlipsLeft )
{
    int CurPos = GetNextValidFlipLoc( S, *End, IsIsland );
    int NextPos = GetNextValidFlipLoc( S, CurPos, IsIsland );
    //try to reach as many islands after us as possible
    while( *FlipsLeft > 0 && CurPos > 0 && NextPos > 0 )
    {
        if( TryMatchLocations( S, CurPos, NextPos, FlipsLeft ) == 0 )
            break;
        *End = NextPos;
        CurPos = GetNextValidFlipLoc( S, NextPos, IsIsland );
        NextPos = GetNextValidFlipLoc( S, CurPos, IsIsland );
    }
    //getting merged with the next island ?
    int ret = 0;
    while( S[ *End + 1 ] != 0 && IsIsland[ *End + 1 ] >= 0 )
    {
        *End += 1;
        ret = 1;
    }

    int BestLen = *End - *Start + 1;
    if( ret == 1 )
        BestLen = ExtendIsland( S, Start, End, FlipsLeft, IsIsland );
    else
        BestLen = *End - *Start + 1;

    return BestLen;
}

int TryReachAnyIsland( char *S, int *Start, int *End, int *IsIsland, int *FlipsLeft )
{
    int PrevStart = *Start;
    int PrevEnd = *End;
    int CurPos = GetPrevValidFlipLoc( S, *Start, IsIsland );
    int NextPos = GetNextValidFlipLoc( S, *End, IsIsland );
    //try to reach as many islands after us as possible
    while( FlipsLeft > 0 && CurPos >= 0 && NextPos > 0 )
    {
        if( TryMatchLocations( S, CurPos, NextPos, FlipsLeft ) == 0 )
            break;
        *Start = CurPos;
        *End = NextPos;
        CurPos = GetPrevValidFlipLoc( S, CurPos, IsIsland );
        NextPos = GetNextValidFlipLoc( S, NextPos, IsIsland );
    }
    //getting merged with the next island ?
    int ret = 0;
    while( S[ *End + 1 ] != 0 && IsIsland[ *End + 1 ] >= 0 )
    {
        *End += 1;
        ret = 1;
    }
    //getting merged with prev island ?
    while( *Start > 0 && IsIsland[ *Start - 1 ] >= 0 )
    {
        *Start -= 1;
        ret = 1;
    }

    int BestLen = *End - *Start + 1;
    if( ret == 1 )
        BestLen = ExtendIsland( S, Start, End, FlipsLeft, IsIsland );
    else
        BestLen = *End - *Start + 1;

    return BestLen;
}

int TryReachPrevIsland( char *S, int *Start, int *End, int *IsIsland, int *FlipsLeft )
{
    int CurPos = GetPrevValidFlipLoc( S, *Start, IsIsland );
    int NextPos = GetPrevValidFlipLoc( S, CurPos, IsIsland );
    //try to reach as many islands after us as possible
    while( *FlipsLeft > 0 && CurPos >= 0 && NextPos >= 0 )
    {
        if( TryMatchLocations( S, CurPos, NextPos, FlipsLeft ) == 0 )
            break;
        *Start = NextPos;
        CurPos = GetPrevValidFlipLoc( S, NextPos, IsIsland );
        NextPos = GetPrevValidFlipLoc( S, CurPos, IsIsland );
    }

    //getting merged with the next island ?
    int ret = 0;
    while( *Start > 0 && IsIsland[ *Start - 1 ] >= 0 )
    {
        *Start -= 1;
        ret = 1;
    }

    int BestLen = *End - *Start + 1;
    if( ret == 1 )
        BestLen = ExtendIsland( S, Start, End, FlipsLeft, IsIsland );
    else
        BestLen = *End - *Start + 1;

    return BestLen;
}

int ExtendIsland( char *S, int *Start, int *End, int *FlipsLeft, int *IsIsland )
{
    int BestLen;

    int Cost1 = *FlipsLeft;
    int Start1 = *Start;
    int End1 = *End;
    int BestLen1 = TryReachNextIsland( S, &Start1, &End1, IsIsland, &Cost1 );

    BestLen = BestLen1;
    *Start = Start1;
    *End = End1;
    *FlipsLeft = Cost1;

    int Cost2 = *FlipsLeft;
    int Start2 = *Start;
    int End2 = *End;
    int BestLen2 = TryReachAnyIsland( S, &Start2, &End2, IsIsland, &Cost2 );

    if( BestLen2 > BestLen || ( BestLen2 == BestLen && Cost2 > Cost1 ) )
    {
        BestLen = BestLen2;
        *Start = Start2;
        *End = End2;
        *FlipsLeft = Cost2;
    }

    int Cost3 = *FlipsLeft;
    int Start3 = *Start;
    int End3 = *End;
    int BestLen3 = TryReachPrevIsland( S, &Start3, &End3, IsIsland, &Cost3 );

    if( BestLen3 > BestLen || ( BestLen3 == BestLen && Cost3 > Cost2 ) )
    {
        BestLen = BestLen3;
        *Start = Start3;
        *End = End3;
        *FlipsLeft = Cost3;
    }

    return BestLen;
}

int solution( char *S, int K )
{
//    N = strlen( S );
    int BestLen;
    int IsIsland[MAX_LENGTH];

    //no need to work on these
    MarkIslands( S, IsIsland );

    //convert Islands format
    int IslandStarts[MAX_LENGTH/2];
    int IslandEnds[MAX_LENGTH/2];
    int IslandCount = 0;
    BestLen = ConvertIslandFormat( S, IsIsland, IslandStarts, IslandEnds, &IslandCount );

    // if bestlen is maxlen, or we have no flips, return bestlen
    if( K == 0 )
        return BestLen;

    // what islands should be merged in order to get the best len ?
    // B + space + C = max ?
    // A + space + B + space + C = max ?
    IslandStarts[ IslandCount ] = N * 10;
    for( int i = 0; i < IslandCount; i++ )
    {
        int Start = IslandStarts[i];
        int End = IslandEnds[i];
        int FlipsLeft = K;
        int CurLenT = ExtendIsland( S, &Start, &End, &FlipsLeft, IsIsland );

        //more flips possible ?
        if( FlipsLeft > 0 )
        {
            //could we merge 1 from the beggining and one from the end ? This should execute only once
            while( TryMatchLocations( S, Start - 1, End + 1, &FlipsLeft ) != 0 )
            {
                Start--;
                End++;
            }
            // could we merge only from the end ignoring a blocking island ? This should execute only once
            while( TryMatchLocations( S, End + 1 , End + 2, &FlipsLeft ) != 0 )
            {
                End++;
                End++;
            }
            //could we merge only from the beggining ? This should execute only once
            while( TryMatchLocations( S, Start - 1, Start - 2, &FlipsLeft ) != 0 )
            {
                Start--;
                Start--;
            }
        }

        //move start and end to the edge of the island they are sitting on in case we just merged
        if( IsIsland[ Start ] < 0 )
            while( Start > 0 && IsIsland[ Start - 1 ] >= 0 )
                Start--;
        if( IsIsland[ End ] < 0 )
            while( S[ End + 1 ] != 0 && IsIsland[ End + 1 ] >= 0 )
                End++;

        int CurLen = End - Start + 1;
        if( CurLen > BestLen )
            BestLen = CurLen;
    }
    //if there are no islands we should investigate best flip case
    if( IslandCount == 0 )
    {
        //count number of lefts and rigths
        int LeftCount = 0;
        int TotalCount;
        for( TotalCount = 0; S[TotalCount] != 0; TotalCount++ )
            if( S[TotalCount] == '(' )
                LeftCount++;
        int RightCount = TotalCount - LeftCount;
        int SameCount = ( LeftCount & ( ~1 ) ) + ( RightCount & ( ~1 ) );
        int ImpairLeftCount = LeftCount & 1;
        int ImpairRightCount = RightCount & 1;
        BestLen = 0;
        int FlipsLeft = K;
        if( SameCount >= FlipsLeft * 2 )
        {
            BestLen = FlipsLeft * 2;
            FlipsLeft = 0;
        }
        else
        {
            BestLen = SameCount;
            FlipsLeft -= SameCount / 2;
        }
        if( FlipsLeft >= 2 && ImpairLeftCount == ImpairRightCount && ImpairRightCount == 1 )
            BestLen += 2;
    }

    return BestLen;
}
#endif