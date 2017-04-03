#define LIBRARY_API __declspec(dllimport)

#include <stdio.h>
#define LIBRARY_API __declspec(dllimport)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <conio.h>
#include <Windows.h>
#include "../LicenseDLL/License_API.h"
#include "../LicenseDLL/License.h"

#ifdef _DEBUG
	#pragma comment(lib, "../LicenseDLL/Debug/LicenseDLL.lib")
#else
	#pragma comment(lib, "../LicenseDLL/Release/LicenseDLL.lib")
#endif

//this list should be generated from "ProjectNameIDs.txt"
enum SiemensProjects
{
	ALMA	= 1,
	WDR		= 2
};

//this list should be generated from "FeatureNameIDs.txt"
enum SiemensProjectFeatures
{
	WDR_SSL		= 1,
	ALMA_KPI	= 2,
};

char GlobalKeyStoreTestCallback[2000];
int ReceivedCallback = 0;
void HandleAsyncKeyAssignCallback(char*key)
{
	ReceivedCallback = 1;
	GlobalKeyStoreTestCallback[0] = 0;
	if (key == NULL)
		return;
	strcpy_s(GlobalKeyStoreTestCallback, sizeof(GlobalKeyStoreTestCallback), key);
}

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

//	UpdateGracePeriodStatus( 10, 0 );

	//create a license
	License *TestLicense = new License;

	if (TestLicense == NULL)
	{
		printf("Unexpected error: feature list object is NULL\n");
		return 1;
	}

	//for testing, load up the saved license and check if we can extract feature keys
	printf("Load license into temp buffer\n");
	int er = TestLicense->LoadFromFile("../License.dat");
	if (er != 0)
	{
		printf("Error %d while loading license. Please solve it to continue\n");
		delete TestLicense;
		return 1;
	}

	printf("Seach for activation key inside license\n");
	//find the key we need to activate a feature
	char ActivationKeyBuffer[200];
	int GetKeyRes = TestLicense->GetActivationKey(ALMA, ALMA_KPI, ActivationKeyBuffer, sizeof(ActivationKeyBuffer));

	//this is for debugging only, you should not need to check for return value inside siemens projects. Hacker might be able to intercept the event and track the variable used for the activation key
	if (GetKeyRes == 0)
		printf("For project ALMA and Feature KPI we obtained activation key '%s'\n", ActivationKeyBuffer);
	else
		printf("License did not contain a valid activation key\n");



	//cleanup
	delete TestLicense;

	printf("\n\nAll done. Push a key to continue.");
	_getch();

	///////////////////////////////////////// get the key in an async way ////////////////////////////////////
	GetActivationKeyAsync(ALMA, ALMA_KPI, &HandleAsyncKeyAssignCallback);
	while (ReceivedCallback==0)
		Sleep(10); 
	printf("For project ALMA and Feature KPI we obtained activation key '%s'\n", GlobalKeyStoreTestCallback);
	printf("\n\nAll done. Push a key to exit.");
	_getch();
	///////////////////////////////////////// get the key in an async way ////////////////////////////////////

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