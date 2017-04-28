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
#include "License_API.h"
#include "src/ComputerFingerprint.h"

#ifdef _DEBUG
	int ENABLE_ERROR_PRONE_TESTS = 0;
	void EnableErrorTests(){ ENABLE_ERROR_PRONE_TESTS = 1; }
#endif

LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s",__FILE__,__LINE__,__FUNCTION__);

	//intitialize return value even if we can not provide a valid one
	strcpy_s(StoreResult, MaxResultSize, "");

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
/*
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

	int IsTriggered = 0;
	int erQuery = lic->IsGracePeriodTriggered(&IsTriggered);
	if (erQuery != 0)
	{
		Log(LL_ERROR, "Unexpected error while asking for grace period. Error %d", erQuery);
		delete lic;
		return ERROR_UNIDENTIFIED_ERROR;
	}

	//cleanup
	delete lic;

	//license is expired and grace period also expired
	*Result = IsTriggered;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return NO_ERROR;
}*/
/**
* \brief    Get time information from license
* \details  License has a start date and duration. Return the remaining seconds until expiration. If license already entered grace period, return remaining grace period also
* \author   Jozsa Istvan
* \date     2017-04-06
* \param	ProjectName - name that is unique for each Siemens software product / project
* \param	FeatureName - name that is unique for each feature within a project
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
		Log(LL_ERROR, "Sever error.Could not create license object");
		return ERROR_UNIDENTIFIED_ERROR;
	}

	int er = lic->LoadFromFile("License.dat");
	if (er != 0)
	{
		Log(LL_ERROR, "License.dat file missing. Could not load data.");
		delete lic;
		return ERROR_COULD_NOT_LOAD_LIC_FILE;
	}

	er = lic->GetRemainingSeconds(LicenseTime); //we can ignore error here. It could be comming from grace activation

	time_t RemainingGraceTimeIfTriggered = 0;
	int erQuery = lic->IsGracePeriodTriggered(&RemainingGraceTimeIfTriggered);
	if (erQuery != 0)
	{
		Log(LL_ERROR, "Unexpected error while asking for grace period. Error %d", erQuery);
		delete lic;
		return ERROR_UNIDENTIFIED_ERROR;
	}

	//cleanup
	delete lic;

	//license is expired and grace period also expired
	*GraceTime = RemainingGraceTimeIfTriggered;

	if (*LicenseTime > 0)
		*GraceReasonCode = GRC_LICENSE_IS_VALID;
	else if (*LicenseTime <= 0 && *GraceTime > 0)
		*GraceReasonCode = GRC_LICENSE_EXPIRED_GRACE_AVAILABLE;
	else
		*GraceReasonCode = GRC_LICENSE_EXPIRED_GRACE_EXPIRED;

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
	NodeList = new GenericDataStore;
	StartStamp = GracePeriod = 0;//mark them uninitialized
	Duration = 0;
	UsageStateFlags = 0;
	DisableEdit = 0;
	DisableQuery = 0;
}

License::~License()
{
	if (NodeList != NULL)
	{
		delete NodeList;
		NodeList = NULL;
	}
	LicenseGracePeriod::UpdateStatus(GP_LICENSE_LIFETIME_USE_FLAGS,UsageStateFlags);
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
/*
int	License::AddProjectFeature(const char *ProjectName, const char *FeatureName)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	
	UsageStateFlags |= LSF_FEATURE_ADDED;

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;
	DisableQuery = 1;

	ProjectFeatureKeyDB DB;
	int ProjectId = DB.GetProjectNameID(ProjectName);
	int FeatureId = DB.GetFeatureNameID(FeatureName);
	int ret = AddProjectFeature(ProjectId, FeatureId);
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return ret;
}*/

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
/*
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
	return NO_ERROR;
}
*/
/**
* \brief    Add to the content of the license
* \details  License will conver this newly added project-feature combination. The activation key is automatically loaded from config file
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	ProjectId - number that is unique for each Siemens software product / project
* \param	FeatureId - number that is unique for each feature within a project
* \return	Error code. 0 is no error
*/
/*
int License::AddProjectFeature(int ProjectId, int FeatureId)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_FEATURE_ADDED;

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;
	
	DisableQuery = 1;

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
	
	int ret = AddProjectFeature(ProjectId, FeatureId, ActivationKey);

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//fall through
	return ret;
}*/

/**
* \brief    Add to the content of the license
* \details  License will conver this newly added project-feature combination. The activation key is automatically loaded from config file
* \author   Jozsa Istvan
* \date     2017-03-20
* \param	ProjectId - number that is unique for each Siemens software product / project
* \param	FeatureId - number that is unique for each feature within a project
* \return	Error code. 0 is no error
*/
int License::AddProjectFeature(int ProjectId, int FeatureId, const char *ActivationKey)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_FEATURE_ADDED;

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;

	DisableQuery = 1;

	//sanity checks
	if (ActivationKey == NULL)
		return ERROR_INVALID_PARAMETER;

	//create a new storable node
	LicenseNode *NewNode = new LicenseNode;
	memset(NewNode, 0, sizeof(LicenseNode));
	NewNode->SetProjectId( ProjectId );
	NewNode->SetFeatureId( FeatureId );
	NewNode->SetKey(ActivationKey);
/*	errno_t er = strcpy_s_ran(NewNode->ActivateKey, sizeof(NewNode->ActivateKey), ActivationKey);
	//wow, we should never trigger this
	if (er)
	{
		Log(LL_ERROR, "Could not copy string");
		delete NewNode;
		return er;
	}*/

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
	return NO_ERROR;
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
int License::SetDuration(time_t pStartDate, unsigned int pDuration, unsigned int pGraceDuration)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;

	DisableQuery = 1;

	//usage flags might get added to logs
	UsageStateFlags |= LSF_DURATION_IS_SET;

	//sanity checks
/*	int LostSeconds = (signed int)(GetUnixtime() - pStartDate);
	if (LostSeconds > 60 * 60)
		Log(LL_DEBUG_INFO, "Start date is smaller than current date. Loosing %d seconds from durtation %d", LostSeconds, pDuration);
	if (LostSeconds > (signed int)pDuration)
	{
		Log(LL_DEBUG_INFO, "Duration is too small. Readjusting start time to current time to avoid negative duration");
		pStartDate = GetUnixtime();
	} */

	StartStamp = pStartDate;
	Duration = pDuration;
	GracePeriod = pGraceDuration;

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
		NodeList->PushData((char*)&StartStamp, sizeof(StartStamp), DB_LICENSE_START);
		NodeList->PushData((char*)&Duration, sizeof(Duration), DB_LICENSE_DURATION);
		NodeList->PushData((char*)&GracePeriod, sizeof(GracePeriod), DB_GRACE_PERIOD);
		time_t LicenseExpire = StartStamp + Duration;
		NodeList->PushData((char*)&LicenseExpire, sizeof(LicenseExpire), DB_LICENSE_END);
	}

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
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
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_FEATURE_REQUESTED;

	if (DisableQuery != 0)
		return ERROR_DISABLED_FUNCTION;

	//sanity checks 
	if (StoreResult == NULL || MaxResultSize <= 0)
	{
		Log(LL_DEBUG_INFO, "Invalid usage of GetActivationKey API");
		return ERROR_INVALID_ADDRESS;
	}

	//just in case for any reason we return prematurly, we should provide a reply
	strcpy_s(StoreResult, MaxResultSize, "");

	time_t SecondsRemaining;
	time_t GracePeriodRemainingSeconds = 0;
	if (GetRemainingSeconds(&SecondsRemaining) != 0 || SecondsRemaining <= 0)
	{
		//trigger grace period if it has not been done before
		LicenseGracePeriod::UpdateStatus(GP_TRIGGER_GRACE_PERIOD, 0);
		//check if license is still in a valid grace period interval
		int er = IsGracePeriodTriggered(&GracePeriodRemainingSeconds);
		if (er != 0 || GracePeriodRemainingSeconds <= 0)
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
			if (CurNode->GetProjectId() == ProjectId && CurNode->GetFeatureId() == FeatureId)
			{
				size_t KeyLen = CurNode->GetKeyLen();
				if (KeyLen >= HardCodedStringLimitLic)
				{
					Log(LL_ERROR, "License is corrupted. Skipping key fetch");
					return ERROR_BUFFER_OVERFLOW;
				}
				if (KeyLen >= (size_t)MaxResultSize)
				{
					Log(LL_ERROR, "Activation key does not fit into return buffer");
					return ERROR_BUFFER_OVERFLOW;
				}
				// should make a change only when hardware configuration comes back online or a new valid license is getting used
				if (GracePeriodRemainingSeconds == 0)
					LicenseGracePeriod::UpdateStatus(GP_RESET_GRACE_PERIOD, (int)GracePeriod);
				// should make a change only when a new valid license is getting used
				LicenseGracePeriod::UpdateStatus(GP_SET_LICENSE_END, (int)StartStamp + Duration);
				//return the activation key
//				errno_t er = strcpy_s(StoreResult, MaxResultSize, CurNode->ActivateKey);
				CurNode->GetKey(StoreResult, MaxResultSize);
				return 0;
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
int	License::GetRemainingSeconds(time_t *RemainingSeconds)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_DURATION_REQUESTED;

	if (DisableQuery != 0)
		return ERROR_DISABLED_FUNCTION;

	DataCollectionIterator itr;
	itr.Init(NodeList);
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
		Log(LL_HACKING, "License duration mismatch");
		*RemainingSeconds = 0;
		return ERROR_LICENSE_EXPIRED;
	}
#endif

	//remaining seconds can become negative !
	if (Duration == PERMANENT_LICENSE_MAGIC_DURATION )
		*RemainingSeconds = PERMANENT_LICENSE_MAGIC_DURATION;
	else if (StartStamp > GetUnixtime())
		*RemainingSeconds = Duration;
	else
		*RemainingSeconds = (StartStamp + Duration) - GetUnixtime();

	if (*RemainingSeconds != 0)
		return NO_ERROR;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return WARNING_MISSING_LICENSE_FIELD;
}
/**
* \brief    Check if license is using grace period right now
* \details  There is a chance license will trigger using grace period even while it is active. This function will signal that scenario
* \author   Jozsa Istvan
* \date     2017-04-05
* \param	Result - 0 if license is either expired or not using grace period. 1 if license is using grace period
* \return	Error code. 0 is no error
*/
int License::IsGracePeriodTriggered(time_t *RemainingSeconds)
{
	//usage flags might get added to logs
	UsageStateFlags |= LSF_GRACE_STATUS_QUERIED;

	if (RemainingSeconds == NULL)
		return ERROR_MISSING_PARAMETER;

	// make sure return value is initialized
	*RemainingSeconds = 0;

	time_t RemainingSec = 0;
	int er = GetRemainingSeconds(&RemainingSec);
	if (er != 0 && er != ERROR_LICENSE_EXPIRED)
	{
		Log(LL_ERROR, "Could not get time info from license.dat");
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
			Log(LL_ERROR, "Could not get grace info");
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

	return NO_ERROR;
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
int	License::SaveToFile(const char *FileName, const char *Key, const int KeyLen, int Append)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//usage flags might get added to logs
	UsageStateFlags |= LSF_SAVED_TO_FILE;

	if (DisableEdit != 0)
		return ERROR_DISABLED_FUNCTION;

	//sanity checks
	if (FileName == NULL)
		return ERROR_BAD_PATHNAME;

	if (Key == NULL || KeyLen == 0)
		return ERROR_MISSING_ENCRYPTION_KEY;

	int TotalSize = NodeList->GetDataSize();
	unsigned char *TempBuff = new unsigned char[TotalSize];
	unsigned int EncryptSalt = 0;

	//if we have a fingerprint file than we can use it for simetric encryption
	NodeList->SetEncription(DCE_EXTERNAL_FINGERPRINT);
	memcpy(TempBuff, NodeList->GetData(), TotalSize);
	EncryptSalt = GenerateSaltKey(StartStamp, Duration);
	int er = EncryptWithFingerprintContent((unsigned char*)Key, KeyLen, (unsigned int)EncryptSalt, TempBuff, TotalSize);
	if (er != 0)
	{
		delete TempBuff;
		return er;
	}

	//dump the content to a file
	//open file
	FILE *f;
	errno_t erf;

	if (Append != 0)
		erf = fopen_s(&f, FileName, "ab"); //can stack multiple licenses into a single file
	else
		erf = fopen_s(&f, FileName, "wb");

	if (erf != NO_ERROR)
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
	fwrite(&TotalSize, 1, sizeof(int), f);
	fwrite(&EncryptSalt, 1, sizeof(unsigned int), f);
	fwrite(TempBuff, 1, TotalSize, f);
	fclose(f);

	delete TempBuff;

	Log(LL_DEBUG_ALL, "License succesfully saved to %s", FileName);
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	return NO_ERROR;
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
int	License::SaveToFile(const char *FileName, const char *FingerprintFilename, int Append)
{
	if (FingerprintFilename == NULL)
		return ERROR_BAD_PATHNAME;

	//if we have a fingerprint file than we can use it for simetric encryption
	ComputerFingerprint CF;
	if (CF.LoadFingerprint(FingerprintFilename) != 0)
		return ERROR_BAD_PATHNAME;

	//load the encryption key from file
	char *EncryptKey;
	int KeyLen;
	if (CF.DupEncryptionKey(&EncryptKey, KeyLen) != 0)
		return ERROR_BAD_ARGUMENTS;

	int er = SaveToFile(FileName, EncryptKey, KeyLen, Append);

	FreeDup(EncryptKey);

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
int	License::LoadFromFile(const char *FileName)
{
	return LoadFromFile(FileName, NULL);
}

//forward declaration of function to get filesize in bytes
long GetFileSize(const char *filename);

/**
* \brief    Load the content of a license from a buffer.
* \details  Buffer the content of the license into internal storage container
* \author   Jozsa Istvan
* \date     2017-04-12
* \param	TempBuff - Buffer containing an encrypted license
* \param	TotalSize - size of the buffer
* \param	EncryptSalt - Salt for the encryption key
* \param	FingerprintFilename - a file ( or NULL ) containing the decode key
* \return	Error code. 0 is no error
*/
int	License::InitFromBuffer(char *TempBuff, int TotalSize, unsigned int EncryptSalt, char *Key, int KeyLen)
{

#ifdef _DEBUG		// test random bad input
	if (ENABLE_ERROR_PRONE_TESTS == 1)
	{
		static int TempBufLoc = 0;
		if (TempBufLoc < TotalSize)
			TempBuff[TempBufLoc] = ~TempBuff[TempBufLoc];
		TempBufLoc++;
	}
#endif

	//backup in case we mess up decryption
	char *TempBuffBackup = new char[TotalSize];
	memcpy(TempBuffBackup, TempBuff, TotalSize);

	//deobfuscate
	int erDecrypt = 0;
	if (EncryptSalt)
		erDecrypt = EncryptWithFingerprintContent((unsigned char*)Key, KeyLen, EncryptSalt, (unsigned char*)TempBuff, TotalSize);

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
		if (erFingerprint == 0 && FingerprintSize > 0)
		{
			NodeList->PushData(TempBuff, TotalSize, DB_RAW_FULL_LIST_CONTENT);
			int erDecrypt = EncryptWithFingerprintContent(Temp, FingerprintSize, EncryptSalt, (unsigned char*)TempBuff, TotalSize);
			if (erDecrypt != 0)
			{
				delete TempBuffBackup;
				return erDecrypt;
			}
			//if there was no decrypt err with grace key, reinitialize the new list. If this fails, we have no backup plan
			NodeList->PushData(TempBuff, TotalSize, DB_RAW_FULL_LIST_CONTENT);
		}
	}

	// we do not need these anymore
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
	if (GetRemainingSeconds(&TempRemainingSeconds) == 0)
	{
		time_t ExpectedSaltKey = GenerateSaltKey(StartStamp, Duration);
		if (ExpectedSaltKey != EncryptSalt)
		{
			NodeList->DisposeData();
			Log(LL_HACKING, "Encryption mismatch while loading license content");
			return ERROR_LICENSE_INVALID;
		}
	}
#endif

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
int	License::LoadFromFile(const char *FileName, char *FingerprintFilename)
{
	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);

	//deny API calls that can change our internal state
	DisableEdit = 1;

	//usage flags might get added to logs
	UsageStateFlags |= LSF_LOADED_FROM_FILE;

	//sanity check
	if (FileName == NULL)
		return ERROR_FILE_INVALID;

	//if we have a fingerprint file than we can use it for simetric encryption
	ComputerFingerprint CF;
	if (FingerprintFilename != NULL && CF.LoadFingerprint(FingerprintFilename) != 0)
		return ERROR_BAD_PATHNAME;
	else if (CF.GenerateFingerprint() != 0)
		return ERROR_FILE_INVALID;

	//load the encryption key from file
	char *EncryptKey;
	int KeyLen;
	if (CF.DupEncryptionKey(&EncryptKey, KeyLen) != 0)
		return ERROR_BAD_ARGUMENTS;

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
		char buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		Log(LL_ERROR, "Could not load license. Error opening file %s/%s", buffer, FileName);
		return ERROR_FILE_INVALID;
	}

	int FoundDecodableLicense = 0;
	size_t TotalBytesRead = 0;
	size_t FileSize = GetFileSize(FileName);

	do{
		size_t BytesRead;
		unsigned int EncryptSalt = 0;
		int TotalSize;

		//get the amount of memory we need to store
		BytesRead = fread(&TotalSize, 1, sizeof(int), f);
		TotalBytesRead += BytesRead;
		if (BytesRead < sizeof(int))
		{
			Log(LL_ERROR, "Could not read license. File too small", FileName);
			fclose(f);
			return ERROR_FILE_INVALID;
		}

		if (TotalSize <= 0 || TotalSize >= MAX_LICENSE_SIZE_BYTES)
		{
			Log(LL_ERROR, "License file has invalid format or data");
			fclose(f);
			return ERROR_FILE_INVALID;
		}

		BytesRead = fread(&EncryptSalt, 1, sizeof(unsigned int), f);
		TotalBytesRead += BytesRead;
		if (BytesRead < sizeof(unsigned int))
		{
			Log(LL_ERROR, "Could not read license. File too small", FileName);
			fclose(f);
			return ERROR_FILE_INVALID;
		}

		//read the whole thing into memory
		char *TempBuff = new char[TotalSize];
		BytesRead = fread(TempBuff, 1, TotalSize, f);
		TotalBytesRead += BytesRead;
		if ((int)BytesRead < TotalSize)
		{
			Log(LL_ERROR, "Could not read license. File too small", FileName);
			fclose(f);
			return ERROR_FILE_INVALID;
		}

		FoundDecodableLicense = !InitFromBuffer(TempBuff, TotalSize, EncryptSalt, EncryptKey, KeyLen);
		delete TempBuff;
	} while (!feof(f) && TotalBytesRead < FileSize && FoundDecodableLicense==0);
	//done with the FILE
	fclose(f);

	FreeDup(EncryptKey);

	if (FoundDecodableLicense == 0)
		return ERROR_FILE_INVALID;

	Log(LL_DEBUG_DEBUG_ONLY, "%s-%d-%s", __FILE__, __LINE__, __FUNCTION__);
	//all good
	return NO_ERROR;
}
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

	return NO_ERROR;
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
