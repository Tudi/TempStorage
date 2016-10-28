#include "StdAfx.h"

#if 0

#define MAX_LENGTH 30000
int SeqStarts[MAX_LENGTH];
int SeqEnds[MAX_LENGTH];
int SeqCount = 0;
int IsIsland[MAX_LENGTH];
char St[MAX_LENGTH];
int Kt;

int ExtendIslandNoFlips( char *S, int *Start, int *End )
{
    if( S[*Start]!='(' || S[*End] != ')' )
        return 0;
    //see if the new island allowed us to extend ourself without flips
    while( *Start - 1 >= 0 && S[*Start - 1] == '(' && S[*End + 1] == ')' )
    {
        *Start = *Start - 1;
        *End = *End + 1;
    }
    return 1;
}

// possible alternative :
//  - check islands
//  - repeat
//      - merge islands
//      - expand islands
void DetectIslands( char *S, int *SeqStarts, int *SeqEnds, int *SeqCount )
{
    //detect islands Islands
    for( int i=0;S[i]!=0;)
    {
        int Start = i;
        int End = i + 1;
        
        if( ExtendIslandNoFlips( S, &Start, &End ) )
        {
            //merge with prev seq ?
            if( *SeqCount > 0 && SeqEnds[ *SeqCount - 1 ] == Start - 1 )
            {
                SeqEnds[ *SeqCount - 1 ] = End;
                //try to expand it
                ExtendIslandNoFlips( S, &SeqStarts[ *SeqCount - 1 ], &SeqEnds[ *SeqCount - 1 ] );
            }
            //add new seq
            else
            {
                SeqStarts[*SeqCount] = Start;
                SeqEnds[*SeqCount] = End;
                *SeqCount = *SeqCount + 1;
            }
            i = End + 1;
        }
        else 
            i = End;
    }
}

int IsPosInIsland( int Pos )
{
    if( IsIsland[ Pos ] != -1 )
        return IsIsland[ Pos ];
    return -1;
}

int SeqStartst[MAX_LENGTH];
int SeqEndst[MAX_LENGTH];
int GetTempBestLen( int FlipPosInString, int FlipsLeft )
{
    //if not worth calculating than simply wait for the end
    if( St[ FlipPosInString + 1 ] != 0 && FlipsLeft > 0 )
        return -1;

    int BestLen = -1;
    int SeqCountt = 0;
    DetectIslands( St, SeqStartst, SeqEndst, &SeqCountt );
    for( int i = 0; i < SeqCountt; i++ )
        if( SeqEndst[i] - SeqStartst[i] > BestLen )
            BestLen = SeqEndst[i] - SeqStartst[i] + 1;
    return BestLen;
}

// flip a specific position in the input string 
// no flips inside an island
int CheckFlipResult( int FlipPosInString, int FlipsMade, char FlipTo )
{
    //not a valid combo
    if( IsPosInIsland( FlipPosInString ) != -1 )
        return GetTempBestLen( FlipPosInString, Kt - FlipsMade );

    //not worth branching here
    if( St[ FlipPosInString ] == FlipTo ) 
        return GetTempBestLen( FlipPosInString, Kt - FlipsMade );

    //flip and remember how to flip it back
    char RevertTo = St[ FlipPosInString ];
    St[ FlipPosInString ] = FlipTo;

    //check a continuation from here, at the end we will measure best length island
    int BestLen = -1;

    //can we flip more ?
    if( FlipsMade < Kt )
    {
        for( int i = FlipPosInString + 1; St[i] != 0; i++ )
        {
            int ret = CheckFlipResult( i, FlipsMade + 1, '(' );
            if( ret > BestLen )
                BestLen = ret;
            ret = CheckFlipResult( i, FlipsMade + 1, ')' );
            if( ret > BestLen )
                BestLen = ret;
        }
    }

    if( strcmp( "()(())()()(", St ) == 0 )
        FlipsMade = FlipsMade;

    //did we reach the end in some way ?
    int ret = GetTempBestLen( FlipPosInString, Kt - FlipsMade );
    if( ret > BestLen )
        BestLen = ret;

    //restore
    St[ FlipPosInString ] = RevertTo;

    return BestLen;
}

int solution2( char *S, int K )
{
    printf( "K is %d\n", K );

    //not enough data
    if( S[0] == 0 || S[1] == 0 )
        return 0;

    int BestLen = 0;
    strcpy( St, S );
    Kt = K;
//    int N = strlen(S);

    // the only perfect way is to : 
    //  - detect islands
    //      - flip 1 in all possible combinations
    //          - detect islands extended and merged
    //              - flip next in all possible ramaining location ( bigger than prev )....
    //                  - detect extended / merged islands
    //                      - if no more flips check longest island

    DetectIslands( S, SeqStarts, SeqEnds, &SeqCount );

    //Create an island string
    memset( IsIsland, -1, sizeof( IsIsland ) );
    for( int i=0;i<SeqCount;i++ )
        for( int j = SeqStarts[i]; j <= SeqEnds[i]; j++ )
            IsIsland[ j ] = i;

    BestLen = 0;
    if( Kt > 0 )
    {
        for( int i = 0; St[i] != 0; i++ )
        {
            int t = CheckFlipResult( i, 1, ')' );
            if( t > BestLen )
                BestLen = t;
            t = CheckFlipResult( i, 1, ')' );
            if( t > BestLen )
                BestLen = t;
        }
    }

    return BestLen;
}

#endif