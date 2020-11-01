#pragma once


class Logger
{
public:
	void Log(int Level, const char* File, int Line, const char* msg, ...);
private:
};

extern Logger sLog;

void SetProgramTerminated();
void DecreaseThreadsRunningCount();
void IncreaseThreadsRunningCount();
int IsWaitingForUserExitProgram();

extern int CommPort;