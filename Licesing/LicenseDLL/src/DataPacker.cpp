#include "../stdafx.h"
#include "DataPacker.h"
#include "Encryption.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

void Init(DataCollectionHeader *&Data)
{
	if (Data == NULL)
		Data = (DataCollectionHeader *)(new char[sizeof(DataCollectionHeader)]);
	Data->Count = 0;
	Data->Size = sizeof(DataCollectionHeader);
	Data->Ver = CURRENT_PACKER_VERSION;
	Data->EncryptType = DCE_INTERNAL_CyclicXOR_KEY;
	Data->Blocks[0] = 0;	/// should remain always 0 unless we mess something up
}

GenericDataStore::GenericDataStore()
{
	Data = NULL;
	Init(Data);
	HaveBuffToStore = 0;
}

GenericDataStore::~GenericDataStore()
{
	DisposeData();
}

void GenericDataStore::DisposeData()
{
	if (Data)
	{
		delete Data;
		Data = NULL;
	}
	HaveBuffToStore = 0;
}

int GenericDataStore::EnsureCanStore(int Required)
{
	// do we need to allocate a new chunk ?
	if (HaveBuffToStore > Required)
		return DALLOC_SUCCESS;

	//make sure we have an initial state to avoid multiple special checks
	if (Data == NULL)
		Init(Data);

	// almost like a copy constructor :P
	int SizeNow = Data->Size;
	int NewSize = SizeNow + Required + ALLOC_BUFFER_EXTRA_SIZE;

	//get new store
	DataCollectionHeader *TData = (DataCollectionHeader *)(new char[NewSize]);
	if (TData == NULL)
		return DALLOC_FAIL;

	// copy from old store
	memcpy(TData, Data, SizeNow);

	// destroy old store
	delete Data;

	// assign new store to old store
	Data = TData;
	HaveBuffToStore = NewSize;

	return DALLOC_SUCCESS;
}

int GenericDataStore::PushData(char *buff, int Size, int Type)
{
	// sanity check
	if (Size <= 0 || buff == NULL || Type >= DB_MAX_TYPES || Type <= DB_INVALID_UNINITIALIZED)
		return DADD_FAIL;

	// make sure we can add it
	int AddedSize = Size + sizeof(DataBlockHeader);
	if (EnsureCanStore(AddedSize) != 0)
		return DADD_FAIL;

	if (Type == DB_RAW_FULL_LIST_CONTENT)
	{
		memcpy(Data, buff, Size);
	}
	else
	{
		//init block store
		DataBlockHeader *Store = (DataBlockHeader *)(&Data->Blocks[Data->Size - sizeof(DataCollectionHeader)]);
		memcpy(Store->Data, buff, Size);
		Store->Size = Size;
		Store->Type = (char)Type;

		//add it to list
		Data->Count++;
		Data->Size += AddedSize;
		HaveBuffToStore -= AddedSize;
	}

	//all went great
	return DADD_SUCCESS;
}

int GenericDataStore::SaveToFile(const char *FileName)
{
	//sanity check
	if (FileName == NULL)
		return ERROR_FILE_INVALID;

	if (Data == NULL)
		return ERROR_BAD_CONFIGURATION;

	//open file
	FILE *f;
	errno_t er = fopen_s( &f, FileName, "wb");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//dump the content
	int TotalSize = Data->Size;
		
	//apply obfuscation. This method ads very little security complexity. It simply hides the content of the data from a humanly readable format !
	unsigned char *TempBuff = new unsigned char[TotalSize];
	if (Data->EncryptType == DCE_INTERNAL_CyclicXOR_KEY)
	{
		Data->XORKey = (GetTickCount() << 16) ^ (unsigned int)time(NULL);	// should be "unqique" every time we generate a new list
		memcpy(TempBuff, Data, TotalSize);
		EncryptBufferXORKeyRotate(&TempBuff[sizeof(DataCollectionHeader)], TotalSize - sizeof(DataCollectionHeader), Data->XORKey);
	}
	else
		memcpy(TempBuff, Data, TotalSize);

	//dump content
	fwrite(&TotalSize, 1, sizeof(int), f);
	fwrite(TempBuff, 1, TotalSize, f);
	fclose(f);

	delete[] TempBuff;

	return 0;
}

int GenericDataStore::LoadFromFile(const char *FileName)
{
	//sanity check
	if (FileName == NULL)
		return ERROR_FILE_INVALID;

	if (Data == NULL)
		return ERROR_BAD_CONFIGURATION;

	//open file
	FILE *f;
	errno_t er = fopen_s(&f, FileName, "rb");

	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//get the amount of memory we need to store
	int TotalSize;
	fread(&TotalSize, 1, sizeof(int), f);

	//make sure we can store it
	EnsureCanStore(TotalSize);

	//read the whole thing into memory
	fread(Data, 1, TotalSize, f);

	fclose(f);

	//deobfuscate
	if (Data->EncryptType == DCE_INTERNAL_CyclicXOR_KEY)
		EncryptBufferXORKeyRotate((unsigned char*)Data+sizeof(DataCollectionHeader), TotalSize - sizeof(DataCollectionHeader), Data->XORKey);

	return 0;
}

void GenericDataStore::PrintContent()
{
	DataCollectionIterator it;
	it.Init(Data);
	char *buff;
	int Type;
	int Size;
	int Index = 0;
	while (it.GetNext(&buff, Size, Type) == DCI_SUCCESS)
	{
		printf("%d)", Index++);
		if (Type == DB_MAC_ADDRESS)
		{
			printf("MAC : \t\t");
			for (int i = 0; i < Size; i++)
				printf("%02X ", (unsigned char)buff[i]);
		}
		else if (Type == DB_CPU_ID)
		{
			printf("CPU ID : \t\t");
			int *t = (int*)buff;
			for (unsigned int i = 0; i < Size / sizeof(int); i++)
				printf("%08X ", t[i]);
		}
		else if (Type == DB_UUID)
		{
			printf("UUID : \t\t");
			for (unsigned int i = 0; i < Size / sizeof(char); i++)
				printf("%02X ", (unsigned char)buff[i]);
		}
		else if (Type == DB_MB_SN)
		{
			printf("Mothernoard SN : \t%s", buff);
		}
		else
		{
			printf("Unk block type %d with size : \t%d", Type, Size);
		}
		printf("\n");
	}
}

int	GenericDataStore::SetEncription(unsigned char EncryptType)
{
	//sanity checks
	if (EncryptType <= DCE_NOT_INITIALIZED || EncryptType >= DCE_INVALID_ENCRYPTION_TYPE)
		return 1;

	if (Data == NULL)
		return ERROR_BAD_CONFIGURATION;

	// we will use the value later
	Data->EncryptType = EncryptType;

	//all went fine
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

DataCollectionIterator::DataCollectionIterator()
{
	IterateWhat = NULL;
	NextData = NULL;
	AtBlock = 0;
}

void DataCollectionIterator::Init(DataCollectionHeader *pIterateWhat)
{
	//sanity checks
	if (pIterateWhat == NULL)
		return;
	if (pIterateWhat->Count <= 0)
		return;
	if (pIterateWhat->Ver != CURRENT_PACKER_VERSION)
		return;
	IterateWhat = pIterateWhat;
	NextData = pIterateWhat->Blocks;
}

int DataCollectionIterator::GetNext(char **Data, int &Size, int &Type)
{
	// sanity checks
	if (IterateWhat == NULL)
		return DCI_ERROR;
	if (AtBlock >= (int)IterateWhat->Count)
		return DCI_NO_MORE_DATA;

	//return data
	DataBlockHeader *DataHeader = (DataBlockHeader *)NextData;
	*Data = (char*)DataHeader->Data;
	Size = DataHeader->Size;
	Type = DataHeader->Type;

	//increase our iterator
	AtBlock++;
	NextData += DataHeader->Size + sizeof(DataBlockHeader);

	//all done
	return DCI_SUCCESS;
}