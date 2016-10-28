#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

using namespace std;

#include "laura.cpp"

int IsRemoveable( vector<int> &pA, int &Start, int &Count, int Inc = 1 ) 
{
    int Sum = 0;
    int Count2 = 2 * Count;
    int AddedCount = 0;
    int End = Start;
    while( 1 )
    {
        Sum += pA[End];
        int IsOdd = pA[End] & 1;
        if( IsOdd )
            AddedCount++;
        else
            AddedCount += 2;
        if( AddedCount >= Count2 )
            break;
        End += Inc;
    }

    if( (Sum & 1 ) == 0 )
    {
        if( Inc > 0 )
            Count = End;
        else
        {
            int t = Start;
            Start = End;
            Count = t;
        }
        return 1;
    }
    return 0;
}

string GenResult( int Start, int End )
{
    char Result[50];
    sprintf( Result, "%d,%d", Start, End );
    string r = Result;
    return r;       
}

string solution(vector<int> &pA) 
{
    char Result[50];
    Result[0]=0;
    // take a slice out so that we end up with : O, EOE, EEOEE
    // cases : 
    // E -> 0,0 win         Even number ODD
    // O -> NO SOLUTION     One ODD, one or more Evens suround the ODD
    // EE -> 0,1 win        Even number ODD     E
    // EO -> 0,0 win                            
    // OE -> 1,1 win
    // OO -> 1,2 win        Even number ODD
    // EEE -> 0,2 win                           E
    // EEO -> 0,1 win                           EO
    // EOE -> NO SOLUTION   One ODD, one or more Evens suround the ODD
    // EOO -> 0,2 win                           EE
    // OEE -> 1,2 win                           OE
    // OEO -> 1,1 win                           OEO
    // OOE -> 0,2 win                           E
    // OOO -> 0,1 win                           EO
    // EOEO -> 0,3 win                          E
    // EOOOE -> 1,2 win                         EEOE
    // OOEOE -> 0,1 win                         EEOE
    // OEOOE -> 1,4 win                         OE
    // EOOEO -> 0,3 win                         EO

    //search backwards
    int LastIndex = pA.size() - 1;
    int EvenCountAfterLastOdd = 0;
    int EvenCountBeforeLastOdd = 0;
    int FirstOddIndex = -1;
    int NextFirstOddIndex = -1;
    int LastOddIndex = -1;
    int PrevLastOddIndex = -1;
    int OddCount = 0;
    for( int itr = LastIndex; itr >= 0; itr-- )
    {
        if( FirstOddIndex != -1 && NextFirstOddIndex == -1 && ( pA[LastIndex - itr] & 1 ) == 1 )
            NextFirstOddIndex = LastIndex - itr;
        if( FirstOddIndex == -1 && ( pA[LastIndex - itr] & 1 ) == 1 )
            FirstOddIndex = LastIndex - itr;
        int IsOdd = pA[itr] & 1;
        if( IsOdd == 1 )
        {
            if( LastOddIndex != -1 && PrevLastOddIndex == -1 )
                PrevLastOddIndex = itr;
            if( LastOddIndex == -1 )
                LastOddIndex = itr;
            OddCount++;
        }
        else //if( IsOdd == 0 )
        { 
            if( LastOddIndex == -1 )
                EvenCountAfterLastOdd++;
            else
                EvenCountBeforeLastOdd++;
        }
    }

    //The sum is Even. Take everything
    if( ( OddCount & 1 ) == 0 )
    {
        sprintf( Result, "0,%d", LastIndex );
        string r = Result;
        return r;       
    }

    //we cannot take the whole input as one slice. And enemy can have a move to force us to loose, than we have no solution
    //O EOE EEOEE 
    if( ( EvenCountBeforeLastOdd == EvenCountAfterLastOdd && EvenCountAfterLastOdd >= 0 && OddCount == 1 ) )
    {
        sprintf( Result, "NO SOLUTION" );
        string r = Result;
        return r;       
    }

    // EO make it remain O
    if( LastIndex == LastOddIndex )
        return GenResult( 0, LastOddIndex - 1 );

    int OddCountHalf = ( OddCount - 1 ) / 2;
    //try to pinch from the beginning [0..LastOddIndex-1] in a way to swallow other odds so that remaining would be EOE EOOOE
    if( LastOddIndex > LastIndex / 2 ) 
    {
        int EvensBeforeLastOdd = LastOddIndex - OddCountHalf;
        int EvensAfterLastOdd = LastIndex - LastOddIndex;
        int NeedToRemoveEvenCount = EvensBeforeLastOdd - EvensAfterLastOdd;
        int RemoveAllOddsRange = PrevLastOddIndex - FirstOddIndex;
        //easy case, remove all Odds in 1 go so at least one Even will remain before the Odd
        if( NeedToRemoveEvenCount > 0 && RemoveAllOddsRange < NeedToRemoveEvenCount + OddCountHalf )
        {
            int Start = PrevLastOddIndex + 1 - NeedToRemoveEvenCount - OddCountHalf;
            if( Start < 0 )
                Start = 0;
            int End = Start + NeedToRemoveEvenCount + OddCountHalf - 1;
            if( Start <= FirstOddIndex || End >= PrevLastOddIndex )
                return GenResult( Start, End );
        }
    }
    // try to remove all Odds after first Odd so that the remaining would be EOE EOOOE
    {
        int EvensBeforeFirstOdd = FirstOddIndex;
        int EvensAfterFirstOdd = LastIndex - FirstOddIndex - OddCountHalf;
        int NeedToRemoveEvenCount = EvensAfterFirstOdd - EvensBeforeFirstOdd;
        int RemoveAllOddsRange = NextFirstOddIndex - LastOddIndex;
        //easy case, remove all Odds in 1 go so at least one Even will remain before the Odd
        if( NeedToRemoveEvenCount > 0 && RemoveAllOddsRange <= NeedToRemoveEvenCount + OddCountHalf )
        {
            int Start = LastOddIndex - NeedToRemoveEvenCount - OddCountHalf;
            if( Start <= FirstOddIndex )
                Start = FirstOddIndex + 1;
            int End = Start + NeedToRemoveEvenCount + OddCountHalf - 1;
            if( End >= LastOddIndex )
                return GenResult( Start, End );
        }
    }

    //we should never get here if we coded it right
    if( Result[0] == 0 )
        sprintf( Result, "NO SOLUTION" );
    string r = Result;

    return r; 
}

int GetNumberDigitCount( int TestCase )
{
    int Count = 0;
    if( TestCase == 0 )
        return 1;
    while( TestCase > 0 )
    {
        Count++;
        TestCase = TestCase / 2;
    }
    return Count;
}

void GenerateTestCase(vector<int> &pA, int TestCase)
{
    pA.resize( GetNumberDigitCount( TestCase ) );
    int tTestCase = TestCase;
    //convert number to binary and add to the vector
    int Ind = 0;
    if( tTestCase == 0 )
        pA[Ind++];
    while( tTestCase > 0 )
    {
        int Digit = tTestCase & 1;
        pA[Ind++]= Digit + 1;
        tTestCase /= 2;
    }
}

int main()
{
    vector<int> tA;
    string ret;

    tA.resize(5);
    tA[0] = 2;
    tA[1] = 2;
    tA[2] = 2;
    tA[3] = 3;
    tA[4] = 2;
    ret = solution( tA );
    printf("0,1:%s\n", ret.c_str() );

    tA.resize(5);
    tA[0] = 4;
    tA[1] = 5;
    tA[2] = 3;
    tA[3] = 7;
    tA[4] = 2;
    ret = solution( tA );
    printf("1,2:%s\n", ret.c_str() );

    tA.resize(5);
    tA[0] = 1;
    tA[1] = 1;
    tA[2] = 2;
    tA[3] = 3;
    tA[4] = 2;
    ret = solution( tA );
    printf("0,1:%s\n", ret.c_str() );

    tA.resize(6);
    tA[0] = 1;
    tA[1] = 1;
    tA[2] = 2;
    tA[3] = 1;
    tA[4] = 2;
    tA[5] = 2;
    ret = solution( tA );
    printf("1,5:%s\n", ret.c_str() );

    tA.resize(7);
    tA[0] = 4;
    tA[1] = 2;
    tA[2] = 2;
    tA[3] = 2;
    tA[4] = 1;
    tA[5] = 6;
    tA[6] = 6;
    ret = solution( tA );
    printf("0,1:%s\n", ret.c_str() );

    tA.resize(7);
    tA[0] = 2;
    tA[1] = 1;
    tA[2] = 2;
    tA[3] = 1;
    tA[4] = 1;
    tA[5] = 2;
    tA[6] = 2;
    ret = solution( tA );
    printf("2,5:%s\n", ret.c_str() );

    tA.resize(6);
    tA[0] = 1;
    tA[1] = 2;
    tA[2] = 2;
    tA[3] = 1;
    tA[4] = 1;
    tA[5] = 2;
    ret = solution( tA );
    printf("1,5:%s\n", ret.c_str() );

    tA.resize(6);
    tA[0] = 2;
    tA[1] = 1;
    tA[2] = 1;
    tA[3] = 1;
    tA[4] = 2;
    tA[5] = 2;
    ret = solution( tA );
    printf("2,4:%s\n", ret.c_str() );

    tA.resize(6);
    tA[0] = 2;
    tA[1] = 3;
    tA[2] = 2;
    tA[3] = 2;
    tA[4] = 2;
    tA[5] = 2;
    ret = solution( tA );
    printf("2,4:%s\n", ret.c_str() );
    
    tA.resize(4);
    tA[0] = 1;
    tA[1] = 3;
    tA[2] = 5;
    tA[3] = 2;
    ret = solution( tA );
    printf("1,3:%s\n", ret.c_str() );
    
    tA.resize(3);
    tA[0] = 2;
    tA[1] = 1;
    tA[2] = 2;
    ret = solution( tA );
    printf("NO SOLUTION:%s\n", ret.c_str() );

    tA.resize(2);
    tA[0] = 4;
    tA[1] = 5;
    ret = solution( tA );
    printf("0,0:%s\n", ret.c_str() );

    tA.resize(2);
    tA[0] = 3;
    tA[1] = 5;
    ret = solution( tA );
    printf("0,1:%s\n", ret.c_str() );

    tA.resize(2);
    tA[0] = 4;
    tA[1] = 2;
    ret = solution( tA );
    printf("0,1:%s\n", ret.c_str() ); 
    
    tA.resize(4);
    tA[0] = 3;
    tA[1] = 3;
    tA[2] = 2;
    tA[3] = 3;
    ret = solution( tA );
    printf("0,2:%s\n", ret.c_str() );
    
    tA.resize(3);
    tA[0] = 3;
    tA[1] = 2;
    tA[2] = 3;
    ret = solution( tA );
    printf("0,2:%s\n", ret.c_str() );

    tA.resize(5);
    tA[0] = 3;
    tA[1] = 3;
    tA[2] = 3;
    tA[3] = 2;
    tA[4] = 3;
    ret = solution( tA );
    printf("0,4:%s\n", ret.c_str() );

    tA.resize(6);
    tA[0] = 2;
    tA[1] = 2;
    tA[2] = 1;
    tA[3] = 2;
    tA[4] = 2;
    tA[5] = 2;
    ret = solution( tA );
    printf("3,3:%s\n", ret.c_str() );

    tA.resize(3);
    tA[0] = 1;
    tA[1] = 2;
    tA[2] = 2;
    ret = solution( tA );
    printf("1,2:%s\n", ret.c_str() );
    /**/

    int TestCase = 0;
    for( int i=0;i<=4096;i++)
    {
        GenerateTestCase( tA, TestCase++ );
        ret = solution( tA );
        string ret2 = solution_Laura( tA );
        if( ret.compare( ret2 ) )
        {
            printf("Input: ");
            for( int j=0;j<tA.size();j++)
                printf("%d,",tA[j]);
            printf("\n");
            printf("Solution : %s \nLaura :    %s\n\n", ret.c_str(), ret2.c_str() );
        }
    }
    /**/
}
