#include "stdafx.h"
#include <stdlib.h>
#include <winerror.h>
#include "ComputerFingerprint.h"

int EncryptBufferXORKey(unsigned char *buf, int BufLen, const unsigned char *Key, int KeyLen)
{
	//sanity checks
	if (buf == NULL || Key == NULL)
		return ERROR_INVALID_ADDRESS;

	if (BufLen == 0 || KeyLen == 0)
		return PEERDIST_ERROR_INVALID_CONFIGURATION;

	//process data in block of 4 bytes
	unsigned int *BufI = (unsigned int *)buf;
	const unsigned int *KeyI = (const unsigned int *)Key;
	int KeyLenI = KeyLen / sizeof(int);
	for (unsigned int i = 0; i < BufLen / sizeof(int); i++)
		BufI[i] = BufI[i] ^ KeyI[ i % KeyLenI];
	//process remaining 1 by 1
	for (int i = BufLen / sizeof(int) * sizeof(int); i < BufLen; i++)
		buf[i] = buf[i] ^ Key[i % KeyLen];

	return 0;
}

template <typename INT>
INT rol(INT val, unsigned char count) {
	return (val << count) | (val >> (sizeof(INT)*CHAR_BIT - count));
}

int EncryptBufferXORKeyRotate(unsigned char *buf, int BufLen, int XORKey)
{
	if (buf == NULL)
		return ERROR_INVALID_ADDRESS;

	if (BufLen == 0 || XORKey == 0)
		return ERROR_INVALID_ADDRESS;

	unsigned int tXORSeed = XORKey;
	for (int i = 0; i < BufLen; i++)
	{
		buf[i] ^= tXORSeed;
		tXORSeed = rol(tXORSeed, 3); // swap upper 3 bits with lower 3 bits ( ror(3) )
	}
	return 0;
}

int EncryptWithFingerprintContent(unsigned char *EncryptKey, int KeyLen, unsigned int Salt, unsigned char *buf, int BufLen)
{
	//Encrypt the encryption key. This adds algorithmic complexity
	int er1 = EncryptBufferXORKeyRotate((unsigned char*)EncryptKey, KeyLen, Salt);
	if (er1)
		return er1;

	//encrypt license content with the client fingerprint
	int er2 = EncryptBufferXORKey(buf, BufLen, (unsigned char*)EncryptKey, KeyLen);
	if (er2)
		return er2;
	return 0;
}
/*
int EncryptWithFingerprint(const char *Filename, unsigned int Salt, unsigned char *buf, int BufLen)
{
	ComputerFingerprint CF;
	if (Filename == NULL)
	{
		if( CF.GenerateFingerprint() != 0 )
			return ERROR_BAD_PATHNAME;
	}
	else if (CF.LoadFingerprint(Filename) != 0)
		return ERROR_BAD_PATHNAME;
	char *EncryptKey;
	int KeyLen;
	if (CF.DupEncryptionKey(&EncryptKey, KeyLen) != 0)
		return ERROR_BAD_ARGUMENTS;

	int er = EncryptWithFingerprintContent((unsigned char*)EncryptKey, KeyLen, Salt, buf, BufLen);

	//strdup uses free
	if (EncryptKey != NULL)
		free(EncryptKey);

	return er;
}

int DecryptWithFingerprint(const char *Filename, unsigned int Salt, unsigned char *buf, int BufLen)
{
	//right now this is simetric. Later might change it
	return EncryptWithFingerprint(Filename, Salt, buf, BufLen);
}
*/
