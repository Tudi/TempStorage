#include "stdafx.h"
#include <time.h>
#include "License.h"
#include "src/DataPacker.h"
#include "ProjectFeatureKeys.h"
#include <stdio.h>
#include "src/Encryption.h"

#define GenerateSaltKey(a,b) ((unsigned int)(1|(a+b)))

License::License()
{
	NodeList = new GenericDataStore;
	StartStamp = Duration = 0;//mark them uninitialized
}

License::~License()
{
	if (NodeList != NULL)
	{
		delete NodeList;
		NodeList = NULL;
	}
}

int License::AddProjectFeature(int ProjectId, int FeatureId)
{
	//check if this is a valid param
	ProjectFeatureKeyDB DB;
	if (DB.IsValidProjectID(ProjectId) == MISSING_OR_INVALID_ID)
	{
		printf("Warning:Found invalid project id %d while constructing license node\n", ProjectId);
	}
	char *ActivationKey = DB.FindProjectFeatureKey(ProjectId, FeatureId);
	if (ActivationKey == NULL)
	{
		printf("Error:Could not find a valid activation key for project %d and feature ID %d. Exiting\n", ProjectId, FeatureId);
		return ERROR_INVALID_PROJECT_FEAURE;
	}
	//create a new storable node
	LicenseNode *NewNode = new LicenseNode;
	memset(NewNode, 0, sizeof(LicenseNode));
	NewNode->ProjectId = ProjectId;
	NewNode->FeatureId = FeatureId;
	errno_t er = strcpy_s(NewNode->ActivateKey, sizeof(NewNode->ActivateKey), ActivationKey);
	//wow, we should never trigger this
	if (er)
		return er;

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
				printf("Warning:This node is already in the License list. There is no point adding it a seconds time.\n");
				break;
			}
		}
	}

	//Add new node to our store
	NodeList->PushData((char*)NewNode, sizeof(LicenseNode), DB_LICENSE_NODE);

	//no errors returned
	return 0;
}

int License::SetDuration(time_t pStartDate, unsigned int pDuration)
{
	//sanity checks
	int LostSeconds = (signed int)(time(NULL) - pStartDate);
	if (LostSeconds > 60 * 60)
		printf("WARNING:Start date is smaller than current date. Loosing %d seconds from durtation %d\n", LostSeconds, pDuration);
	if (LostSeconds > (signed int)pDuration)
	{
		printf("Warning:Duration is too small. Readjusting start time to current time to avoid negative duration");
		pStartDate = time(NULL);
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

	return 0;
}

char *License::GetActivationKey(int ProjectId, int FeatureId)
{
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
				return CurNode->ActivateKey;
		}
	}
	return NULL;
}

int	License::GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize)
{
	//sanity checks 
	if (StoreResult == NULL || MaxResultSize <= 0 )
		return ERROR_INVALID_ADDRESS;

	time_t SecondsRemaining;
	if (GetRemainingSeconds(SecondsRemaining) != 0 || SecondsRemaining==0)
		return WARNING_NO_LICENSE_FOUND;

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
					return ERROR_BUFFER_OVERFLOW;
				return strcpy_s(StoreResult, MaxResultSize, CurNode->ActivateKey);
			}
		}
	}

	//we did not find a valid license
	strcpy_s(StoreResult, MaxResultSize, "");
	return WARNING_NO_LICENSE_FOUND;
}

int	License::GetRemainingSeconds(time_t &RemainingSeconds)
{
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
			time_t CurTime = time(NULL);
			time_t *EndTime = (time_t *)Data;
			if (CurTime < *EndTime)
				RemainingSeconds = *EndTime - CurTime;
		}
		//we loaded data recently ? Update our internal state
		if (Type == DB_LICENSE_START && StartStamp == 0 && Size >= sizeof(time_t))
			StartStamp = *(time_t *)Data;
		if (Type == DB_LICENSE_DURATION && Duration == 0 && Size >= sizeof(unsigned int))
			Duration = *(unsigned int *)Data;
	}

	if (RemainingSeconds != 0)
		return 0;

	return WARNING_MISSING_LICENSE_FIELD;
}

int	License::SaveToFile(char *FileName, char *FingerprintFilename)
{
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
		EncryptWithFingerprint(FingerprintFilename, (unsigned int)EncryptSalt, TempBuff, TotalSize);
	}
	else
		memcpy(TempBuff, NodeList->GetData(), TotalSize);

	//dump the content to a file
	//open file
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "wb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//dump the content
	fwrite(&EncryptSalt, 1, sizeof(unsigned int), f);
	fwrite(&TotalSize, 1, sizeof(int), f);
	fwrite(TempBuff, 1, TotalSize, f);
	fclose(f);

	delete TempBuff;

	return 0;
}

int	License::LoadFromFile(char *FileName)
{
	//sanity check
	if (FileName == NULL)
		return ERROR_FILE_INVALID;

	//open file
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "rb");

	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	unsigned int EncryptSalt = 0;
	fread(&EncryptSalt, 1, sizeof(unsigned int), f);

	//get the amount of memory we need to store
	int TotalSize;
	fread(&TotalSize, 1, sizeof(int), f);

	//read the whole thing into memory
	char *TempBuff = new char[TotalSize];
	fread(TempBuff, 1, TotalSize, f);

	//deobfuscate
	if (EncryptSalt)
		DecryptWithFingerprint(NULL, EncryptSalt, (unsigned char*)TempBuff, TotalSize);

	//done with the FILE
	fclose(f);

	//make sure we can store it
	NodeList->DisposeData();
	NodeList->PushData(TempBuff, TotalSize, DB_RAW_FULL_LIST_CONTENT);

	delete TempBuff;

	//check for validity of the data we loaded
	time_t RemainingSeconds = 0;
	if (GetRemainingSeconds(RemainingSeconds) != 0 || RemainingSeconds == 0 || StartStamp == 0 || Duration == 0)
	{
		NodeList->DisposeData();
		return ERROR_LICENSE_EXPIRED_OR_INVALID;
	}
	time_t ExpectedSaltKey = GenerateSaltKey(StartStamp, Duration);
	if (ExpectedSaltKey != EncryptSalt)
	{
		NodeList->DisposeData();
		return ERROR_LICENSE_EXPIRED_OR_INVALID;
	}

	//all good
	return 0;
}
