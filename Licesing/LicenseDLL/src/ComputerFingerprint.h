#pragma once

#define ERROR_CF_CONTENT_INVALID (-1)

class GenericDataStore;

class LIBRARY_API ComputerFingerprint
{
public:
	ComputerFingerprint();
	~ComputerFingerprint();

	int	GenerateFingerprint();												// gather PC specific info
	int AppendClientInfo(const char *MachineRole, const char *ClientName);	// append data to the info block

	int SaveFingerprint(const char *FileName);			// Save fingerprint to a file that can be sent to Siemens licensing team
	int LoadFingerprint(const char *FileName);			// load fingerprint to a file that can be sent to Siemens licensing team

	int DupEncryptionKey(char **Key, int &Len);		    // use internal state to decode license. Only a similar PC can decode license

	void Print();										// print out Fingerprint data in a humanly readable format. This is for debugging purpuses only
private:
	GenericDataStore *FingerprintData;
};

// factory functions to avoid mixing usage of allocators
extern "C"
{
	LIBRARY_API ComputerFingerprint *CreateComputerFingerprint();
	LIBRARY_API void DestroyComputerFingerprint(ComputerFingerprint **);
}