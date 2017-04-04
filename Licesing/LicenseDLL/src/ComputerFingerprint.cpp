#include "../stdafx.h"
#include "ComputerFingerprint.h"
#include "DataPacker.h"
#include <winerror.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "GetInterfaceMacs.h"
#include "DumpSMBIOS.h"

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
			FingerprintData->PushData((char*)&MAC, sizeof(__int64), DB_MAC_ADDRESS);
		}
	}

	//get the CPU id
	{
		int cpuid[4];
		__cpuid(cpuid, 0);
		FingerprintData->PushData((char*)cpuid, sizeof(cpuid), DB_CPU_ID);
	}

	//get BIOS UUID
	{
		unsigned char UUID[16];
		GetBiosUUID(UUID, sizeof(UUID));
		FingerprintData->PushData((char*)UUID, sizeof(UUID), DB_UUID);
	}

	// mother board SN
	{
		unsigned char SN[255];
		GetMotherBoardSN(SN, sizeof(SN));
		FingerprintData->PushData((char*)SN, sizeof(SN), DB_MB_SN);
	}

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

void ComputerFingerprint::Print()
{
	FingerprintData->PrintContent();
}

// this is the lasy solution, later extract plain content
int ComputerFingerprint::GetEncryptionKey(char **Key, int &Len)
{
	//sanity checks
	if (*Key == NULL)
		return ERROR_BAD_ARGUMENTS;

	//assign internal state as encrypt key
	*Key = (char*)FingerprintData->GetData();
	Len = FingerprintData->GetDataSize(); // savage. later will work on it

	//everything went well ?
	if (*Key == NULL || Len == 0)
		return ERROR;

	//all ok
	return 0;
}
