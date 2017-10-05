#pragma once
/// @cond DEV

//has to be enough to be able to store a valid fingerprint
//#define COMPUTER_FINGERPRINT_STORE_SIZE	500
#define GRACE_PERIOD_STORE_SIZE			(1000 + COMPUTER_FINGERPRINT_STORE_SIZE)

#define ERROR_COULD_NOT_LOAD_GRACE_DATA		(-1)
#define ERROR_GRACE_DATA_NOT_INITIALIZED	(-2)
#define ERROR_INPUT_BUFFER_TOO_SMALL		(-3)
#define ERROR_INVALID_PARAMETER_G			(-4)
#define ERROR_COULD_NOT_SAVE_GRACE_DATA		(-5)
#define ERROR_STORE_VERSION_MISMATCH		(-6)

#define MAX_GRACE_PERIOD_SECONDS			(31*24*60*60)		// 1 month ? Even if they reset everything, they can hack it only once

//#define ENABLE_INCLUDE_BACKUP_DECRYPT_KEY_GRACE

enum GracePeriodUpdateTypes
{
	GP_TRIGGER_GRACE_PERIOD = 1,			// when Hardware changes occure or when license duration ended
	GP_RESET_GRACE_PERIOD,					// when a valid license in valid conditions is get used, grace period is "reset"
	GP_SET_LICENSE_END,						// valid license will copy duration to grace period
	GP_CONSUME_REMAINING,					// periodic event update. Even if clock is rewinded, they can cheat only 1 update period
	GP_LICENSE_LIFETIME_USE_FLAGS,			// on this PC license was used with specific API calls. Clients should never use some of the API calls
};

#define GRACE_STORE_VERSION (0x010002)		// max 4 bytes

// use resource inside the DLL to store the grace period related data
#pragma pack(push,1)
struct GraceStatusResourceStore
{
	char	IsFileInitialized;		// Mark that we did run this function at least once. Required for decoding
	unsigned int XORKey;			// slightly encrypt ourself to avoid humanly readable mode
	//everything below should get encrypted
	int		StoreVersion;			// Use it to check if we loaded something valid and usable
#ifdef ENABLE_INCLUDE_BACKUP_DECRYPT_KEY_GRACE
	char	FingerPrint[COMPUTER_FINGERPRINT_STORE_SIZE];	// on first time init we store a valid fingerprint. We will need this to be able to decode our license on HW changes
#endif
	int		FingerPrintSize;		//number of bytes used for fingerprinting
	int		TriggerCount;
	int		APIsUsedFlags;			// License APIs called will set flags. These flags are never removed, only set
	time_t	LicenseFirstUsed;		// only trigger grace period if the license was used while it was valid. Avoid reusing an expired license to abuse grace period
	time_t	LicensePeriodicLastUpdate; // timestamp when we last updated usage amount
	time_t	LicenseSecondsUsed;		// seconds the license was used
	time_t	LicenseShouldEnd;		// used for cross checking
	char	LicenseShouldEndc[22];	// anti data tempering. Store data in different format but alost the same value
	time_t	GraceTriggeredAt;		// on hardware change or license expire we will start ticking
	time_t	GracePeriod;			// redundant value to be able to remake grace end on hack
	time_t	GraceShouldEnd;			// for security, double store the value
	time_t	GraceRemainingSeconds;	// we will periodically update remaining seconds
};
struct GraceStatusDLLResourceStore
{
	char						Header[25];				// we will seek to this header inside the DLL
	GraceStatusResourceStore	Data;
};
//extern char GracePeriodStore[GRACE_PERIOD_STORE_SIZE];
#pragma pack(pop)

class LicenseGracePeriod
{
public:
	static int UpdateStatus(int ActionType, int Value);
	static int GetStatus(int *IsTriggered, time_t *SecondsRemain);
	static int GetSavedFingerprint(unsigned char *Store, int MaxStoreLen, int *RetSize);
private:
	GraceStatusDLLResourceStore Status;
};

//this is a "gettickcount" implementation for our DLL. It is not precise! Expected deviation at every update interval is around 50ms
//if we use 5 minute update intervals, daily deviation is 14 seconds ! Yearly is 86 minutes !
#define PERIODIC_UPDATE_EXPECTED_YEARLY_DEVIATION_SECONDS	5126	
void StartLicenseGraceWatchdogThread();
void EndLicenseGraceWatchdogThread();

/// @endcond