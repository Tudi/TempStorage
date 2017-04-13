#define LIBRARY_API __declspec(dllimport)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include "../LicenseDLL/SimpleList.h"
#include "../LicenseDLL/ProjectFeatureKeys.h"
#include "../LicenseDLL/License.h"

#ifdef _DEBUG
	#pragma comment(lib, "../Debug/LicenseDLL.lib")
#else
	#pragma comment(lib, "../Release/LicenseDLL.lib")
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

	ProjectFeatureKeyDB *ProjectFeatureDropdownList;
	ProjectFeatureDropdownList = new ProjectFeatureKeyDB;

	if (ProjectFeatureDropdownList == NULL)
	{
		printf("Unexpected error: feature list object is NULL\n");
		return 1;
	}

	//print out what we could add to the dropdown
	printf("Printing all available projects and their features : \n");
	for (ProjectFeatureKeyStore *itr = ProjectFeatureDropdownList->ListStart(); itr != NULL; itr = ProjectFeatureDropdownList->ListNext())
		printf("%s %s %s\n", itr->ProjectName, itr->FeatureName, itr->ActivatorKey);
	printf("\n");

	printf("Started generating license :\n");
	//pick 2 whatever rows for the sake of our testing
	ProjectFeatureKeyStore *desc1 = ProjectFeatureDropdownList->ListStart();
	ProjectFeatureKeyStore *desc2 = ProjectFeatureDropdownList->ListNext();

	int LicenseDuration = 30 * 24 * 60 * 60;	// given in seconds
	time_t LicenseStart = time(NULL) + 5 * 60;		// 5 minutes in the future

	//create a license
	License *TestLicense = new License;
	TestLicense->SetDuration(LicenseStart, LicenseDuration, 5 * 60);
	TestLicense->AddProjectFeature(desc1->ProjectID, desc1->FeatureID);
//	TestLicense->AddProjectFeature(desc2->ProjectID, desc2->FeatureID);

	TestLicense->SaveToFile("../License.dat","../LicenseSeed.dat");

	delete TestLicense;
	TestLicense = NULL;
	delete ProjectFeatureDropdownList;
	ProjectFeatureDropdownList = NULL;

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