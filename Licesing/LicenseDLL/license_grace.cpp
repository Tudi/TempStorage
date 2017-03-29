#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include "License.h"
#include "src/Encryption.h"
#include <sys/stat.h>

#define GenerateSaltKey2		(GenerateSaltKey( GetTickCount(), time(NULL) ))

long GetFileSize(const char *filename)
{
	struct stat stat_buf;
	int rc = stat(filename, &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

int CODECGraceData(DLLResourceStoreGraceStatus *StaticDLLResourceDataInFile, int Encode)
{
	int BytesToEncrypt = sizeof(StaticDLLResourceDataInFile->LicenseFirstUsed) + sizeof(StaticDLLResourceDataInFile->FirstTriggered) + sizeof(StaticDLLResourceDataInFile->ShouldEnd) + sizeof(StaticDLLResourceDataInFile->RemainingSeconds);
	if (Encode == 0)
	{
		//first time initialization. Nothing to do now
		if (StaticDLLResourceDataInFile->IsFileInitialized != 1)
			return 0;
	}
	else
	{
		//generate new XOR key
		StaticDLLResourceDataInFile->XORKey = GenerateSaltKey2;
	}
	EncryptBufferXORKeyRotate((unsigned char*)&StaticDLLResourceDataInFile->LicenseFirstUsed, BytesToEncrypt, StaticDLLResourceDataInFile->XORKey);
	return 0;
}

int LoadCurrectGracePeriodData(DLLResourceStoreGraceStatus *StaticDLLResourceDataInFile, long *StaticDataStoredAt)
{
	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, "LicenseDLL.dll", "rb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	DLLResourceStoreGraceStatus *StaticDLLResourceDataInMemory = (DLLResourceStoreGraceStatus *)GracePeriodStore;

	//search for our resource store header
#define FileReadBlockSize (10 * 1024 * 1024)		//our dll file should be about 150k
	int FileSize = GetFileSize("LicenseDLL.dll");
	if (FileSize <= 0)
		FileSize = FileReadBlockSize;
	unsigned char *FileContentBuffered = (unsigned char *)malloc(FileSize);
	size_t ReadCount = fread_s(FileContentBuffered, FileSize, 1, FileSize, f);
	*StaticDataStoredAt = 0;
	for (size_t i = 0; i < ReadCount; i++)
		if (memcmp(&FileContentBuffered[i], StaticDLLResourceDataInMemory->Header, sizeof(StaticDLLResourceDataInMemory->Header)) == 0)
		{
			memcpy(StaticDLLResourceDataInFile, &FileContentBuffered[i], sizeof(DLLResourceStoreGraceStatus));
			*StaticDataStoredAt = i;
			break;
		}
	free(FileContentBuffered);
	fclose(f);

	//if data is encoded, decode it
	CODECGraceData(StaticDLLResourceDataInFile, 0);

	return 0;
}

LIBRARY_API int SetGracePeriod(int MaxGracePeriod, int SecondsConsumed)
{
	DLLResourceStoreGraceStatus StaticDLLResourceDataInFile;
	int LoadErr;
	long StaticDataStoredAt = 0;

	//load current status
	LoadErr = LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile, &StaticDataStoredAt);

	if (LoadErr != 0)
		return LoadErr;

	if (StaticDataStoredAt == 0)
		return ERROR_COULD_NOT_STORE_GRACE_PERIOD;

	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, "LicenseDLL.dll", "rb+");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	int SeekSuccess = fseek(f, StaticDataStoredAt, SEEK_SET);
	if (SeekSuccess != 0)
	{
		fclose(f);
		return ERROR_COULD_NOT_STORE_GRACE_PERIOD;
	}

	//first and oncetime initialization
	if (StaticDLLResourceDataInFile.IsFileInitialized != 1)
	{
		StaticDLLResourceDataInFile.XORKey = 0;
		StaticDLLResourceDataInFile.IsFileInitialized = 1;
		StaticDLLResourceDataInFile.LicenseFirstUsed = time(NULL);
		StaticDLLResourceDataInFile.LicenseSecondsUsed = 0;
	}

	//reset grace period. This should happen when a new license was added to the Siemens project, or hardware issue got fixed
	if (MaxGracePeriod == 0)
	{
		StaticDLLResourceDataInFile.FirstTriggered = 0;
		StaticDLLResourceDataInFile.ShouldEnd = 0;
		StaticDLLResourceDataInFile.GracePeriod = 0;
	}
	//watchdog timer periodically updating the grace status
	else if (MaxGracePeriod < 0)
	{
		StaticDLLResourceDataInFile.LicenseSecondsUsed += SecondsConsumed;
		if (StaticDLLResourceDataInFile.RemainingSeconds > SecondsConsumed)
			StaticDLLResourceDataInFile.RemainingSeconds -= SecondsConsumed;
		else
			StaticDLLResourceDataInFile.RemainingSeconds = 0;
	}
	//This is license check event. We realized that the license expired
	else
	{
		if (StaticDLLResourceDataInFile.GracePeriod == 0)
			StaticDLLResourceDataInFile.GracePeriod = MaxGracePeriod;
		//how did this happen ? It got triggered in the future ?
		if (StaticDLLResourceDataInFile.FirstTriggered > time(NULL))
			StaticDLLResourceDataInFile.FirstTriggered = time(NULL);
		StaticDLLResourceDataInFile.ShouldEnd = StaticDLLResourceDataInFile.FirstTriggered + MaxGracePeriod;
	}

	//encode data to avoid humanly readable mode
	CODECGraceData(&StaticDLLResourceDataInFile, 1);

	//write back to our DLL
	fwrite(&StaticDLLResourceDataInFile, 1, sizeof(StaticDLLResourceDataInFile), f);
	fclose(f);

	return 0;
}

int WatchdogThreadIsRunning = 0;
HANDLE  WatchdogThread = 0;
DWORD __stdcall LicenseGraceWatchdogThread(LPVOID lpParam)
{
	int *ThreadIsRunning = (int*)lpParam;
	const int OneLoopSleepAmt = 1000;
	int RemainingTimeUntilNextUpdate = LICENSE_DURATION_UPDATE_INTERVAL;
	while (*ThreadIsRunning == 1)
	{
		Sleep(OneLoopSleepAmt);
		RemainingTimeUntilNextUpdate -= OneLoopSleepAmt;
		if (RemainingTimeUntilNextUpdate <= 0)
		{
			SetGracePeriod(-1, LICENSE_DURATION_UPDATE_INTERVAL - RemainingTimeUntilNextUpdate);
			RemainingTimeUntilNextUpdate = LICENSE_DURATION_UPDATE_INTERVAL;
		}
	}

	// signal back that we finished running this thread
	*ThreadIsRunning = 0;

	//all went as expected
	return 0;
}

void StartLicenseGraceWatchdogThread()
{
	WatchdogThreadIsRunning = 1;
	
	DWORD   ThreadId;
	WatchdogThread = CreateThread(
		NULL,						// default security attributes
		0,							// use default stack size  
		LicenseGraceWatchdogThread,	// thread function name
		&WatchdogThreadIsRunning,	// argument to thread function 
		0,							// use default creation flags 
		&ThreadId);					// returns the thread identifier 

	//this is bad
	if (WatchdogThread == 0)
	{
		WatchdogThreadIsRunning = 0;
	}
}

void EndLicenseGraceWatchdogThread()
{
	if (WatchdogThreadIsRunning != 1 || WatchdogThread == 0)
		return;

	//initiate shutdown
	WatchdogThreadIsRunning = 2;

	//wait for shutdown - there is a chance exit thread is not caught by us for, if thread is terminated
	DWORD result = WaitForSingleObject(WatchdogThread, 1000);
	int AntiDeadlockCountdown = LICENSE_DURATION_UPDATE_INTERVAL * 2;
	while (WatchdogThreadIsRunning == 2 && AntiDeadlockCountdown > 0 && !(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT))
	{
		Sleep(500);
		AntiDeadlockCountdown -= 500; // should never trigger in theory
		result = WaitForSingleObject(WatchdogThread, 0);
	}
	WatchdogThread = 0;
}