#include "stdafx.h"
#include <time.h>
#include "License.h"
#include "DataPacker.h"
#include "ProjectFeatureKeys.h"
#include <stdio.h>
#include "Encryption.h"
#include <random>
#include <thread>
#include "License_Grace.h"
#include "License_API.h"
#include "ComputerFingerprint.h"

#ifdef _DEBUG
	int ENABLE_ERROR_PRONE_TESTS = 0;
	void EnableErrorTests(){ ENABLE_ERROR_PRONE_TESTS = 1; }
#endif
/**
* \brief    C style wrapper ot return an activation key for a project-feature
* \details  License content is loaded automatically. Query for the activation key to be returned if available
* \author   Jozsa Istvan
* \date     2017-04-06
* \param	ProjectId - number that is unique for each Siemens software product / project
* \param	FeatureId - number that is unique for each feature within a project
* \param	StoreResult - buffer where to store the activation key
* \param	MaxResultSize - max number of bytes we can use to store the activation key
* \return	Error code. 0 is no error
*/
LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s",__FILE__,__LINE__,__FUNCTION__);

	//intitialize return value even if we can not provide a valid one
	strcpy_s(StoreResult, MaxResultSize, "");

	License *lic = new License;
	if (!lic)
		return ERROR_UNIDENTIFIED_ERROR;

	int er = lic->LoadFromFile("License.dat");
	if (er != 0)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_LOAD_LIC_FILE, "License.dat file missing. Could not load activation key.");
		delete lic;
		return ERROR_COULD_NOT_LOAD_LIC_FILE;
	}

	int GetKeyRes = lic->GetActivationKey(ProjectId, FeatureId, StoreResult, MaxResultSize);

	//cleanup
	delete lic;

	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return GetKeyRes;
}
/**
* \brief    Get time information from license
* \details  License has a start date and duration. Return the remaining seconds until expiration. If license already entered grace period, return remaining grace period also
* \author   Jozsa Istvan
* \date     2017-04-06
* \param	LicenseTime - Number of seconds remaining until license expires
* \param	GraceTime - Number of seconds remaining until grace period ends
* \param	GraceReasonCode - The reason why license is in grace period. It could be because license expired or hardware changes detected.
* \return	Error code. 0 is no error
*/
LIBRARY_API int	GetRemainingTime(time_t *LicenseTime, time_t *GraceTime, int *GraceReasonCode)
{
	//sanity checks
	if (LicenseTime == NULL || GraceTime == NULL || GraceReasonCode == NULL)
		return ERROR_MISSING_PARAMETER;
	
	//safety initializations
	*LicenseTime = 0;
	*GraceTime = 0;
	*GraceReasonCode = GRC_LICENSE_EXPIRED_GRACE_EXPIRED;

	License *lic = new License;
	if (!lic)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_CREATE_OBJECT, "Sever error.Could not create license object");
		return ERROR_UNIDENTIFIED_ERROR;
	}

	int er = lic->LoadFromFile("License.dat");
	if (er != 0)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_LOAD_LIC_FILE, "License.dat file missing. Could not load data.");
		delete lic;
		return ERROR_COULD_NOT_LOAD_LIC_FILE;
	}

	er = lic->GetRemainingSeconds(LicenseTime); //we can ignore error here. It could be comming from grace activation

	time_t	RemainingGraceTime = 0;
	char	IsGraceTriggered;
	int erQuery = lic->IsGracePeriodTriggered(&RemainingGraceTime, &IsGraceTriggered);
	if (erQuery != 0)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_QUERY_GRACE_PERIOD, "Unexpected error while asking for grace period. Error %d", erQuery);
		delete lic;
		return ERROR_UNIDENTIFIED_ERROR;
	}

	//cleanup
	delete lic;

	//license is expired and grace period also expired
	*GraceTime = RemainingGraceTime;

	if (*LicenseTime > 0 && IsGraceTriggered == 0)
		*GraceReasonCode = GRC_LICENSE_IS_VALID;
	else if (*LicenseTime > 0 && IsGraceTriggered != 0)
		*GraceReasonCode = GRC_LICENSE_IS_VALID_GRACE_TRIGGERED;
	else if (*LicenseTime <= 0 && *GraceTime > 0)
		*GraceReasonCode = GRC_LICENSE_EXPIRED_GRACE_AVAILABLE;
	else if (*LicenseTime <= 0 && *GraceTime <= 0)
		*GraceReasonCode = GRC_LICENSE_EXPIRED_GRACE_EXPIRED;
	else
		*GraceReasonCode = GRC_LICENSE_UNEXPECTED_VALUE;

	//if we got here, we managed to finish our job successfully
	return NO_ERROR;
}

// in case we will get time from WINCCOA, than you should only need to change this implementation
time_t GetUnixtime()
{
	return time(NULL);
}

License::License()
{
	NodeListEncrypted = new GenericDataStore;
	StartStamp = GracePeriod = 0;//mark them uninitialized
	Duration = 0;
	UsageStateFlags = 0;
	DisableEdit = 0;
	DisableQuery = 0;
	InitializedWithBackupKey = 0;
	LoadedEncryptedDataFromFile = 0;
}

License::~License()
{
	if (NodeListEncrypted != NULL)
	{
		delete NodeListEncrypted;
		NodeListEncrypted = NULL;
	}
	LicenseGracePeriod::UpdateStatus(GP_LICENSE_LIFETIME_USE_FLAGS,UsageStateFlags);
}

/**
* \brief    Add to the content of the license
* \details  License will conver this newly added project-feature combination. The activation key is automatically loaded from config file
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	ProjectId - number that is unique for each Siemens software product / project
* \param	FeatureId - number that is unique for each feature within a project
* \param	ActivationKey - String that will be used as an activation key inside Siemens projects
* \return	Error code. 0 is no error
*/
int License::AddProjectFeature(int ProjectId, int FeatureId, const char *ActivationKey)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_FEATURE_ADDED;

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;

	DisableQuery = 1;

	//sanity checks
	if (ActivationKey == NULL)
		return ERROR_INVALID_PARAMETER;

#if 0
	//check if we already have this node. There is no point for dounble adding it
	char ExistingKey[200];
	GetActivationKey(ProjectId, FeatureId, ExistingKey, sizeof(ExistingKey));
	if(strcmp(ExistingKey,ActivationKey)==0)
	{
		Log(LL_DEBUG_INFO, 0, "This node is already in the License list. There is no point adding it a seconds time");
		return 0;
	}

#endif

	//create a new storable node
	LicenseNode *NewNode = new LicenseNode;
#ifdef _DEBUG
	memset(NewNode, 0, sizeof(LicenseNode));	// in release, let it get random values
#endif
	NewNode->SetProjectId( ProjectId );
	NewNode->SetFeatureId( FeatureId );
	NewNode->SetKey(ActivationKey);

	//Add new node to our store
	NodeListEncrypted->PushData((char*)NewNode, sizeof(LicenseNode), DB_LICENSE_NODE);

	delete NewNode;
	NewNode = NULL;

	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//no errors returned
	return NO_ERROR;
}

/**
* \brief    Time limits for the license
* \details  License will be active from a specific date for a specific time
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	pStartDate - time counted in seconds since 1970. A unix timestamp. Used as a starting date for the license
* \param	pDuration - number of seconds the license will be active for counted from start date
* \param	pGraceDuration - number of seconds of maximum grace duration, if grace duration gets triggered
* \return	Error code. 0 is no error
*/
int License::SetDuration(time_t pStartDate, unsigned int pDuration, unsigned int pGraceDuration)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;

	DisableQuery = 1;

	//usage flags might get added to logs
	UsageStateFlags |= LSF_DURATION_IS_SET;

	StartStamp = pStartDate;
	Duration = pDuration;
	GracePeriod = pGraceDuration;

	//check if we already have this node. There is no point for dounble adding it
	DataCollectionIterator itr;
	itr.Init(NodeListEncrypted);
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
			*(unsigned int*)Data = Duration;
		}
		if (Type == DB_LICENSE_END)
		{
			FoundLicenseTimingInfo = 1;
			*(time_t*)Data = StartStamp + Duration;
		}
		if (Type == DB_GRACE_PERIOD)
		{
			FoundLicenseTimingInfo = 1;
			*(time_t*)Data = pGraceDuration;
		}
	}

	//Add new node to our store
	if (FoundLicenseTimingInfo == 0)
	{
		time_t LicenseExpire = StartStamp + Duration;
		NodeListEncrypted->PushData((char*)&StartStamp, sizeof(StartStamp), DB_LICENSE_START);
		NodeListEncrypted->PushData((char*)&Duration, sizeof(Duration), DB_LICENSE_DURATION);
		NodeListEncrypted->PushData((char*)&GracePeriod, sizeof(GracePeriod), DB_GRACE_PERIOD);
		NodeListEncrypted->PushData((char*)&LicenseExpire, sizeof(LicenseExpire), DB_LICENSE_END);
	}

	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return NO_ERROR;
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
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_FEATURE_REQUESTED;

	if (DisableQuery != 0)
		return ERROR_DISABLED_FUNCTION;

	//sanity checks 
	if (StoreResult == NULL || MaxResultSize <= 0)
	{
		Log(LL_DEBUG_INFO, ERROR_API_INVALID_USAGE, "Invalid usage of GetActivationKey API");
		return ERROR_INVALID_ADDRESS;
	}

	//just in case for any reason we return prematurly, we should provide a reply
	strcpy_s(StoreResult, MaxResultSize, "");

	time_t SecondsRemaining;
	time_t GracePeriodRemainingSeconds = 0;
	if (GetRemainingSeconds(&SecondsRemaining) != 0 || SecondsRemaining <= 0)
	{
		//check if license is still in a valid grace period interval
		char IsGraceTriggered;
		int er = IsGracePeriodTriggered(&GracePeriodRemainingSeconds, &IsGraceTriggered);
		if (er != 0 || (GracePeriodRemainingSeconds <= 0 && IsGraceTriggered == 1) )
		{
			Log(LL_IMPORTANT, ERROR_LICENSE_EXPIRED, "License expired. Could not get activation key for %d-%d", ProjectId, FeatureId);
			return WARNING_NO_LICENSE_FOUND;
		}
	}

	DataCollectionIterator itr;
	itr.Init(NodeListEncrypted);
	char *Data;
	int Size;
	int Type;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_LICENSE_NODE)
		{
			LicenseNode *CurNode = (LicenseNode *)Data;
			if (CurNode->GetProjectId() == ProjectId && CurNode->GetFeatureId() == FeatureId)
			{
				size_t KeyLen = CurNode->GetKeyLen();
				if (KeyLen >= HardCodedStringLimitLic)
				{
					Log(LL_ERROR, ERROR_LICENSE_IS_CORRUPTED, "License is corrupted. Skipping key fetch");
					return ERROR_BUFFER_OVERFLOW;
				}
				if (KeyLen >= (size_t)MaxResultSize)
				{
					Log(LL_ERROR, ERROR_RETURN_BUFFER_TOO_SMALL, "Activation key does not fit into return buffer");
					return ERROR_BUFFER_OVERFLOW;
				}
				//return the activation key
//				errno_t er = strcpy_s(StoreResult, MaxResultSize, CurNode->ActivateKey);
				CurNode->GetKey(StoreResult, MaxResultSize);
				return 0;
			}
		}
	}

	//we did not find a valid license
	strcpy_s(StoreResult, MaxResultSize, "");
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return WARNING_NO_LICENSE_FOUND;
}
/**
* \brief    Time remaining for this license until it expires
* \details  Change grace period status based on successfull license decode/usage
* \author   Jozsa Istvan
* \date     2017-08-31
* \param	GraceDisable Trigger or disable grace period
* \return	Error code. 0 is no error
*/
void License::UpdateGraceStatus()
{
	int er = 0;
//	DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << "Initialized with backup key " << InitializedWithBackupKey << " LoadedEncryptedDataFromFile " << LoadedEncryptedDataFromFile << " Duration " << Duration << std::endl;);
	if (InitializedWithBackupKey == 0)
	{
		//only reset grace data if we have valid info from the file
		if (LoadedEncryptedDataFromFile != 0 && Duration != 0)
		{
			// should make a change only when hardware configuration comes back online or a new valid license is getting used
			er = LicenseGracePeriod::UpdateStatus(GP_RESET_GRACE_PERIOD, (int)GracePeriod);
			// should make a change only when a new valid license is getting used
			er = LicenseGracePeriod::UpdateStatus(GP_SET_LICENSE_END, (int)StartStamp + Duration);
		}
	}
	else
	{
		er = LicenseGracePeriod::UpdateStatus(GP_TRIGGER_GRACE_PERIOD, 0);
	}
	if (er != 0)
	{
		Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << "Error changing license grace status" << std::endl;);
	}
}
/**
* \brief    Time remaining for this license until it expires
* \details  Counted from current time ( or start of the license if not yet active) until the end stamp of the license the number of seconds.
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	RemainingSeconds = Endstamp - max( current_time, start_time )
* \return	Error code. 0 is no error
*/
int	License::GetRemainingSeconds(time_t *RemainingSeconds)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_DURATION_REQUESTED;

	if (DisableQuery != 0)
		return ERROR_DISABLED_FUNCTION;

	DataCollectionIterator itr;
	itr.Init(NodeListEncrypted);
	char *Data;
	int Size;
	int Type;
	*RemainingSeconds = 0;
	int ItrRet;
	ItrRet = itr.GetNext(&Data, Size, Type);
	time_t EndTime = 0;
	while (DCI_SUCCESS == ItrRet)
	{
		if (Type == DB_LICENSE_END && Size >= sizeof(time_t))
		{
			time_t CurTime = GetUnixtime();
			EndTime = *(time_t *)Data;
			if (CurTime < EndTime)
				*RemainingSeconds = EndTime - CurTime;
		}
		//we loaded data recently ? Update our internal state
		if (Type == DB_LICENSE_START && StartStamp == 0 && Size >= sizeof(time_t))
			StartStamp = *(time_t *)Data;
		if (Type == DB_LICENSE_DURATION && Duration == 0 && Size >= sizeof(unsigned int))
			Duration = *(unsigned int *)Data;
		if (Type == DB_GRACE_PERIOD && Size >= sizeof(time_t))
			GracePeriod = *(time_t *)Data;
		ItrRet = itr.GetNext(&Data, Size, Type);
	}

#ifdef ENABLE_ANTI_HACK_CHECKS
	if (StartStamp + Duration != EndTime)
	{
		Log(LL_HACKING, ERROR_LICENSE_IS_CORRUPTED, "License duration mismatch");
		*RemainingSeconds = 0;
		return ERROR_LICENSE_EXPIRED;
	}
#endif

	//this is a valid license API use. We probably have a valid license and we should not be using grace
	UpdateGraceStatus();

	//remaining seconds can become negative !
	if (Duration == PERMANENT_LICENSE_MAGIC_DURATION )
		*RemainingSeconds = PERMANENT_LICENSE_MAGIC_DURATION;
	else if (StartStamp > GetUnixtime())
		*RemainingSeconds = Duration;
	else
		*RemainingSeconds = (StartStamp + Duration) - GetUnixtime();

	if (*RemainingSeconds != 0)
		return NO_ERROR;

	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return WARNING_MISSING_LICENSE_FIELD;
}
/**
* \brief    Check if license is using grace period right now
* \details  There is a chance license will trigger using grace period even while it is active. This function will signal that scenario
* \author   Jozsa Istvan
* \date     2017-04-05
* \param	RemainingSeconds - Number of seconds remaining from a triggered grace period. If grace period is not triggered, the returned value is 0
* \return	Error code. 0 is no error
*/
int License::IsGracePeriodTriggered(time_t *RemainingSeconds, char *IsTriggered)
{
	//usage flags might get added to logs
	UsageStateFlags |= LSF_GRACE_STATUS_QUERIED;

	if (RemainingSeconds == NULL || IsTriggered == NULL)
		return ERROR_MISSING_PARAMETER;

	// make sure return value is initialized
	*RemainingSeconds = 0;
	*IsTriggered = 1;

	int IsGraceTriggered;
	time_t GracePeriodRemaining;
	int er = LicenseGracePeriod::GetStatus(&IsGraceTriggered, &GracePeriodRemaining);
	if (er != 0)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_QUERY_GRACE_PERIOD, "Could not get grace info");
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Grace info not yet initialized. Could not get data " << std::endl;);
		return ERROR_UNIDENTIFIED_ERROR;
	}

	//is it triggered ?
	if (IsGraceTriggered != 0)
		*IsTriggered = 1;
	else
		*IsTriggered = 0;

	//license has grace period remaining unused. This is only a valid value if grace has been triggered
	*RemainingSeconds = GracePeriodRemaining;

/*
	time_t RemainingSec = 0;
	int er = GetRemainingSeconds(&RemainingSec);
	if (er != 0 && er != ERROR_LICENSE_EXPIRED)
	{
		Log(LL_ERROR, ERROR_COULD_NOT_QUERY_GRACE_PERIOD, "Could not get time info from license.dat");
		return ERROR_UNIDENTIFIED_ERROR;
	}

	//license is still valid. It is not considered grace period if everything is fine
	if (RemainingSec > 0)
	{
		*RemainingSeconds = 0;
		return NO_ERROR;
	}

	if (RemainingSec <= 0)
	{
		int IsGraceTriggered;
		time_t GracePeriodRemaining;
		int er = LicenseGracePeriod::GetStatus(&IsGraceTriggered, &GracePeriodRemaining);
		if (er != 0)
		{
			Log(LL_ERROR, ERROR_COULD_NOT_QUERY_GRACE_PERIOD, "Could not get grace info");
			return ERROR_UNIDENTIFIED_ERROR;
		}

		//license is in a valid grace period
		if (IsGraceTriggered != 0 && GracePeriodRemaining > 0)
		{
			*RemainingSeconds = GracePeriodRemaining;
			return NO_ERROR;
		}
	}

	// we should have not got here. License is in an unexpected state
	*RemainingSeconds = 0;
*/
	return NO_ERROR;
}
/**
* \brief    From what date is this license usable
* \details  Get the unix timestamp from when the license can be used client side
* \author   Jozsa Istvan
* \date     2017-04-15
* \param	StartStamp - The path and the name of the file where to save the content
* \param	Key - encryption key
* \return	Error code. 0 is no error
*/
int	License::GetStartStamp(time_t *pStartStamp)
{
	*pStartStamp = -1;
	time_t RemainingSec = 0;
	//load or refresh timer values
	int er = GetRemainingSeconds(&RemainingSec);
	if (er != 0)
		return er;
	*pStartStamp = StartStamp;
	//no error
	return 0;
}
/**
* \brief    From what date is this license usable
* \details  Get the unix timestamp from when the license can be used client side
* \author   Jozsa Istvan
* \date     2017-04-15
* \param	StartStamp - The path and the name of the file where to save the content
* \param	Key - encryption key
* \return	Error code. 0 is no error
*/
int	License::GetEndStamp(time_t *EndStamp)
{
	*EndStamp = -1;
	time_t RemainingSec = 0;
	//load or refresh timer values
	int er = GetRemainingSeconds(&RemainingSec);
	if (er != 0)
		return er;
	if (Duration == PERMANENT_LICENSE_MAGIC_DURATION)
		*EndStamp = 0;
	else
		*EndStamp = StartStamp + Duration;
	//no error
	return 0;
}
/**
* \brief    Save the licesense content to a file
* \details  Save the license info to a file. The content of the license will be encoded and can only be decoded on the machine the fingerprint was generated
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	FileName - The path and the name of the file where to save the content
* \param	Key - encryption key
* \param	KeyLen - encryption key length
* \param	Append - append multiple licenses to the same file ?
* \return	Error code. 0 is no error
*/
int	License::SaveToFile(const char *FileName, ComputerFingerprint *FP, int Append)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_SAVED_TO_FILE;

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;

	//sanity checks
	if (FileName == NULL)
		return ERROR_BAD_PATHNAME;

	int er = 0;

	//copy content of the license and encrypt it 
	int TotalSizeEncrypted = NodeListEncrypted->GetDataSize();
	unsigned char *TempBuff = new unsigned char[TotalSizeEncrypted];
	memcpy(&TempBuff[0], NodeListEncrypted->GetData(), TotalSizeEncrypted);

	int KeyLen = 0;
	char *Key = NULL;
	unsigned int EncryptSalt = 0;
	if (FP != NULL)
	{
		FP->DupEncryptionKey(&Key, &KeyLen);
		//encrypt with fingerprint te list of protected keys
		if (KeyLen > 0)
		{
			NodeListEncrypted->SetEncription(DCE_EXTERNAL_FINGERPRINT);
			EncryptSalt = GenerateSaltKey(StartStamp, Duration);
			er = EncryptWithFingerprintContent((unsigned char*)Key, KeyLen, (unsigned int)EncryptSalt, TempBuff, TotalSizeEncrypted);
			if (er != 0)
			{
				delete TempBuff;
				return er;
			}
		}
		else
		{
			NodeListEncrypted->SetEncription(DCE_NONE);
		}
	}

	//encrypt the list of values that are not encrypted
	int KeyListLen = 0;
	char *KeyList = NULL;
	unsigned int EncryptSalt2 = 0;
#ifdef ENABLE_INCLUDE_BACKUP_DECRYPT_KEY
	if (FP)
	{
		FP->DupEncryptionList(&KeyList, &KeyListLen);
		if (KeyListLen > 0 && KeyList != NULL)
		{
			EncryptSalt2 = GenerateSaltKey2;
			er = EncryptBufferXORKey((unsigned char*)KeyList, KeyListLen, (unsigned char*)&EncryptSalt2, sizeof(EncryptSalt2));
			if (er != 0)
			{
				delete TempBuff;
				return er;
			}
		}
	}
#endif

	//dump the content to a file
	//open file
	FILE *f;
	errno_t erf;

	if (Append != 0)
		erf = fopen_s(&f, FileName, "ab"); //can stack multiple licenses into a single file
	else
		erf = fopen_s(&f, FileName, "wb");

	if (erf != NO_ERROR || f == NULL )
	{
		Log(LL_ERROR, ERROR_COULD_NOT_SAVE_LICENSE, "Could not save license. Error opening file %s", FileName);
		return ERROR_FILE_INVALID;
	}

	//dump the content
	FileLicenseHeader LicenseHeader(TotalSizeEncrypted + KeyListLen, TotalSizeEncrypted, EncryptSalt, EncryptSalt2);
	fwrite(&LicenseHeader, 1, sizeof(FileLicenseHeader), f);
	fwrite(TempBuff, 1, TotalSizeEncrypted, f);
	fwrite(KeyList, 1, KeyListLen, f);
	fclose(f);

	delete TempBuff;
	if (Key)
		FreeDup(Key);
	if (KeyList)
		FreeDup(KeyList);

	Log(LL_DEBUG_ALL, 0, "License succesfully saved to %s", FileName);
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return NO_ERROR;
}
/**
* \brief    Save the licesense content to a file
* \details  Save the license info to a file. The content of the license will be encoded and can only be decoded on the machine the fingerprint was generated
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	FileName - The path and the name of the file where to save the content
* \param	FingerprintFilename - The path and the name of the fingerprint file that will be used to encode the content of the license
* \param	Append - Add another license to an already existing license file. A single file can contain multiple licenses
* \return	Error code. 0 is no error
*/
int	License::SaveToFile_(const char *FileName, const char *FingerprintFilename, int Append)
{
	if (FingerprintFilename == NULL)
		return ERROR_BAD_PATHNAME;

	//if we have a fingerprint file than we can use it for simetric encryption
	ComputerFingerprint CF;
	if (CF.LoadFingerprint(FingerprintFilename) != 0)
		return ERROR_BAD_PATHNAME;

	int er = SaveToFile(FileName, &CF, Append);

	return er;
}
/**
* \brief    Load the content of a license from a file
* \details  Buffer the content of the license into internal storage container
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	FileName - The path and the name of the file where to load from the content of the license
* \return	Error code. 0 is no error
*/
/*
int	License::LoadFromFile(const char *FileName)
{
	return LoadFromFile(FileName, NULL);
}
*/
//forward declaration of function to get filesize in bytes
long GetFileSize(const char *filename);

/**
* \brief    Load the content of a license from a buffer.
* \details  Buffer the content of the license into internal storage container
* \author   Jozsa Istvan
* \date     2017-04-12
* \param	FileContent - Buffer containing an encrypted license
* \param	TotalSize - size of the buffer
* \param	EncryptSalt - Salt for the encryption key
* \param	FingerprintFilename - a file ( or NULL ) containing the decode key
* \return	Error code. 0 is no error
*/
int	License::InitFromBuffer(char *FileContent, FileLicenseHeader *LFH, char *Key, int KeyLen)
{
	//backup in case we mess up decryption
	char *FileContentBackup = new char[LFH->TotalSize];
	memcpy(FileContentBackup, FileContent, LFH->TotalSize);

	//deobfuscate
	int erDecrypt = 0;
	if (LFH->EncryptSalt)
		erDecrypt = EncryptWithFingerprintContent((unsigned char*)Key, KeyLen, LFH->EncryptSalt, (unsigned char*)FileContent, LFH->SizeEncrypted);

	//make sure we can store it
	GenericDataStore LicenseFromFile;
	LicenseFromFile.PushData(FileContent, LFH->SizeEncrypted, DB_RAW_FULL_LIST_CONTENT);

	//maybe we messed up encryption ? Could happen if we entered grace period due to HW changes
	int SizeNotEncrypted = LFH->TotalSize - LFH->SizeEncrypted;
	if (SizeNotEncrypted <= 0)
	{
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Missing backup key " << std::endl;);
	}
	//generate a temp decrypt key
	if (erDecrypt != 0 || LicenseFromFile.IsDataValid() == 0)
	{
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Failed to descrypt with auto key. Try backup key " << std::endl;);
		//dispose unusable data
		LicenseFromFile.DisposeData();
		//restore backup
		memcpy(FileContent, FileContentBackup, LFH->SizeEncrypted);
#ifdef ENABLE_INCLUDE_BACKUP_DECRYPT_KEY
		int er = EncryptBufferXORKey((unsigned char*)&FileContent[LFH->SizeEncrypted], SizeNotEncrypted, (unsigned char*)&LFH->EncryptSaltNE, sizeof(LFH->EncryptSaltNE));
		if (er != 0)
		{
			delete FileContentBackup;
			return er;
		}
#endif
		unsigned char *DecryptKeyFromFile;
		int DecryptKeySize;
		int erFingerprint = GetTempDecryptKeyDup(&DecryptKeyFromFile, &DecryptKeySize, &FileContent[LFH->SizeEncrypted], SizeNotEncrypted);
		if (erFingerprint == 0 && DecryptKeySize > 0)
		{
			//try to decrypt file content with grace key again
			int erDecrypt = EncryptWithFingerprintContent(DecryptKeyFromFile, DecryptKeySize, LFH->EncryptSalt, (unsigned char*)FileContent, LFH->SizeEncrypted);
			if (erDecrypt != 0)
			{
				FreeDup(DecryptKeyFromFile);
				delete FileContentBackup;
				return erDecrypt;
			}

			//if there was no decrypt err with grace key, reinitialize the new list. If this fails, we have no backup plan
			LicenseFromFile.PushData(FileContent, LFH->SizeEncrypted, DB_RAW_FULL_LIST_CONTENT);

			//trigger grace period as our hardware key was mismatching but we managed to decode using the backup key
			if (LicenseFromFile.IsDataValid() == 1)
			{
				DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Managed to decode with backup key" << std::endl;);
				InitializedWithBackupKey = 1;
			}
			FreeDup(DecryptKeyFromFile);
		}
	}

	// we do not need these anymore
	delete FileContentBackup;
	FileContentBackup = NULL;

	//still not a valid license content ? That is bad. Very bad
	if (LicenseFromFile.IsDataValid() == 0)
	{
		return ERROR_LICENSE_INVALID;
	}

	//did we use decryption keys while adding content to the License ?
	if (LFH->SizeEncrypted > 0 && LFH->EncryptSalt!=0)
		LoadedEncryptedDataFromFile = 1;

	//merge data from this block to the sum of blocks
	if (NodeListEncrypted->GetData() == NULL)
	{
		NodeListEncrypted->PushData(FileContent, LFH->SizeEncrypted, DB_RAW_FULL_LIST_CONTENT);
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " First license loaded from file" << std::endl;);
	}
	else
	{
		NodeListEncrypted->MergeLicenseNodesFrom(&LicenseFromFile);
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Additional license loaded from file" << std::endl;);
	}

	//all good
	return NO_ERROR;
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
int	License::LoadFromFile(const char *FileName, const char *FingerprintFilename)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//deny API calls that can change our internal state
	DisableEdit = 1;

	//usage flags might get added to logs
	UsageStateFlags |= LSF_LOADED_FROM_FILE;

	//sanity check
	if (FileName == NULL)
		return ERROR_MISSING_PARAMETER;

	// dispose of old content. Support object reuse
	NodeListEncrypted->DisposeData();
	InitializedWithBackupKey = 0;
	LoadedEncryptedDataFromFile = 0;

	//open file
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "rb");

	if (er != NO_ERROR || f == NULL)
	{
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not open Input file" << std::endl;);
		char buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		Log(LL_ERROR, ERROR_COULD_NOT_LOAD_LIC_FILE, "Could not load license. Error opening file %s/%s", buffer, FileName);
		return ERROR_COULD_NOT_LOAD_LIC_FILE;
	}

	//if we have a fingerprint file than we can use it for simetric encryption
	int Error;
	ComputerFingerprint CF;
	if (FingerprintFilename != NULL && CF.LoadFingerprint(FingerprintFilename) != 0)
	{
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not load fingerprint file " << FingerprintFilename << std::endl;);
		fclose(f);
		return ERROR_BAD_PATH_NAME;
	}
	else if ((Error = CF.GenerateFingerprint()) != 0)
	{
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not generate fingerprint "<< std::endl;);
		// do not exit yet, license might contain non encrypted content yet
//		return Error;
	}
	//load the encryption key from file
	char *EncryptKey;
	int KeyLen;
	if ((Error = CF.DupEncryptionKey(&EncryptKey, &KeyLen)) != 0)
	{
		KeyLen = 0;
		// do not exit yet, license might contain non encrypted content yet
//		return Error;
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not generate encryption key " << std::endl;);
	}

	int FoundDecodableLicense = 0;
	int FoundDecodableLicenseEncrypted = 0;
	int LicenseBlocksProcessed = 0;
	size_t TotalBytesRead = 0;
	size_t FileSize = GetFileSize(FileName);

	do{
		size_t BytesRead;
		FileLicenseHeader LFH;

		//get the amount of memory we need to store
		BytesRead = fread(&LFH, 1, sizeof(FileLicenseHeader), f);
		TotalBytesRead += BytesRead;
		if (BytesRead < sizeof(FileLicenseHeader))
		{
			Log(LL_ERROR, ERROR_COULD_NOT_LOAD_LIC_FILE, "Could not read license. File too small", FileName);
			DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not read license. File too small" << std::endl;);
			fclose(f);
			return ERROR_LICENSE_IS_CORRUPTED;
		}

		//read the whole thing into memory
		char *FileContent = new char[LFH.TotalSize];
		BytesRead = fread(FileContent, 1, LFH.TotalSize, f);
		TotalBytesRead += BytesRead;
		if ((int)BytesRead < LFH.TotalSize)
		{
			Log(LL_ERROR, ERROR_COULD_NOT_LOAD_LIC_FILE, "Could not read license. File too small", FileName);
			DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not read license. File too small" << std::endl;);
			fclose(f);
			return ERROR_LICENSE_IS_CORRUPTED;
		}

		LicenseBlocksProcessed++;
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Trying to decode license block " << LicenseBlocksProcessed << " with size " << LFH.TotalSize << std::endl;);
		if (InitFromBuffer(FileContent, &LFH, EncryptKey, KeyLen) == 0)
		{
			FoundDecodableLicense = 1;
			if (LFH.SizeEncrypted > 0 && LFH.EncryptSalt != 0)
			{
				FoundDecodableLicenseEncrypted = 1;
				DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Successfully decoded license block " << LicenseBlocksProcessed << " with size " << LFH.TotalSize << " Nodecount : " << NodeListEncrypted->GetNodeCount() << std::endl;);
			}
			else
			{
				DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Successfully loaded unencrypted license block " << LicenseBlocksProcessed << " with size " << LFH.TotalSize << " Nodecount : " << NodeListEncrypted->GetNodeCount() << std::endl;);
			}
		}
		delete FileContent;
	} while (!feof(f) && TotalBytesRead < FileSize);
	//done with the FILE
	fclose(f);

	FreeDup(EncryptKey);

	if (FoundDecodableLicense == 0)
	{
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " No decodable license found" << std::endl;);
		return ERROR_NO_DECODABLE_LICENSE_FOUND;
	}
	if (FoundDecodableLicenseEncrypted == 0)
		DEBUG_LIC_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " No decodable encrypted license found" << std::endl;);

	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//all good
	return NO_ERROR;
}

int	License::GetTempDecryptKeyDup(unsigned char **Key, int *KeyLen, char *GoodHWKeys, int GoodHWKeysSize)
{
	*KeyLen = 0;
#ifdef ENABLE_INCLUDE_BACKUP_DECRYPT_KEY
	//try to load up the non encrypted data and generate a decryption key from there. This should only be possible if max 1 HW key difference is detected or inside grace period
	{
		GenericDataStore FingerprintDataGood;
		FingerprintDataGood.PushData(GoodHWKeys, GoodHWKeysSize, DB_RAW_FULL_LIST_CONTENT);
		if (FingerprintDataGood.IsDataValid()) //should pass all the time
		{
			//this is the default fingerprint
			int Error;
			ComputerFingerprint CF;
			if ((Error = CF.GenerateFingerprint()) != 0)
				return Error;

			Error = CF.DupEncryptionKey((char**)Key, KeyLen, &FingerprintDataGood);
		}
	}
#endif
#ifdef ENABLE_INCLUDE_BACKUP_DECRYPT_KEY_GRACE
	//try to borrow fingerprint from grace. This is a depracated feature !!
	if (*KeyLen==0)
	{
		int erFingerprint = LicenseGracePeriod::GetSavedFingerprint(Key, COMPUTER_FINGERPRINT_STORE_SIZE, KeyLen);
	}
#endif
	
	return 0;
}
#ifdef _DEBUG
void License::PrintNodes()
{
	DataCollectionIterator itr;
	itr.Init(NodeListEncrypted);
	char *tData;
	int tSize;
	int tType;
	int NodeCount = 0;
	while (DCI_SUCCESS == itr.GetNext(&tData, tSize, tType))
	{
		printf("%d)Type %d, Size %d", NodeCount, tType, tSize);
		if (tType == DB_LICENSE_NODE)
		{
			LicenseNode *NewNode = (LicenseNode *)tData;
			char KeyStore[500];
			NewNode->GetKey(KeyStore, sizeof(KeyStore));
			printf(",ProjID %d, FeatureId %d, Key %s", NewNode->GetProjectId(), NewNode->GetFeatureId(), KeyStore);
		}
		printf("\n");
		NodeCount++;
	}

}
#endif
/**
* \brief    Deallocate memory allocated by dup functions
* \details  This is a very important function if you are using different versions of compilers. You might get heap corruption warnings if you mix different versions of CRT
* \author   Jozsa Istvan
* \date     2017-04-19
* \param	p - pointer returned by one of the Dup functions
* \return	Error code. 0 is no error
*/
int FreeDup(void *p)
{
	if (p == NULL)
		return ERROR_MISSING_PARAMETER;

	//using same CRT free as the one we used to allocate the memory
	free(p);

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
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
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
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	return NO_ERROR;
}

// for extra security we could detach ourself to a new thread and return result later to an unknown buffer
LIBRARY_API void GetActivationKeyAsync(int ProjectId, int FeatureId, void *CallBack)
{
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
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
	Log(LL_DEBUG_DEBUG_ONLY, 0, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
}
#endif

//////////////////// END ASYNC CALLBACK IMPLEMENTATION FOR WINDOWS //////////////////////////////////
