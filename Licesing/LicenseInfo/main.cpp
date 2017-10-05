#include <stdio.h>
#define LIBRARY_API __declspec(dllimport)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <conio.h>
#include <Windows.h>
#include "../LicenseDLL/License.h"
#include "../LicenseDLL/License_API.h"

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

void PrintNonEncryptedData(License *TestLicense)
{
	printf("Printing non encrypted license data : \n");
#define LICENSE_PRODUCT_ID_SALES_DEPARTMENT			10
#define LICENSE_FEATURE_ID_CONTACT_NAME				1
#define LICENSE_FEATURE_ID_CONTACT_EMAIL			2
#define LICENSE_PRODUCT_ID_WINCCOA					9
#define LICENSE_FEATURE_ID_MAJOR_REV				9
#define LICENSE_FEATURE_ID_MINOR_REV				10
	char ActivationKeyBuffer[200];
	char ProductList[] = { LICENSE_PRODUCT_ID_SALES_DEPARTMENT, LICENSE_PRODUCT_ID_WINCCOA, 0 };
	char FeatureList[] = { LICENSE_FEATURE_ID_CONTACT_NAME, LICENSE_FEATURE_ID_CONTACT_EMAIL, LICENSE_FEATURE_ID_MAJOR_REV, LICENSE_FEATURE_ID_MINOR_REV, 0 };
	for (int IndP = 0; ProductList[IndP] != 0; IndP++)
		for (int IndF = 0; FeatureList[IndF] != 0; IndF++)
		{
			int GetKeyRes = TestLicense->GetActivationKey(ProductList[IndP], FeatureList[IndF], ActivationKeyBuffer, sizeof(ActivationKeyBuffer));
			if (GetKeyRes == 0)
				printf("\rFor project %d and Feature %d we obtained activation key '%s'\n", ProductList[IndP], FeatureList[IndF], ActivationKeyBuffer);
		}
	printf("\n");
}

void PrintScanActivationKeys(License *TestLicense)
{
	printf("Printing all license activation keys : \n");
	//find the key we need to activate a feature
	char PB[4] = { '\\', '|', '/', '-' };
	for (int ProductID = 0; ProductID < MAX_USED_SIEMENS_PROJECT_IDS; ProductID++)
		for (int FeatureId = 0; FeatureId < MAX_USED_SIEMENS_FEATURE_IDS; FeatureId++)
		{
			char ActivationKeyBuffer[200];
			int GetKeyRes = TestLicense->GetActivationKey(ProductID, FeatureId, ActivationKeyBuffer, sizeof(ActivationKeyBuffer));
			if (GetKeyRes == 0)
				printf("\rFor project %d and Feature %d we obtained activation key '%s'\n", ProductID, FeatureId, ActivationKeyBuffer);
			printf("\r%c %d", PB[FeatureId % 4], ((1 + FeatureId + MAX_USED_SIEMENS_FEATURE_IDS * ProductID) * 100) / (MAX_USED_SIEMENS_PROJECT_IDS * MAX_USED_SIEMENS_FEATURE_IDS));
		}
	printf("\n");
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

	//create a license
	License *TestLicense = new License;

	if (TestLicense == NULL)
	{
		printf("Unexpected error: feature list object is NULL\n");
		return 1;
	}

	//for testing, load up the saved license and check if we can extract feature keys
	printf("Load license into temp buffer\n");
//	int er = TestLicense->LoadFromFile("d:/Temp/Licensing builds/3 - LicenseInfo/License.dat");
	int er = 1;
/*	if (er != 0)
	{
		printf("Loading license from ../ \n");
		er = TestLicense->LoadFromFile("../License.dat");
	}
	if (er != 0)
	{
		printf("Error : %d\n", er);
		printf("Loading license from ../ and also loading licenseSeed\n");
		er = TestLicense->LoadFromFile("../License.dat", "../LicenseSeed.dat");
	}*/
	//maybe we are running it in a local directory ? 
	if (er != 0)
	{
		printf("Error : %d\n", er);
		printf("Loading license from ./ \n");
		er = TestLicense->LoadFromFile("License.dat");
	}
/*	if (er != 0)
	{
		printf("Error : %d\n", er);
		printf("Loading license from ./ and also loading licenseSeed\n");
		er = TestLicense->LoadFromFile("License.dat", "LicenseSeed.dat");
	}
	//	int er = TestLicense->LoadFromFile("D:/Projects/Licensing_SVN/trunk/src/build-LicenseManager-MyQT-Release/release/License.dat");
	if (er != 0)
	{
		printf("Error %d while loading license. Please solve it to continue\n", er);
		delete TestLicense;
		return 1;
	}*/

	//check data that should not be encrypted and available all the time. Even if we did not have a valid decrypt key
	PrintNonEncryptedData(TestLicense);

	time_t RemainingSecondsInLicense = 0;
	er = TestLicense->GetRemainingSeconds(&RemainingSecondsInLicense);
	if (er != 0)
		printf("Could not extract remaining seconds from license. Error code %d\n", er);
	printf("License is still valid for %d seconds\n", (int)RemainingSecondsInLicense);
	time_t RemainingSecondsGracePeriod = 0;
	char IsGraceTriggered;
	er = TestLicense->IsGracePeriodTriggered(&RemainingSecondsGracePeriod, &IsGraceTriggered);
	if (IsGraceTriggered == 1)
		printf("Grace period has been triggered and we have %d seconds remaining\n", RemainingSecondsGracePeriod);
	else
		printf("Grace period has not been triggered\n");

//	if (er != 0 || RemainingSecondsGracePeriod)
	if (RemainingSecondsInLicense < 0)
		printf("License has expired. There is no point searching for activation keys inside it");
	else
		PrintScanActivationKeys(TestLicense);

#ifdef _DEBUG
	// added late to debug what is happening in the background
	printf("\nDumping license content using internal debug function\n");
	TestLicense->PrintNodes();
//	printf("Max grace time in license is : %d\n", (int)TestLicense->GracePeriod);
#endif

	//cleanup
	delete TestLicense;
	TestLicense = NULL;

	printf("\n\nStart monitoring grace status changes. Push a 'Esc' key to exit.\n");

	//periodically test licenso to see how grace triggers or disables
	int PrevGraceReasonCode = -1;
	int PrevTick = GetTickCount();
	while (GetAsyncKeyState(VK_ESCAPE)==false)
	{
		time_t LicenseTime, GraceTime;
		int GraceReasonCode;
		int	er = GetRemainingTime(&LicenseTime, &GraceTime, &GraceReasonCode);
		if (er != 0 || GraceReasonCode != PrevGraceReasonCode)
		{
			if (PrevGraceReasonCode != -1)
				printf("#### Grace status changed, new code is : %d. Remaining time is : %d. Remaining grace time is : %d\n", GraceReasonCode, (int)LicenseTime, (int)GraceTime);
			PrevGraceReasonCode = GraceReasonCode;
		}
		if (GetTickCount() - PrevTick > 5000)
		{
			PrevTick = GetTickCount();
			printf("#### Still monitoring grace status changes\n");
		}
		Sleep(2000);
		printf("\n\n\n");
	}
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