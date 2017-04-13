#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include "License_Grace.h"
#include "License.h"
#include "src/Encryption.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <io.h>
#include "src\ComputerFingerprint.h"

char GracePeriodStore[GRACE_PERIOD_STORE_SIZE] = "RoundBrownFoxJumpedTheFence";

long GetFileSize(const char *filename)
{
	struct stat stat_buf;
	int rc = stat(filename, &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

int DirExists(const char *Path)
{
	if (Path == NULL)
		return 0;
	struct stat info;
	if (stat(Path, &info) != 0)
		return 0;
	else if (info.st_mode & S_IFDIR)
	{
		//check if we have access to it
		if (_access(Path, 6) == 0)
			return 1;
	}
	return 0;
}

void RemoveEndSlashes(char *Path)
{
	if (Path == NULL)
		return;
	while (Path[0] != 0)
	{
		int Len = strlen(Path);
		if (Len > 0 && Path[Len - 1] == '\\')
			Path[Len - 1] = 0;
		else
			break;
	} 
}

int HaveStringInList(const char *s, char **List, int count)
{
	for (int i = 0; i < count; i++)
		if (strcmp(s, List[i]) == 0)
			return 1;
	return 0;
}

int GetGraceSerializationPaths(int &OutputCount, char **List, int MaxCount)
{
	OutputCount = 0;
	if (*List == NULL)
		return 1;
	//add local directory
	{
		char *LocalPath = new char[2];
		strcpy_s(LocalPath, 2, ".");
		if (DirExists(LocalPath) && OutputCount<MaxCount)
			List[OutputCount++] = LocalPath;
		else
			delete LocalPath;
	}

	//add temp directory 1
	{
		char *TempPath = new char[500];
		DWORD ret = GetTempPath(500, TempPath);
		if (ret < MAX_PATH && ret != 0)
		{
			RemoveEndSlashes(TempPath);
			if (HaveStringInList(TempPath, List, OutputCount) == 0 && DirExists(TempPath) && OutputCount<MaxCount)
				List[OutputCount++] = TempPath;
			else
				delete[] TempPath;
		}
		else
			delete[] TempPath;
	}

	//add temp directory 2
	{
		char *env_p;
		size_t len;
		errno_t err = _dupenv_s(&env_p, &len, "TMP");
		if (err == 0 && len > 0)
		{
			RemoveEndSlashes(env_p);
			if (HaveStringInList(env_p, List, OutputCount) == 0 && DirExists(env_p) && OutputCount < MaxCount)
			{
				char *TempPath = new char[500];
				strcpy_s(TempPath, 500, env_p);
				List[OutputCount++] = TempPath;
			}
			free(env_p);
		}
	}

	//add temp directory 3
	{
		char *env_p;
		size_t len;
		errno_t err = _dupenv_s(&env_p, &len, "TEMP");
		if (err == 0 && len > 0)
		{
			RemoveEndSlashes(env_p);
			if (HaveStringInList(env_p, List, OutputCount) == 0 && DirExists(env_p) && OutputCount < MaxCount)
			{
				char *TempPath = new char[500];
				strcpy_s(TempPath, 500, env_p);
				List[OutputCount++] = TempPath;
			}
			free(env_p);
		}
	}

	//add temp directory 4
	{
		char *env_p;
		size_t len;
		errno_t err = _dupenv_s(&env_p, &len, "USERPROFILE");
		if (err == 0 && len > 0)
		{
			RemoveEndSlashes(env_p);
			if (HaveStringInList(env_p, List, OutputCount) == 0 && DirExists(env_p) && OutputCount < MaxCount)
			{
				char *TempPath = new char[500];
				strcpy_s(TempPath, 500, env_p);
				List[OutputCount++] = TempPath;
			}
			free(env_p);
		}
	}

	//add temp directory 5
	{
		char *env_p;
		size_t len;
		errno_t err = _dupenv_s(&env_p, &len, "%WINDIR%\temp");
		if (err == 0 && len > 0)
		{
			RemoveEndSlashes(env_p);
			if (HaveStringInList(env_p, List, OutputCount) == 0 && DirExists(env_p) && OutputCount < MaxCount)
			{
				char *TempPath = new char[500];
				strcpy_s(TempPath, 500, env_p);
				List[OutputCount++] = TempPath;
			}
			free(env_p);
		}
	}
	return 0;
}

int CODECGraceData(GraceStatusResourceStore *StaticDLLResourceDataInFile, int Encode)
{
	//only encrypt a part of our structure
	int BytesToEncrypt = sizeof(GraceStatusResourceStore) - (sizeof(StaticDLLResourceDataInFile->IsFileInitialized) + sizeof(StaticDLLResourceDataInFile->XORKey));
	if (Encode == 0)
	{
		//first time initialization. Nothing to do now
		if (StaticDLLResourceDataInFile->IsFileInitialized != 1)
			return 0;
	}
	//generate new XOR key
	else
	{
		if (Encode == 1)
			StaticDLLResourceDataInFile->XORKey = GenerateSaltKey2;
		else
			StaticDLLResourceDataInFile->XORKey = GenerateSaltKey3(1,Encode % 8);
	}
	//actual crypt
	int er = EncryptBufferXORKeyRotate((unsigned char*)&StaticDLLResourceDataInFile->XORKey + sizeof(StaticDLLResourceDataInFile->XORKey), BytesToEncrypt, StaticDLLResourceDataInFile->XORKey);
	//fall through errors
	return er;
}
/*
int LoadCurrectGracePeriodDataDLL(GraceStatusResourceStore *StaticDLLResourceDataInFile, long *StaticDataStoredAt)
{
	//make sure we do not return uninitialized values
	memset(StaticDLLResourceDataInFile, 0, sizeof(GraceStatusResourceStore));

	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, "LicenseDLL.dll", "rb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	GraceStatusResourceStore *StaticDLLResourceDataInMemory = (GraceStatusResourceStore *)GracePeriodStore;

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
			if (StaticDLLResourceDataInFile != NULL)
				memcpy(StaticDLLResourceDataInFile, &FileContentBuffered[i], sizeof(GraceStatusResourceStore));
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
*/
int LoadCurrectGracePeriodDataFile(const char *FileName,GraceStatusResourceStore *StaticDLLResourceDataInFile)
{
	//make sure we do not return uninitialized values
	memset(StaticDLLResourceDataInFile, 0, sizeof(GraceStatusResourceStore));
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "rb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	size_t ReadCount = fread_s(StaticDLLResourceDataInFile, sizeof(GraceStatusResourceStore), 1, sizeof(GraceStatusResourceStore), f);
	fclose(f);
	if (ReadCount != sizeof(GraceStatusResourceStore))
		return ERROR_COULD_NOT_LOAD_GRACE_DATA;

	//if data is encoded, decode it
	int erEncrypt = CODECGraceData(StaticDLLResourceDataInFile, 0);

	//pass through errors
	return erEncrypt;
}

//load grace period from 2 diffrent sources encoded with 2 different encryption keys. Use the most up to date version
int LoadCurrectGracePeriodData(GraceStatusResourceStore *StaticDLLResourceDataInFile)
{
//	GraceStatusResourceStore Store1;
//	int erLoad1 = LoadCurrectGracePeriodDataDLL(&Store1, StaticDataStoredAt);

	StaticDLLResourceDataInFile->LicenseSecondsUsed = 0;
	GraceStatusResourceStore CurStore;

	//generate a list of paths we will save our content to
	int PathCount;
	char *PathList[20];
	GetGraceSerializationPaths(PathCount, PathList, sizeof(PathList));
	int SuccesfulLoads = 0;
	for (int i = 0; i < PathCount; i++)
	{
		char FullPath[500];
		sprintf_s(FullPath, sizeof(FullPath), "%s\\grace.dat", PathList[i]);	// TODO : Add some randomization to filename
		int erLoad = LoadCurrectGracePeriodDataFile(FullPath, &CurStore);
		if (erLoad != 0)
			Log(LL_DEBUG_DEBUG_ONLY, "Could not save grace period status to sources!");
		else
		{
#ifdef ENABLE_ANTI_HACK_CHECKS
			CurStore.XORKey = StaticDLLResourceDataInFile->XORKey;//because this is different for each save
			if (SuccesfulLoads > 0 && CurStore.IsFileInitialized == StaticDLLResourceDataInFile->IsFileInitialized && memcmp(&CurStore, StaticDLLResourceDataInFile, sizeof(GraceStatusResourceStore)) != 0)
				Log(LL_ERROR, "Grace period state does not match when loaded from multiple places. If it persist, investigate the issue");
#endif
			if (CurStore.LicenseSecondsUsed > StaticDLLResourceDataInFile->LicenseSecondsUsed)
				memcpy(StaticDLLResourceDataInFile, &CurStore, sizeof(GraceStatusResourceStore));
			SuccesfulLoads++;
		}
		delete [] PathList[i];
	}
	//check if both loads where successfull
	if (SuccesfulLoads == 0)
	{
		Log(LL_ERROR, "Could not load grace period status from any sources!");
		return ERROR_COULD_NOT_LOAD_GRACE_DATA;
	}
	return 0;
}
/*
//should not call directly. Let it get called from generic save function
int SaveCurrectGracePeriodDataDLL(GraceStatusResourceStore *StaticDLLResourceDataInFile, long StaticDataStoredAt)
{
	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, "LicenseDLL.dll", "rb+");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//jump to the location where the static variable is stored inside of the DLL
	int SeekSuccess = fseek(f, StaticDataStoredAt, SEEK_SET);
	if (SeekSuccess != 0)
	{
		fclose(f);
		return ERROR_COULD_NOT_STORE_GRACE_PERIOD;
	}

	//encode data to avoid humanly readable mode
	CODECGraceData(StaticDLLResourceDataInFile, 1);

	//write back to our DLL
	size_t BytesWritten = fwrite(StaticDLLResourceDataInFile, 1, sizeof(GraceStatusResourceStore), f);
	fclose(f);

	int erWrite = (BytesWritten != sizeof(GraceStatusResourceStore));

	return erWrite;
}
*/
int SaveCurrectGracePeriodDataFile(const char *FileName, int UseCodecType, GraceStatusResourceStore *StaticDLLResourceDataInFile)
{
	//open self DLL
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "wb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//encode data to avoid humanly readable mode
	CODECGraceData(StaticDLLResourceDataInFile, UseCodecType);

	//write back to our DLL
	size_t BytesWritten = fwrite(StaticDLLResourceDataInFile, 1, sizeof(GraceStatusResourceStore), f);
	fclose(f);

	int erWrite = (BytesWritten != sizeof(GraceStatusResourceStore));

	return erWrite;
}

//save grace period in 2 place with 2 different encryption keys
int SaveCurrectGracePeriodData(GraceStatusResourceStore *StaticDLLResourceDataInFile)
{
	//sanity checks
	if (StaticDLLResourceDataInFile == NULL)
		return ERROR_INVALID_PARAMETER;

	//main save type. Sadly this randomly fails if DLL is write locked
//	int erSave1 = SaveCurrectGracePeriodDataDLL(StaticDLLResourceDataInFile, LocalStaticDataStoredAt);
//	if (erSave1 != 0)
//		Log(LL_DEBUG_DEBUG_ONLY, "Could not save grace period status to source1!");

	//generate a list of paths we will save our content to
	int PathCount;
	char *PathList[20];
	GetGraceSerializationPaths(PathCount, PathList, sizeof(PathList));
	int SuccesfulSaves = 0;
	for (int i = 0; i < PathCount; i++)
	{
		char FullPath[500];
		sprintf_s(FullPath, sizeof(FullPath), "%s\\grace.dat", PathList[i]);
		int erSave = SaveCurrectGracePeriodDataFile(FullPath, i, StaticDLLResourceDataInFile);
		if (erSave != 0)
			Log(LL_DEBUG_DEBUG_ONLY, "Could not save grace period status to sources!");
		else
			SuccesfulSaves++;
		delete [] PathList[i];
	}

	//no path were good to save the grace data
	if (SuccesfulSaves == 0)
		return ERROR_COULD_NOT_SAVE_GRACE_DATA;

	//we managed to save it at least once
	return 0;
}

int LicenseGracePeriod::GetStatus(int *IsTriggered, time_t *SecondsRemain)
{
	//sanity initialization in case of early unhandled exit. Treat bugs as expired license :(
	*IsTriggered = 1;
	*SecondsRemain = 0;

	GraceStatusResourceStore StaticDLLResourceDataInFile;
	int LoadErr;

	//load current status
	LoadErr = LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile);
	if (LoadErr != 0)
		return LoadErr;

	if (StaticDLLResourceDataInFile.IsFileInitialized != 1)
	{
		//this should never happen. At the point this API is available we already should have data initialized
		return ERROR_UNHANDLED_EXCEPTION;
	}

	if (StaticDLLResourceDataInFile.GraceRemainingSeconds > 0)
	{
		// double check values. Just in case our dynamic clock got shut down for some reason, check differential clock
		time_t SecondsRemainSystemClock = StaticDLLResourceDataInFile.GraceShouldEnd - time(NULL);
		if (SecondsRemainSystemClock < StaticDLLResourceDataInFile.GraceRemainingSeconds)
		{
			if (SecondsRemainSystemClock<0)
				*SecondsRemain = 0;
			else
				*SecondsRemain = SecondsRemainSystemClock;
		}
		else
			*SecondsRemain = StaticDLLResourceDataInFile.GraceRemainingSeconds;
	}
	else
		*IsTriggered = 0;

#ifdef ENABLE_ANTI_HACK_CHECKS
	//cross check values. If we have been using this license more than we max should be allowed than refuse to be available
	if (StaticDLLResourceDataInFile.LicenseShouldEnd != 0 && StaticDLLResourceDataInFile.LicenseFirstUsed + StaticDLLResourceDataInFile.LicenseSecondsUsed > StaticDLLResourceDataInFile.LicenseShouldEnd + PERIODIC_UPDATE_EXPECTED_YEARLY_DEVIATION_SECONDS + MAX_GRACE_PERIOD_SECONDS)
		*SecondsRemain = 0;
	int LicenseShouldEnd = atoi(StaticDLLResourceDataInFile.LicenseShouldEndc);
	if (LicenseShouldEnd != StaticDLLResourceDataInFile.LicenseShouldEnd)
		*SecondsRemain = 0;
#endif

	return 0;
}

int LicenseGracePeriod::UpdateStatus(int ActionType, int Value)
{
	GraceStatusResourceStore StaticDLLResourceDataInFile;

	//load current status. We can ignore load error. If grace data can not be loaded, you will not get grace period
	LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile);

	GraceStatusResourceStore OldValues;
	memcpy(&OldValues, &StaticDLLResourceDataInFile, sizeof(OldValues));

	//first and oncetime initialization
	if (StaticDLLResourceDataInFile.IsFileInitialized != 1)
	{
		memset(&StaticDLLResourceDataInFile, 0, sizeof(StaticDLLResourceDataInFile)); //generic set all to 0 just in case we forgot to set some manually
		StaticDLLResourceDataInFile.XORKey = 0;
		StaticDLLResourceDataInFile.IsFileInitialized = 1;
		StaticDLLResourceDataInFile.LicenseFirstUsed = time(NULL);
		StaticDLLResourceDataInFile.LicenseSecondsUsed = 0;
		StaticDLLResourceDataInFile.TriggerCount = 0;
		StaticDLLResourceDataInFile.APIsUsedFlags = 0;
		StaticDLLResourceDataInFile.LicenseShouldEnd = 0;
		memset(StaticDLLResourceDataInFile.LicenseShouldEndc, 0, sizeof(StaticDLLResourceDataInFile.LicenseShouldEndc));
		StaticDLLResourceDataInFile.LicensePeriodicLastUpdate = time(NULL);
		StaticDLLResourceDataInFile.FingerPrintSize = 0;
		StaticDLLResourceDataInFile.GraceTriggeredAt = 0;
		Log(LL_IMPORTANT, "License first time use at %d", time(NULL));
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
		if (StaticDLLResourceDataInFile.GraceTriggeredAt != 0)
		{
			StaticDLLResourceDataInFile.GraceTriggeredAt = 0;
			StaticDLLResourceDataInFile.GraceShouldEnd = 0;
			StaticDLLResourceDataInFile.GracePeriod = Value;
			StaticDLLResourceDataInFile.GraceRemainingSeconds = 0;
			StaticDLLResourceDataInFile.TriggerCount++;	// it would be strange to trigger grace period multiple times
			Log(LL_IMPORTANT, "Grace period removed by valid license. Triggered %d times\n", StaticDLLResourceDataInFile.TriggerCount);
		}
		//generate and save a valid fingerprint and store it. Maybe later we will need it on Hardware changes
		if (StaticDLLResourceDataInFile.FingerPrintSize == 0)
		{
			ComputerFingerprint CF;
			CF.GenerateFingerprint();
			char *Key;
			int er = CF.DupEncryptionKey(&Key, StaticDLLResourceDataInFile.FingerPrintSize);
			//store this fingerprint
			if (StaticDLLResourceDataInFile.FingerPrintSize < COMPUTER_FINGERPRINT_STORE_SIZE && er == 0)
				memcpy(StaticDLLResourceDataInFile.FingerPrint, Key, StaticDLLResourceDataInFile.FingerPrintSize);
			else
				StaticDLLResourceDataInFile.FingerPrintSize = 0;

			//strdup uses free
			if (Key != NULL)
				free(Key);
		}
	}

	//update timers if this is a periodic tick
	if (ActionType == GP_CONSUME_REMAINING)
	{
		int ConsumeAmount = 0;
		time_t TimeNow = time(NULL);
		int ExternalTimeDiffSec = Value / 1000;
		// this should never happen. It indicated a hack attempt by rewinding the computer clock
		// Note ! Daytime saving might trigger this once a year
		if (TimeNow < StaticDLLResourceDataInFile.LicensePeriodicLastUpdate)
		{
			int Diff = (int)(StaticDLLResourceDataInFile.LicensePeriodicLastUpdate - TimeNow);
			Log(LL_IMPORTANT, "Time rewind detected. Seconds difference %d. Last Update %d. Now %d\n", Diff, (int)StaticDLLResourceDataInFile.LicensePeriodicLastUpdate, (int)TimeNow);
			ConsumeAmount = ExternalTimeDiffSec;
		}
		//periodic update came faster than what we expected
		else if (TimeNow < StaticDLLResourceDataInFile.LicensePeriodicLastUpdate + ExternalTimeDiffSec)
		{
			ConsumeAmount = (int)(StaticDLLResourceDataInFile.LicensePeriodicLastUpdate + ExternalTimeDiffSec - TimeNow);
		}
		// maybe DLL got unloaded since last update. We trust system clock for time consume
		else
			ConsumeAmount = ExternalTimeDiffSec;
		// never assume, always know
		if (ConsumeAmount<=0)
			Log(LL_DEBUG_RARE, "Unexpected grace period state. Time passed %d. Val %d\n", ConsumeAmount, Value);
		StaticDLLResourceDataInFile.LicenseSecondsUsed += ConsumeAmount;
		if (StaticDLLResourceDataInFile.GraceTriggeredAt != 0)
			StaticDLLResourceDataInFile.GraceRemainingSeconds -= ConsumeAmount;
		StaticDLLResourceDataInFile.LicensePeriodicLastUpdate = time(NULL);
	}

	//this should be triggered when a NON valid license is getting used. Expired / HW changes...
	if (ActionType == GP_TRIGGER_GRACE_PERIOD && StaticDLLResourceDataInFile.GraceTriggeredAt == 0)
	{
		Log(LL_IMPORTANT, "Grace period started. Triggered %d times\n", StaticDLLResourceDataInFile.TriggerCount);
		StaticDLLResourceDataInFile.GraceTriggeredAt = time(NULL);
		StaticDLLResourceDataInFile.GraceShouldEnd = StaticDLLResourceDataInFile.GraceTriggeredAt + StaticDLLResourceDataInFile.GracePeriod;
		StaticDLLResourceDataInFile.GraceRemainingSeconds = StaticDLLResourceDataInFile.GracePeriod;
	}

	//backup when license ends
	if (ActionType == GP_SET_LICENSE_END)
	{
		if (StaticDLLResourceDataInFile.LicenseShouldEnd!=0)
			Log(LL_IMPORTANT, "License duration got extended. A new license should been loaded. End date %d\n", StaticDLLResourceDataInFile.LicenseShouldEnd);
		StaticDLLResourceDataInFile.LicenseShouldEnd = Value;
		_itoa_s(Value, StaticDLLResourceDataInFile.LicenseShouldEndc, sizeof(StaticDLLResourceDataInFile.LicenseShouldEndc), 10);
	}

	//this always gets added and never consumed. Client should never use some of the API calls. Siemens field engeneer can confirm by investigating flags
	if (ActionType == GP_LICENSE_LIFETIME_USE_FLAGS)
	{
		//report strange behavior
#ifdef ENABLE_ANTI_HACK_CHECKS
		if ((Value & LSF_SIEMENS_ONLY_FLAGS) != 0 && (StaticDLLResourceDataInFile.APIsUsedFlags & LSF_SIEMENS_ONLY_FLAGS) == 0 )
			Log(LL_HACKING, "Client used forbidden API calls %d", Value);
#endif
		StaticDLLResourceDataInFile.APIsUsedFlags |= Value;
	}

	//if nothing changed, there is no need to write it to file. This happens when we query for activation keys
	if (memcmp(&OldValues, &StaticDLLResourceDataInFile, sizeof(OldValues)) == 0)
		return 0;

	int er = SaveCurrectGracePeriodData(&StaticDLLResourceDataInFile);

	return er;
}

int LicenseGracePeriod::GetSavedFingerprint(unsigned char *Store, int MaxStoreLen, int *RetSize)
{
	//sanity checks
	if (Store == NULL || RetSize == NULL || MaxStoreLen == 0)
		return ERROR_INVALID_PARAMETER_G;
	*RetSize = 0;

	GraceStatusResourceStore StaticDLLResourceDataInFile;
	int LoadErr;

	//load current status
	LoadErr = LoadCurrectGracePeriodData(&StaticDLLResourceDataInFile);

	//make sure loaded data is ok
	if (LoadErr != 0)
		return LoadErr;

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
		RemainingTimeUntilNextUpdate -= OneLoopSleepAmt; //multiple small sleeps, in case we want to shut down the thread, to have time to react to the command
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