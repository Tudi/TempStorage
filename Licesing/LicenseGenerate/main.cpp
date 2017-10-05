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

void AddAlmaTestRequiredData(License *TestLicense)
{
#define LICENSE_PRODUCT_ID_ALMA				1
#define LICENSE_FEATURE_ID_KPI_STATIC		3
	int er;
	er = TestLicense->AddProjectFeature(LICENSE_PRODUCT_ID_ALMA, LICENSE_FEATURE_ID_KPI_STATIC, "STATIC.Config");
	er = TestLicense->AddProjectFeature(1, 2, "KPITestEnable");
}

void AddGenericUnencryptedLicenseData(License *TestLicense)
{
#define LICENSE_PRODUCT_ID_SALES_DEPARTMENT			10
#define LICENSE_FEATURE_ID_CONTACT_NAME				1
#define LICENSE_FEATURE_ID_CONTACT_EMAIL			2
	int er;
	er = TestLicense->AddProjectFeature(LICENSE_PRODUCT_ID_SALES_DEPARTMENT, LICENSE_FEATURE_ID_CONTACT_NAME, "Andales Mercador");
	er = TestLicense->AddProjectFeature(LICENSE_PRODUCT_ID_SALES_DEPARTMENT, LICENSE_FEATURE_ID_CONTACT_EMAIL, "hombre@internet.net");
#define LICENSE_PRODUCT_ID_WINCCOA			9
#define LICENSE_FEATURE_ID_MAJOR_REV		9
#define LICENSE_FEATURE_ID_MINOR_REV		10
	er = TestLicense->AddProjectFeature(LICENSE_PRODUCT_ID_WINCCOA, LICENSE_FEATURE_ID_MAJOR_REV, "3");
	er = TestLicense->AddProjectFeature(LICENSE_PRODUCT_ID_WINCCOA, LICENSE_FEATURE_ID_MINOR_REV, "12");
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

	int er;

	int LicenseDuration = 7 * 24 * 60 * 60;			// given in seconds. 1 week to test warning
	time_t LicenseStart = time(NULL) + 5 * 60;		// 5 minutes in the future

	//create a license
	License *TestLicense = new License;
	er = TestLicense->SetDuration(LicenseStart, LicenseDuration, 5 * 60);
	if (er != 0)
		printf("Could not set duration of license\n");


	//this is for testing ALMA
	AddAlmaTestRequiredData(TestLicense);

	er = TestLicense->SaveToFile_("../License.dat", "../LicenseSeed.dat");
	if (er != 0)
		printf("Could not save license\n");

	delete TestLicense;
	TestLicense = NULL;



	// add a new unencrypted license that should contain sales department related info
	{
		TestLicense = new License;
		er = TestLicense->SetDuration(time(NULL), PERMANENT_LICENSE_MAGIC_DURATION, 0);
		if (er != 0)
			printf("Could not set duration of license\n");
		AddGenericUnencryptedLicenseData(TestLicense);
		er = TestLicense->SaveToFile("../License.dat", NULL, 1);
		if (er != 0)
			printf("Could not save license\n");

		delete TestLicense;
		TestLicense = NULL;
	}/**/


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