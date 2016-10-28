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

void CountLeftsRights( char *S, int from, int to, int *Lefts, int *Rights )
{
    *Lefts = 0;
    *Rights = 0;
    for( int i = from + 1; i < to; i++ )
        if( S[ i ] == '(' )
            *Lefts += 1;
        else
            *Rights += 1;
}

int ExtendIsland( char *S, int *Start, int *End, int *FlipsRemain, int Type )
{
    int Execute = 0;
    //to the left
    if( Type == 0 )
    {
        if( S[ *End + 1 ] != 0 && S[ *End + 2 ] != 0 )
        {
            if( S[ *End + 1 ] == S[ *End + 2 ] && *FlipsRemain >= 1 )
            {
                *FlipsRemain -= 1;
                *End += 2;
                Execute = 1;
            }
            else if( S[ *End + 1 ] != S[ *End + 2 ] && *FlipsRemain >= 2 )
            {
                *FlipsRemain -= 2;
                *End += 2;
                Execute = 1;
            }
        }
    }
    //before and after
    if( Type == 1 )
    {
        if( *Start > 0 && S[ *End + 1 ] != 0 )
        {
            if( S[ *Start - 1 ] == S[ *End + 1 ] && *FlipsRemain >= 1 )
            {
                *Start -= 1;
                *End += 1;
                Execute = 1;
                *FlipsRemain -= 1;
            }
            else if( S[ *Start - 1 ] != S[ *End + 1 ] && *FlipsRemain >= 2 )
            {
                *Start -= 1;
                *End += 1;
                Execute = 1;
                *FlipsRemain -= 2;
            }
        }
    }
    //before
    if( Type == 2 )
    {
        if( *Start > 1 )
        {
            if( S[ *Start - 1 ] == S[ *Start - 2 ] && *FlipsRemain >= 1 )
            {
                *Start -= 2;
                Execute = 1;
                *FlipsRemain -= 1;
            }
            else if( S[ *Start - 1 ] != S[ *Start - 2 ] && *FlipsRemain >= 2 )
            {
                *Start -= 2;
                Execute = 1;
                *FlipsRemain -= 1;
            }
        }
    }
    //all
    if( Type == 4 )
        Execute = 1;

    int BestLen = *End + 1 - *Start;
    if( Execute )
    {
        int TS1, TS2, TS3, TE1, TE2, TE3, F1, F2, F3;
        TS1 = TS2 = TS3 = *Start;
        TE1 = TE2 = TE3 = *End;
        F1 = F2 = F3 = *FlipsRemain;
        int ret1 = ExtendIsland( S, &TS1, &TE1, &F1, 0 );
        int ret2 = ExtendIsland( S, &TS2, &TE2, &F2, 1 );
        int ret3 = ExtendIsland( S, &TS3, &TE3, &F3, 2 );
        if( ret1 > BestLen )
            BestLen = ret1;
        if( ret2 > BestLen )
            BestLen = ret2;
        if( ret3 > BestLen )
            BestLen = ret3;
    }

    return BestLen;
}

int CountRemainUnflipped( int RightCount, int LeftCount, int *FlipsLeft )
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
    return RightCount + LeftCount - ConsumedCount;
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

    int LeftsBeforeIsland[MAX_LENGTH/2];
    int RightsBeforeIsland[MAX_LENGTH/2];

    int LeftsAfterIsland[MAX_LENGTH/2];
    int RightsAfterIsland[MAX_LENGTH/2];

    if( IslandCount > 0 )
    {
        N = strlen( S );
        CountLeftsRights( S, -1, IslandStarts[ 0 ], &LeftsBeforeIsland[0], &RightsBeforeIsland[0] );
        for( int i = 1; i < IslandCount; i++ )
            CountLeftsRights( S, IslandEnds[ i - 1 ], IslandStarts[ i ], &LeftsBeforeIsland[i], &RightsBeforeIsland[i] );
        for( int i = 0; i < IslandCount - 1; i++ )
        {
            LeftsAfterIsland[i] = LeftsBeforeIsland[ i + 1 ];
            RightsAfterIsland[i] = RightsBeforeIsland[ i + 1 ];
        }
        LeftsAfterIsland[ IslandCount ] = N * 10;
        RightsAfterIsland[ IslandCount ] = N * 10;

        CountLeftsRights( S, IslandEnds[ IslandCount - 1 ], N, &LeftsAfterIsland[ IslandCount - 1 ], &RightsAfterIsland[ IslandCount - 1 ] );

        //what is the best zone len we could get by merging : bad + good + bad + ...
        for( int i = 0; i < IslandCount; i++ )
        {
            int Start = IslandStarts[i];
            int End = IslandEnds[i];

            //try to reach the next island if we can afford the cost
            //try to reach prev island if we can afford the cost
            int Rigths = 0;
            int Lefts = 0;
            int FlipsLeft = K;
            int RightsGood = 0;
            int LeftsGood = 0;

            int j;
            for( j = i; j < IslandCount; j++ )
            {
                int RightCount = Rigths + RightsAfterIsland[ j ];
                int LeftCount = Lefts + LeftsAfterIsland[ j ];

                int tFlips = K;
                int UnflippedRemained = CountRemainUnflipped( RightCount, LeftCount, &tFlips );

                //if we are left with one, try to combine it with next zone, maybe we can merge
                if( UnflippedRemained > 1 )
                    break;

                Rigths = RightCount;
                Lefts = LeftCount;

                //it might seem there is no way to merge 2 islands simply based on the gaps between. Ex : )())()
                int ThereIsHope = 0;
                if( UnflippedRemained == 1 )
                {
                    //if we can find a perfect match at the start or end than we can still merge these islands
//                    if( tFlips == 0 )
                    {
                        if( Rigths + 1 == Lefts && j + 1 < IslandCount && S[ IslandEnds[ j + 1 ] + 1 ] == ')' )
                            ThereIsHope = 1;
                        if( Rigths == Lefts + 1 && Start > 0 && S[ Start - 1 ] == '(' )
                            ThereIsHope = 1;
                    }
                    //if we can find the same symbol at the start or end than we can still merge these islands
                    if( tFlips >= 1 && ThereIsHope == 0 )
                    {
                        if( Start > 0 )
                        {
                            int ttFlips = K;
                            if( S[ Start - 1 ] == '(' )
                                UnflippedRemained = CountRemainUnflipped( RightCount, LeftCount + 1, &ttFlips );
                            else
                                UnflippedRemained = CountRemainUnflipped( RightCount + 1, LeftCount, &ttFlips );
                            if( UnflippedRemained == 0 )
                                ThereIsHope = 1;
                        }
                        else if( j + 1 < IslandCount && S[ IslandEnds[ j + 1 ] + 1 ] != 0 )
                        {
                            int ttFlips = K;
                            if( S[ IslandEnds[ j + 1 ] + 1 ] == '(' )
                                UnflippedRemained = CountRemainUnflipped( RightCount, LeftCount + 1, &ttFlips );
                            else
                                UnflippedRemained = CountRemainUnflipped( RightCount + 1, LeftCount, &ttFlips );
                            if( UnflippedRemained == 0 )
                                ThereIsHope = 1;
                        }
                    }
                }

                if( UnflippedRemained == 0 || ThereIsHope == 1 )
                {
                    if( j + 1 < IslandCount )
                        End = IslandEnds[j + 1];
//                    else
//                        End = N - 1;
                    FlipsLeft = tFlips;
                    RightsGood = Rigths;
                    LeftsGood = Lefts;
                }
            }

            //solution was this built on hope ?
            {
                //need matching symbol at the beggining or the end
                if( LeftsGood == RightsGood + 1 && S[ End + 1 ] == ')' )
                {
                    End++;
                    RightsGood++;
                }
                if( LeftsGood + 1 == RightsGood && Start > 0 && S[ Start - 1 ] == '(' )
                {
                    LeftsGood++;
                    Start--;
                }
            }
            if( LeftsGood + 1 == RightsGood || LeftsGood == RightsGood + 1 )
            {
                //need same symbol at the beggining or at the end
                if( FlipsLeft == 1 )
                {
                    int NeedSymbol;
                    if( LeftsGood + 1 == RightsGood )
                        NeedSymbol = ')';
                    else
                        NeedSymbol = '(';
                    if( S[ End + 1 ] == NeedSymbol )
                    {
                        End++;
                        FlipsLeft--;
                    }
                    if( Start > 0 && S[ Start - 1 ] == NeedSymbol )
                    {
                        Start--;
                        FlipsLeft--;
                    }
                }
                //need any symbol to have a pair to our uneven count of symbols
                if( FlipsLeft >= 2 )
                {
                    int Cost1 = 3;
                    int Cost2 = 3;
                    if( Start > 0 )
                    {
                        //matches
                        if( S[ Start - 1 ] == '(' && RightsGood > LeftsGood )
                            Cost1 = 1;
                        if( S[ Start - 1 ] == ')' && RightsGood > LeftsGood )
                            Cost1 = 2;
                    }
                    else if( S[ End + 1 ] != 0 )
                    {
                        if( S[ End + 1 ] == '(' && RightsGood > LeftsGood )
                            Cost2 = 2;
                        if( S[ End + 1 ] == ')' && RightsGood > LeftsGood )
                            Cost2 = 1;
                    }
                    if( Cost1 < Cost2 )
                    {
                        Start--;
                        FlipsLeft -= Cost1;
                    }
                    else if( Cost1 >= Cost2 && Cost2 != 3 )
                    {
                        End++;
                        FlipsLeft -= Cost2;
                    }
                }
            }

            //maybe after merging islands we could still extend to the left / right / both ways ? This is very CPU extensive
            int CurLen = ExtendIsland( S, &Start, &End, &FlipsLeft, 4 );
            if( CurLen > BestLen )
                BestLen = CurLen;
        }
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