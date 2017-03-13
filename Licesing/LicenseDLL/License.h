#pragma once

#define CURENT_LICENSE_NODE_VER			1
#define ERROR_INVALID_PROJECT_FEAURE	-2

#pragma pack(push,1)
struct LicenseNode
{
	int		ProjectId;			// get the list from static files
	int		FeatureId;			// get the list from static files
	char	ActivateKey[HardCodedStringLimit];
};
#pragma pack(pop)

class GenericDataStore;

class LIBRARY_API License
{
public:
	License();
	~License();
	int SetDuration(unsigned int pStartDate, unsigned int pDuration);
	int				AddProjectFeature(int ProjectId, int FeatureId);
	int				SaveToFile(char *FileName);					// Save License to a file that can be sent to Siemens licensing team
	int				LoadFromFile(char *FileName);				// load license from a file that can be sent to Siemens licensing team
private:
	unsigned int		StartStamp;
	unsigned int		Duration;
	GenericDataStore	*NodeList;
};