#define LIBRARY_API __declspec(dllimport)

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include "../LicenseDLL/License.h"

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

int main()
{
	//create a license
	License *TestLicense = new License;

	//for testing, load up the saved license and check if we can extract feature keys
	int er = TestLicense->LoadFromFile("testLic.dat");
	if (er != 0)
	{
		printf("Error %d while loading license. Please solve it to continue\n");
		delete TestLicense;
		return 1;
	}

	//find the key we need to activate a feature
	char ActivationKeyBuffer[200];
	int GetKeyRes = TestLicense->GetActivationKey(ALMA, ALMA_KPI, ActivationKeyBuffer, sizeof(ActivationKeyBuffer));
	if (GetKeyRes == 0)
		printf("For project ALMA and Feature KPI we obtained activation key '%s'\n", ActivationKeyBuffer);
	else
		printf("License did not contain a valid activation key\n");

	//cleanup
	delete TestLicense;

	printf("\n\nAll done. Push a key to exit.");
	_getch();
	return 0;
}