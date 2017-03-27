#include "LordsMobileControl.h"
#include "HTTPSendData.h"

#pragma comment(lib,"ImageSearch/ImageSearchDLL_x64.lib")

extern void RunLordsMobileTestsNoOCR();

#include <windows.h>
#include <stdio.h>

int		KeepGameScanThreadsRunning = 1;
int		ThreadParamScanGameData = 0;
HANDLE	ScanGameProcessThreadHandle = 0;

DWORD WINAPI BackgroundProcessScanGame(LPVOID lpParam)
{
	while (KeepGameScanThreadsRunning == 1)
	{
		RunLordsMobileTestsNoOCR();	// we do not have a break mechanism for this atm
		remove("KingdomScanStatus.txt");
		HTTP_GenerateMaps();
	}
	KeepGameScanThreadsRunning = 0;
	return 0;
}

void	LordsMobileControlStartup()
{
	//1 processing thread is enough
	if (ScanGameProcessThreadHandle != 0)
		return;

	//create the processing thread 
	DWORD   PacketProcessThreadId;
	ScanGameProcessThreadHandle = CreateThread(
		NULL,							// default security attributes
		0,								// use default stack size  
		BackgroundProcessScanGame,		// thread function name
		&ThreadParamScanGameData,		// argument to thread function 
		0,								// use default creation flags 
		&PacketProcessThreadId);		// returns the thread identifier 

	printf("Done creating background thread to scan ingame\n");
}

void	LordsMobileControlShutdown()
{
	if (ScanGameProcessThreadHandle == 0)
		return;

	//signal that we want to break the processing loop
	KeepGameScanThreadsRunning = 2;
	//wait for the processing thread to finish
	while (KeepGameScanThreadsRunning != 0)
		Sleep(10);
	//close the thread properly
	CloseHandle(ScanGameProcessThreadHandle);
	ScanGameProcessThreadHandle = 0;
}