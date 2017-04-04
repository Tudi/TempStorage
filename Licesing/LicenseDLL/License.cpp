#include "stdafx.h"
#include <time.h>
#include "License.h"
#include "src/DataPacker.h"
#include "ProjectFeatureKeys.h"
#include <stdio.h>
#include "src/Encryption.h"
#include <random>
#include <thread>
#include "License_Grace.h"

LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s",__FILE__,__LINE__,__FUNCTION__);
	License *lic = new License;
	if (!lic)
		return ERROR_UNIDENTIFIED_ERROR;

	int er = lic->LoadFromFile("License.dat");
	if (er != 0)
	{
		Log(LL_ERROR,"License.dat file missing. Could not load activation key.");
		delete lic;
		return ERROR_COULD_NOT_LOAD_LIC_FILE;
	}

	int GetKeyRes = lic->GetActivationKey(ProjectId, FeatureId, StoreResult, MaxResultSize);

	//cleanup
	delete lic;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return GetKeyRes;
}

LIBRARY_API int	IsLicenseInGracePeriod(int *Result)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	if (Result == NULL)
	{
		Log(LL_ERROR, "Invalid use of IsLicenseInGracePeriod API");
		return ERROR_INVALID_PARAMETER;
	}

	//unless everything goes well, we are not in grace period. If license expires than 
	*Result = 0;

	License *lic = new License;
	if (!lic)
	{
		Log(LL_ERROR, "Sever error.Could not create license object");
		return ERROR_UNIDENTIFIED_ERROR;
	}

	int er = lic->LoadFromFile("License.dat");
	if (er != 0)
	{
		Log(LL_ERROR, "License.dat file missing. Could not load activation key.");
		delete lic;
		return ERROR_COULD_NOT_LOAD_LIC_FILE;
	}

	time_t RemainingSec = 0;
	er = lic->GetRemainingSeconds(RemainingSec);
	if (er != 0 && er != ERROR_LICENSE_EXPIRED)
	{
		Log(LL_ERROR, "Could not get time info from license.dat");
		delete lic;
		return ERROR_UNIDENTIFIED_ERROR;
	}

	//license is still valid. It is not considered grace period if everything is fine
	if (RemainingSec > 0)
	{
		*Result = 0;
		delete lic;
		return 0;
	}

	if (RemainingSec <= 0)
	{
		int IsGraceTriggered;
		time_t GracePeriodRemaining;
		int er = LicenseGracePeriod::GetStatus(&IsGraceTriggered, &GracePeriodRemaining);
		if (er != 0)
		{
			Log(LL_ERROR, "Could not get grace info");
			delete lic;
			return ERROR_UNIDENTIFIED_ERROR;
		}

		//license is in a valid grace period
		if (IsGraceTriggered != 0 && GracePeriodRemaining > 0)
		{
			*Result = 1;
			delete lic;
			return 0;
		}
	}

	//cleanup
	delete lic;

	//license is expired and grace period also expired
	*Result = 0;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

// in case we will get time from WINCCOA, than you should only need to change this implementation
time_t GetUnixtime()
{
	return time(NULL);
}

License::License()
{
	NodeList = new GenericDataStore;
	StartStamp = GracePeriod = 0;//mark them uninitialized
	Duration = 0;
}

License::~License()
{
	if (NodeList != NULL)
	{
		delete NodeList;
		NodeList = NULL;
	}
}
/**
* \brief    Add to the content of the license
* \details  License will conver this newly added project-feature combination. The activation key is automatically loaded from config file
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	ProjectName - name that is unique for each Siemens software product / project
* \param	FeatureName - name that is unique for each feature within a project
* \return	Error code. 0 is no error
*/
int	License::AddProjectFeature(const char *ProjectName, const char *FeatureName)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	ProjectFeatureKeyDB DB;
	int ProjectId = DB.GetProjectNameID(ProjectName);
	int FeatureId = DB.GetFeatureNameID(FeatureName);
	int ret = AddProjectFeature(ProjectId, FeatureId);
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return ret;
}
/**
* \brief    Copy string to destination
* \details  Similar to original strcpy_s it's a safe string copy. The difference here is that we will fill the remaining buffer with random data to confuse the reader
* \author   Jozsa Istvan
* \date     2017-03-22
* \param	dst - destination buffer pointer
* \param	dstMaxSize - maximum number of bytes we can copy to the destination
* \param	src - string buffer to copy data from
* \return	Error code. 0 is no error
*/

int strcpy_s_ran(char *dst, int dstMaxSize, const char *src)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//sanity checks
	if (dst == NULL || dstMaxSize == 0 || src == NULL)
		return ERROR_BAD_ARGUMENTS;

	//barbaric string copy
	int i = 0;
	for (; i < dstMaxSize && src[i] != 0; i++)
		dst[i] = src[i];

	//maybe source string was too long ?
	if (i == dstMaxSize)
		dst[i - 1] = 0;
	//fill the rest of the buffer with junk
	else
	{
		for (i++; i < dstMaxSize; i++)
			dst[i] = (char)rand();
	}
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

/**
* \brief    Add to the content of the license
* \details  License will conver this newly added project-feature combination. The activation key is automatically loaded from config file
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	ProjectId - number that is unique for each Siemens software product / project
* \param	FeatureId - number that is unique for each feature within a project
* \return	Error code. 0 is no error
*/
int License::AddProjectFeature(int ProjectId, int FeatureId)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//check if this is a valid param
	ProjectFeatureKeyDB DB;
	if (DB.IsValidProjectID(ProjectId) == MISSING_OR_INVALID_ID)
	{
		Log(LL_ERROR, "Found invalid project id %d while constructing license node", ProjectId);
	}
	char *ActivationKey = DB.FindProjectFeatureKey(ProjectId, FeatureId);
	if (ActivationKey == NULL)
	{
		Log(LL_ERROR, "Could not find a valid activation key for project %d and feature ID %d. Exiting", ProjectId, FeatureId);
		return ERROR_INVALID_PROJECT_FEAURE;
	}
	//create a new storable node
	LicenseNode *NewNode = new LicenseNode;
	memset(NewNode, 0, sizeof(LicenseNode));
	NewNode->ProjectId = ProjectId;
	NewNode->FeatureId = FeatureId;
	errno_t er = strcpy_s_ran(NewNode->ActivateKey, sizeof(NewNode->ActivateKey), ActivationKey);
	//wow, we should never trigger this
	if (er)
	{
		Log(LL_ERROR, "Could not copy string");
		delete NewNode;
		return er;
	}

	//check if we already have this node. There is no point for dounble adding it
	DataCollectionIterator itr;
	itr.Init(NodeList);
	char *Data;
	int Size;
	int Type;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_LICENSE_NODE)
		{
			if (memcmp(Data, NewNode, Size) == 0)
			{
				Log(LL_DEBUG_INFO, "This node is already in the License list. There is no point adding it a seconds time");
				break;
			}
		}
	}

	//Add new node to our store
	NodeList->PushData((char*)NewNode, sizeof(LicenseNode), DB_LICENSE_NODE);

	delete NewNode;
	NewNode = NULL;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//no errors returned
	return 0;
}

/**
* \brief    Time limits for the license
* \details  License will be active from a specific date for a specific time
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	pStartDate - time counted in seconds since 1970. A unix timestamp. Used as a starting date for the license
* \param	pDuration - number of seconds the license will be active for counted from start date
* \return	Error code. 0 is no error
*/
int License::SetDuration(time_t pStartDate, unsigned int pDuration)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//sanity checks
	int LostSeconds = (signed int)(GetUnixtime() - pStartDate);
	if (LostSeconds > 60 * 60)
		Log(LL_DEBUG_INFO, "Start date is smaller than current date. Loosing %d seconds from durtation %d\n", LostSeconds, pDuration);
	if (LostSeconds > (signed int)pDuration)
	{
		Log(LL_DEBUG_INFO, "Duration is too small. Readjusting start time to current time to avoid negative duration");
		pStartDate = GetUnixtime();
	}

	StartStamp = pStartDate;
	Duration = pDuration;

	//check if we already have this node. There is no point for dounble adding it
	DataCollectionIterator itr;
	itr.Init(NodeList);
	char *Data;
	int Size;
	int Type;
	int FoundLicenseTimingInfo = 0;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_LICENSE_START)
		{
			FoundLicenseTimingInfo = 1;
			*(time_t*)Data = StartStamp;
		}
		if (Type == DB_LICENSE_DURATION)
		{
			FoundLicenseTimingInfo = 1;
			*(time_t*)Data = Duration;
		}
		if (Type == DB_LICENSE_END)
		{
			FoundLicenseTimingInfo = 1;
			*(time_t*)Data = StartStamp + Duration;
		}
	}

	//Add new node to our store
	if (FoundLicenseTimingInfo == 0)
	{
		NodeList->PushData((char*)&StartStamp, sizeof(StartStamp), DB_LICENSE_START);
		NodeList->PushData((char*)&Duration, sizeof(Duration), DB_LICENSE_DURATION);
		time_t LicenseExpire = StartStamp + Duration;
		NodeList->PushData((char*)&LicenseExpire, sizeof(LicenseExpire), DB_LICENSE_END);
	}

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

/**
* \brief    Get a valid activation key for this project-feature from the loaded license
* \details  License will be active from a specific date for a specific time. If license does not contain a still valid activation key, warning error code will be returned and result will be an empty string
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	ProjectId - number that is unique for each Siemens software product / project
* \param	FeatureId - number that is unique for each feature within a project
* \param	StoreResult - buffer where to store the activation key
* \param	MaxResultSize - max number of bytes we can use to store the activation key
* \return	Error code. 0 is no error
*/
int	License::GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//sanity checks 
	if (StoreResult == NULL || MaxResultSize <= 0)
	{
		Log(LL_DEBUG_INFO, "Invalid usage of GetActivationKey API");
		return ERROR_INVALID_ADDRESS;
	}

	//just in case for any reason we return prematurly, we should provide a reply
	strcpy_s(StoreResult, MaxResultSize, "");

	time_t SecondsRemaining;
	if (GetRemainingSeconds(SecondsRemaining) != 0 || SecondsRemaining == 0)
	{
		//trigger grace period if it has not been done before
		LicenseGracePeriod::UpdateStatus(GP_TRIGGER_GRACE_PERIOD, 0);
		//check if license is still in a valid grace period interval
		int res;
		int er = IsLicenseInGracePeriod(&res);
		if (er != 0 || res == 0)
		{
			Log(LL_IMPORTANT, "License expired. Could not get activation key for %d-%d", ProjectId, FeatureId);
			return WARNING_NO_LICENSE_FOUND;
		}
	}

	DataCollectionIterator itr;
	itr.Init(NodeList);
	char *Data;
	int Size;
	int Type;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_LICENSE_NODE)
		{
			LicenseNode *CurNode = (LicenseNode *)Data;
			if (CurNode->ProjectId == ProjectId && CurNode->FeatureId == FeatureId)
			{
				if (strlen(CurNode->ActivateKey) >= (unsigned int)MaxResultSize)
				{
					Log(LL_ERROR, "Activation key does not fit into return buffer");
					return ERROR_BUFFER_OVERFLOW;
				}
				LicenseGracePeriod::UpdateStatus(GP_RESET_GRACE_PERIOD, (int)GracePeriod);
				LicenseGracePeriod::UpdateStatus(GP_SET_LICENSE_END, (int)StartStamp + Duration);
				return strcpy_s(StoreResult, MaxResultSize, CurNode->ActivateKey);
			}
		}
	}

	//we did not find a valid license
	strcpy_s(StoreResult, MaxResultSize, "");
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return WARNING_NO_LICENSE_FOUND;
}

/**
* \brief    Time remaining for this license until it expires
* \details  Counted from current time ( or start of the license if not yet active) until the end stamp of the license the number of seconds.
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	RemainingSeconds = Endstamp - max( current_time, start_time )
* \return	Error code. 0 is no error
*/
int	License::GetRemainingSeconds(time_t &RemainingSeconds)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	DataCollectionIterator itr;
	itr.Init(NodeList);
	char *Data;
	int Size;
	int Type;
	RemainingSeconds = 0;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_LICENSE_END && Size >= sizeof(time_t))
		{
			time_t CurTime = GetUnixtime();
			time_t *EndTime = (time_t *)Data;
			if (CurTime < *EndTime)
				RemainingSeconds = *EndTime - CurTime;
		}
		//we loaded data recently ? Update our internal state
		if (Type == DB_LICENSE_START && StartStamp == 0 && Size >= sizeof(time_t))
			StartStamp = *(time_t *)Data;
		if (Type == DB_LICENSE_DURATION && Duration == 0 && Size >= sizeof(unsigned int))
			Duration = *(unsigned int *)Data;
		if (Type == DB_GRACE_PERIOD && Size >= sizeof(time_t))
			GracePeriod = *(time_t *)Data;
	}

	//remaining seconds can become negative
	if (StartStamp > GetUnixtime())
		RemainingSeconds = Duration;
	else
		RemainingSeconds = (StartStamp + Duration) - GetUnixtime();

	if (RemainingSeconds != 0)
		return 0;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return WARNING_MISSING_LICENSE_FIELD;
}

/**
* \brief    Save the licesense content to a file
* \details  Save the license info to a file. The content of the license will be encoded and can only be decoded on the machine the fingerprint was generated
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	FileName - The path and the name of the file where to save the content
* \param	FingerprintFilename - The path and the name of the fingerprint file that will be used to encode the content of the license
* \return	Error code. 0 is no error
*/
int	License::SaveToFile(const char *FileName, const char *FingerprintFilename)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//sanity checks
	if (FileName == NULL)
		return ERROR_BAD_PATHNAME;

	int TotalSize = NodeList->GetDataSize();
	unsigned char *TempBuff = new unsigned char[TotalSize];
	unsigned int EncryptSalt = 0;

	//if we have a fingerprint file than we can use it for simetric encryption
	if (FingerprintFilename != NULL)
	{
		NodeList->SetEncription(DCE_EXTERNAL_FINGERPRINT);
		memcpy(TempBuff, NodeList->GetData(), TotalSize);
		EncryptSalt = GenerateSaltKey( StartStamp, Duration );
		int er = EncryptWithFingerprint(FingerprintFilename, (unsigned int)EncryptSalt, TempBuff, TotalSize);
		if (er != 0)
		{
			delete TempBuff;
			return er;
		}
	}
	else
		memcpy(TempBuff, NodeList->GetData(), TotalSize);

	//dump the content to a file
	//open file
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "wb");
	if (er != NO_ERROR)
	{
		Log(LL_ERROR, "Could not save license. Error opening file %s", FileName);
		return er;
	}

	if (f == NULL)
	{
		Log(LL_ERROR, "Could not save license. Error opening file %s", FileName);
		return ERROR_FILE_INVALID;
	}

	//dump the content
	fwrite(&EncryptSalt, 1, sizeof(unsigned int), f);
	fwrite(&TotalSize, 1, sizeof(int), f);
	fwrite(TempBuff, 1, TotalSize, f);
	fclose(f);

	delete TempBuff;

	Log(LL_DEBUG_ALL, "License succesfully saved to %s", FileName);
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

int	License::LoadFromFile(char *FileName)
{
	return LoadFromFile(FileName, NULL);
}

/**
* \brief    Load the content of a license from a file
* \details  Buffer the content of the license into internal storage container
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	FileName - The path and the name of the file where to load from the content of the license
* \param	FingerprintFilename - optional paramether. You should leave this NULL
* \return	Error code. 0 is no error
*/
int	License::LoadFromFile(char *FileName, char *FingerprintFilename)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//sanity check
	if (FileName == NULL)
		return ERROR_FILE_INVALID;

	//open file
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "rb");

	if (er != NO_ERROR)
	{
		Log(LL_ERROR, "Could not load license. Error opening file %s", FileName);
		return er;
	}

	if (f == NULL)
	{
		Log(LL_ERROR, "Could not load license. Error opening file %s", FileName);
		return ERROR_FILE_INVALID;
	}

	unsigned int EncryptSalt = 0;
	fread(&EncryptSalt, 1, sizeof(unsigned int), f);

	//get the amount of memory we need to store
	int TotalSize;
	fread(&TotalSize, 1, sizeof(int), f);

	//read the whole thing into memory
	char *TempBuff = new char[TotalSize];
	fread(TempBuff, 1, TotalSize, f);

	//backup in case we mess up decryption
	char *TempBuffBackup = new char[TotalSize];
	memcpy(TempBuffBackup, TempBuff, TotalSize);

	//deobfuscate
	int erDecrypt = 0;
	if (EncryptSalt)
		erDecrypt = DecryptWithFingerprint(FingerprintFilename, EncryptSalt, (unsigned char*)TempBuff, TotalSize);

	//done with the FILE
	fclose(f);

	//make sure we can store it
	NodeList->DisposeData();
	NodeList->PushData(TempBuff, TotalSize, DB_RAW_FULL_LIST_CONTENT);

	//maybe we messed up encryption ? Could happen if we entered grace period due to HW changes
	if (erDecrypt != 0 || NodeList->IsDataValid() == 0)
	{
		//dispose unusable data
		NodeList->DisposeData();
		//restore backup
		memcpy(TempBuff, TempBuffBackup, TotalSize);
		//try to borrow fingerprint from grace
		unsigned char Temp[COMPUTER_FINGERPRINT_STORE_SIZE];
		int FingerprintSize = 0;
		int erFingerprint = LicenseGracePeriod::GetSavedFingerprint(Temp, sizeof(Temp), &FingerprintSize);
		if (erFingerprint == 0 && FingerprintSize > 0 )
		{
			NodeList->PushData(TempBuff, TotalSize, DB_RAW_FULL_LIST_CONTENT);
			int erDecrypt = EncryptWithFingerprintContent(Temp, FingerprintSize, EncryptSalt, (unsigned char*)TempBuff, TotalSize);
			if (erDecrypt != 0)
			{
				delete TempBuff;
				delete TempBuffBackup;
				return erDecrypt;
			}
			//if there was no decrypt err with grace key, reinitialize the new list. If this fails, we have no backup plan
			NodeList->PushData(TempBuff, TotalSize, DB_RAW_FULL_LIST_CONTENT);
		}
	}

	// we do not need these anymore
	delete TempBuff;
	TempBuff = NULL;
	delete TempBuffBackup;
	TempBuffBackup = NULL;

	//still not a valid license content ? That is bad. Very bad
	if (NodeList->IsDataValid() == 0)
	{
		NodeList->DisposeData();
		return ERROR_LICENSE_INVALID;
	}

#ifdef ENABLE_ANTI_HACK_CHECKS
	time_t TempRemainingSeconds = 0;
	if (GetRemainingSeconds(TempRemainingSeconds) == 0)
	{
		time_t ExpectedSaltKey = GenerateSaltKey(StartStamp, Duration);
		if (ExpectedSaltKey != EncryptSalt)
		{
			NodeList->DisposeData();
			Log(LL_DEBUG_INFO, "Encription mismatch while loading license content", FileName);
			return ERROR_LICENSE_INVALID;
		}
	}
#endif

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//all good
	return 0;
}


//////////////////// BEGIN ASYNC CALLBACK IMPLEMENTATION FOR WINDOWS //////////////////////////////////
//getting activation key in async way might seem a bit hecktic. Feel free to use simple way
#ifdef WIN32
#include <Windows.h>
struct ThreadParamStore
{
	int ProjectId;
	int FeatureId;
	void *CallBack;
};
//this simply detaches from main thread to make debugging a hell. This will almost instantly return the activation key
DWORD __stdcall ReturnActivationKey(LPVOID lpParam)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	ThreadParamStore *Param = (ThreadParamStore*)lpParam;
	char TempStore[2000];
	TempStore[0] = 0;//sanity initialization

	//get the activation key
	GetActivationKey(Param->ProjectId, Param->FeatureId, TempStore, sizeof(TempStore));

	//return it by using the callback function
	void(*ResultReturnCallback)(char *);
	ResultReturnCallback = (void(*)(char *))Param->CallBack;
	ResultReturnCallback(TempStore);

	//cleanup 
	delete lpParam;
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	return 0;
}

// for extra security we could detach ourself to a new thread and return result later to an unknown buffer
LIBRARY_API void GetActivationKeyAsync(int ProjectId, int FeatureId, void *CallBack)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//create a new thread
	ThreadParamStore *Param = new ThreadParamStore;
	Param->ProjectId = ProjectId;
	Param->FeatureId = FeatureId;
	Param->CallBack = CallBack;
	DWORD   ThreadId;
	CreateThread(
		NULL,						// default security attributes
		0,							// use default stack size  
		ReturnActivationKey,		// thread function name
		Param,						// argument to thread function 
		0,							// use default creation flags 
		&ThreadId);					// returns the thread identifier 
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
}
#endif

//////////////////// END ASYNC CALLBACK IMPLEMENTATION FOR WINDOWS //////////////////////////////////
