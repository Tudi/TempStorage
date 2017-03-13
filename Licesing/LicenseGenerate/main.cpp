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
	int LicenseStart = time(NULL) + 5 * 60;		// 5 minutes in the future

	//we are done with this "dropdown"
	delete ProjectFeatureDropdownList;

	printf("\n\nAll done. Push a key to exit.");
	_getch();
	return 0;
}