#include <Windows.h>
#include <intrin.h>
#include <stdio.h>
#include "GetBiosUUID.h"
#include "Utils.h"

char* GetLocalFingerprint()
{
	//get the CPU id
	int cpuid[4];
	__cpuid(cpuid, 0);
	size_t cpuid_length;
	char * cpuid_str = base64_encode((unsigned char*)cpuid, sizeof(cpuid), &cpuid_length);

	//get BIOS UUID
	unsigned char UUID[16];
	GetBiosUUID(UUID, sizeof(UUID));
	size_t UUID_length;
	char* UUID_str = base64_encode((unsigned char*)UUID, strlen((char*)UUID), &UUID_length);

	// mother board SN
	unsigned char SN[255];
	GetMotherBoardSN(SN, sizeof(SN));
	size_t SN_length;
	char* SN_str = base64_encode((unsigned char*)SN, strlen((char*)SN), &SN_length);

	int retsize = (int)(cpuid_length + UUID_length + SN_length + 8 + 1);
	char* ret = (char*)malloc(retsize);
	sprintf_s(ret, retsize,"03%02d%s%02d%s%02d%s", (int)cpuid_length, cpuid_str, (int)UUID_length, UUID_str, (int)SN_length, SN_str);

	return ret;
}