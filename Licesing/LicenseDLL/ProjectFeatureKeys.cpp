#include "stdafx.h"
#include "ProjectFeatureKeys.h"
#include <stdio.h>
#include <stdlib.h>

ProjectFeatureKeyDB::ProjectFeatureKeyDB()
{
	ReloadDB();
}

ProjectFeatureKeyDB::~ProjectFeatureKeyDB()
{
	DestroyList();
}

void ProjectFeatureKeyDB::DestroyList()
{
	//get rid of our lists
	for (ProjectFeatureKeyStore *itr = Data.begin(); itr != Data.end(); itr = Data.GetNext())
		delete itr;
	Data.clear();
	for (NameIdStore *itr = ProjectIDs.begin(); itr != ProjectIDs.end(); itr = ProjectIDs.GetNext())
		delete itr;
	ProjectIDs.clear();
	for (NameIdStore *itr = FeatureIDs.begin(); itr != FeatureIDs.end(); itr = FeatureIDs.GetNext())
		delete itr;
	FeatureIDs.clear();
	for (NameIdStore *itr = Keys.begin(); itr != Keys.end(); itr = Keys.GetNext())
		delete itr;
	Keys.clear();
}

void ReadStringFromTabDelimitedFile(FILE *f, char *str, int maxlen)
{
	if (str == NULL)
		return;
	if (maxlen == 0)
		return;
	str[0] = 0;
	if (f == NULL)
		return;
	int ReadIndex = 0;
	int SkipWholeLine = 0;
	size_t rcount;
	char tcbuf;
	while ((rcount = fread(&tcbuf, 1, 1, f))>0)
	{
		// is this a comment line ? 
		if (ReadIndex == 0 && tcbuf == ';')
			SkipWholeLine = 1;
		//if we are on a comment line, read it until we skip the whole line
		if (tcbuf != '\n' && SkipWholeLine == 1)
			continue;
		//only read until the end of the line
		if (tcbuf == '\n')
		{
			//finished reading comment line
			if (SkipWholeLine == 1)
			{
				SkipWholeLine = 0;
				continue;
			}
			//empty line ?
			else if (ReadIndex == 0)
				continue;
			else
				break;
		}

		//skip tab characters until we get an actual value
		if (ReadIndex == 0 && tcbuf == '\t')
			continue;

		//done reading this string
		if (tcbuf == '\t')
			break;

		//can we store it ? Store it
		if (ReadIndex < maxlen)
			str[ReadIndex++] = tcbuf;
		else
			break;
	}
	if (ReadIndex < maxlen)
		str[ReadIndex] = 0;
}

int ProjectFeatureKeyDB::LoadKeys()
{
	FILE *f;
	errno_t er = fopen_s(&f, "FeatureKeysNames.txt", "rt");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//as long as we can find another value...keep on reading
	NameIdStore tname;
	do {
		//feature name
		ReadStringFromTabDelimitedFile(f, tname.Name, sizeof(tname.Name));
		if (tname.Name[0] == 0)
			break;
		tname.ID = GetFeatureNameID(tname.Name);
		if (tname.ID == MISSING_OR_INVALID_ID)
		{
			printf("Warning:Could not find id for feature %s.Exiting!\n", tname.Name);
			return DB_LOAD_ERROR;
		}

		//key name
		ReadStringFromTabDelimitedFile(f, tname.Name, sizeof(tname.Name));
		if (tname.Name[0] == 0)
			break;

		//store it
		NameIdStore *store = new NameIdStore(tname);
		Keys.push_front(store);

	} while (tname.Name[0] != 0);

	//all went as expected
	return 0;
}

int ProjectFeatureKeyDB::LoadFeatureNameIDs()
{
	FILE *f;
	errno_t er = fopen_s(&f, "FeatureNameIDs.txt", "rt");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//as long as we can find another value...keep on reading
	NameIdStore tname;
	do {
		//load name
		ReadStringFromTabDelimitedFile(f, tname.Name, sizeof(tname.Name));
		if (tname.Name[0] == 0)
			break;

		//load ID
		char IntBuff[10];
		ReadStringFromTabDelimitedFile(f, IntBuff, sizeof(IntBuff));
		if (IntBuff[0] == 0)
			break;
		tname.ID = atoi(IntBuff);

		//add it to our "DB"
		NameIdStore *store = new NameIdStore(tname);
		FeatureIDs.push_front(store);
	} while (tname.Name[0] != 0);

	//all went as expected
	return 0;
}

int ProjectFeatureKeyDB::LoadProjectNameIDs()
{
	FILE *f;
	errno_t er = fopen_s(&f, "ProjectNameIDs.txt", "rt");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//as long as we can find another value...keep on reading
	NameIdStore tname;
	do {
		//load name
		ReadStringFromTabDelimitedFile(f, tname.Name, sizeof(tname.Name));
		if (tname.Name[0] == 0)
			break;

		//load ID
		char IntBuff[10];
		ReadStringFromTabDelimitedFile(f, IntBuff, sizeof(IntBuff));
		if (IntBuff[0] == 0)
			break;
		tname.ID = atoi(IntBuff);

		//add it to our "DB"
		NameIdStore *store = new NameIdStore(tname);
		ProjectIDs.push_front(store);
	} while (tname.Name[0] != 0);

	//all went as expected
	return 0;
}

int ProjectFeatureKeyDB::LoadProjectFeatureKeys()
{
	FILE *f;
	errno_t er = fopen_s(&f, "ProjectFeatureKeys.txt", "rt");
	if (er != NO_ERROR)
		return er;

	if (f == NULL)
		return ERROR_FILE_INVALID;

	//as long as we can find another value...keep on reading
	ProjectFeatureKeyStore tname;
	int AntiInfinitLoop = 10000;//because programing will never ever have bugs
	do {
		//load project name
		ReadStringFromTabDelimitedFile(f, tname.ProjectName, sizeof(tname.ProjectName));
		if (tname.ProjectName[0] == 0)
			break;
		tname.ProjectID = GetProjectNameID(tname.ProjectName);
		if (tname.ProjectID == MISSING_OR_INVALID_ID)
		{
			printf("Warning : Could not find the ID of project %s\n", tname.ProjectName);
			break;
		}

		//load feature name
		ReadStringFromTabDelimitedFile(f, tname.FeatureName, sizeof(tname.FeatureName));
		if (tname.FeatureName[0] == 0)
			break;
		tname.FeatureID = GetFeatureNameID(tname.FeatureName);
		if (tname.FeatureID == MISSING_OR_INVALID_ID)
		{
			printf("Warning : Could not find the ID of feature %s\n", tname.FeatureName);
			break;
		}

		//load key
		ReadStringFromTabDelimitedFile(f, tname.ActivatorKey, sizeof(tname.ActivatorKey));
		if (tname.ActivatorKey[0] == 0)
			break;
		if (IsValidKey(tname.ActivatorKey) != tname.FeatureID)
		{
			printf("Warning : Feature %s, with ID %d does not have key %s\n", tname.FeatureName, tname.FeatureID, tname.ActivatorKey);
			break;
		}

		//add it to our "DB"
		ProjectFeatureKeyStore *store = new ProjectFeatureKeyStore(tname);
		Data.push_front(store);
	} while (AntiInfinitLoop-- > 0);

	//all went as expected
	return 0;
}

int ProjectFeatureKeyDB::GetFeatureNameID(const char *FeatureName)
{
	for (NameIdStore* itr = FeatureIDs.begin(); itr != FeatureIDs.end(); itr = FeatureIDs.GetNext())
		if (strcmp(FeatureName, itr->Name) == 0)
			return itr->ID;
	return MISSING_OR_INVALID_ID;
}

int ProjectFeatureKeyDB::GetProjectNameID(const char *ProjectName)
{
	int PossibleID = atoi(ProjectName);
	if (PossibleID > 0 && IsValidProjectID(PossibleID))
		return PossibleID;
	else
	{
		//search in the list to get the valid ID
		for (NameIdStore* itr = ProjectIDs.begin(); itr != ProjectIDs.end(); itr = ProjectIDs.GetNext())
			if (strcmp(ProjectName, itr->Name) == 0)
				return itr->ID;
	}
	return MISSING_OR_INVALID_ID;
}

char *ProjectFeatureKeyDB::FindProjectFeatureKey(char *ProjectName, char *FeatureName)
{
	for (ProjectFeatureKeyStore* itr = Data.begin(); itr != Data.end(); itr = Data.GetNext())
		if (strcmp(ProjectName, itr->ProjectName) == 0 && strcmp(FeatureName, itr->FeatureName) == 0)
			return itr->ActivatorKey;
	return NULL;
}

char *ProjectFeatureKeyDB::FindProjectFeatureKey(int ProjectId, int FeatureId)
{
	for (ProjectFeatureKeyStore* itr = Data.begin(); itr != Data.end(); itr = Data.GetNext())
		if (ProjectId == itr->ProjectID && FeatureId == itr->FeatureID)
			return itr->ActivatorKey;
	return NULL;
}

int ProjectFeatureKeyDB::IsValidProjectID(int ProjectID)
{
	for (NameIdStore* itr = ProjectIDs.begin(); itr != ProjectIDs.end(); itr = ProjectIDs.GetNext())
		if (ProjectID == itr->ID)
			return 0;
	return MISSING_OR_INVALID_ID;
}

int ProjectFeatureKeyDB::IsValidKey(char *Key)
{
	for (NameIdStore* itr = Keys.begin(); itr != Keys.end(); itr = Keys.GetNext())
		if (strcmp(Key, itr->Name) == 0)
			return itr->ID;
	return MISSING_OR_INVALID_ID;
}

int ProjectFeatureKeyDB::ReloadDB()
{
	//get rid of our old list
	DestroyList();

	int er;
	//load order matters. Don't be too smart and reorder it.
	if ((er = LoadFeatureNameIDs()) != 0)
		return er;
	if ((er = LoadKeys()) != 0)
		return er;
	if ((er = LoadProjectNameIDs()) != 0)
		return er;
	if ((er = LoadProjectFeatureKeys()) != 0)
		return er;

	//should 
	return 0;
}
