#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "StdAfx.h"

//these are functions borrowed from other project. For some, we do not need an implementation
Logger sLog;

void Logger::Log(int Level, const char* File, int Line, const char* msg, ...) {}
void SetProgramTerminated() {}
void DecreaseThreadsRunningCount() {}
void IncreaseThreadsRunningCount() {}
int IsWaitingForUserExitProgram() { return 1; }
char CurrentPath[MAX_PATH];
char* GetFullPath()
{
	//get the path where the exe is
	int bytes = GetModuleFileName(NULL, CurrentPath, sizeof(CurrentPath));
	//eat up until first slash
	for (int i = bytes - 1; i > 0; i--)
		if (CurrentPath[i] == '\\')
		{
			CurrentPath[i + 1] = 0;
			break;
		}
}

int CommPort = 5558;
unsigned short GetOurProxyPort()
{
	return CommPort;
}