#pragma once

/*
* Everythong is a placeholder for real logging module
* Constants, values, resources might differ in the real application
*/

#include <string>
#include <fmt/core.h>

enum LogSeverityFlags
{
	LogSeverityTrace = 1,
	LogSeverityInfo = 2,
	LogSeverityError = 4,
	LogSeverityAllFlags = 0xFFFFFF
};

class SimpleLogger;
// C like interface for the logger class
void Log(SimpleLogger *logger, LogSeverityFlags severity, const char *file, const char *function, int line, const char *logString);
using namespace std;

#define LOG_TRACE(loggerClass, logFormat, ...) { \
		std::string logString = fmt::format(logFormat, __VA_ARGS__); \
		Log(loggerClass, LogSeverityFlags::LogSeverityTrace, __FILE__, __FUNCTION__, __LINE__, logString.c_str()); \
	}
#define LOG_INFO(loggerClass, logFormat, ...) { \
		std::string logString = fmt::format(logFormat, __VA_ARGS__); \
		Log(loggerClass, LogSeverityFlags::LogSeverityInfo, __FILE__, __FUNCTION__, __LINE__, logString.c_str()); \
	}
#define LOG_ERROR(loggerClass, logFormat, ...) { \
		std::string logString = fmt::format(logFormat, __VA_ARGS__); \
		Log(loggerClass, LogSeverityFlags::LogSeverityError, __FILE__, __FUNCTION__, __LINE__, logString.c_str()); \
	}

// Logger class to hold a logging session details : output, flags ...
// Here is used as a global resource, but possible to be instantiated multiple times
class SimpleLogger
{
public:
	SimpleLogger();
	~SimpleLogger();
	void Init(const char* FilePath);
	bool HasOutput() { return mLogFile != NULL; }
	void Log(LogSeverityFlags flags, const char* str);
private:
	FILE* mLogFile = NULL;
	bool mRotateLogs = false;
	bool mClearLogs = false;
	bool mLogToConsole = true;
	int mLogSeverityFilter = LogSeverityFlags::LogSeverityAllFlags;
};

extern SimpleLogger sLogger;