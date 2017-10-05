#include "Logging.h"
#include <stdio.h>
#include <ctime>
#include <cstdarg>
#include <cstring>
#include "Encryption.h"
#include "SimpleList.h"
#include "License_API.h"
#include <mutex>

//forward declarations
void PushNotificationMsgToQueue(int ErrorCode, char *Msg);
void StartNotificationPushThread();

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
	Log(LL_DEBUG_INFO, 0, "\n\nLogging started");
	StartNotificationPushThread();
}

void ShutDownLogging()
{
	Log(LL_DEBUG_INFO, 0, "Logging stopped");
	if (LogFile)
	{
		fclose(LogFile);
		LogFile = 0;
	}
}

void Log(int level, int ErrorCode, char *Msg, ...)
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
		EncryptBufferXORKeyRotate((unsigned char*)MsgBuffer, (int)strlen(MsgBuffer), Key);
		fprintf(LogFile, "%02d%s:%s\n\r", level, bufTime, MsgBuffer);
	}
	//only log errors. Debug messages are only for testing
	if (level == LL_ERROR || level == LL_IMPORTANT)
		PushNotificationMsgToQueue(ErrorCode, MsgBuffer);
}

//Siemens SIPIPE components will subscribe to notifications. We are insterested if license expires while we are still using it
TSLinkedList <NotificationCallback> LogCallbacks;
int	NotificationRegister(NotificationCallback cb)
{
	return LogCallbacks.push_front_unique(cb);
}

// message queue. Queue is filled by license.dll usage. Queue is consumed by message push thread
struct NotificationStoreStruct
{
	int ErrorCode;
	char *Msg;
};
TSLinkedList <NotificationStoreStruct*> QueuedNotifications;
int MsgPushThreadIsRunning = 0;

void PushNotificationMsgToQueue(int ErrorCode, char *Msg)
{
	if (Msg == NULL)
		return;
	//wait until DLL properly loads up. Playing with threads while program did not yet start up may lead to deadlocks/crashes and bangs
	if (MsgPushThreadIsRunning == 0)
		return;
	//do not push messages until someone subscribes to receive them
	if (LogCallbacks.empty())
		return;
	//create a temp store for this msg and store it for later to be pushed to 
	int MsgSize = (int)strlen(Msg) + 1;
	NotificationStoreStruct *TStore = new NotificationStoreStruct;
	TStore->ErrorCode = ErrorCode;
	TStore->Msg = new char[MsgSize];
	memcpy(TStore->Msg, Msg, MsgSize);
	QueuedNotifications.push_front(TStore);
}

HANDLE  MsgPushThread = 0;
DWORD __stdcall NotificationPushThread(LPVOID lpParam)
{
	int *ThreadIsRunning = (int*)lpParam;
	*ThreadIsRunning = 1;
	const int OneLoopSleepAmt = 1000;
	while (*ThreadIsRunning == 1)
	{
		//do we have messages to push ?
		if (!QueuedNotifications.empty())
		{
			NotificationStoreStruct *Msg = QueuedNotifications.popFirst(); // pop a message to be pushed to all threads that want to get notified by us
			//iterate through callback handlers and allow them to use the data 
			for (NotificationCallback cb = LogCallbacks.begin(); cb != NULL; cb = LogCallbacks.GetNext())
				cb(Msg->ErrorCode, Msg->Msg);
			// we no longer use this message
			delete Msg;
		}
		else 
			Sleep(OneLoopSleepAmt);
	}

	// signal back that we finished running this thread
	*ThreadIsRunning = 0;

	//all went as expected
	return 0;
}

void StartNotificationPushThread()
{
	Log(LL_DEBUG_INFO, 0, "Started timer thread");

	DWORD   ThreadId;
	MsgPushThreadIsRunning = 0;
	MsgPushThread = CreateThread(
		NULL,						// default security attributes
		0,							// use default stack size  
		NotificationPushThread,		// thread function name
		&MsgPushThreadIsRunning,	// argument to thread function 
		0,							// use default creation flags 
		&ThreadId);					// returns the thread identifier 

	//this is bad
	if (MsgPushThread == 0)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_CREATE_THREAD, "Could not start message push thread");
		MsgPushThreadIsRunning = 0;
	}
}
