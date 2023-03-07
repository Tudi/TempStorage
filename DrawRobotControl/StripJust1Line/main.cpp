#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


int getFileSize(FILE* f, size_t* size)
{
	if (fseek(f, 0, SEEK_END) != 0)
	{
		return 1;
	}

	*size = ftell(f);

	if (fseek(f, 0, SEEK_SET) != 0)
	{
		return 1;
	}

	return 0;
}

typedef enum PenRobotMovementCodes
{
	Move_Down = 0x8,
	Move_Left = 0x9,
	Move_Right = 0xA,
	Move_Up = 0xB,
}PenRobotMovementCodes;

static int LinesParsedCounter = 0;
int ExtractMovement(FILE* f, uint32_t& readPos, size_t fileSize)
{
	if (readPos >= fileSize - 20)
	{
		return 1;
	}

	//read the line
	uint8_t byte, prevByte;
	uint32_t sequence = 0;
	uint32_t SumFirstBytes = 0;
	uint32_t SumFirstBytesFlipped = 0;
	uint32_t moveByteCount = 0;
	uint8_t FollowMoveType = 0;
	uint8_t FollowPenPosition = 0;
	uint32_t* unique8bits_0 = (uint32_t*)malloc(sizeof(uint32_t) * 256);
	uint32_t* unique8bits_1 = (uint32_t*)malloc(sizeof(uint32_t) * 256);
	uint32_t* unique8bits_2 = (uint32_t*)malloc(sizeof(uint32_t) * 65536);
	uint32_t* unique16bits_0 = (uint32_t*)malloc(sizeof(uint32_t) * 65536);
	uint32_t* unique16bits_1 = (uint32_t*)malloc(sizeof(uint32_t) * 65536);
	uint32_t* unique16bits_2 = (uint32_t*)malloc(sizeof(uint32_t) * 65536);
	memset(unique8bits_0, 0, sizeof(uint32_t) * 256);
	memset(unique8bits_1, 0, sizeof(uint32_t) * 256);
	memset(unique8bits_2, 0, sizeof(uint32_t) * 65536);
	memset(unique16bits_0, 0, sizeof(uint32_t) * 65536);
	memset(unique16bits_1, 0, sizeof(uint32_t) * 65536);
	memset(unique16bits_2, 0, sizeof(uint32_t) * 65536);
	uint32_t bit6count = 0;
	uint32_t bit7count = 0;
	uint32_t bit6FlipCount = 0;
	uint32_t bit7FlipCount = 0;
	if (unique8bits_0 == NULL || unique8bits_1 == NULL || unique16bits_0 == NULL || unique16bits_1 == NULL || unique16bits_2 == NULL)
	{
		return 1;
	}

	while ((FollowMoveType == 0 || (byte & 0x0F) == FollowMoveType)
		&& readPos < fileSize - 20) // 10*0x08 and 10*0x00
	{
		fread(&byte, 1, 1, f);
		readPos++;
		moveByteCount++;

		if (FollowMoveType == 0)
		{
			FollowMoveType = (byte & 0x0F);
			FollowPenPosition = (byte & 0x20);
			prevByte = byte;
			printf("%d) Will follow move type %d. Pen is %d\n", LinesParsedCounter, FollowMoveType, FollowPenPosition != 0);
			LinesParsedCounter++;
		}
		if (((byte & 0x0F) != FollowMoveType)
			|| (byte & 0x20) != FollowPenPosition)
		{
			if (fseek(f, -1, SEEK_CUR) != 0)
			{
				return 1;
			}
			readPos--;
			moveByteCount--;
			break;
		}

		sequence = (sequence << 8) | byte;

		if (moveByteCount % 2 == 0)
		{
			unique16bits_0[sequence & 0xFFFF]++;
		}
		if (moveByteCount % 2 == 1)
		{
			unique16bits_1[sequence & 0xFFFF]++;
		}
		unique16bits_2[sequence & 0xFFFF]++;
		unique8bits_0[byte]++;
		if ((((byte & 0xF0) >> 4) & 1) == 1)
		{
			byte = byte;
		}

		uint32_t seqHighBits = ((byte & 0xF0) >> 4);
		if (seqHighBits != 0)
		{
			seqHighBits = (seqHighBits << 28) | (seqHighBits << 20) | (seqHighBits << 12) | (seqHighBits << 4);
			if ((sequence & 0xF0F0F0F0) == seqHighBits)
			{
//				printf("\t Pen status change detected at position %d = %04X value %08X\n", readPos, readPos, sequence);
			}
		}

		if ((byte & 0x04) != 0)
		{
			printf("!!Always zero1 is not always one at pos %d value %d ( paper swap )\n", readPos, byte);
		}
		if ((byte & 0x08) != 0x08)
		{
			printf("!!Always one is not always one at pos %d value %d ( pen move )\n", readPos, byte);
		}
		if ((byte & 0x10) != 0)
		{
			printf("!!Always zero2 is not always one at pos %d value %d\n", readPos, byte);
		}
		if ((byte & 0x20) != FollowPenPosition)
		{
			printf("!!Pen position was expected to be persistent during draw sequence. Pos %d value %d\n", readPos, byte);
		}
		uint32_t byteMasked = byte & (~0x08); // always one
		byteMasked = byteMasked & (~0x20); // remove pen position
		byteMasked = byteMasked & (~0x03); // remove motor movement
		unique8bits_1[byteMasked]++;
		printf("%02X ", byteMasked);
		unique8bits_2[moveByteCount] = byteMasked;

//		SumFirstBytes += ((byteMasked & 0xF0) >> 4);
		SumFirstBytes += ((byteMasked & (0x80|0x40)) >> 4);
		SumFirstBytesFlipped += (0x0C - ((byteMasked & (0x80 | 0x40)) >> 4));

		if (byteMasked & 0x40)bit6count++;
		if (byteMasked & 0x80)bit7count++;

		if ((byteMasked & 0x40) != (prevByte & 0x40)) bit6FlipCount++;
		if ((byteMasked & 0x80) != (prevByte & 0x80)) bit7FlipCount++;

		prevByte = byte;
	}
	printf("\n");

/*	printf("\t reveresed movement : ");
	for (size_t i = 0; i < moveByteCount; i++)
	{
		if (unique8bits_2[moveByteCount - i] != 0)
		{
			printf("%02X ", unique8bits_2[i]);
		}
	}
	free(unique8bits_2);
	printf("\n");*/

//	printf("\t Move first 4 bits summed = %d . Flipped summed = %d\n", SumFirstBytes, SumFirstBytesFlipped);
	printf("\t Move contained steps = %d . Remaining bytes = %d\n", moveByteCount, (uint32_t)fileSize - readPos);
	printf("\t Bit 6 count = %d \n", bit6count);
	printf("\t Bit 7 count = %d \n", bit7count);
//	printf("\t Bit 6+7 count = %d \n", bit6count + bit7count);
	printf("\t Bit 6 Flip count = %d \n", bit6FlipCount);
	printf("\t Bit 7 Flip count = %d \n", bit7FlipCount);
	printf("\t Bit 6 + 7 Flip count = %d \n", bit6FlipCount + bit7FlipCount);

	printf("\t 0 Unique 8 bit numbers : ");
	for (size_t i = 0; i < 256; i++)
	{
		if (unique8bits_0[i] != 0)
		{
			printf("%02X-%d ", (int)i, unique8bits_0[i]);
		}
	}
	free(unique8bits_0);
	printf("\n");

#if 0
	printf("\t 1 Unique 8 bit numbers : ");
	for (size_t i = 0; i < 256; i++)
	{
		if (unique8bits_1[i] != 0)
		{
			printf("%02X-%d ", (int)i, unique8bits_1[i]);
		}
	}
	free(unique8bits_1);
	printf("\n");

	printf("\t 0 Unique 16 bit numbers : ");
	for (size_t i = 0; i < 65536; i++)
	{
		if (unique16bits_0[i] != 0)
		{
			printf("%04X-%d ", (int)i, unique16bits_0[i]);
		}
	}
	free(unique16bits_0);
	printf("\n");

	printf("\t 1 Unique 16 bit numbers : ");
	for (size_t i = 0; i < 65536; i++)
	{
		if (unique16bits_1[i] != 0)
		{
			printf("%04X-%d ", (int)i, unique16bits_1[i]);
		}
	}
	free(unique16bits_1);
	printf("\n");

	printf("\t 2 Unique 16 bit numbers : ");
	for (size_t i = 0; i < 65536; i++)
	{
		if (unique16bits_2[i] != 0)
		{
			printf("%04X-%d ", (int)i, unique16bits_2[i]);
		}
	}
	free(unique16bits_2);
	printf("\n");
#else
	free(unique16bits_0);
	free(unique16bits_1);
	free(unique16bits_2);
#endif
	return 0;
}

void ReadHeader(FILE* f, uint32_t& readPos)
{
	uint8_t byte[10];
	fread(&byte, 1, 10, f);
	readPos += 10;
	for (size_t i = 0; i < sizeof(byte); i++)
	{
		if (byte[i] != 0x08)
		{
			printf("Was expecting header byte 0x08 at %d, got %d\n", (int)i, byte[i]);
		}
	}
}

void ReadFooter(FILE* f, uint32_t& readPos)
{
	uint8_t byte[10];
	fread(&byte, 1, 10, f);
	readPos += 10;
	for (size_t i = 0; i < sizeof(byte); i++)
	{
		if (byte[i] != 0x08)
		{
			printf("Was expecting footer byte 0x08 at %d, got %02X\n", (int)i, byte[i]);
		}
	}
	fread(&byte, 1, 10, f);
	readPos += 10;
	for (size_t i = 0; i < sizeof(byte); i++)
	{
		if (byte[i] != 0x00)
		{
			printf("Was expecting footer byte 0x00 at %d, got %02X\n", (int)i, byte[i]);
		}
	}
}

void ReadPauseTransition(FILE* f, uint32_t& readPos)
{
	uint32_t sequence = 0;
	fread(&sequence, 1, 4, f);
	readPos += 4;
}

int main()
{
//	const char* fileName = "../BinFiles/0001 Vertical Half Inch Line.bin";
//	const char* fileName = "../BinFiles/0003 Horizontal Half Inch Line.bin";
//	const char* fileName = "../BinFiles/0002 Three Half Inch Vertical Lines Half Inch Apart.bin";
//	const char* fileName = "../BinFiles/0004 Three Half Inch Horizontal Lines Half Inch Apart.bin";
//	const char* fileName = "../BinFiles/0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.bin";
//	const char* fileName = "../BinFiles/0007 Vertical Half Inch Line followed by Horizontal Half Inch Line with Transition.bin";
//	const char* fileName = "../BinFiles/Envelope With Transition.bin";
	const char* fileName = "../ConstructBinFiles/BinFiles/CheckArmAngle_11_0_20_40_60_80.bin";
	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, fileName, "rb");
	if (f == NULL)
	{
		return 1;
	}
	size_t fileSize = 0;
	getFileSize(f, &fileSize);
	printf("File %s has %d bytes\n", fileName, (int)fileSize);
	uint32_t readPos = 0;
	ReadHeader(f, readPos);
//	ReadPauseTransition(f, readPos);
	for (size_t i = 0; i< 5000; i++)
	{
		int ret = ExtractMovement(f, readPos, fileSize);
		if (ret != 0)
		{
			break;
		}
	}
	ReadFooter(f, readPos);

	fclose(f);

	return 0;
}