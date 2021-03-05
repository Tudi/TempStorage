#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ctime>
#include "Logger.h"

SimpleLogger sLogger;

void Log(SimpleLogger *logger, LogSeverityFlags severity, const char *file, const char *function, int line, const char *logString)
{
	if (logger == NULL)
	{
		logger = &sLogger;
	}
	if (logger == NULL)
	{
		return;
	}
	if (logger->HasOutput() == false)
	{
		return;
	}

	UNREFERENCED_PARAMETER(file);

//	char LogLine[32000];
//	int WrittenLen = 0;
	std::string LogLine;
	time_t* rawtime = new time_t;
	struct tm* timeinfo = new tm;
	time(rawtime);
	errno_t er = localtime_s(timeinfo, rawtime);
	if (er == NO_ERROR)
	{
//		WrittenLen += sprintf_s(LogLine, sizeof(LogLine), "[%d-%d %d:%d:%03d]", timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, GetTickCount() % 1000);
		LogLine += fmt::format("[{}-{} {}:{}:{}]", timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, GetTickCount() % 1000);
		delete rawtime;
		delete timeinfo;
	}

//	WrittenLen += sprintf_s(&LogLine[WrittenLen], sizeof(LogLine) - WrittenLen, " %s:%d", function, line);
//	WrittenLen += sprintf_s(&LogLine[WrittenLen], sizeof(LogLine) - WrittenLen, " :: %s\n", logString);
	LogLine += fmt::format(" {}:{}", function, line);
	LogLine += fmt::format(" :: {}", logString);

	logger->Log(severity, LogLine.c_str());
}

void SimpleLogger::Log(LogSeverityFlags flags, const char* str)
{
	// Are we logging this type of message ?
	if ((mLogSeverityFilter & flags) == 0)
	{
		return;
	}
	// Have valid output ?
	if (mLogFile == NULL)
	{
		return;
	}
	// Sanity check
	if (str == NULL)
	{
		return;
	}

	fwrite(str, 1, strlen(str), mLogFile);

	if (mLogToConsole)
	{
		printf(str);
	}
}

SimpleLogger::SimpleLogger()
{
	Init("logs.txt");
}

void SimpleLogger::Init(const char *FilePath)
{
	// Close previous file if already open
	if (mLogFile != NULL)
	{
		fclose(mLogFile);
		mLogFile = NULL;
	}

	// Open a new file
	errno_t er = fopen_s(&mLogFile, FilePath, "wt");

	// Should already be NULL
	if (er != NOERROR)
	{
		mLogFile = NULL;
	}
}

SimpleLogger::~SimpleLogger()
{
	if(mLogFile != NULL)
	{
		fclose(mLogFile);
		mLogFile = NULL;
	}
}