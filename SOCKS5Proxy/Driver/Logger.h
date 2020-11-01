#pragma once

/*********************************************
* Program messages can be logged / filtered
*********************************************/

enum LogLevelFlags
{
	LL_Info = 1,
	LL_Warning = 2,
	LL_ERROR = 4,
	LL_Connections = 8,
	LL_Traffic = 16,
};

enum LogOutputFlags
{
	LOF_Console = 1,
	LOF_File = 2,
};

class Logger
{
public:
	Logger();
	~Logger();
	void Log(int Level, const char *File, int Line, const char* msg, ...);
	void SetLogLevelFlags(int flags) { LevelFlags = flags; }
	void SetLogOutputFlags(int flags) { OutputFlags = flags; }
	void SetLogFile(const char* Path);
	int LogConnections() { return (OutputFlags != 0 && (LevelFlags & LL_Connections)); }
	int LogTraffic() { return (OutputFlags != 0 && (LevelFlags & LL_Traffic)); }
private:
	void InitOutputFile();
	int LevelFlags;
	int OutputFlags;
	FILE* outf;
};

extern Logger sLog;