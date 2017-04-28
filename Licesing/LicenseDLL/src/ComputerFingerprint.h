#pragma once

#define ERROR_CF_CONTENT_INVALID	(-1)
#define ERROR_GENERIC				(-2)

class GenericDataStore;

class LIBRARY_API ComputerFingerprint
{
public:
	ComputerFingerprint();
	~ComputerFingerprint();

	int	GenerateFingerprint();												// gather PC specific info
	int AppendClientInfo(const char *MachineRole, const char *ClientName);	// append data to the info block

	int SaveFingerprint(const char *FileName);			// Save fingerprint to a file that can be sent to Siemens licensing team
	int LoadFingerprint(const char *buf, int size);		// load fingerprint from a buffer that can be sent to Siemens licensing team
	int LoadFingerprint(const char *FileName);			// load fingerprint from a file that can be sent to Siemens licensing team

	int DupEncryptionKey(char **Key, int &Len);		    // use internal state to decode license. Only a similar PC can decode license
	int DupContent(char **Content, int *Len);		    // when we want to save it to another storage like DB
	int DupField(char **Content, int *Size, int Type);	// get only 1 field from the fingerprint

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