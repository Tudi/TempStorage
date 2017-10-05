#include "stdafx.h"
#include "ComputerFingerprint.h"
#include "DataPacker.h"
#include <winerror.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "GetInterfaceMacs.h"
#include "DumpSMBIOS.h"
#include "VMTools.h"
#include "SSLClient\RemoteInfo_API.h"
#include "Config\ConfigLoader.h"

#include <DSRole.h>
#pragma comment(lib, "netapi32.lib")

#define DLL_EXPORT

ComputerFingerprint *CreateComputerFingerprint()
{
	return new ComputerFingerprint;
}

void DestroyComputerFingerprint(ComputerFingerprint **Deleteme)
{
	if (*Deleteme == NULL)
		return;
	delete *Deleteme;
	*Deleteme = NULL;
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
/**
* \brief   Gather computer hardware specific info
* \details  List of hardware IDs that can uniquely identify a PC
* \author   Jozsa Istvan
* \date     2017-05-08
* \return	Error code. 0 is no error
*/
int	ComputerFingerprint::GenerateFingerprint()
{
	if (FingerprintData == NULL)
		return ERROR_INVALID_ADDRESS;

	FingerprintData->DisposeData();

	//check if VM and Append remote UUID first
	if (Detect_VM())
	{
		DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " VM detected" << std::endl;);
		//load remote server details
		char	ConfigIP[20];
		int		ConfigPort;
		if (GetStrConfig("config.txt", "QueryServiceIP", ConfigIP, sizeof(ConfigIP)))
		{
			DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Could not load config options" << std::endl;);
			return ERROR_COULD_NOT_LOAD_CONFIG_FILE;
		}
		if (GetIntConfig("config.txt", "QueryServicePort", &ConfigPort))
		{
			return ERROR_COULD_NOT_LOAD_CONFIG_FILE;
		}
		//fetch data we need from remote server
		char RemoteUUID[16];
		int err = GetRemoteUUID(ConfigIP, ConfigPort, RemoteUUID, sizeof(RemoteUUID));
		if (err != 0)
		{
			DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " could not get remote UUID from " << ConfigIP << ":" << ConfigPort << std::endl;);
//			return err;	// do not exit generating fingerprint just because one key fails, maybe rest of the keys succeed and we can enter grace period
		}
		else
		{
			//append remote data to our list
			err = AppendRemoteUUID(RemoteUUID, sizeof(RemoteUUID));
			if (err != 0)
			{
				DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Unexpected buffer add error " << std::endl;);
				return err;
			}
		}
	}

	int err = GenerateFingerprint_Local();

	return err;
}

int	ComputerFingerprint::GenerateFingerprint_Local()
{
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
/**
* \brief   Gather optional computer specific info
* \details  Custom strings added to the fingerprint to help license generator identify the PC
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	MachineRole - String to identify the role of the PC
* \param	ClientName - Person name that is a customer for a specific project
* \return	Error code. 0 is no error
*/
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
			if( FingerprintData->PushData(info->DomainNameDns, (int)wcslen(info->DomainNameDns) * 2, DB_DOMAIN_NAME) != 0)
				return ERROR_CF_CONTENT_INVALID;
			if( FingerprintData->PushData(info->DomainNameFlat, (int)wcslen(info->DomainNameFlat) * 2, DB_NETBIOS_NAME) != 0)
				return ERROR_CF_CONTENT_INVALID;
		}
		DsRoleFreeMemory(info);
	}

	if (MachineRole!=NULL && FingerprintData->PushData(MachineRole, (int)strlen(MachineRole) + 1, DB_MACHINE_ROLE) != 0 )
		return ERROR_CF_CONTENT_INVALID;

	if (ClientName!=NULL && FingerprintData->PushData(ClientName, (int)strlen(ClientName) + 1, DB_CLIENT_NAME ) != 0 )
		return ERROR_CF_CONTENT_INVALID;

	return 0;
}

//more fields that came after initial design
/**
* \brief   Gather optional computer specific info
* \details  Custom strings added to the fingerprint to help license generator identify the PC
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	MachineRole - String to identify the role of the PC
* \param	ClientName - Person name that is a customer for a specific project
* \return	Error code. 0 is no error
*/
int ComputerFingerprint::AppendRemoteUUID(const char *RemoteUUID, int UUIDSize)
{
	if (FingerprintData->PushData(RemoteUUID, UUIDSize, DB_UUID_REMOTE) != 0)
		return ERROR_CF_CONTENT_INVALID;
	return 0;
}

/**
* \brief   Save the fingerprint content to file. 
* \details  File content will be encoded to make it harder for a text editor to interpret or temper content
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	FileName - Name and path of the file where the content will be saved to
* \return	Error code. 0 is no error
*/
int ComputerFingerprint::SaveFingerprint(const char *FileName)
{
	return FingerprintData->SaveToFile(FileName);
}
/**
* \brief   Load the fungerprint content from a file.
* \details  Load and decode file content
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	FileName - Name and path of the file where the content will be saved to
* \return	Error code. 0 is no error
*/
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
/**
* \brief   Initialize internal state based on a decoded buffer stored in file or DataBase
* \details  Load and decode file content
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	buf - pointer to a fingerprint stored in memory
* \param	size - size of the buffer
* \return	Error code. 0 is no error
*/
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
/**
* \brief   Print out fingerprint field contents in a humanly readable way.
* \details  Only some of the fields can be printed out. Unknown fields will be printed out in raw format
* \author   Jozsa Istvan
* \date     2017-05-08
*/
void ComputerFingerprint::Print()
{
	FingerprintData->PrintContent();
}

/**
* \brief   Generate an encryption key from the content of the fingerprint
* \details  Optional data is skipped. Esential data is organized so it will generate the same encryption key even if hardware order changes ( network cards )
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	Key - Point of pointer where to store the encryption key
* \param	Len - Length of the encryption key
* \return	Error code. 0 is no error
*/
int ComputerFingerprint::DupEncryptionKey(char **Key, int *Len)
{
	//sanity checks
	if (Key == NULL)
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
		if (IsDBFingerprintEssential(Type))
			SumSize += Size;
	}
	//our fingerprint is empty ? What and how ?
	if (SumSize == 0)
	{
		*Len = 0;
		*Key = NULL;
		return ERROR_GENERIC;
	}

	//assign internal state as encrypt key
	*Len = SumSize; // savage. later will work on it
	char *DestBuf = (char*)malloc(SumSize);
	*Key = DestBuf;

	//everything went well ?
	if (*Key == NULL || *Len == 0)
		return ERROR_GENERIC;

	//now extract info we are interested in. Optional nodes are not used for encryption
	SumSize = 0;
	itr.Init(FingerprintData);
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (IsDBFingerprintEssential(Type))
		{
			memcpy(&DestBuf[SumSize], (char*)Data, Size);
			SumSize += Size;
		}
	}

	//all ok
	return 0;
}
/**
* \brief   Duplicate a single field content
* \details  Memory is allocated and the content of the field is copied into the new buffer
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	Content - Point of pointer where to store the field content
* \param	Size - Length of the field data
* \param	Type - field type
* \return	Error code. 0 is no error
*/
int ComputerFingerprint::DupField(char **Content, int *Size, int Type)
{
	if (Content == NULL || Size == NULL)
		return ERROR_BAD_ARGUMENTS;

	int ret = FingerprintData->GetDataDup(Content, Size, Type);

	return ret;
}
/**
* \brief   Duplicate every field of the fingerprint collection
* \details  duplicate the fingerprint store
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	Content - Point of pointer where to store the field content
* \param	Len - Length of all the fields
* \return	Error code. 0 is no error
*/
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

/**
* \brief   Generate an encryption key from the content of the fingerprint
* \details  Optional data is skipped. Esential data is organized so it will generate the same encryption key even if hardware order changes ( network cards )
* \author   Jozsa Istvan
* \date     2017-05-08
* \param	Key - Point of pointer where to store the encryption key
* \param	Len - Length of the encryption key
* \param	Ref - Reference list. If reference list is very similar, use that list to generate the encryption key
* \return	Error code. 0 is no error
*/
int ComputerFingerprint::DupEncryptionKey(char **Key, int *Len, GenericDataStore *Ref)
{
	//sanity checks
	if (Key == NULL)
		return ERROR_BAD_ARGUMENTS;

	//calculate amount of buffer we need to store the encryption key
	DataCollectionIterator itr;
	itr.Init(Ref);
	char *Data;
	int Size;
	int Type;
	int MissingNodeCount = 0;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (FingerprintData->HasNode(Data, Size, Type) == 0)
		{
			DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Missing node : " << Type << ":" << Size << std::endl;);
			MissingNodeCount++;
		}
	}

	//too many nodes are missing, we can not generate a decryption key for this computer
	if (MissingNodeCount > 1)
	{
		DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Total missing node count : " << MissingNodeCount << std::endl;);
		return ERROR_GENERIC;
	}

	//reinit our CF list with proper data
	DEBUG_FP_PRINT(std::cout << __FILE__ << ":" << __LINE__ << " Found enough nodes to trigger grace period and decode " << std::endl;);
	FingerprintData->PushData(Ref->GetData(), Ref->GetDataSize(), DB_RAW_FULL_LIST_CONTENT);

	//all ok
	return DupEncryptionKey(Key, Len);
}

int  ComputerFingerprint::DupEncryptionList(char **List, int *Len)
{
	//sanity checks
	if (List == NULL)
		return ERROR_BAD_ARGUMENTS;

	GenericDataStore FData;
	//dump only relevant data into the new list
	DataCollectionIterator itr;
	itr.Init(FingerprintData);
	char *Data;
	int Size;
	int Type;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (IsDBFingerprintEssential(Type))
			FData.PushData(Data, Size, Type);
	}

	//our fingerprint is empty ? What and how ?
	if (FData.GetDataSize() == 0)
		return ERROR_GENERIC;

	//assign internal state as encrypt key
	*Len = FData.GetDataSize(); // savage. later will work on it
	char *DestBuf = (char*)malloc(FData.GetDataSize());
	*List = DestBuf;

	//everything went well ?
	if (*List == NULL || Len == 0)
		return ERROR_GENERIC;

	//dump the whole list into the new buffer
	memcpy(*List, FData.GetData(), FData.GetDataSize());

	//all ok
	return 0;
}