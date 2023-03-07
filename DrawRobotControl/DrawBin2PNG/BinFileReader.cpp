#include "StdAfx.h"

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

FILE* OpenBinFile(const char* fileName, uint32_t& readPos, size_t& fileSize)
{
	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, fileName, "rb");
	if (f == NULL)
	{
		return NULL;
	}
	getFileSize(f, &fileSize);
	printf("File %s has %d bytes\n", fileName, (int)fileSize);
	readPos = 0;

	return f;
}

void ReadBinHeader(FILE* f, uint32_t& readPos)
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

void ReadBinFooter(FILE* f, uint32_t& readPos)
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

static int LinesParsedCounter = 0;
int ReadBinLine(FILE* f, uint32_t& readPos, size_t fileSize, int32_t** line)
{
	if (readPos >= fileSize - 20)
	{
		return 1;
	}

	//read the line
	uint8_t byte, prevByte;
	RobotCommand comm, prevComm;
	uint32_t moveByteCount = 0;
	uint8_t FollowMoveType = 0xFF;
	uint8_t FollowPenPosition = 0;
	uint32_t writePosition = 2; // first is counter, second is pen position
	*line = (int32_t*)malloc(sizeof(int32_t) * MAX_LINE_NODES);
	int32_t xInc = 0, yInc = 0;
	memset(*line, 0, sizeof(int32_t) * MAX_LINE_NODES);
	if (*line == NULL)
	{
		return 1;
	}

	while ((FollowMoveType == 0xFF || comm.motorDirection == FollowMoveType)
		&& readPos < fileSize - BIN_FOOTER_BYTE_COUNT) // 10*0x08 and 10*0x00
	{
		fread(&byte, 1, 1, f);
		readPos++;
		moveByteCount++;
		*(uint8_t*)&comm = byte;

		if (FollowMoveType == 0xFF)
		{
			FollowMoveType = comm.motorDirection;
			FollowPenPosition = comm.penPosition;
			(*line)[1] = comm.penPosition;
			prevByte = ~byte;
			*(uint8_t*)&prevComm = prevByte;
			LinesParsedCounter++;
			printf("%d) Will follow move type %d. Pen is %d\n", LinesParsedCounter, FollowMoveType, FollowPenPosition != 0);

			if (comm.motorDirection == Move_Up)
			{
				yInc = -1;
				xInc = 0; // would expect 0
			}
			if (comm.motorDirection == Move_Left)
			{
				yInc = 0; // would expect 0
				xInc = -1;
			}
			if (comm.motorDirection == Move_Right)
			{
				yInc = 0; // would expect 0
				xInc = 1;
			}
			if (comm.motorDirection == Move_Down)
			{
				yInc = 1;
				xInc = 0; // would expect 0
			}
		}
		if (comm.motorDirection != FollowMoveType || comm.penPosition != FollowPenPosition)
		{
			if (fseek(f, -1, SEEK_CUR) != 0)
			{
				return 1;
			}
			readPos--;
			moveByteCount--;
			break;
		}

		if (comm.Transition != 0)
		{
			printf("!!Always zero1 is not always one at pos %d value %d ( paper swap )\n", readPos, byte);
		}
		if (comm.penIsMoving != 1)
		{
			printf("!!Always one is not always one at pos %d value %d ( pen move )\n", readPos, byte);
		}
		if (comm.alwaysZero_2 != 0)
		{
			printf("!!Always zero2 is not always one at pos %d value %d\n", readPos, byte);
		}
		if (comm.penPosition != FollowPenPosition)
		{
			printf("!!Pen position was expected to be persistent during draw sequence. Pos %d value %d\n", readPos, byte);
		}

		int xChanged = comm.motor1TriggerMovement != prevComm.motor1TriggerMovement;
		int yChanged = comm.motor2TriggerMovement != prevComm.motor2TriggerMovement;
		if (xChanged || yChanged)
		{
			if (xChanged && yChanged)
			{
				(*line)[writePosition + 0] = xInc;
				(*line)[writePosition + 1] = yInc;
			}
			else
			{
				(*line)[writePosition + 0] = xChanged;
				(*line)[writePosition + 1] = yChanged;
			}
			writePosition += 2;
		}

		prevByte = byte;
		prevComm = comm;
	}
	(*line)[0] = writePosition;
	printf("\tDone reading line. Has %d moves\n", moveByteCount);

	return 0;
}
