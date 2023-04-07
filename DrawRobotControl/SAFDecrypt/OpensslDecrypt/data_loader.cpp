#include <stdio.h>
#include <stdlib.h>

unsigned char bf1[1000], bf2[1000];
unsigned char* FirmWareFile = NULL;
size_t FirmWareFileSize;
int LoadFileContent()
{
	FILE* f1, * f2;
	errno_t open_err;
	open_err = fopen_s(&f1, "1.saf", "rb");
	open_err = fopen_s(&f2, "2.saf", "rb");
	if (f1 == NULL || f2 == NULL)
	{
		return 1;
	}

	fread(bf1, 1, sizeof(bf1), f1);
	fread(bf2, 1, sizeof(bf2), f2);

	fclose(f1);
	fclose(f2);

	FirmWareFile = (unsigned char*)malloc(100 * 1024 * 1024);
	if (FirmWareFile == NULL)
	{
		printf("Failed to allocate buffer for firmware file");
	}
	open_err = fopen_s(&f1, "Head-1.7.bin", "rb");
	if (f1 == NULL)
	{
		return 1;
	}

	FirmWareFileSize = fread(FirmWareFile, 1, 100 * 1024 * 1024, f1);
	fclose(f1);

	return 0;
}

int key1Start = 0;
int key1Len = 0;
int key2Start = 0;
int key2Len = 0;

void ResetFileReadStatus()
{
	key1Start = 0x08 - 1;
	key1Len = 16; // 128 bits
	key2Start = 0;
	key2Len = 16; // 128 bits
}

void SetKeySizes(int k1, int k2)
{
	key1Len = k1; // this is actual key
	key2Len = k2; // this is salt key aka. IV
}

int AES_GetNextKeyBuffs(unsigned char* out1_buf, unsigned char* out2_buf, unsigned char* out3_buf, unsigned char* out4_buf)
{
	key1Start++;
	// try all key positions first
	if (key1Start == 0xE8)
	{
		key1Start = 0x08 - 1;
		key2Start++;
	}
	if (key2Start + 16 > FirmWareFileSize)
	{
		return 1;
	}
	static __int64 prevReportedProgress = 0;
	__int64 progress = (__int64)key2Start * (__int64)100 / FirmWareFileSize;
	if (progress != prevReportedProgress)
	{
		prevReportedProgress = progress;
		printf("Progress : %lld   \r", progress);
	}

	for (int i = 0; i < key1Len; i++)
	{
		out1_buf[i] = bf1[key1Start + i];
		out3_buf[i] = bf2[key1Start + i];
	}

	int allEmpty = 1;
	do {
		allEmpty = 1;
		for (int i = 0; i < key2Len; i++)
		{
			out2_buf[i] = FirmWareFile[(key2Start + i) * 2];
			out4_buf[i] = FirmWareFile[(key2Start + i) * 2];
			if (out2_buf[i] != 0xFF)
			{
				allEmpty = 0;
			}
		}
		if (allEmpty == 1)
		{
			key2Start++;
			if (key2Start >= FirmWareFileSize)
			{
				return 1;
			}
		}
	} while (allEmpty == 1);

	return 0;
}

int RSA_GetNextKeyBuffs(unsigned char* out1_buf, unsigned char* out2_buf)
{
	key1Start++;
	// try all key positions first
	if (key1Start >= 0xE8)
	{
		return 1;
	}
	static __int64 prevReportedProgress = 0;
	__int64 progress = (__int64)key1Start * (__int64)100 / 0xE8;
	if (progress != prevReportedProgress)
	{
		prevReportedProgress = progress;
		printf("Progress : %lld   \r", progress);
	}

	for (int i = 0; i < key1Len; i++)
	{
		out1_buf[i] = bf1[key1Start + i];
		out2_buf[i] = bf2[key1Start + i];
	}

	return 0;
}

int GetDataBlocks(unsigned char** out_buf1, unsigned char** out_buf2)
{
	*out_buf1 = &bf1[0x158];
	*out_buf2 = &bf2[0x158];

	return 0;
}