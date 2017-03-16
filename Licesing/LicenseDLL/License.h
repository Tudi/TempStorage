#pragma once

#define CURENT_LICENSE_NODE_VER				1
#define ERROR_INVALID_PROJECT_FEAURE		(-2)
#define WARNING_NO_LICENSE_FOUND			(-3)
#define WARNING_MISSING_LICENSE_FIELD		(-4)
#define ERROR_LICENSE_EXPIRED_OR_INVALID	(-5)

#define HardCodedStringLimitLic 50

#pragma pack(push,1)
struct LicenseNode
{
	int		ProjectId;			// get the list from static files
	int		FeatureId;			// get the list from static files
	char	ActivateKey[HardCodedStringLimitLic];
};
#pragma pack(pop)

class GenericDataStore;

class LIBRARY_API License
{
public:
	License();
	~License();
	int					SetDuration(time_t pStartDate, unsigned int pDuration);
	int					AddProjectFeature(int ProjectId, int FeatureId);
	int					SaveToFile(char *FileName, char *FingerprintFilename);					// Save License to a file that can be sent to Siemens licensing team
	int					LoadFromFile(char *FileName);				// load license from a file that can be sent to Siemens licensing team
	//returned value should not be disposed of ! Do not mix allocators when using DLLs !
	char				*GetActivationKey(int ProjectId, int FeatureId);
	int					GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	int					GetRemainingSeconds(time_t &RemainingSeconds);
private:
	time_t				StartStamp;
	unsigned int		Duration;
	GenericDataStore	*NodeList;
};