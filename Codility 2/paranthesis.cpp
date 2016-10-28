#include "stdafx.h"

int K;
int GenerateString( char *S, int N )
{
    if( S[0] == 0 )
    {
        for( int i = 0; i < N; i++ )
            S[i] = '(';
        S[N] = 0;
        return 1;
    }
    //do a combination increment
    S[0]++;
    for( int i = 0; i < N; i++ )
        if( S[i] > '(' + 1 )
        {
            S[i] = '(';
            S[i+1] ++;
        }
    return 1;
}

void PrintString( char *S, int WithNewLine = 0 )
{
    int i;
    for(i = 0; i < S[i] != 0; i++ )
        printf( "%c", S[i] );
    printf( " - len : %d K : %d ", i, K );
    if( WithNewLine != 0 )
        printf( "\n" );
}

void PrepareTest( char *S, int *N, int *K, char *FromS, int fK )
{
    strcpy( S, FromS );
    *N = strlen( S );
    *K = fK;
}

int _tmain(int argc, _TCHAR* argv[])
{
    char S[MAX_LENGTH];
    int N;
    int ret;

    S[0] = 0;
    N = 2;
    K = 1;

#if 1
    PrepareTest( S, &N, &K, "()(())", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 ()(()) sol start 0 end 5 \n\n");
    /**/
    PrepareTest( S, &N, &K, "()(()(())", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 8 ()(())()) sol start 0 end 7 \n\n");
    /**/
    PrepareTest( S, &N, &K, "()(()()", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 ()(())) sol start 0 end 5 \n\n");
    /**/
    PrepareTest( S, &N, &K, "(()))()", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 (()()() sol start 1 end 6 \n\n");
    /**/
    PrepareTest( S, &N, &K, "())(()(()()(", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 10 (()(())()()( sol start 1 end 10 \n\n");
    /**/
    PrepareTest( S, &N, &K, "(()()()))))(()((()))", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 10 (()()())())(()((())) sol start 0 end 9 \n\n");
    /**/
    PrepareTest( S, &N, &K, "()()()", 0 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 ()()() sol start 0 end 5 \n\n");
    /**/
    PrepareTest( S, &N, &K, "()())()", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 ((())() sol start 1 end 6 \n\n");
    /**/
    PrepareTest( S, &N, &K, ")())()(()()(", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 8 )())()(()()) sol start 4 end 12 \n\n");
    /**/
    PrepareTest( S, &N, &K, "())()((()(", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 8 (()()()()( sol start 1 end 8 \n\n");
    /**/
    PrepareTest( S, &N, &K, ")()())()()()(()(", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 12 (()())()()()(()( sol start 0 end 11 \n\n");
    /**/
    PrepareTest( S, &N, &K, "())()(", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 4 (()()( sol start 1 end 5 \n\n");
    /**/
    PrepareTest( S, &N, &K, "())(()", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 ()()()\n\n");
    /**/
    PrepareTest( S, &N, &K, "())()())(()((", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 8 ()(()())(()(( sol start 0 end 7 \n\n");
    /**/
    PrepareTest( S, &N, &K, "()((()()(", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 8 ()()()()( sol start 0 end 7 \n\n");
    /**/
    PrepareTest( S, &N, &K, "(()()))(()(()())((()())", 0 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 8 (()()))(()(()())((()()) sol start 8 end 8 \n\n");
    /**/
    PrepareTest( S, &N, &K, ")))(((", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 4 )()()(\n\n");
    /**/
    PrepareTest( S, &N, &K, ")(()(", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 4 )(())\n\n");
    /**/
    PrepareTest( S, &N, &K, "))()))())(()()(()(((", 3 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    //one after island, one before island, another before island : ()()(()( -> ()()(())
    printf("Expected : 16 )(())(())(()())()((( len 20 sol start 1 end 17 \n\n"); // 1F : (()()(() -> 2F -> )())(()()(() -> 3F -> )()))())(()()(()
    /**/
    PrepareTest( S, &N, &K, "())(()", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 2 ())(()\n\n");
    /**/
    PrepareTest( S, &N, &K, ")))(((", 0 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 0 )))(((\n\n");
    /**/
    PrepareTest( S, &N, &K, "()", 0 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 2 ()\n\n");
    /**/
    PrepareTest( S, &N, &K, "()(()", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 4 ()())\n\n");
    /**/
    PrepareTest( S, &N, &K, ")(()", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 4 ()()\n\n");
    /**/
    PrepareTest( S, &N, &K, "()", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 2 ()\n\n");
    /**/
    PrepareTest( S, &N, &K, "()(()))()(((", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 10 ()(())(())(( len 12 sol start 0, end 9 \n\n");
    /**/
    PrepareTest( S, &N, &K, "())()(()((", 3 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 10 ()(())()()( len 11 sol 0, 9 \n\n");
    /**/
    PrepareTest( S, &N, &K, ")()()(", 3 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 6 (()())\n\n");
    /**/
    PrepareTest( S, &N, &K, ")())(", 1 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 4 (())(\n\n");
    /**/
    PrepareTest( S, &N, &K, ")()()(((()", 4 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 10 (()())()()\n\n");
    /**/
    PrepareTest( S, &N, &K, ")(", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 2 ()\n\n");
    /**/
    PrepareTest( S, &N, &K, "((((()()))", 2 );
    PrintString( S, 1 );
    ret = solution( S, K );
    printf("Longest possible : %d\n", ret );
    printf("Expected : 10 ()((()()))\n\n");
    /**/
#endif
    PrepareTest( S, &N, &K, "()(()()", 1 );
    PrintString( S, 1 );
    ret = solution_ref2( S, K );
    /**/

    //automated testing
    N = 12;
    //25 / 2
    for( int N = 1; N <= 25; N++ )
    {
        for( int i = 0; i < N + 1; i++ )
        {
            K = i;
            memset( S, 0, sizeof( S ) );
            printf( "N = %d, K = %d\n", N, K );
            while( GenerateString( S, N ) && S[N] == 0 )
            {
                PrintString( S, 1 );
                ret = solution( S, K );
//                int ret2 = solution_ref( S, K );
                int ret2 = ret;
                int ret3 = solution_ref2( S, K );
//                printf(" ret : %d ret2 : %d ret3 : %d\n", ret, ret2, ret3 );
                if( ret != ret2 || ret != ret3 )
                {
                    PrintString( S, 1 );
                    printf(" ret : %d ret2 : %d ret3 : %d\n", ret, ret2, ret3 );
                    printf("Baaad\n");
                }
            }
        }
    }
/**/
  _getch();
	return 0;
}

