#include <stdio.h>
#define LIBRARY_API __declspec(dllimport)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <conio.h>
#include <Windows.h>
#include "../LicenseDLL/License.h"

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

//this list should be generated from "ProjectNameIDs.txt"
enum SiemensProjects
{
	ALMA = 1,
	WDR = 2,
	MAX_USED_SIEMENS_PROJECT_IDS = 50
};

//this list should be generated from "FeatureNameIDs.txt"
enum SiemensProjectFeatures
{
	WDR_SSL = 1,
	ALMA_KPI = 2,
	MAX_USED_SIEMENS_FEATURE_IDS = 50,
};

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

	//create a license
	License *TestLicense = new License;

	if (TestLicense == NULL)
	{
		printf("Unexpected error: feature list object is NULL\n");
		return 1;
	}

	//for testing, load up the saved license and check if we can extract feature keys
	printf("Load license into temp buffer\n");
	int er = TestLicense->LoadFromFile("../License.dat", "../LicenseSeed.dat");
	//maybe we are running it in a local directory ? 
	if (er!=0)
		er = TestLicense->LoadFromFile("License.dat", "LicenseSeed.dat");
	if (er != 0)
	{
		printf("Error %d while loading license. Please solve it to continue\n", er);
		delete TestLicense;
		return 1;
	}

	time_t RemainingSecondsInLicense = 0;
	er = TestLicense->GetRemainingSeconds(&RemainingSecondsInLicense);
	if (er != 0)
		printf("Could not extract remaining seconds from license. Error code %d\n", er);
	printf("License is still valid for %d seconds\n", (int)RemainingSecondsInLicense);

	//find the key we need to activate a feature
	char PB[4] = { '\\', '|', '/', '-' };
	for (int ProductID = 0; ProductID < MAX_USED_SIEMENS_PROJECT_IDS; ProductID++)
		for (int FeatureId = 0; FeatureId < MAX_USED_SIEMENS_FEATURE_IDS; FeatureId++)
		{
			char ActivationKeyBuffer[200];
			int GetKeyRes = TestLicense->GetActivationKey(ProductID, FeatureId, ActivationKeyBuffer, sizeof(ActivationKeyBuffer));
			if (GetKeyRes == 0)
				printf("\rFor project %d and Feature %d we obtained activation key '%s'\n", ProductID, FeatureId, ActivationKeyBuffer);
			printf("\r%c %d", PB[FeatureId % 4], ((1 + FeatureId + MAX_USED_SIEMENS_FEATURE_IDS * ProductID) * 100) / (MAX_USED_SIEMENS_PROJECT_IDS * MAX_USED_SIEMENS_FEATURE_IDS) );
		}

	//cleanup
	delete TestLicense;
	TestLicense = NULL;

	printf("\n\nAll done. Push a key to exit.");
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