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
}

GenericDataStore::GenericDataStore()
{
	Data = NULL;
	Init(Data);
	XORSeed = ( GetTickCount() << 16 ) ^ (unsigned int )time(NULL);	// should be "unqique" every time we generate a new list
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

	//init block store
	DataBlockHeader *Store = (DataBlockHeader *)(&Data->Blocks[Data->Size - sizeof(DataCollectionHeader)]);
	memcpy(Store->Data, buff, Size);
	Store->Size = Size;
	Store->Type = (char)Type;

	//add it to list
	Data->Count++;
	Data->Size += AddedSize;
	HaveBuffToStore -= AddedSize;

	//all went great
	return DADD_SUCCESS;
}

int GenericDataStore::SaveToFile(char *FileName)
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
	int TotalSizeInt = TotalSize / sizeof(int);
		
	//apply obfuscation
	int *TempBuff = new int[TotalSizeInt];
	memcpy(TempBuff, Data, TotalSize);
	EncryptBufferXORKeyRotate((unsigned char*)TempBuff, TotalSizeInt, XORSeed);

	//dump content
	fwrite(&TotalSize, 1, sizeof(int), f);
	fwrite(&XORSeed, 1, sizeof(int), f);
	fwrite(TempBuff, 1, TotalSize, f);
	fclose(f);

	return 0;
}

int GenericDataStore::LoadFromFile(char *FileName)
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

	fread(&XORSeed, 1, sizeof(int), f);

	//make sure we can store it
	EnsureCanStore(TotalSize);

	//read the whole thing into memory
	fread(Data, 1, TotalSize, f);

	//deobfuscate
	int TotalSizeInt = TotalSize / sizeof(int);
	EncryptBufferXORKeyRotate((unsigned char*)Data, TotalSizeInt, XORSeed);

	fclose(f);
	
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
		printf("\n");
	}
}

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