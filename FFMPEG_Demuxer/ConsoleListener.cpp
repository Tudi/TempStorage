#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "ConsoleListener.h"

int ExitAllThreads;
PlayerCommands PlayerCommand;
void HandleConsoleLine( char *Line )
{
	if( strncmp( Line, "e\0", strlen( "e\0" ) ) == 0 ) 
	{
		ExitAllThreads = 1;
		PlayerCommand = PC_ABORT_DECODING;
		printf("Console Listener : Shutdown initiated\n");
	}
	else if( strncmp( Line, "f\0", strlen( "f\0" ) ) == 0 ) 
	{
		PlayerCommand = PC_DECODE_1_PACKET;
	}
	else if( strncmp( Line, "p\0", strlen( "p\0" ) ) == 0 ) 
	{
		PlayerCommand = PC_CONTINUE_DECODING_PACKETS;
	}
	else if( strncmp( Line, " \0", strlen( " \0" ) ) == 0 ) 
	{
		PlayerCommand = PC_PAUSE_DECODING;
	}
}

void PrintConsoleOptions()
{
	printf("Press 'f' to process next packets from input file\n");
	printf("Press 'p' to process input file\n");
	printf("Press ' ' to pause processing input file\n");
	printf("Press 'e' to abort decoding rest of the file\n"); 
}

DWORD WINAPI LoopListenConsole_(LPVOID lpParam)
{
	char LineBuffer[ 16000 ];
	int WriteIndex = 0;
	ExitAllThreads = 0;
	PlayerCommand = PC_PAUSE_DECODING;
	PrintConsoleOptions();
	while( ExitAllThreads == 0 )
	{
		//this will not let us auto exit program until keypressed
		char c = _getch();
		if( c == '\r' || c == '\n' )
		{
			LineBuffer[ WriteIndex ] = '\0';
			if( WriteIndex > 1 )
			{
				printf("\n");
				HandleConsoleLine( LineBuffer );
			}
			WriteIndex = 0;
		}
		else if( WriteIndex < sizeof(LineBuffer) - 1)
		{
			LineBuffer[ WriteIndex ] = c;
			WriteIndex++;
			printf("%c",c);

			HandleConsoleLine(LineBuffer);
			WriteIndex = 0;
		}
	}
	return 0;
}

//run thread to monitor the console in the background
void LoopListenConsole()
{
	DWORD   dwThreadIdArray[1];
	HANDLE  hThreadArray[1]; 
	PlayerCommand = PC_PAUSE_DECODING;
	hThreadArray[0] = CreateThread( 
		NULL,                   // default security attributes
		0,                      // use default stack size  
		LoopListenConsole_,			// thread function name
		0,						// argument to thread function 
		0,                      // use default creation flags 
		&dwThreadIdArray[0]);   // returns the thread identifier 
	if (hThreadArray[0] == NULL)
	   ExitProcess(3);
}

void Sleep_(int MS)
{
	Sleep(MS);
}