#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include "FunctionCallHooks.h"

using namespace std;

void GetLastError2()
{
	LPTSTR errorText = NULL;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM| FORMAT_MESSAGE_ALLOCATE_BUFFER| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,    
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if (NULL != errorText)
	{
		printf("Last error : %s\n", errorText);
		LocalFree(errorText);
		errorText = NULL;
	}
}

/**/

void ThisIsACallTestFunc()
{
	printf("Check working directory for file containing call logs\n");
}

int main()
{
	StartLogFunctionEntrances();
	ThisIsACallTestFunc();
	_getch();
	return 0;
}