#include <Windows.h>
#include <stdio.h>
#include <conio.h>

void main()
{
    HWND target = FindWindow( NULL, "receive" );
    if( target )
    {
        for( int i=0;i<2;i++)
        {
            int s1 = GetTickCount();
            LRESULT ret4 = SendMessage( target, WM_PAINT, 0, 0 );
            printf("SendMessage - %d \n", GetTickCount() - s1 );
            int e1 = GetTickCount();
            printf("it took %d to post message \n", e1 - s1 );
        }
    }
    printf("Press any key to exit :\n");
    char c = _getch();
}