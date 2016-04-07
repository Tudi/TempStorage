#ifndef _STACKTRACE_H_
#define _STACKTRACE_H_

#include "DbgHelp.h"
#include <WinBase.h>
#pragma comment(lib, "Dbghelp.lib")

void printStack( char *Output, int MaxSize )
{
    Output[0]=0;
     unsigned int   i;
     void         * stack[ 100 ];
     unsigned short frames;
     SYMBOL_INFO  * symbol;
     HANDLE         process;

     process = GetCurrentProcess();
     SymInitialize( process, NULL, TRUE );
     frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
     symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
     symbol->MaxNameLen   = 255;
     symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

     for( i = 1; i < frames; i++ )
     {
         SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

         sprintf_s( Output, MaxSize, "%s%i: %s - 0x%0X\n", Output, frames - i - 1, symbol->Name, symbol->Address );
     }

     free( symbol );
}

#endif // _STACKTRACE_H_