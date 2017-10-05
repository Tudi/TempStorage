#pragma once
/*! \mainpage Licensing module
*
* This project helps helps adding licensing to other projects or modules.
* Current licensing mechanism :
* - \subpage LicenseSeed "Obtain target PC unique fingerprint"
*
* - \subpage GenerateLicense "Generate a time bound license"
*
* - \subpage ConsumeLicense "Query a license for activation key"
*
*/
/*! \page LicenseSeed Obtain target PC unique fingerprint
*
* To uniquely identify a PC multiple computer specific hardware information is gathered. Ex: MAC, CPU ID, BIOS ID...
* From the unique fignerprint an encryption key is generated that will be used to encoder activation keys
* Once an activation key is encoded by this fingerprint, only on the target machine can be decoded
*/
/*! \page GenerateLicense Generate a time bound license
*
* Take Start date, duration and a list of activation keys. Use a specific license seed to encrypt the list in a way that only on target machine can be decoded
* List will be protected against tempering on multiple levels and ways. 
* Data is redundantly stored and can self check for validity
*/
/*! \page ConsumeLicense Query a license for activation key
*
* API provided allows Siemens projects to query a license for specific activation keys.
* By design an activation key is a missing part of a project or feature. Without the missing part the project will be unable to run. This way it is impossible to create a false license without having a valid license.
* 
*/

#ifdef _DEBUG
	#define LICENSE_DURATION_UPDATE_INTERVAL	( 20000 )	// trigger lots of events to not need to wait for testing. Needs to be at least 1 second
#else
	#define LICENSE_DURATION_UPDATE_INTERVAL	( 5 * 60 * 1000 )
#endif

#define GenerateSaltKey(a,b)	((unsigned int)(1|(a+b)))
#define GenerateSaltKey2		(GenerateSaltKey( GetTickCount(), time(NULL) ))
#define GenerateSaltKey3(i,j)	( GetTickCount() | ( GetTickCount() << ((0+i)*8+j)) | ( GetTickCount() << ((1+i)*8+j)) | ( GetTickCount() << ((2+i)*8+j)))

#define HardCodedStringLimitLic 50

#define ENABLE_ANTI_HACK_CHECKS

#define MAX_LICENSE_SIZE_BYTES				(100*1024*1024)
#define PERMANENT_LICENSE_MAGIC_DURATION	(~((unsigned int)0))

#ifndef min
    #define min(a,b) (a>b?a:b)
#endif

#define ENABLE_INCLUDE_BACKUP_DECRYPT_KEY

/// @cond DEV
#pragma pack(push,1)
class LicenseNode
{
public:
	int		GetProjectId() { return ProjectId; }
	void	SetProjectId(int pProjectId) { ProjectId = pProjectId; }
	int		GetFeatureId() { return FeatureId; }
	void	SetFeatureId(int pFeatureId) { FeatureId = pFeatureId; }
	size_t	GetKeyLen() { return min(HardCodedStringLimitLic, strlen(ActivateKey1)); }
	int		GetKey(char *Store, int MaxStore)
	{
		if (Store == NULL || MaxStore == 0)
			return 1;
		int Ind = 0;
		while (Ind < MaxStore - 1 && ActivateKey1[Ind] != 0)
		{
			Store[Ind] = ActivateKey1[Ind] + ActivateKey2[Ind] - 11;
			Ind++;
		}
		Store[Ind] = 0;
		return 0;
	}
	//we could simply copy the string. Making store complicated to avoid humanly readable format. We could use some simple xorkey, but reading from 2 addresses is more complicated to debug
	void	SetKey(const char *pKey)
	{
		if (pKey == NULL)
			return;
		int Ind = 0;
		while (Ind < HardCodedStringLimitLic - 1 && pKey[Ind] != 0)
		{
			char a = pKey[Ind] ^ ( (( Ind + 3 ) % pKey[Ind]) >> 1 );
			char b = 11 + pKey[Ind] - a; // should be always greateer than 0
			ActivateKey1[Ind] = b;
			ActivateKey2[Ind] = a;
			Ind++;
		}
		ActivateKey1[Ind] = 0;
		ActivateKey2[Ind] = 0;
	}
private:
	int		ProjectId;			// get the list from static files
	int		FeatureId;			// get the list from static files
	char	ActivateKey1[HardCodedStringLimitLic];
	char	ActivateKey2[HardCodedStringLimitLic];
};
struct FileLicenseHeader
{
	FileLicenseHeader() {};
	FileLicenseHeader(int tTotalSize, int tSizeEncrypted, int tEncryptSalt, int tEncryptSaltNE)
	{
		TotalSize = tTotalSize;
		SizeEncrypted = tSizeEncrypted;
		EncryptSalt = tEncryptSalt;
		EncryptSaltNE = tEncryptSaltNE;
	}
	int TotalSize;
	int SizeEncrypted;
	int EncryptSalt;
	int EncryptSaltNE;
};
#pragma pack(pop)

class GenericDataStore;
/// @endcond

//save status. Might be used at some point to investigate hacking
enum LicenseStateFlags
{
	LSF_LOADED_FROM_FILE		= 1,
	LSF_DURATION_IS_SET			= 2,		// should never be present in client side logs
	LSF_FEATURE_ADDED			= 4,		// should never be present in client side logs
	LSF_FEATURE_REQUESTED		= 8,
	LSF_SAVED_TO_FILE			= 16,		// should never be present in client side logs
	LSF_DURATION_REQUESTED		= 32,		// should never be present in client side logs
	LSF_GRACE_STATUS_QUERIED	= 64,
	LSF_SIEMENS_ONLY_FLAGS		= (LSF_DURATION_IS_SET | LSF_FEATURE_ADDED | LSF_SAVED_TO_FILE),
};

class ComputerFingerprint;

// Main class to handle license related API calls
class LIBRARY_API License
{
public:
	License();
	~License();
	int					SetDuration(time_t pStartDate, unsigned int pDuration, unsigned int pGraceDuration);	// duration values are in seconds
	int					AddProjectFeature(int ProjectId, int FeatureId, const char *ActivationKey);
	int					SaveToFile(const char *FileName, ComputerFingerprint *FP, int Append);					// Save License to a file that can be sent to Siemens licensing team
	int					SaveToFile_(const char *FileName, const char *FingerprintFilename, int Append = 0);		// depracated save 
	int					LoadFromFile(const char *FileName, const char *FingerprintFilename = NULL);				// load a license while not using a local fingerprint file. This should only be used by LicenseInfo Project
	int					GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);	// API to check if a project-feature is usable based on the license file we loaded
	int					GetRemainingSeconds(time_t *RemainingSeconds);											// this does not include grace period
	int					IsGracePeriodTriggered(time_t *RemainingSeconds,char *IsTriggered);						// check if grace period is getting used right now
	int					GetStartStamp(time_t *StartStamp);														// internally used function by LicenseManager to get the start stamp of the license
	int					GetEndStamp(time_t *EndStamp);															// internally used function by LicenseManager to get the end stamp of the license
//	int					GetTimeInfo(time_t *StartStamp, time_t *EndStamp, bool IsGraceTriggered, time_t *GraceRemainingSeconds);
#ifdef _DEBUG
	void				PrintNodes();
#endif
private:
	int					InitFromBuffer(char *buf, FileLicenseHeader *LFH, char *key, int KeyLen);				// init internal states from a buffer fetched either from file or DB
	int					GetTempDecryptKeyDup(unsigned char **Key, int *KeyLen, char *KeyList, int KeyListSize);	// we appended a temp HW key list to the license we can use to regenerate a decrypt key if we must
	void				UpdateGraceStatus();																	// grace disable event might appear at each correct usage of the license
	time_t				StartStamp;
	unsigned int		Duration;
	time_t				GracePeriod;	GenericDataStore	*NodeListEncrypted;
	int					UsageStateFlags;			// logs and more logs about license usage. 
	int					DisableEdit;				// minor protection to deny API usage to edit a license if it was loaded from a file
	int					DisableQuery;				// minor protection to deny API calls on a newly constructed license
	int					InitializedWithBackupKey;	// this license instance is using the backup key to provide activation keys
	int					LoadedEncryptedDataFromFile;// If the content of the license was loaded from a file that required decryption key
};

#ifdef _DEBUG
	extern int ENABLE_ERROR_PRONE_TESTS;
	LIBRARY_API void EnableErrorTests();
#endif
