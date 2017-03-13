#include "stdafx.h"
#include <time.h>
#include "License.h"
#include "src/DataPacker.h"
#include "ProjectFeatureKeys.h"
#include <stdio.h>

License::License()
{
	NodeList = new GenericDataStore;
}

License::~License()
{
	if (NodeList != NULL)
	{
		delete NodeList;
		NodeList = NULL;
	}
}

int License::AddProjectFeature(int ProjectId, int FeatureId)
{
	//check if this is a valid param
	ProjectFeatureKeyDB DB;
	if (DB.IsValidProjectID(ProjectId) == MISSING_OR_INVALID_ID)
	{
		printf("Warning:Found invalid project id %d while constructing license node\n", ProjectId);
	}
	char *ActivationKey = DB.FindProjectFeatureKey(ProjectId, FeatureId);
	if (ActivationKey == NULL)
	{
		printf("Error:Could not find a valid activation key for project %d and feature ID %d. Exiting\n", ProjectId, FeatureId);
		return ERROR_INVALID_PROJECT_FEAURE;
	}
	//create a new storable node
	LicenseNode *NewNode = new LicenseNode;
	memset(NewNode, 0, sizeof(LicenseNode));
	NewNode->ProjectId = ProjectId;
	NewNode->FeatureId = FeatureId;
	errno_t er = strcpy_s(NewNode->ActivateKey, sizeof(NewNode->ActivateKey), ActivationKey);
	//wow, we should never trigger this
	if (er)
		return er;

	//check if we already have this node. There is no point for dounble adding it
	DataCollectionIterator itr;
	itr.Init(NodeList);
	char *Data;
	int Size;
	int Type;
	while (DCI_SUCCESS == itr.GetNext(&Data, Size, Type))
	{
		if (Type == DB_LICENSE_NODE)
		{
			if (memcmp(Data, NewNode, Size) == 0)
			{
				printf("Warning:This node is already in the License list. There is no point adding it a seconds time.\n");
				break;
			}
		}
	}

	//Add new node to our store
	NodeList->PushData((char*)NewNode, sizeof(LicenseNode), DB_LICENSE_NODE);

	//no errors returned
	return 0;
}

int License::SetDuration(unsigned int pStartDate, unsigned int pDuration)
{
	//sanity checks
	int LostSeconds = (signed int )time(NULL) - pStartDate;
	if (LostSeconds > 60 * 60 )
		printf("WARNING:Start date is smaller than current date. Loosing %d seconds from durtation %d\n", LostSeconds, pDuration);
	if (LostSeconds > (signed int)pDuration)
	{
		printf("Warning:Duration is too small. Readjusting start time to current time to avoid negative duration");
		pStartDate = (unsigned int)time(NULL);
	}

	StartStamp = pStartDate;
	Duration = pDuration;
	return 0;
}
