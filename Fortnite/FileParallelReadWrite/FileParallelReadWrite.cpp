#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <comdef.h>

int ThreadExits = 0;
DWORD WINAPI BackgroundProcessWriteStuff(LPVOID lpParam)
{
	FILE *f;
	errno_t openerr = fopen_s(&f, "test.replay", "wb");
	DWORD ThreadTimeout = GetTickCount() + 100000;
	while (ThreadTimeout > GetTickCount())
	{
		char randombuff[65535];
		fwrite(randombuff, 1, sizeof(randombuff), f);
		printf("written some bytes\n");
//		fflush(f);
		Sleep(500);
	}
	fclose(f);
	printf("Closing write thread\n"); Sleep(100);
	return 0;
}

#if 0
DWORD WINAPI BackgroundProcessReadStuff(LPVOID lpParam)
{
	FILE *f;
	printf("Started read thread\n"); Sleep(100);
	try
	{
		errno_t openerr = fopen_s(&f, "test.replay", "rb");
		if (f == NULL)
		{
			int ret = CopyFile("test.replay", "test.replay_cpy", false);
			printf("Could not read file - original. Copy result %d\n", ret);
		}
		else
			printf("Managed to open file for read\n"); Sleep(100);
	}
	catch (int e)
	{
		printf("Could not copy file.Exiting\n"); Sleep(100);
		return 1;
	}
	if (f == NULL)
	{
		errno_t openerr = fopen_s(&f, "test.replay_cpy", "rb");
		if (f == NULL)
		{
			printf("Could not read file - copy. Exiting\n");
			return 1;
		}
		else
			printf("Managed to open file copy for read\n"); Sleep(100);
	}
	DWORD ThreadTimeout = GetTickCount() + 10000;
	while (ThreadTimeout > GetTickCount())
	{
		char randombuff[65535];
		fread(randombuff, 1, sizeof(randombuff), f);
		printf("read some bytes\n");
		Sleep(1000);
	}
	fclose(f);
	printf("Closing read thread\n"); Sleep(100);
	return 0;
}
#endif

#if 0
DWORD WINAPI BackgroundProcessReadStuffShadow(LPVOID lpParam)
{
	// can check in cmd if these work : https://ss64.com/nt/vssadmin.html
	// VSSADMIN add shadowstorage /for=c: /on=d: /maxsize=900mb
	// VSSADMIN create shadow /for=c:
	int retcode = 0;
	int i = 0;
	HRESULT hr;
	IVssEnumObject *pIEnumSnapshots;
	IVssBackupComponents *ab;
	VSS_OBJECT_PROP	Prop;
	VSS_SNAPSHOT_PROP& Snap = Prop.Obj.Snap;
	WCHAR existingFilePath[MAX_PATH] = TEXT("C:\\PageFile.sys");
	WCHAR newFileLocation[MAX_PATH] = TEXT("C:\\PageFileTEST.sys");
	TCHAR existingFileLocation[MAX_PATH];


	TCHAR sourceFile[_MAX_PATH] = TEXT("C:\\PageFile.sys");
	LPTSTR DestinationFile = TEXT("D:\\PageFileTEST.sys");


	if (CoInitialize(NULL) != S_OK)
	{
		return 2;
	}
	// initialize security 
	if (FAILED(CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL))) 
	{
		return(0);
	}

	hr = CreateVssBackupComponents(&ab);
	if (hr != S_OK)
	{

		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 2;
	}

	hr = ab->InitializeForBackup();
	if (hr != S_OK)
	{

		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		printf("erorr..");
		return 4;
	}



	IVssAsync *pAsync = NULL;
	hr = ab->GatherWriterMetadata(&pAsync);
	if (hr != S_OK)
	{

		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 5;
	}
	hr = pAsync->Wait();
	if (hr != S_OK)
	{
		return 1;
	}

	pAsync->Release();


	VSS_ID snapshotID;
	hr = ab->StartSnapshotSet(&snapshotID);
	if (hr != S_OK)
	{

		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 6;
	}


	VSS_ID SnapShotId;



	WCHAR volumeName[4] = TEXT("C:\\");
	hr = ab->AddToSnapshotSet(volumeName, GUID_NULL, &SnapShotId);
	if (hr != S_OK)
	{

		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 7;
	}

	hr = ab->SetBackupState(false, false, VSS_BT_FULL, true);
	if (hr != S_OK)
	{
		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();
		return 1;

	}

	IVssAsync *pPrepare = NULL;
	hr = ab->PrepareForBackup(&pPrepare);
	if (hr != S_OK)
	{
		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 8;
	}
	hr = pPrepare->Wait();
	if (hr != S_OK)
	{

		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 9;
	}

	IVssAsync *pDoShadowCpy = NULL;
	hr = ab->DoSnapshotSet(&pDoShadowCpy);
	if (hr != S_OK)
	{
		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();

		return 10;
	}
	hr = pDoShadowCpy->Wait();
	if (hr == S_OK)
	{
		VSS_SNAPSHOT_PROP snapshotprop = { 0 };
		hr = ab->GetSnapshotProperties(SnapShotId, &snapshotprop);
		_com_error error(hr);
		LPCTSTR errorText = error.ErrorMessage();


		if (hr == S_OK)
		{
			int sourcelength = wcslen(snapshotprop.m_pwszSnapshotDeviceObject) + _tcslen(sourceFile);

			WCHAR test[260];
			wsprintf(test, L"%s%s", snapshotprop.m_pwszSnapshotDeviceObject, L"C:\\logFile.log");

			CopyFile(test, L"C:\\test.txt", false);
			//exit(1);



			LPTSTR sourcesnapshotfile = new TCHAR[sourcelength];
			_tcscpy_s(sourcesnapshotfile, sourcelength, snapshotprop.m_pwszSnapshotDeviceObject);
			_tcscat_s(sourcesnapshotfile, sourcelength, &sourceFile[2]);


			if (!::CopyFile(sourcesnapshotfile, DestinationFile, FALSE))
			{
				DWORD lastError = GetLastError();
			}
			else
			{
				printf("File copied..");
			}

			delete[] sourcesnapshotfile;
			VssFreeSnapshotProperties(&snapshotprop);
			pDoShadowCpy->Release();
			/*pPrepare->Release();*/
			LONG deletedsnapshotid = 0;
			VSS_ID nonDeletedSnapshotID;
			hr = ab->DeleteSnapshots(SnapShotId, VSS_OBJECT_SNAPSHOT, true, &deletedsnapshotid, &nonDeletedSnapshotID);
			if (hr != S_OK)
			{

				return 11;

			}
		}
}
#endif

DWORD WINAPI BackgroundProcessReadStuffSystemShadow(LPVOID lpParam)
{
	while (1)
	{
		system("copy /Y /Z test.replay t.1");
	}
}

int main()
{
	printf("Parallel write_read started\n");
	//create a thread that keeps the file open and write some stuff from time to time
	DWORD   WriterThreadId;
	DWORD   WriterThreadParam;
	HANDLE	WriterThreadHandle = CreateThread(
		NULL,							// default security attributes
		0,								// use default stack size  
		BackgroundProcessWriteStuff,		// thread function name
		&WriterThreadParam,		// argument to thread function 
		0,								// use default creation flags 
		&WriterThreadId);		// returns the thread identifier 

	Sleep(1000);

	//create a thread that tries to read the file from time to time
	DWORD   ReaderThreadId;
	DWORD   ReaderThreadParam;
	HANDLE	ReaderThreadHandle = CreateThread(
		NULL,							// default security attributes
		0,								// use default stack size  
		BackgroundProcessReadStuffSystemShadow,		// thread function name
		&ReaderThreadParam,		// argument to thread function 
		0,								// use default creation flags 
		&ReaderThreadId);		// returns the thread identifier }
	/**/
	printf("Main thread will wait for subthreads\n");
	Sleep(100000);
	printf("Done waiting on threads\n");
}