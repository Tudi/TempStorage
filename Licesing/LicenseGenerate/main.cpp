#define LIBRARY_API __declspec(dllimport)

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include "../LicenseDLL/SimpleList.h"
#include "../LicenseDLL/ProjectFeatureKeys.h"
#include "../LicenseDLL/License.h"

int main()
{
	ProjectFeatureKeyDB *ProjectFeatureDropdownList;
	ProjectFeatureDropdownList = new ProjectFeatureKeyDB;

	//print out what we could add to the dropdown
	for (ProjectFeatureKeyStore *itr = ProjectFeatureDropdownList->ListStart(); itr != NULL; itr = ProjectFeatureDropdownList->ListNext())
		printf("%s %s %s\n", itr->ProjectName, itr->FeatureName, itr->ActivatorKey);

	//pick 2 whatever rows for the sake of our testing
	ProjectFeatureKeyStore *desc1 = ProjectFeatureDropdownList->ListStart();
	ProjectFeatureKeyStore *desc2 = ProjectFeatureDropdownList->ListNext();

	int LicenseDuration = 30 * 24 * 60 * 60;	// given in seconds
	time_t LicenseStart = time(NULL) + 5 * 60;		// 5 minutes in the future

	//create a license
	License *TestLicense = new License;
	TestLicense->SetDuration(LicenseStart, LicenseDuration);
	TestLicense->AddProjectFeature(desc1->ProjectID, desc1->FeatureID);
	TestLicense->AddProjectFeature(desc2->ProjectID, desc2->FeatureID);

	TestLicense->SaveToFile("testLic.dat","ClientSeed.dat");

	delete TestLicense;
	TestLicense = NULL;

	/////////////////////////////////////////////////////////////

	//for testing, load up the saved license and check if we can extract feature keys
	TestLicense = new License;
	TestLicense->LoadFromFile("testLic.dat");
	char *ProjectFeatureActivationKey;	//we will need to dipose of this

	ProjectFeatureActivationKey = TestLicense->GetActivationKey(desc1->ProjectID, desc1->FeatureID);
	if (ProjectFeatureActivationKey != NULL)
		printf("For project %s and Feature %s we obtained activation key %s\n", desc1->ProjectName, desc1->FeatureName, ProjectFeatureActivationKey);

	ProjectFeatureActivationKey = TestLicense->GetActivationKey(desc2->ProjectID, desc2->FeatureID);
	if (ProjectFeatureActivationKey != NULL)
		printf("For project %s and Feature %s we obtained activation key %s\n", desc2->ProjectName, desc2->FeatureName, ProjectFeatureActivationKey);

	//we are done with this "dropdown"
	delete ProjectFeatureDropdownList;
	delete TestLicense;


	printf("\n\nAll done. Push a key to exit.");
	_getch();
	return 0;
}