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

	LIBRARY_API void DestroyComputerFingerprint(ComputerFingerprint *Deleteme)
	{
		delete Deleteme;
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

int ComputerFingerprint::SaveFingerprint(char *FileName)
{
	return FingerprintData->SaveToFile(FileName);
}

int ComputerFingerprint::LoadFingerprint(char *FileName)
{
	return FingerprintData->LoadFromFile(FileName);
}
/*
int ComputerFingerprint::EncodeLicense(char *License, int Len)
{
	return 0;
}

int ComputerFingerprint::DecodeLicense(char *License, int Len)
{
	return 0;
}
*/
void ComputerFingerprint::Print()
{
	FingerprintData->PrintContent();
}
