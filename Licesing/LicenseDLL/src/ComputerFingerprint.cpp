#include "../stdafx.h"
#include "ComputerFingerprint.h"
#include "DataPacker.h"
#include <winerror.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "GetInterfaceMacs.h"
#include "DumpSMBIOS.h"

#include <DSRole.h>
#pragma comment(lib, "netapi32.lib")

#define DLL_EXPORT

extern "C"
{
	LIBRARY_API ComputerFingerprint *CreateComputerFingerprint()
	{
		return new ComputerFingerprint;
	}

	LIBRARY_API void DestroyComputerFingerprint(ComputerFingerprint **Deleteme)
	{
		if (*Deleteme == NULL)
			return;
		delete *Deleteme;
		*Deleteme = NULL;
	}
}

ComputerFingerprint::ComputerFingerprint()
{
	FingerprintData = new GenericDataStore;
}

ComputerFingerprint::~ComputerFingerprint()
{
	if (FingerprintData)
	{
		delete FingerprintData;
		FingerprintData = NULL;
	}
}

int	ComputerFingerprint::GenerateFingerprint()
{
	if (FingerprintData == NULL)
		return ERROR_INVALID_ADDRESS;

	FingerprintData->DisposeData();

	// add MACs to the fingerprint
	{
		// get a list of MACs
		std::vector<__int64> vMacs;

		//get a list of MACs and push it into our store
		GetMacsWithIP(vMacs);

		//generate CRC32 for the MACs and order them so we can make sure we get the same list for every fingerprint request
		std::sort(vMacs.begin(), vMacs.end());

		for (std::vector<__int64>::iterator itr = vMacs.begin(); itr != vMacs.end(); itr++)
		{
			__int64 MAC = *itr;
			FingerprintData->PushData(&MAC, sizeof(__int64), DB_MAC_ADDRESS);
		}
	}

	//get the CPU id
	{
		int cpuid[4];
		__cpuid(cpuid, 0);
		FingerprintData->PushData(cpuid, sizeof(cpuid), DB_CPU_ID);
	}

	//get BIOS UUID
	{
		unsigned char UUID[16];
		GetBiosUUID(UUID, sizeof(UUID));
		FingerprintData->PushData(UUID, sizeof(UUID), DB_UUID);
	}

	// mother board SN
	{
		unsigned char SN[255];
		GetMotherBoardSN(SN, sizeof(SN));
		FingerprintData->PushData(SN, sizeof(SN), DB_MB_SN);
	}

	return 0;
}

//these are the fields that came after initial design
int ComputerFingerprint::AppendClientInfo(const char * MachineRole, const char *ClientName)
{

	// volume serial number. For windows only
	{
		DWORD serialNum = 0;
		GetVolumeInformation("c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0);
		if (FingerprintData->PushData(&serialNum, sizeof(serialNum), DB_C_SN) != 0)
			return ERROR_CF_CONTENT_INVALID;
	}

	// Computer name. Should be unique in workgroup
	{
		char computerName[1024];
		DWORD size = sizeof(computerName);
		GetComputerName(computerName, &size);
		if (FingerprintData->PushData(computerName, size + 1, DB_COMPUTER_NAME) != 0)
			return ERROR_CF_CONTENT_INVALID;
	}

	//get DNS info. Move this code to platform specific implementation
	{
		DSROLE_PRIMARY_DOMAIN_INFO_BASIC * info;
		DWORD dw = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE *)&info);
		if (dw == ERROR_SUCCESS && info->DomainNameDns != NULL)
		{
			if( FingerprintData->PushData(info->DomainNameDns, wcslen(info->DomainNameDns) * 2, DB_DOMAIN_NAME) != 0)
				return ERROR_CF_CONTENT_INVALID;
			if( FingerprintData->PushData(info->DomainNameFlat, wcslen(info->DomainNameFlat) * 2, DB_NETBIOS_NAME) != 0)
				return ERROR_CF_CONTENT_INVALID;
		}
		DsRoleFreeMemory(info);
	}

	if (MachineRole!=NULL && FingerprintData->PushData(MachineRole, strlen(MachineRole) + 1, DB_MACHINE_ROLE) != 0 )
		return ERROR_CF_CONTENT_INVALID;

	if (ClientName!=NULL && FingerprintData->PushData(ClientName, strlen(ClientName) + 1, DB_CLIENT_NAME ) != 0 )
		return ERROR_CF_CONTENT_INVALID;

	return 0;
}

int ComputerFingerprint::SaveFingerprint(const char *FileName)
{
	return FingerprintData->SaveToFile(FileName);
}

int ComputerFingerprint::LoadFingerprint(const char *FileName)
{
	int erLoad = FingerprintData->LoadFromFile(FileName);
	if (erLoad == 0)
	{
		if (FingerprintData->IsDataValid() == 0)
		{
			FingerprintData->DisposeData();
			return ERROR_CF_CONTENT_INVALID;
		}
	}
	return erLoad;
}

int ComputerFingerprint::LoadFingerprint(const char *buf, int size)
{
	FingerprintData->DisposeData();
	int erLoad = FingerprintData->PushData(buf, size, DB_RAW_FULL_LIST_CONTENT);
	if (erLoad == 0)
	{
		if (FingerprintData->IsDataValid() == 0)
		{
			FingerprintData->DisposeData();
			return ERROR_CF_CONTENT_INVALID;
		}
	}
	return erLoad;
}

void ComputerFingerprint::Print()
{
	FingerprintData->PrintContent();
}

// this is the lasy solution, later extract plain content
int ComputerFingerprint::DupEncryptionKey(char **Key, int &Len)
{
	//sanity checks
	if (*Key == NULL)
		return ERROR_BAD_ARGUMENTS;

	//calculate amount of buffer we need to store the encryption key
	int SumSize = 0;
	DataCollectionIterator itr;
	itr.Init(FingerprintData);
	char *Data;
	int Size;
	int Type;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_MAC_ADDRESS || Type == DB_CPU_ID || Type == DB_UUID)
			SumSize += Size;
	}
	//our fingerprint is empty ? What and how ?
	if (SumSize == 0)
		return ERROR_GENERIC;

	//assign internal state as encrypt key
	Len = SumSize; // savage. later will work on it
	char *DestBuf = (char*)malloc(SumSize);
	*Key = DestBuf;

	//everything went well ?
	if (*Key == NULL || Len == 0)
		return ERROR_GENERIC;

	//now extract info we are interested in. Optional nodes are not used for encryption
	SumSize = 0;
	itr.Init(FingerprintData);
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_MAC_ADDRESS || Type == DB_CPU_ID || Type == DB_UUID)
		{
			memcpy(&DestBuf[SumSize], (char*)Data, Size);
			SumSize += Size;
		}
	}

	//all ok
	return 0;
}

int ComputerFingerprint::DupField(char **Content, int *Size, int Type)
{
	if (Content == NULL || Size == NULL)
		return ERROR_BAD_ARGUMENTS;

	int ret = FingerprintData->GetDataDup(Content, Size, Type);

	return ret;
}

// this is the lasy solution, later extract plain content
int ComputerFingerprint::DupContent(char **Content, int *Len)
{
	//sanity checks
	if (Content == NULL || Len == NULL)
		return ERROR_BAD_ARGUMENTS;

	*Len = FingerprintData->GetDataSize(); // savage. later will work on it
	if (*Len == 0)
		return ERROR_GENERIC;

	char *DestBuf = (char*)malloc(1 + *Len);
	*Content = DestBuf;

	//everything went well ?
	if (*Content == NULL)
		return ERROR_GENERIC;

	memcpy(DestBuf, FingerprintData->GetData(), *Len);

	//all ok
	return 0;
}
