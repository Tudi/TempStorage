#pragma once

#define CURENT_LICENSE_NODE_VER				1
#define ERROR_INVALID_PROJECT_FEAURE		(-2)
#define WARNING_NO_LICENSE_FOUND			(-3)
#define WARNING_MISSING_LICENSE_FIELD		(-4)
#define ERROR_LICENSE_EXPIRED_OR_INVALID	(-5)
#define ERROR_COULD_NOT_LOAD_LIC_FILE		(-6)
#define ERROR_COULD_NOT_STORE_GRACE_PERIOD	(-7)

#define LICENSE_DURATION_UPDATE_INTERVAL	( 5 * 60 * 1000 )
#define GenerateSaltKey(a,b)	((unsigned int)(1|(a+b)))

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
	int					AddProjectFeature(const char *ProjectName, const char *FeatureName);
	int					AddProjectFeature(int ProjectId, int FeatureId);
	int					SaveToFile(const char *FileName, const char *FingerprintFilename);	// Save License to a file that can be sent to Siemens licensing team
	int					LoadFromFile(char *FileName);										// load license from a file that can be sent to Siemens licensing team
	int					LoadFromFile(char *FileName, char *FingerprintFilename);			// load a license while not using a local fingerprint file. This should only be used by LicenseInfo Project
	int					GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	int					GetRemainingSeconds(time_t &RemainingSeconds, int IncludeGrace = 1);// this does not include grace period
private:
	time_t				StartStamp;
	unsigned int		Duration;
	GenericDataStore	*NodeList;
};

extern "C"
{
	// C style activation key fetch
	LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	// for extra security we could detach ourself to a new thread and return result later to an unknown buffer
	LIBRARY_API void GetActivationKeyAsync(int ProjectId, int FeatureId, void *CallBackFunc);
	//grace period related functions
	LIBRARY_API int	IsLicenseInGracePeriod(int *Result);
	LIBRARY_API int SetGracePeriod(int MaxGracePeriod, int SecondsConsumed);
}

//this is a "gettickcount" implementation for our DLL
void StartLicenseGraceWatchdogThread();
void EndLicenseGraceWatchdogThread();

// use resource inside the DLL to store the grace period related data
#pragma pack(push,1)
struct DLLResourceStoreGraceStatus
{
	char	Header[8];			// we will seek to this header inside the DLL
	char	IsFileInitialized;	// Mark that we did run this function at least once. Required for decoding
	int		XORKey;				// slightly encrypt ourself to avoid humanly readable mode
	time_t	LicenseFirstUsed;	// only trigger grace period if the license was used while it was valid. Avoid reusing an expired license to abuse grace period
	time_t	LicenseSecondsUsed;	// seconds the license was used
	time_t	FirstTriggered;		// on hardware change or license expire we will start ticking
	time_t	GracePeriod;		// redundant value to be able to remake grace end on hack
	time_t	ShouldEnd;			// for security, double store the value
	time_t	RemainingSeconds;	// we will periodically update remaining seconds
};
extern char GracePeriodStore[sizeof(DLLResourceStoreGraceStatus)];
#pragma pack(pop)
