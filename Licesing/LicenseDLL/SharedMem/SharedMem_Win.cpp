#include "SharedMem.h"
#include <windows.h>
#include "../Tools.h"

#pragma pack(push,1)
struct SharedMemVariableHeader
{
	unsigned long	NameHash;
	int				Size;
};
struct SharedMemVariable : SharedMemVariableHeader
{
	char data;
};
#pragma pack(pop)

#define SHARED_MEMORY_SIZE (32 * 1024)

#ifdef ROSES_ARE_READ_AND_WE_RUN_IN_RING_0
int GetSetValue(const char *SessionName, const char *VarName, char*Store, int Size, int Set)
{
	int CreatedMapFile = 0;

	HANDLE	hMapFile;
	char	*pBuf;

	//try to attach to an existing location
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		SessionName);          // name of mapping object

	//try to create a new one if we could not open it
	if (hMapFile == NULL)
	{
		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE | SEC_COMMIT,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			SHARED_MEMORY_SIZE,      // maximum object size (low-order DWORD)
			SessionName);            // name of mapping object

		if (hMapFile == NULL)
		{
			return GetLastError();
		}
		CreatedMapFile = 1;
	}

	//get acccess to the mapped file
	pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS,				 // read/write permission
		0,
		0,
		SHARED_MEMORY_SIZE);

	if (pBuf == NULL)
	{
		DWORD er = GetLastError();
		CloseHandle(hMapFile);
		return er;
	}

	//one time init
	if (CreatedMapFile == 1)
	{
		//zero out evrything
		SharedMemVariableHeader *SharedMem = (SharedMemVariableHeader*)pBuf;
		memset(SharedMem, 0, SHARED_MEMORY_SIZE);
		SharedMem->NameHash = 0;
		SharedMem->Size = 0;
	}

	unsigned int VarNameHash = crc32((const unsigned char*)VarName, (unsigned int)strlen((char*)VarName));

	SharedMemVariableHeader *SharedMemHeader = (SharedMemVariableHeader*)pBuf;

	//search for our variable
	void *SharedMemEnd = (char*)SharedMemHeader + SHARED_MEMORY_SIZE;
	while (SharedMemHeader < SharedMemEnd && SharedMemHeader->NameHash != VarNameHash && SharedMemHeader->Size != 0)
		SharedMemHeader += SharedMemHeader->Size + sizeof(SharedMemVariableHeader);

	int retcode = 0;
	//we did not find this variable
	if (SharedMemHeader->Size != 0)
	{
		//copy back the data if we can
		SharedMemVariable *SharedVarData = (SharedMemVariable*)SharedMemHeader;
		if (Set == 1)
			memcpy(&SharedVarData->data, Store, min(SharedVarData->Size, Size));
		else
			memcpy(Store, &SharedVarData->data, min(SharedVarData->Size, Size));
	}
	else if (Set == 1 && SharedMemHeader + Size + sizeof(SharedMemVariableHeader) < SharedMemEnd )
	{
		SharedMemVariable *SharedVarData = (SharedMemVariable*)SharedMemHeader;
		SharedMemHeader->NameHash = VarNameHash;
		SharedMemHeader->Size = Size;
		memcpy(&SharedVarData->data, Store, min(SharedVarData->Size, Size));
	}
	else
		retcode = 2; //signal that we did not find this variable

	//cleanup
	//detach if we are not the creators of this shared memory location. Last instance should kill it
	if (CreatedMapFile != 1)
	{
		UnmapViewOfFile(pBuf);
		CloseHandle(hMapFile);
	}

	return retcode;
}
#else
#pragma warning ( push )
#pragma warning ( disable:4996 )
int ResetSharedMemory(const char *SessionName)
{
	int retcode;
	char Path[2000];
	char Pathp[2000];
	retcode = GetTempPath(sizeof(Path), Path);
	retcode = sprintf_s(Pathp, sizeof(Pathp), "%s%s.pshr", Path, SessionName);

	//check all processes that used this shared memory
	int FoundProcessUsedSharedMem = 0;
	int FoundOurProcessId = 0;
	DWORD OurProcessId = GetCurrentProcessId();
	FILE *f = fopen(Pathp, "rb");
	if (f != NULL)
	{
		while (!feof(f))
		{
			DWORD ProcessId;
			size_t ByteCount = fread(&ProcessId, 1, sizeof(ProcessId), f);
			if (ByteCount == sizeof(ProcessId))
			{
				if (FoundProcessUsedSharedMem == 0)
				{
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessId);
					if (NULL != hProcess)
					{
						FoundProcessUsedSharedMem = 1;
					}
				}
				if (OurProcessId == ProcessId)
				{
					FoundOurProcessId = 1;
				}
				if (FoundOurProcessId == 1 && FoundProcessUsedSharedMem == 1)
				{
					break;
				}
			}
		}
		fclose(f);
	}
	//reset file content if no more processes used this shared memory
	if (FoundProcessUsedSharedMem == 0)
	{
		FILE *f = fopen(Pathp, "wb");
		if (f != NULL)
		{
			fclose(f);
		}
		char Paths[2000];
		retcode = sprintf_s(Paths, sizeof(Paths), "%s%s.shr", Path, SessionName);
		f = fopen(Paths, "wb");
		if (f != NULL)
		{
			fclose(f);
		}
	}
	if (FoundOurProcessId == 0)
	{
		FILE *f = fopen(Pathp, "ab");
		if (f != NULL)
		{
			fwrite(&OurProcessId, 1, sizeof(OurProcessId), f);
			fclose(f);
		}
	}
	//nothing can go wrong. Unless something goes wrong
	return 0;
}

int GetSetValue(const char *SessionName, const char *VarName, void *Store, int Size, int Set)
{
	//this should reset it if a new session is required
	ResetSharedMemory(SessionName);

	int retcode;
	char Path[2000];
	retcode = GetTempPath(sizeof(Path), Path);
	retcode = sprintf_s(Path, sizeof(Path), "%s%s.shr", Path, SessionName);

	char *pBuf = (char*)malloc(SHARED_MEMORY_SIZE);

	size_t ReadCount = 0;
	FILE *f = fopen(Path, "rb");
	if (f != NULL)
	{
		//at this point we should check for all the processes that wrote into us. If none of them are still running than we should reset it's content
		ReadCount = fread(pBuf, 1, SHARED_MEMORY_SIZE, f);
		fclose(f);
	}
	//new implementation ? Something changed ?
	if (ReadCount != SHARED_MEMORY_SIZE)
	{
		SharedMemVariableHeader *SharedMem = (SharedMemVariableHeader*)pBuf;
		memset(SharedMem, 0, SHARED_MEMORY_SIZE);
		SharedMem->NameHash = 0;
		SharedMem->Size = 0;
	}
	f = fopen(Path, "wb");

	unsigned int VarNameHash = crc32((const unsigned char*)VarName, (unsigned int)strlen((char*)VarName));

	SharedMemVariableHeader *SharedMemHeader = (SharedMemVariableHeader*)pBuf;

	//search for our variable
	void *SharedMemEnd = (char*)SharedMemHeader + SHARED_MEMORY_SIZE;
	while (SharedMemHeader < SharedMemEnd && SharedMemHeader->NameHash != VarNameHash && SharedMemHeader->Size != 0)
		SharedMemHeader += SharedMemHeader->Size + sizeof(SharedMemVariableHeader);

	retcode = 0;
	//we did not find this variable
	if (SharedMemHeader->Size != 0)
	{
		//copy back the data if we can
		SharedMemVariable *SharedVarData = (SharedMemVariable*)SharedMemHeader;
		if (Set == 1)
			memcpy(&SharedVarData->data, Store, min(SharedVarData->Size, Size));
		else
			memcpy(Store, &SharedVarData->data, min(SharedVarData->Size, Size));
	}
	else if (Set == 1 && SharedMemHeader + Size + sizeof(SharedMemVariableHeader) < SharedMemEnd)
	{
		SharedMemVariable *SharedVarData = (SharedMemVariable*)SharedMemHeader;
		SharedMemHeader->NameHash = VarNameHash;
		SharedMemHeader->Size = Size;
		memcpy(&SharedVarData->data, Store, min(SharedVarData->Size, Size));
	}
	else
		retcode = 2; //signal that we did not find this variable

	fwrite(pBuf, 1, SHARED_MEMORY_SIZE, f);
	fclose(f);
	free(pBuf);

	return retcode;
}
#pragma warning ( pop )
#endif

LIBRARY_API int SharedMemGetValue(const char *SessionName, const char *VarName, void *Store, int Size)
{
	return GetSetValue(SessionName, VarName, Store, Size, 0);
}

LIBRARY_API int SharedMemSetValue(const char *SessionName, const char *VarName, void *Store, int Size)
{
	return GetSetValue(SessionName, VarName, Store, Size, 1);
}