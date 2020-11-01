#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Utils.h"

char* EncodeFingerPrint(char* FP, int pKey)
{
	size_t FPLen = strlen(FP);

	unsigned char* t = (unsigned char*)malloc(FPLen + 12 + 4); // store key + add buffer overun protection
	sprintf_s((char*)t, FPLen+12+4, "%12d%s", pKey, FP);
	unsigned char* Keys = (unsigned char*)&pKey;
	for (int i = 0; i < FPLen; i++)
		t[i+12] ^= Keys[i%4];

	size_t OutputLen;
	char* ret = base64_encode((unsigned char*)t, FPLen + 12, &OutputLen);
	free(t);

	return ret;
}

char* DecodeFingerPrint(char* FP, int Key)
{
	int pKey = Key - 0x00001234;
	size_t FPLen = strlen(FP);

	size_t OutputLen = 0;
	unsigned char* ret = base64_decode(FP, FPLen, &OutputLen);

	unsigned char* Keys = (unsigned char*)&pKey;
	for (int i = 0; i < OutputLen; i++)
		ret[i] ^= Keys[i % 4];

	return (char*)ret;
}