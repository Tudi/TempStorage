#pragma once

/*********************************************
* Filters outptu messages based on program configuration
*********************************************/

enum LogLevelFlags
{
	LL_Info = 1,
	LL_Warning = 2,
	LL_ERROR = 4,
	LL_TimeInfo = 8,
	LL_Always = 16,
};

enum LogOutputFlags
{
	LOF_Console = 1,
	LOF_File = 2,
};

//a class that replaces simple printf to be able to filter writing or not the output
class Logger
{
public:
	Logger();
	~Logger();
	void Log(int Level, const char *File, int Line, const char* msg, ...);
	void SetLogLevelFlags(int flags) { LevelFlags = flags; }
	void SetLogOutputFlags(int flags) { OutputFlags = flags; }
	void SetLogFile(const char* Path);
private:
	void InitOutputFile();
	int LevelFlags;
	int OutputFlags;
	FILE* outf;
};

extern Logger sLog;