#pragma once

#define CURENT_LICENSE_NODE_VER				1
#define ERROR_INVALID_PROJECT_FEAURE		(-2)
#define WARNING_NO_LICENSE_FOUND			(-3)
#define WARNING_MISSING_LICENSE_FIELD		(-4)
#define ERROR_LICENSE_EXPIRED				(-5)
#define ERROR_COULD_NOT_LOAD_LIC_FILE		(-6)
#define ERROR_COULD_NOT_STORE_GRACE_PERIOD	(-7)
#define ERROR_LICENSE_INVALID				(-8)

#ifdef _DEBUG
	#define LICENSE_DURATION_UPDATE_INTERVAL	( 1000 )	// trigger lots of events to not need to wait for testing
#else
	#define LICENSE_DURATION_UPDATE_INTERVAL	( 5 * 60 * 1000 )
#endif

#define GenerateSaltKey(a,b)	((unsigned int)(1|(a+b)))
#define GenerateSaltKey2		(GenerateSaltKey( GetTickCount(), time(NULL) ))

#define HardCodedStringLimitLic 50

#define ENABLE_ANTI_HACK_CHECKS

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
	int					AddProjectFeature(const char *ProjectName, const char *FeatureName);
	int					AddProjectFeature(int ProjectId, int FeatureId);
	int					SaveToFile(const char *FileName, const char *FingerprintFilename);	// Save License to a file that can be sent to Siemens licensing team
	int					LoadFromFile(char *FileName);										// load license from a file that can be sent to Siemens licensing team
	int					LoadFromFile(char *FileName, char *FingerprintFilename);			// load a license while not using a local fingerprint file. This should only be used by LicenseInfo Project
	int					GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	int					GetRemainingSeconds(time_t &RemainingSeconds);// this does not include grace period
private:
	time_t				StartStamp;
	unsigned int		Duration;
	time_t				GracePeriod;
	GenericDataStore	*NodeList;
};
