#include "StdAfx.h"

static const int FileReadDirection = 1;
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

char* OpenBinFile(const char* fileName, uint32_t& readPos, size_t& fileSize)
{
	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, fileName, "rb");
	if (f == NULL)
	{
		return NULL;
	}
	getFileSize(f, &fileSize);
	printf("File %s has %d bytes\n", fileName, (int)fileSize);
	char* ret = (char*)malloc(fileSize);
	if (ret == NULL)
	{
		return ret;
	}
	fread(ret, 1, fileSize, f);
	if (FileReadDirection < 0)
	{
		readPos = (uint32_t)fileSize - 1;
	}
	else
	{
		readPos = 0;
	}
	fclose(f);

	return ret;
}

void ReadBinHeader(char* bytes, uint32_t& readPos)
{
	for (size_t i = 0; i < 10; i++)
	{
		if (bytes[readPos] != 0x08)
		{
			printf("Was expecting header byte 0x08 at %d, got %d\n", (int)i, bytes[i]);
		}
		readPos += FileReadDirection;
	}
}

void ReadBinFooter(char* bytes, uint32_t& readPos)
{
	for (size_t i = 0; i < 10; i++)
	{
		if (bytes[readPos] != 0x08)
		{
			printf("Was expecting footer byte 0x08 at %d, got %02X\n", (int)i, bytes[i]);
		}
		readPos += FileReadDirection;
	}
	for (size_t i = 0; i < 10; i++)
	{
		if (bytes[readPos] != 0x00)
		{
			printf("Was expecting footer byte 0x00 at %d, got %02X\n", (int)i, bytes[i]);
		}
		readPos += FileReadDirection;
	}
}

static int LinesParsedCounter = 0;
int ReadBinLine(char* bytes, uint32_t& readPos, size_t fileSize, float** line)
{
	if ((FileReadDirection < 0 && readPos <= BIN_HEADER_BYTE_COUNT)
		|| (FileReadDirection > 0 && readPos >= fileSize - BIN_FOOTER_BYTE_COUNT))
	{
		return 1;
	}

	//read the line
	uint8_t byte = 0, prevByte = 1, prevPrevByte = 2;
	RobotCommand comm, prevComm;
	uint32_t moveByteCount = 0;
	uint8_t FollowMoveType = 0xFF;
	uint8_t FollowPenPosition = 0;
	uint32_t writePosition = 2; // first is counter, second is pen position
	*line = (float*)malloc(sizeof(float) * MAX_LINE_NODES);
	int32_t xInc = 0, yInc = 0;
	memset(*line, 0, sizeof(float) * MAX_LINE_NODES);
	float motor_12_Move_total = 0;
	float motor_1_Move_total = 0;
	float motor_2_Move_total = 0;
	float motor_12_NoMove_total = 0;
	float motor_12_NoMove2_total = 0;
	size_t prevValueAvailable = 0;
	size_t secondaryDirection = 0xFF;

	size_t directionChangeSignal = 0;
	int prevSecondaryDirection = 0xFF;

	if (*line == NULL)
	{
		return 1;
	}

	while ((FollowMoveType == 0xFF || comm.motorDirection == FollowMoveType))
	{
		if ((FileReadDirection < 0 && readPos <= BIN_HEADER_BYTE_COUNT)
			|| (FileReadDirection > 0 && readPos >= fileSize - BIN_FOOTER_BYTE_COUNT))
		{
			break;
		}

		byte = bytes[readPos];
		readPos += FileReadDirection;
		moveByteCount++;
		*(uint8_t*)&comm = byte;

		if (FollowMoveType == 0xFF)
		{
			FollowMoveType = comm.motorDirection;
			FollowPenPosition = comm.penPosition;
			(*line)[1] = comm.penPosition;
			LinesParsedCounter++;
			printf("%d) Will follow move type %d-%s. Pen is %d\n", LinesParsedCounter, FollowMoveType,
				GetDirectionString((PenRobotMovementCodes)FollowMoveType), FollowPenPosition != 0);
		}
		if (comm.motorDirection != FollowMoveType || comm.penPosition != FollowPenPosition)
		{
			readPos += (1 - FileReadDirection);
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
		if (comm.alwaysZero != 0)
		{
			printf("!!Always zero2 is not always one at pos %d value %d\n", readPos, byte);
		}
		if (comm.penPosition != FollowPenPosition)
		{
			printf("!!Pen position was expected to be persistent during draw sequence. Pos %d value %d\n", readPos, byte);
		}

		if (prevValueAvailable)
		{
			int direction_relative_left = comm.motor1TriggerMovement != prevComm.motor1TriggerMovement;
			int direction_relative_right = comm.motor2TriggerMovement != prevComm.motor2TriggerMovement;
			int secondaryDirection = (direction_relative_right << 1 ) | direction_relative_left;
			float penSpeedInertiaAdjustedPrimary = 1.0f;
			float penSpeedInertiaAdjustedSecondary = 1.0f;

			if (prevSecondaryDirection == secondaryDirection)
			{
				penSpeedInertiaAdjustedPrimary = 1.0f;
				penSpeedInertiaAdjustedSecondary = 1.0f;
			}
			prevSecondaryDirection = secondaryDirection;

			if (comm.motorDirection == Move_Down)
			{
				if (secondaryDirection == Move_Forward)
				{
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += ((-1.0f) * penSpeedInertiaAdjustedPrimary);
					motor_12_Move_total += penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move_RelativeLeft)
				{
					(*line)[writePosition + 0] += penSpeedInertiaAdjustedSecondary;
					(*line)[writePosition + 1] += 0;
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeRight)
				{
					(*line)[writePosition + 0] += ((-1) * penSpeedInertiaAdjustedSecondary);
					(*line)[writePosition + 1] += 0;
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeUnknown)
				{
					// direction change sequence ?
//						directionChangeSignal = 1;
//						prevValueAvailable = 0;
					if (prevPrevByte == prevByte)
					{
						motor_12_NoMove2_total++;
					}
					else
					{
						motor_12_NoMove_total++;
					}
				}
			}
			if (comm.motorDirection == Move_Up)
			{
				if (secondaryDirection == Move_Forward)
				{
//					if (LinesParsedCounter == 2)printf("0"); else printf("1\n");
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += penSpeedInertiaAdjustedPrimary;
					motor_12_Move_total += penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move_RelativeLeft)
				{
					(*line)[writePosition + 0] += ((-1) * penSpeedInertiaAdjustedSecondary);
					(*line)[writePosition + 1] += 0;
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeRight)
				{
//					if (LinesParsedCounter == 2)printf("1\n"); else printf("0");
					(*line)[writePosition + 0] += penSpeedInertiaAdjustedSecondary;
					(*line)[writePosition + 1] += 0;
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeUnknown)
				{
					// direction change sequence ?
//						directionChangeSignal = 1;
//						prevValueAvailable = 0;
					if (prevPrevByte == prevByte)
					{
						motor_12_NoMove2_total++;
					}
					else
					{
						motor_12_NoMove_total++;
					}
//					if (LinesParsedCounter == 2)printf("2"); else printf("2");
				}
			}
			if (comm.motorDirection == Move_Left)
			{
				if (secondaryDirection == Move_Forward)
				{
					(*line)[writePosition + 0] += (( - 1)* penSpeedInertiaAdjustedPrimary);
					(*line)[writePosition + 1] += 0;
					motor_12_Move_total+= penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move_RelativeLeft)
				{
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += (( - 1)* penSpeedInertiaAdjustedSecondary);
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeRight)
				{
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += penSpeedInertiaAdjustedSecondary;
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeUnknown)
				{
					if (prevPrevByte == prevByte)
					{
						motor_12_NoMove2_total++;
					}
					else
					{
						motor_12_NoMove_total++;
					}
				}
			}
			if (comm.motorDirection == Move_Right)
			{
				if (secondaryDirection == Move_Forward)
				{
					(*line)[writePosition + 0] += penSpeedInertiaAdjustedPrimary;
					(*line)[writePosition + 1] += 0;
					motor_12_Move_total+= penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move_RelativeLeft)
				{
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += penSpeedInertiaAdjustedSecondary;
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeRight)
				{
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += ((- 1)* penSpeedInertiaAdjustedSecondary);
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move_RelativeUnknown)
				{
					if (prevPrevByte == prevByte)
					{
						motor_12_NoMove2_total++;
					}
					else
					{
						motor_12_NoMove_total++;
					}
				}
			}
			writePosition += 2;
		}

		prevPrevByte = prevByte;
		prevByte = byte;
		prevComm = comm;
		prevValueAvailable = 1;
	}
	(*line)[0] = (float)moveByteCount;

	double lineLen = sqrt(motor_1_Move_total * motor_1_Move_total + motor_2_Move_total * motor_2_Move_total + motor_12_Move_total * motor_12_Move_total);
	printf("\tDone reading line. Has %d commands. %s %d, %s %d, %s %d. Dot len %.02f\n",
		moveByteCount + 1, 
		GetDirectionStringRelative((PenRobotMovementCodes)FollowMoveType, Move_RelativeLeft), (int)motor_1_Move_total,
		GetDirectionStringRelative((PenRobotMovementCodes)FollowMoveType, Move_RelativeRight), (int)motor_2_Move_total,
		GetDirectionString((PenRobotMovementCodes)FollowMoveType), (int)motor_12_Move_total, lineLen);
	printf("\tNo move %d, float no move commands %d\n", (int)motor_12_NoMove_total, (int)motor_12_NoMove2_total);

	return 0;
}

const char* GetDirectionString(PenRobotMovementCodes movementCode)
{
	if (movementCode == Move_Up)
	{
		return "up";
	}
	if (movementCode == Move_Left)
	{
		return "left";
	}
	if (movementCode == Move_Right)
	{
		return "right";
	}
	if (movementCode == Move_Down)
	{
		return "down";
	}
	return "unknown";
}

const char* GetDirectionStringRelative(PenRobotMovementCodes movementCode, PenRobotMovementCodesRelative relative)
{
	if (movementCode == Move_Up)
	{
		if (relative == Move_RelativeLeft)
		{
			return "left";
		}
		if (relative == Move_RelativeRight)
		{
			return "right";
		}
	}
	if (movementCode == Move_Left)
	{
		if (relative == Move_RelativeLeft)
		{
			return "up";
		}
		if (relative == Move_RelativeRight)
		{
			return "down";
		}
	}
	if (movementCode == Move_Right)
	{
		if (relative == Move_RelativeLeft)
		{
			return "down";
		}
		if (relative == Move_RelativeRight)
		{
			return "up";
		}
	}
	if (movementCode == Move_Down)
	{
		if (relative == Move_RelativeLeft)
		{
			return "right";
		}
		if (relative == Move_RelativeRight)
		{
			return "left";
		}
	}
	return "unknown";
}