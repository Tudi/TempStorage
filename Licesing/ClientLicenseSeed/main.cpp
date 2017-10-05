#define LIBRARY_API __declspec(dllimport)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <stdio.h>
#include <conio.h>
#include "../LicenseDLL/ComputerFingerprint.h"

#ifndef X64
	#ifdef _DEBUG
		#pragma comment(lib, "../Debug/LicenseDLL.lib")
	#else
		#pragma comment(lib, "../Release/LicenseDLL.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "../x64/Debug/LicenseDLL.lib")
	#else
		#pragma comment(lib, "../x64/Release/LicenseDLL.lib")
	#endif
#endif

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_16_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	_CrtMemState s1, s2, s3;
	_CrtMemCheckpoint(&s1);

	ComputerFingerprint *ClientSeed;
	int er = 0;

	//create a new store
	ClientSeed = CreateComputerFingerprint();
	if (ClientSeed == NULL)
	{
		printf("Could not create ComputerFingerprint object\n");
		return 1;
	}

	//test generate API
	er = ClientSeed->GenerateFingerprint();
	if (er != 0)
	{
		printf("Could not generate ComputerFingerprint content\n");
		DestroyComputerFingerprint(&ClientSeed);
		return 1;
	}

	//test extended computer info
	ClientSeed->AppendClientInfo("NoRole", "Istvan");

	//test save
	er = ClientSeed->SaveFingerprint("../LicenseSeed.dat");
	if (er != 0)
	{
		printf("Could not save ComputerFingerprint content\n");
		DestroyComputerFingerprint(&ClientSeed);
		return 1;
	}

	//test debugprint
	printf("For debug purpuses, prin the content : \n");
	ClientSeed->Print();

	//test destroy
	DestroyComputerFingerprint(&ClientSeed);
	
	printf("\nSaved the license seed. You will need this file to generate a new license for this PC\n");
	//wait for keypress
	_getch();
	
	_CrtMemCheckpoint(&s2);
	if (_CrtMemDifference(&s3, &s1, &s2) && (s3.lCounts[0]>0 || s3.lCounts[1] > 0 || s3.lCounts[3] > 0 || s3.lCounts[3] > 0)) //ignore CRT block allocs, can't do much about them. Some come from printf...
	{
		_CrtMemDumpStatistics(&s3);
		printf("!!Memory issues detected. Investigate !\n");
		_getch();
	}

	_CrtCheckMemory();
	_CrtDumpMemoryLeaks();
	return 0;
}