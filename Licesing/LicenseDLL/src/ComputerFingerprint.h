#pragma once

class GenericDataStore;

class LIBRARY_API ComputerFingerprint
{
public:
	ComputerFingerprint();
	~ComputerFingerprint();

	int	GenerateFingerprint();							// gather PC specific info

	int SaveFingerprint(char *FileName);				// Save fingerprint to a file that can be sent to Siemens licensing team
	int LoadFingerprint(char *FileName);				// load fingerprint to a file that can be sent to Siemens licensing team

	int EncodeLicense(char *License, int Len);			// use internal state to encode license. Only a similar PC can decode license
	int DecodeLicense(char *License, int Len);		    // use internal state to decode license. Only a similar PC can decode license

	bool CompareFingerprint(ComputerFingerprint *cf);	// very basic check if 2 fingerprints match. Not all features are required to be present. The order of the featues can be different
	void Print();										// print out Fingerprint data in a humanly readable format. This is for debugging purpuses only
private:
	void ApplyHash(int Seed);							// Some encryption. Seed should not be stored here
	GenericDataStore *FingerprintData;
};

// factory functions to avoid mixing usage of allocators
extern "C"
{
	LIBRARY_API ComputerFingerprint *CreateComputerFingerprint();
	LIBRARY_API void DestroyComputerFingerprint(ComputerFingerprint *);
}