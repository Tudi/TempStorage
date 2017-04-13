#include "Logging.h"
#include <stdio.h>
#include <ctime>
#include <cstdarg>
#include <cstring>
#include "src/Encryption.h"

int		LogLevel = 0;
FILE	*LogFile = 0;
int		EnableObfuscation = 1;
void InitLogging()
{
	LogLevel = 0;
	EnableObfuscation = 1;
#ifdef _DEBUG
	LogLevel = LL_DEBUG_ALL;
	EnableObfuscation = 0;
#endif
	errno_t er = fopen_s(&LogFile, "LDLL.log", "ab");
	if (er)
		LogFile = NULL;
	Log(LL_DEBUG_INFO,"\n\nLogging started");
}

void ShutDownLogging()
{
	Log(LL_DEBUG_INFO,"Logging stopped");
	if (LogFile)
	{
		fclose(LogFile);
		LogFile = 0;
	}
}

void Log(int level, char *Msg, ...)
{
	//sanity checks
	if (level > LogLevel)
		return;
#ifndef _DEBUG
	if (level == LL_DEBUG_DEBUG_ONLY)
		return;
#endif
	if (Msg == NULL)
		return;
	if (LogFile == NULL)
		return;

	//get the current time and date
	time_t rawtime;
	struct tm timeinfo;
	char bufTime[180];

	time(&rawtime);
	errno_t er = localtime_s(&timeinfo, &rawtime);
	if (er==0)
		strftime(bufTime, sizeof(bufTime), "%d-%m-%Y %I:%M:%S", &timeinfo);
	else
		bufTime[0]=0;

	//format arguments into a string than write it to a file
	char MsgBuffer[32000];
	va_list arguments;                    
	va_start(arguments, Msg);			  
	er = vsprintf_s(MsgBuffer, sizeof(MsgBuffer), Msg, arguments);
	va_end(arguments);                 

	//write it to a file
	if (EnableObfuscation == 0 )
		fprintf(LogFile, "%02d%s:%s\n\r", level, bufTime, MsgBuffer);
	else
	{
		int Key = bufTime[strlen(bufTime) - 1];
		Key = (Key << 24) | (Key << 16) | (Key << 8) | Key;
		EncryptBufferXORKeyRotate((unsigned char*)MsgBuffer, strlen(MsgBuffer), Key);
		fprintf(LogFile, "%02d%s:%s\n\r", level, bufTime, MsgBuffer);
	}
}