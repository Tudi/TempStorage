#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include "License_Grace.h"
#include "License.h"
#include "src/Encryption.h"
#include <sys/stat.h>
#include "src\ComputerFingerprint.h"

char GracePeriodStore[GRACE_PERIOD_STORE_SIZE] = "RoundBrownFoxJumpedTheFence";

long GetFileSize(const char *filename)
{
	struct stat stat_buf;
	int rc = stat(filename, &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

int CODECGraceData(GraceStatusDLLResourceStore *StaticDLLResourceDataInFile, int Encode)
{
	//only encrypt a part of our structure
	int BytesToEncrypt = sizeof(GraceStatusDLLResourceStore) - (sizeof(StaticDLLResourceDataInFile->Header) + sizeof(StaticDLLResourceDataInFile->IsFileInitialized) + sizeof(StaticDLLResourceDataInFile->XORKey));
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
	//actual crypt
	int er = EncryptBufferXORKeyRotate((unsigned char*)&StaticDLLResourceDataInFile->XORKey + sizeof(StaticDLLResourceDataInFile->XORKey), BytesToEncrypt, StaticDLLResourceDataInFile->XORKey);
	//fall through errors
	return er;
}

int LoadCurrectGracePeriodDataDLL()
{
	return 0;
}

int LoadCurrectGracePeriodDataFile()
{
	return 0;
}

int LoadCurrectGracePeriodData(GraceStatusDLLResourceStore *StaticDLLResourceDataInFile, long *StaticDataStoredAt)
{
	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, "LicenseDLL.dll", "rb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	GraceStatusDLLResourceStore *StaticDLLResourceDataInMemory = (GraceStatusDLLResourceStore *)GracePeriodStore;

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
			if (StaticDLLResourceDataInFile!=NULL)
				memcpy(StaticDLLResourceDataInFile, &FileContentBuffered[i], sizeof(GraceStatusDLLResourceStore));
			*StaticDataStoredAt = i;
			break;
		}
	free(FileContentBuffered);
	fclose(f);

	//if data is encoded, decode it
	int erEncrypt = CODECGraceData(StaticDLLResourceDataInFile, 0);

	//pass through errors
	return erEncrypt;
}

int SaveCurrectGracePeriodDataDLL()
{
	return 0;
}

int SaveCurrectGracePeriodDataFile()
{
	return 0;
}

int SaveCurrectGracePeriodData(GraceStatusDLLResourceStore *StaticDLLResourceDataInFile, long StaticDataStoredAt)
{
	//sanity checks
	if (StaticDLLResourceDataInFile == NULL)
		return ERROR_INVALID_PARAMETER;
	long LocalStaticDataStoredAt = StaticDataStoredAt;
	if (LocalStaticDataStoredAt == 0)
		LoadCurrectGracePeriodData(NULL, &LocalStaticDataStoredAt);

	//actual store

	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, "LicenseDLL.dll", "rb+");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//jump to the location where the static variable is stored inside of the DLL
	int SeekSuccess = fseek(f, LocalStaticDataStoredAt, SEEK_SET);
	if (SeekSuccess != 0)
	{
		fclose(f);
		return ERROR_COULD_NOT_STORE_GRACE_PERIOD;
	}

	//encode data to avoid humanly readable mode
	CODECGraceData(StaticDLLResourceDataInFile, 1);

	//write back to our DLL
	size_t BytesWritten = fwrite(StaticDLLResourceDataInFile, 1, sizeof(GraceStatusDLLResourceStore), f);
	fclose(f);

	int erWrite = (BytesWritten != sizeof(GraceStatusDLLResourceStore));

	return erWrite;
}

int LicenseGracePeriod::GetStatus(int *IsTriggered, time_t *SecondsRemain)
{
	//sanity initialization in case of early unhandled exit. Treat bugs as expired license :(
	*IsTriggered = 1;
	*SecondsRemain = 0;

	GraceStatusDLLResourceStore StaticDLLResourceDataInFile;
	int LoadErr;
	long StaticDataStoredAt = 0;

	//load current status
	LoadErr = LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile, &StaticDataStoredAt);
	if (LoadErr != 0)
		return LoadErr;

	if (StaticDLLResourceDataInFile.IsFileInitialized != 1)
	{
		//this should never happen. At the point this API is available we already should have data initialized
		return ERROR_UNHANDLED_EXCEPTION;
	}

	if (StaticDLLResourceDataInFile.RemainingSeconds > 0)
		*SecondsRemain = StaticDLLResourceDataInFile.RemainingSeconds;

#ifdef ENABLE_ANTI_HACK_CHECKS
	//cross check values. If we have been using this license more than we max should be allowed than refuse to be available
	if (StaticDLLResourceDataInFile.LicenseShouldEnd != 0 && StaticDLLResourceDataInFile.LicenseFirstUsed + StaticDLLResourceDataInFile.LicenseSecondsUsed > StaticDLLResourceDataInFile.LicenseShouldEnd + PERIODIC_UPDATE_EXPECTED_YEARLY_DEVIATION_SECONDS + MAX_GRACE_PERIOD_SECONDS)
		*SecondsRemain = 0;
#endif

	return 0;
}

int LicenseGracePeriod::UpdateStatus(int ActionType, int Value)
{
	GraceStatusDLLResourceStore StaticDLLResourceDataInFile;
	int LoadErr;
	long StaticDataStoredAt = 0;

	//load current status
	LoadErr = LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile, &StaticDataStoredAt);

	if (LoadErr != 0)
		return LoadErr;

	if (StaticDataStoredAt == 0)
		return ERROR_COULD_NOT_STORE_GRACE_PERIOD;

	GraceStatusDLLResourceStore OldValues;
	memcpy(&OldValues, &StaticDLLResourceDataInFile, sizeof(OldValues));

	//first and oncetime initialization
	if (StaticDLLResourceDataInFile.IsFileInitialized != 1)
	{
		StaticDLLResourceDataInFile.XORKey = 0;
		StaticDLLResourceDataInFile.IsFileInitialized = 1;
		StaticDLLResourceDataInFile.LicenseFirstUsed = time(NULL);
		StaticDLLResourceDataInFile.LicenseSecondsUsed = 0;
		StaticDLLResourceDataInFile.TriggerCount = 0;
		Log(LL_IMPORTANT, "License first time use at %d\n", time(NULL));
	}

	//this happens when a valid!! license is used. Reset grace period status.
	if (ActionType == GP_RESET_GRACE_PERIOD)
	{
		//avoid extra large values
		if (Value > MAX_GRACE_PERIOD_SECONDS)
		{
			Log(LL_HACKING, "Grace period is getting set to %d seconds. Max allowed is %d\n", Value);
			Value = MAX_GRACE_PERIOD_SECONDS;
		}
		StaticDLLResourceDataInFile.FirstTriggered = 0;
		StaticDLLResourceDataInFile.GraceShouldEnd = 0;
		StaticDLLResourceDataInFile.GracePeriod = Value;
		StaticDLLResourceDataInFile.RemainingSeconds = 0;
		StaticDLLResourceDataInFile.TriggerCount++;	// it would be strange to trigger grace period multiple times
		Log(LL_IMPORTANT, "Grace period removed by valid license. Triggered %d times\n", StaticDLLResourceDataInFile.TriggerCount);
		//generate and save a valid fingerprint and store it. Maybe later we will need it on Hardware changes
		if (StaticDLLResourceDataInFile.FingerPrintSize == 0)
		{
			ComputerFingerprint CF;
			CF.GenerateFingerprint();
			char *Key;
			int er = CF.GetEncryptionKey(&Key, StaticDLLResourceDataInFile.FingerPrintSize);
			//store this fingerprint
			if (StaticDLLResourceDataInFile.FingerPrintSize < COMPUTER_FINGERPRINT_STORE_SIZE && er == 0)
				memcpy(StaticDLLResourceDataInFile.FingerPrint, Key, StaticDLLResourceDataInFile.FingerPrintSize);
			else
				StaticDLLResourceDataInFile.FingerPrintSize = 0;
		}
	}

	//update timers if this is a periodic tick
	if (ActionType == GP_CONSUME_REMAINING)
	{
		StaticDLLResourceDataInFile.LicenseSecondsUsed += Value;
		if (StaticDLLResourceDataInFile.FirstTriggered != 0)
			StaticDLLResourceDataInFile.RemainingSeconds -= Value;
	}

	//this should be triggered when a NON valid license is getting used. Expired / HW changes...
	if (ActionType == GP_TRIGGER_GRACE_PERIOD && StaticDLLResourceDataInFile.FirstTriggered == 0)
	{
		Log(LL_IMPORTANT, "Grace period started. Triggered %d times\n", StaticDLLResourceDataInFile.TriggerCount);
		StaticDLLResourceDataInFile.FirstTriggered = time(NULL);
		StaticDLLResourceDataInFile.GraceShouldEnd = StaticDLLResourceDataInFile.FirstTriggered + StaticDLLResourceDataInFile.GracePeriod;
		StaticDLLResourceDataInFile.RemainingSeconds = StaticDLLResourceDataInFile.GracePeriod;
	}

	if (ActionType == GP_SET_LICENSE_END)
		StaticDLLResourceDataInFile.LicenseShouldEnd = Value;

	//if nothing changed, there is no need to write it to file. This happens when we query for activation keys
	if (memcmp(&OldValues, &StaticDLLResourceDataInFile, sizeof(OldValues)) == 0)
		return 0;

	int er = SaveCurrectGracePeriodData(&StaticDLLResourceDataInFile, StaticDataStoredAt);

	return er;
}

int LicenseGracePeriod::GetSavedFingerprint(unsigned char *Store, int MaxStoreLen, int *RetSize)
{
	//sanity checks
	if (Store == NULL || RetSize == NULL || MaxStoreLen == 0)
		return ERROR_INVALID_PARAMETER_G;
	*RetSize = 0;

	GraceStatusDLLResourceStore StaticDLLResourceDataInFile;
	int LoadErr;
	long StaticDataStoredAt = 0;

	//load current status
	LoadErr = LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile, &StaticDataStoredAt);

	//make sure loaded data is ok
	if (LoadErr != 0)
		return LoadErr;

	if (StaticDataStoredAt == 0)
		return ERROR_COULD_NOT_LOAD_GRACE_DATA;

	if (StaticDLLResourceDataInFile.FingerPrintSize == 0 || StaticDLLResourceDataInFile.IsFileInitialized == 0)
		return ERROR_GRACE_DATA_NOT_INITIALIZED;

	if (StaticDLLResourceDataInFile.FingerPrintSize > MaxStoreLen)
		return ERROR_INPUT_BUFFER_TOO_SMALL;

	//return the fingerprint data
	memcpy(Store, StaticDLLResourceDataInFile.FingerPrint, StaticDLLResourceDataInFile.FingerPrintSize);
	*RetSize = StaticDLLResourceDataInFile.FingerPrintSize;

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
			LicenseGracePeriod::UpdateStatus(GP_CONSUME_REMAINING, LICENSE_DURATION_UPDATE_INTERVAL - RemainingTimeUntilNextUpdate);
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
	Log(LL_DEBUG_INFO,"Started timer thread");
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
		Log(LL_ERROR, "Could not start timer thread");
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
	Log(LL_DEBUG_INFO,"Stopped timer thread");
}