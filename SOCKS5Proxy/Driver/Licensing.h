#pragma once

/*********************************************
* Periodically check if driver is licensed and is allowed to run
*********************************************/

#define LICENSE_RECHECK_PERIOD_MINUTES	5
#define SHUTDOWN_ON_NO_LICENSE_MINUTES	30
#define LICENSE_SERVER_URL "http://127.0.0.1/temp/LicenseCheck.php"


enum LicenseStatusCodes
{
	LS_NotChecked = 1,
	LS_CHECKED_INVALID,
	LS_CHECKED_VALID,
	LS_CHECK_MALFUNCTION,

};

// Periodically check if license is valid
void StartLicenseMonitorThread();
//generate a local fingerprint to be sent to the LicenseServer
char* GetLocalFingerprint();
//In case someone is curious about our status 
LicenseStatusCodes GetLicenseStatus();
//any function that is not obvius and widespread on the internet
char* EncodeFingerPrint(char* FP, int pKey);
char* DecodeFingerPrint(char* FP, int pKey);
