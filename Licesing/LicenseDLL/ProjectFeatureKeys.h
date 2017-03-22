#pragma once

#define MISSING_OR_INVALID_ID	(-1)
#define DB_LOAD_ERROR			(-1)

#define HardCodedStringLimitFeatures 50

//humanly described
struct ProjectFeatureKeyStore
{
	char	ProjectName[HardCodedStringLimitFeatures];
	int		ProjectID;
	char	FeatureName[HardCodedStringLimitFeatures];
	int		FeatureID;
	char	ActivatorKey[HardCodedStringLimitFeatures];
};

//map names to IDs
struct NameIdStore
{
	char	Name[200];
	int		ID;
};

class LIBRARY_API ProjectFeatureKeyDB
{
public:
	ProjectFeatureKeyDB();
	~ProjectFeatureKeyDB();
	int ReloadDB();
	int GetFeatureNameID(const char *FeatureName);
	int GetProjectNameID(const char *ProjectName);
	char *FindProjectFeatureKey(char *ProjectName, char *FeatureName);
	char *FindProjectFeatureKey(int ProjectId, int FeatureId);
	ProjectFeatureKeyStore *ListStart() { return Data.begin(); }
	ProjectFeatureKeyStore *ListNext() { return Data.GetNext(); }
	int IsValidProjectID(int ProjectID);
	int IsValidKey(char *Key);
protected:
	void DestroyList();
	int LoadKeys();
	int LoadFeatureNameIDs();
	int LoadProjectNameIDs();
	int LoadProjectFeatureKeys();
	LinkedList<ProjectFeatureKeyStore*>		Data;
	LinkedList<NameIdStore*>				ProjectIDs;
	LinkedList<NameIdStore*>				FeatureIDs;
	LinkedList<NameIdStore*>				Keys;
};