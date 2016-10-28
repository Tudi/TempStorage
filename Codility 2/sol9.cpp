#include "StdAfx.h"
#if 0
#define MAX_LENGTH          300007
#define MAX_LENGTH_SIZE     (sizeof(int) * MAX_LENGTH)
#define NON_ISLAND_VAL      (-10)

int LongestIsland;
int IslandCount;
int IslandStarts[ MAX_LENGTH ];
int IsIsland[MAX_LENGTH];
int N;
int LongestIslandIndex;

void ConvertIslandFormat( char *S, int *IsIsland )
{
    int i = 0;
    for(; S[i] != 0; )
    {
        //skip parsing islands
        if( IsIsland[ i ] >= 0 )
        {
            IslandStarts[ IslandCount ] = i;
            IslandCount++;

            //monitor largest island
            int CurLen = IsIsland[ i ] - i + 1;
            if( CurLen > LongestIsland )
            {
                LongestIsland = CurLen;
                LongestIslandIndex = IslandCount - 1;
            }

            //so that we can later fast forward islands + non island zones
            i = IsIsland[ i ] + 1;
            continue;
        }
        i++;
    }
    N = i;
}

void MarkIslands( char *S, int *IsIsland )
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

int IsStartIsland( char *S, int Pos, int *IsIsland )
{
    if( IsIsland[ Pos ] >= 0 && ( Pos == 0 || IsIsland[ Pos - 1 ] < 0 ) )
        return 1;
    return 0;
}

int SkipIfFullIsland( char *S, int *Pos, int *IsIsland, int StepOutOfIsland )
{
    if( IsStartIsland( S, *Pos, IsIsland ) == 1 )
    {
        int IslandEnd = IsIsland[ *Pos ];
        *Pos = IslandEnd;
        if( StepOutOfIsland != 0 )
            *Pos += 1;
        return 1;
    }
    return 0;
}

void AdvanceToMaxLen( char *S, int K, int *StartEnd, int *IsIsland, int *NonIslandsSwallowed )
{
    int End = *StartEnd;
    int LeftCount = 0;
    int RightCount = 0;
    int j = End;
    *NonIslandsSwallowed = 0;

    for(; S[ j ] != 0; )
    {
        int Cur = j;
        int Next;

        //island skips are free
        if( SkipIfFullIsland( S, &Cur, IsIsland, 1 ) )
        {
            if( S[ Cur ] == 0 )
            {
                End = Cur - 1; //mark the end of the island
                break;
            }
            //first step is an island. Could be the largest yet..
            if( End == *StartEnd )
                End = Cur - 1;
        }

        Next = Cur + 1;
        //island skips are free
        SkipIfFullIsland( S, &Next, IsIsland, 1 );
        if( S[ Next ] == 0 )
        {
            //if we could not find a pair for "cur", than try to bite out of the last island
            //!!! there is a chance last island is a merge of multiple small islands. We really need the last island and not the merge island. Ex ()()
            if( S[ Cur + 1 ] != 0 )
                Next = Cur + 1;
            else
                break;
        }

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
        int RemainUnflipped = CountRemainUnflipped( RightCount, LeftCount, &FlipsLeft );

        //we can't continue anymore
        if( RemainUnflipped > 0 )
            break;

        //writeback only if find this a possible solution
        j = Next + 1;

        //memorize last good combo
        End = Next;
        *NonIslandsSwallowed = LeftCount + RightCount;
    }

    if( *StartEnd != End )
    {
        //maybe an island comes right after we finsihed searching
        int TEnd = End + 1;
        SkipIfFullIsland( S, &TEnd, IsIsland, 0 );
        if( TEnd > End + 1 )
            End = TEnd;

        *StartEnd = End;
    }
}

int SearchBestLenInterval( char *S, int K, int *IsIsland, int Starti, int Endi, int *BestLen )
{
    int IslandLengthSwallowed = -1;
    for( int i = Starti; i <= Endi; i++ )
    {
        int Start = i;
        int End = i;
        int NonIslandsSwallowed;

        AdvanceToMaxLen( S, K, &End, IsIsland, &NonIslandsSwallowed );

        if( End != Start )
        {
            int CurLen = End + 1 - Start;
            if( CurLen > *BestLen )
                *BestLen = CurLen;

            if( CurLen - NonIslandsSwallowed > IslandLengthSwallowed )
                IslandLengthSwallowed = CurLen - NonIslandsSwallowed;

            //no need to continue if we already have max len
            if( S[ End + 1 ] == 0 )
                break;
        }
    }
    return IslandLengthSwallowed;
}

struct LocationToInvestigate
{
    int FirstIslandIndex;
    int Starti,Endi;
    int SumIslandsTheoretic;
    int NonIslandsTheoretic;
    int SumIslands;
};

struct LocationToInvestigate Locations[ MAX_LENGTH ];
int LocationCount;

void GetLocationsToInvestigate(int K, int IslandLengthSumReq)
{
    LocationCount = 0;
    // from every position try to swallow as much as possible, islands get swallowed without cost
    for( int k = 0; k < IslandCount; k++ )
    {
        int IslandLengthSum = 0;
        int NeedFlipsLastGood = 0;
        int IslandLengthSumLastGood = 0;
//        if( k != LongestIslandIndex ) //always try to extend longest island
        {
            //we need sum of islands to be similar to max island
            int IslandEnd = 0;
            int TotalLenght = 0;
            for( int j = k; j < IslandCount; j++ )
            {
                IslandEnd = IsIsland[ IslandStarts[ j ] ];
                int IslandLen = IslandEnd - IslandStarts[ j ] + 1;
                IslandLengthSum += IslandLen;

                TotalLenght = IslandEnd - IslandStarts[ k ] + 1; 
                int NeedFlipsLen = TotalLenght - IslandLengthSum;
                if( NeedFlipsLen > 2 * K )
                    break;
                NeedFlipsLastGood = NeedFlipsLen;
                IslandLengthSumLastGood = IslandLengthSum;
            }
            if( IslandLengthSumLastGood < IslandLengthSumReq || NeedFlipsLastGood > 2 * K )
                continue;
            //if in best case we can only reach bestlen, no point trying
//            if( IslandLengthSumLastGood + 2 * K <= BestLen )
//                continue;
        }

        //try to seach before the island. Search for best combo
        int Starti = IslandStarts[ k ] - 2 * K; 
        //sanity check
        if( Starti < 0 )
            Starti = 0;
        //no need to eat the previous island, we already tried that combination
        if( k > 0 )
        {
            int PrevIslandStart = IslandStarts[ k - 1 ];
            if( PrevIslandStart > Starti )
                Starti = PrevIslandStart;
        }
        //search as if we are eating up this island
        int Endi = IslandStarts[ k ];

        Locations[ LocationCount ].FirstIslandIndex = k;
        Locations[ LocationCount ].Starti = Starti;
        Locations[ LocationCount ].Endi = Endi;
        Locations[ LocationCount ].SumIslandsTheoretic = IslandLengthSumLastGood;
        Locations[ LocationCount ].NonIslandsTheoretic = NeedFlipsLastGood;
        Locations[ LocationCount ].SumIslands = -1;
        LocationCount++;
    }
}

int GetNextBestLocationCandidate()
{
    int CurBestSum = 0;
    int BestIndex = -1;

    //get the best possible case we did not try yet
    for( int i = 0; i < LocationCount; i++ )
        if( Locations[ i ].SumIslands == -1 && CurBestSum <= Locations[ i ].SumIslandsTheoretic )
        {
            CurBestSum = Locations[ i ].SumIslandsTheoretic;
            BestIndex = i;
        }

    return BestIndex;
}

int UnusedCases = 0;

int solution( char *S, int K )
{
    int BestLen = 0;
    IslandCount = 0;
    LongestIsland = 0;

    //no need to work on these
    MarkIslands( S, IsIsland );

    // island starts and ends converted into a list
    ConvertIslandFormat( S, IsIsland );
    // get largest island length
    BestLen = LongestIsland;

    if( IslandCount == 0 )
        SearchBestLenInterval( S, K, IsIsland,0, N, &BestLen );
    else
    {
        // we only care if ( 2 * K + IslandEnd( Y ) - IslandStart( X ) ) > LargestIsland
        // should investigate positions starting with ( IslandStart - 2 * K )
        GetLocationsToInvestigate( K, LongestIsland );

        int LocationIndex = -1;
        int TheoreticalLimit = -1;
        do{
            LocationIndex = GetNextBestLocationCandidate();
            if( LocationIndex < 0 )
                break;
            TheoreticalLimit = Locations[ LocationIndex ].SumIslandsTheoretic + 2 * K;
            //check if merging islands is better than simply using K for extending
            if( TheoreticalLimit > BestLen )
                Locations[ LocationIndex ].SumIslands = SearchBestLenInterval( S, K, IsIsland, Locations[ LocationIndex ].Starti, Locations[ LocationIndex ].Endi, &BestLen );
        }while( TheoreticalLimit > BestLen );

        //statistics only
//        for( int i = 0; i < LocationCount; i++ )
//            if( Locations[ i ].SumIslands < 0 )
//                UnusedCases++;
    }

//printf(" Unused count : %d \n", UnusedCases );
    return BestLen;
}
#endif