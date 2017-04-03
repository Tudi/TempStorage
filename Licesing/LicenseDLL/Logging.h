#pragma once

enum LogLevels
{
	LL_ERROR			= 1,
	LL_HACKING,
	LL_WARNING,
	LL_ALERT,
	LL_IMPORTANT,
	//use these to check for proper functionality of the project
	LL_DEBUG_RARE,
	LL_DEBUG_INFO,
	LL_DEBUG_FREQUENT,
	LL_DEBUG_DEBUG_ONLY,	// will not get written to release builds
	LL_DEBUG_ALL,			// keep it as last one
};

void InitLogging();
void ShutDownLogging();
void Log(int level = LL_DEBUG_ALL, char *Msg = 0, ...);
