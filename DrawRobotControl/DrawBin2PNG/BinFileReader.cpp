#include "StdAfx.h"

// Should set it to 0x08
void RobotCommand_Constructor(RobotCommand *comm, BYTE fromByte = 0)
{
	if (fromByte != 0)
	{
		*(BYTE*)comm = fromByte;
	}
	else
	{
		memset(comm, 0, sizeof(RobotCommand));
		comm->penIsMoving = 1;
	}
}

// in theory, robot will not recognize commands that are exactly the same
// this is just a theory at this point
int IsSameCommand(RobotCommand* a, RobotCommand* b)
{
	return memcmp(a, b, sizeof(RobotCommand)) == 0;
}

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

uint8_t* OpenBinFile(const char* fileName, uint32_t& readPos, size_t& fileSize)
{
	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, fileName, "rb");
	if (f == NULL)
	{
		return NULL;
	}
	getFileSize(f, &fileSize);
	printf("File %s has %d bytes\n", fileName, (int)fileSize);
	uint8_t* ret = (uint8_t*)malloc(fileSize);
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

void ReadBinHeader(uint8_t* bytes, uint32_t& readPos, RobotDrawSession* robotSession)
{
	for (size_t i = 0; i < BIN_HEADER_BYTE_COUNT; i++)
	{
		if (bytes[readPos] != BIN_HEADER_BYTE)
		{
			printf("Was expecting header byte %d at %d, got %02X\n", BIN_HEADER_BYTE, (uint32_t)readPos, (uint32_t)bytes[readPos]);
		}
		readPos += FileReadDirection;
	}
	RobotCommand_Constructor(&robotSession->prevCMD, BIN_HEADER_BYTE);
}

void ReadBinFooter(uint8_t* bytes, uint32_t& readPos, RobotDrawSession* robotSession)
{
	for (size_t i = 0; i < BIN_FOOTER_BYTE_COUNT1; i++)
	{
		if (bytes[readPos] != BIN_FOOTER_BYTE1)
		{
			printf("Was expecting footer byte %d at %d, got %02X\n", BIN_FOOTER_BYTE1, (uint32_t)readPos, (uint32_t)bytes[readPos]);
		}
		readPos += FileReadDirection;
	}
	for (size_t i = 0; i < BIN_FOOTER_BYTE_COUNT2; i++)
	{
		if (bytes[readPos] != BIN_FOOTER_BYTE2)
		{
			printf("Was expecting footer byte %d at %d, got %02X\n", BIN_FOOTER_BYTE2, (uint32_t)readPos, (uint32_t)bytes[readPos]);
		}
		readPos += FileReadDirection;
	}
	RobotCommand_Constructor(&robotSession->prevCMD, BIN_FOOTER_BYTE2);
}

//#define TEST_BACKWARD_MOVE_REVERSES_MAIN_DIRECTION
//#define TEST_SAME_MOVEMENT_ADDS_INERTIA
//#define TEST_PRIMARY_DIRECTION_RELATIVE
//#define TEST_SAME_COMMAND_HALF_SPEED //looks like some sort of 'arm' movement compensation
//#define TEST_SAME_COMMAND_HALF_SPEED_EXCEPT_FORWARD
//#define TEST_TRIPPLE_COMMAND_FORWARD_MOVE //looks like some sort of 'arm' movement compensation
//#define		TEST_TRIPPLE_COMMAND_REVERTS_LAST_MOVE
//#define		TEST_DOUBLE_ADDS_FORWARD_HALF

static int LinesParsedCounter = 0;
int ReadBinLine(uint8_t* bytes, uint32_t& readPos, size_t fileSize, RelativePointsLine** line, RobotDrawSession* robotSession)
{
	if ((FileReadDirection < 0 && readPos <= BIN_HEADER_BYTE_COUNT)
		|| (FileReadDirection > 0 && readPos >= fileSize - (BIN_FOOTER_BYTE_COUNT1 + BIN_FOOTER_BYTE_COUNT2)))
	{
		return 1;
	}

	//read the line
	uint8_t byte = 0, prevByte = 1, prevPrevByte = 2;
	RobotCommand comm;
	uint32_t moveByteCount = 0;
	uint8_t primaryRelativeDirection = Move1_RelativeNoChange;
	uint8_t followPrimaryDirection = UNINITIALIZED_VALUE_8;
	uint8_t followPenPosition = 0;

	float motor_12_Move_total = 0;
	float motor_1_Move_total = 0;
	float motor_2_Move_total = 0;
	float motor_12_NoMove_total = 0;
	float motor_12_NoMove2_total = 0;
	size_t secondaryDirection = UNINITIALIZED_VALUE_8;
	size_t prevSecondaryDirection = UNINITIALIZED_VALUE_8;
	size_t directionChangeSignal = 0;

	while (1)
	{
		if ((FileReadDirection < 0 && readPos <= BIN_HEADER_BYTE_COUNT)
			|| (FileReadDirection > 0 && readPos >= fileSize - (BIN_FOOTER_BYTE_COUNT1 + BIN_FOOTER_BYTE_COUNT2)))
		{
			break;
		}

		byte = bytes[readPos];
		readPos += FileReadDirection;
		moveByteCount++;
		*(uint8_t*)&comm = byte;
		primaryRelativeDirection = comm.primaryDirection ^ robotSession->prevCMD.primaryDirection;

		if (followPrimaryDirection == UNINITIALIZED_VALUE_8)
		{
			followPenPosition = comm.penPosition;
#ifndef TEST_PRIMARY_DIRECTION_RELATIVE
			followPrimaryDirection = comm.primaryDirection;
//			if (LinesParsedCounter == 3 - 1)followPrimaryDirection = Move1_Right;
//			if (LinesParsedCounter == 6 - 1)followPrimaryDirection = Move1_Right;
//			if (LinesParsedCounter == 7 - 1)followPrimaryDirection = Move1_Up;
//			if (LinesParsedCounter == 8 - 1)followPrimaryDirection = Move1_Down;
//			if (LinesParsedCounter == 11 - 1)followPrimaryDirection = Move1_Right;
//			if (LinesParsedCounter == 12 - 1)followPrimaryDirection = Move1_Left;
#else
			if (primaryRelativeDirection != Move1_RelativeNoChange)
			{
				if (*prevDirection == Move1_Down)
				{
					if (primaryRelativeDirection == Move1_RelativeLeft)
					{
						followPrimaryDirection = Move1_Left;
					}
					if (primaryRelativeDirection == Move1_RelativeRight)
					{
						followPrimaryDirection = Move1_Right;
					}
					if (primaryRelativeDirection == Move1_RelativeReverse)
					{
						followPrimaryDirection = Move1_Up;
					}
				}
				if (*prevDirection == Move1_Up)
				{
					if (primaryRelativeDirection == Move1_RelativeLeft)
					{
						followPrimaryDirection = Move1_Right;
					}
					if (primaryRelativeDirection == Move1_RelativeRight)
					{
						followPrimaryDirection = Move1_Left;
					}
					if (primaryRelativeDirection == Move1_RelativeReverse)
					{
						followPrimaryDirection = Move1_Down;
					}
				}
				if (*prevDirection == Move1_Left)
				{
					if (primaryRelativeDirection == Move1_RelativeLeft)
					{
						followPrimaryDirection = Move1_Down;
					}
					if (primaryRelativeDirection == Move1_RelativeRight)
					{
						followPrimaryDirection = Move1_Up;
					}
					if (primaryRelativeDirection == Move1_RelativeReverse)
					{
						followPrimaryDirection = Move1_Right;
					}
				}
				if (*prevDirection == Move1_Right)
				{
					if (primaryRelativeDirection == Move1_RelativeLeft)
					{
						followPrimaryDirection = Move1_Up;
					}
					if (primaryRelativeDirection == Move1_RelativeRight)
					{
						followPrimaryDirection = Move1_Down;
					}
					if (primaryRelativeDirection == Move1_RelativeReverse)
					{
						followPrimaryDirection = Move1_Left;
					}
				}
				followPrimaryDirection = primaryRelativeDirection;
			}
			else
			{
				followPrimaryDirection = *prevDirection;
			}
#endif
			robotSession->prevMoveDir = (PenRobotMovementCodesPrimary)followPrimaryDirection;
			RelativePointsLine::setPenPosition(line, comm.penPosition);
			LinesParsedCounter++;
			printf("%d) Will follow move type %d-%s. Pen is %d. start at pos %d, Prev byte %02X, cur byte %02X\n",
				LinesParsedCounter, followPrimaryDirection,
				GetDirectionString((PenRobotMovementCodesPrimary)followPrimaryDirection), 
				followPenPosition != 0, readPos - 1, bytes[readPos - 2], bytes[readPos - 1]);
		}
		else if (primaryRelativeDirection != Move1_RelativeNoChange || comm.penPosition != followPenPosition)
		{
			readPos -= FileReadDirection;
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

		// interpret it as extra move for main direction
		int isNext1Prev1CommandsSame = (bytes[readPos - 2] == bytes[readPos - 1]) && (bytes[readPos - 1] == bytes[readPos + 0]);
		int isNext2CommandsSame = (bytes[readPos - 1] == bytes[readPos - 0]) && (bytes[readPos - 0] == bytes[readPos + 1]);
		// interpret it as only half movement
		int isNext1CommandSame = bytes[readPos - 1] == bytes[readPos];
		if (
			// need to confirm theory : robot is unable to sense commands that are exactly the same
			// but multiple of these "useless" commands will trigger a robot response. Ex : paper swap, reposition pen to origin
			IsSameCommand(&comm, &robotSession->prevCMD) == 1
			)
		{
			if (prevPrevByte == prevByte)
			{
				motor_12_NoMove2_total++;
//				if (LinesParsedCounter == 3)printf("6   \n");

#if 0
				if (LinesParsedCounter == 3 
					|| LinesParsedCounter == 5
					)
				{
					//reverse one right movement
					(*line)[writePosition + 0] += 1;
					(*line)[writePosition + 1] += 0;
					motor_1_Move_total -= 1;
					//and convert it to a down movement
//					(*line)[writePosition + 0] += 0;
//					(*line)[writePosition + 1] -= 1.0f;
//					motor_12_Move_total += 1;
					writePosition += 2;
				}
#endif
			}
			else
			{
//				if (LinesParsedCounter == 3)printf("5   ");
				motor_12_NoMove_total++;
			}
		}
//		else
		{
			int secondaryDirection = comm.secondaryDirection ^ robotSession->prevCMD.secondaryDirection;
//			printf("%d%d ", secondaryDirection, comm.secondaryDirection);
			float penSpeedInertiaAdjustedPrimary = 1.0f;
			float penSpeedInertiaAdjustedSecondary = 1.0f;

			if (comm.penPosition == Pen_Up)
			{
#ifdef TEST_TRIPPLE_COMMAND_FORWARD_MOVE
				if (isNext1Prev1CommandsSame)
				{
					secondaryDirection = Move2_RelativeForward;
					// or maybe it's adjust position of shorter / longer arm ?
				}
#endif
#ifdef TEST_SAME_COMMAND_HALF_SPEED
				else if (isNext1CommandSame)
				{
					penSpeedInertiaAdjustedPrimary = penSpeedInertiaAdjustedPrimary * 0.5;
					penSpeedInertiaAdjustedSecondary = penSpeedInertiaAdjustedSecondary * 0.5;
				}
#elif defined(TEST_SAME_COMMAND_HALF_SPEED_EXCEPT_FORWARD)
				else if (isNext1CommandSame && isNext2CommandsSame == 0)
				{
					penSpeedInertiaAdjustedSecondary = penSpeedInertiaAdjustedSecondary / 2;
				}
#endif
			}

#ifdef TEST_SAME_MOVEMENT_ADDS_INERTIA
			if (prevSecondaryDirection == secondaryDirection)
			{
				penSpeedInertiaAdjustedPrimary = 1.3f;
				penSpeedInertiaAdjustedSecondary = 1.0f;
			}
#endif
/*			if (secondaryDirection == Move_RelativeNoChange && prevSecondaryDirection != 0xFF)
			{
				if (prevSecondaryDirection == Move_RelativeLeft)
				{
					secondaryDirection = Move_RelativeRight;
				}
				if (prevSecondaryDirection == Move_RelativeRight)
				{
					secondaryDirection = Move_RelativeLeft;
				}
			}*/

			prevSecondaryDirection = secondaryDirection;

			if (followPrimaryDirection == Move1_Down)
			{
				if (secondaryDirection == Move2_RelativeForward)
				{
//					if (LinesParsedCounter == 3)printf("3+3 ");
					RelativePointsLine::storeNextPoint(line, 0, -penSpeedInertiaAdjustedPrimary);
					motor_12_Move_total += penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move2_RelativeLeft)
				{
//					if (LinesParsedCounter == 3)printf("3+1 ");
					RelativePointsLine::storeNextPoint(line, penSpeedInertiaAdjustedSecondary, 0);
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move2_RelativeRight)
				{
//					if (LinesParsedCounter == 3)printf("3+2 ");
					RelativePointsLine::storeNextPoint(line, -penSpeedInertiaAdjustedSecondary, 0);
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
/*				if (secondaryDirection == Move2_RelativeNoChange)
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
#ifdef TEST_BACKWARD_MOVE_REVERSES_MAIN_DIRECTION
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] += penSpeedInertiaAdjustedPrimary;
					motor_12_Move_total -= penSpeedInertiaAdjustedPrimary;
#endif
				}*/
			}
			if (followPrimaryDirection == Move1_Up)
			{
				if (secondaryDirection == Move2_RelativeForward)
				{
//					if (LinesParsedCounter == 2)printf("0"); else printf("1\n");
					RelativePointsLine::storeNextPoint(line, 0, penSpeedInertiaAdjustedSecondary);
					motor_12_Move_total += penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move2_RelativeLeft)
				{
					RelativePointsLine::storeNextPoint(line, -penSpeedInertiaAdjustedSecondary, 0);
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move2_RelativeRight)
				{
//					if (LinesParsedCounter == 2)printf("1\n"); else printf("0");
					RelativePointsLine::storeNextPoint(line, penSpeedInertiaAdjustedSecondary, 0);
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
/*				if (secondaryDirection == Move2_RelativeNoChange)
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
#ifdef TEST_BACKWARD_MOVE_REVERSES_MAIN_DIRECTION
					(*line)[writePosition + 0] += 0;
					(*line)[writePosition + 1] -= penSpeedInertiaAdjustedPrimary;
					motor_12_Move_total -= penSpeedInertiaAdjustedPrimary;
#endif
				}*/
			}
			if (followPrimaryDirection == Move1_Left)
			{
				if (secondaryDirection == Move2_RelativeForward)
				{
					RelativePointsLine::storeNextPoint(line, -penSpeedInertiaAdjustedSecondary, 0);
					motor_12_Move_total+= penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move2_RelativeLeft)
				{
					RelativePointsLine::storeNextPoint(line, 0, -penSpeedInertiaAdjustedSecondary);
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move2_RelativeRight)
				{
					RelativePointsLine::storeNextPoint(line, 0, penSpeedInertiaAdjustedSecondary);
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
/*				if (secondaryDirection == Move2_RelativeNoChange)
				{
					if (prevPrevByte == prevByte)
					{
						motor_12_NoMove2_total++;
					}
					else
					{
						motor_12_NoMove_total++;
					}
#ifdef TEST_BACKWARD_MOVE_REVERSES_MAIN_DIRECTION
					(*line)[writePosition + 0] -= ((-1) * penSpeedInertiaAdjustedPrimary);
					(*line)[writePosition + 1] -= 0;
					motor_12_Move_total -= penSpeedInertiaAdjustedPrimary;
#endif
				}*/
			}
			if (followPrimaryDirection == Move1_Right)
			{
				if (secondaryDirection == Move2_RelativeForward)
				{
					RelativePointsLine::storeNextPoint(line, penSpeedInertiaAdjustedSecondary, 0);
					motor_12_Move_total+= penSpeedInertiaAdjustedPrimary;
				}
				if (secondaryDirection == Move2_RelativeLeft) // move up
				{
					RelativePointsLine::storeNextPoint(line, 0, penSpeedInertiaAdjustedSecondary);
					motor_1_Move_total += penSpeedInertiaAdjustedSecondary;
				}
				if (secondaryDirection == Move2_RelativeRight) // move down
				{
					RelativePointsLine::storeNextPoint(line, 0, -penSpeedInertiaAdjustedSecondary);
					motor_2_Move_total += penSpeedInertiaAdjustedSecondary;
				}
/*				if (secondaryDirection == Move2_RelativeNoChange)
				{
					if (prevPrevByte == prevByte)
					{
						motor_12_NoMove2_total++;
					}
					else
					{
						motor_12_NoMove_total++;
					}
#ifdef TEST_BACKWARD_MOVE_REVERSES_MAIN_DIRECTION
				(*line)[writePosition + 0] -= penSpeedInertiaAdjustedPrimary;
				(*line)[writePosition + 1] += 0;
				motor_12_Move_total -= penSpeedInertiaAdjustedPrimary;
#endif
				}*/
			}
		}

		prevPrevByte = prevByte;
		prevByte = byte;
		robotSession->prevCMD = comm;
	}

	double lineLen = sqrt(motor_1_Move_total * motor_1_Move_total + motor_2_Move_total * motor_2_Move_total + motor_12_Move_total * motor_12_Move_total);
	printf("\tDone reading line. Has %d commands. %s %d, %s %d, %s %d. Dot len %.02f\n",
		moveByteCount, 
		GetDirectionStringRelative((PenRobotMovementCodesPrimary)followPrimaryDirection, (PenRobotMovementCodesRelative)Move2_RelativeLeft), (int)motor_1_Move_total,
		GetDirectionStringRelative((PenRobotMovementCodesPrimary)followPrimaryDirection, (PenRobotMovementCodesRelative)Move2_RelativeRight), (int)motor_2_Move_total,
		GetDirectionString((PenRobotMovementCodesPrimary)followPrimaryDirection), (int)motor_12_Move_total, lineLen);
	printf("\tNo move %d, double no move commands %d\n", (int)motor_12_NoMove_total, (int)motor_12_NoMove2_total);

	return 0;
}

const char* GetDirectionString(PenRobotMovementCodesPrimary movementCode)
{
	if (movementCode == Move1_Up)
	{
		return "up";
	}
	if (movementCode == Move1_Left)
	{
		return "left";
	}
	if (movementCode == Move1_Right)
	{
		return "right";
	}
	if (movementCode == Move1_Down)
	{
		return "down";
	}
	return "unknown";
}

const char* GetDirectionStringRelative(PenRobotMovementCodesPrimary movementCode, PenRobotMovementCodesRelative relative)
{
	if (movementCode == Move1_Up)
	{
		if (relative == Move2_RelativeLeft)
		{
			return "left";
		}
		if (relative == Move2_RelativeRight)
		{
			return "right";
		}
	}
	if (movementCode == Move1_Left)
	{
		if (relative == Move2_RelativeLeft)
		{
			return "up";
		}
		if (relative == Move2_RelativeRight)
		{
			return "down";
		}
	}
	if (movementCode == Move1_Right)
	{
		if (relative == Move2_RelativeLeft)
		{
			return "down";
		}
		if (relative == Move2_RelativeRight)
		{
			return "up";
		}
	}
	if (movementCode == Move1_Down)
	{
		if (relative == Move2_RelativeLeft)
		{
			return "right";
		}
		if (relative == Move2_RelativeRight)
		{
			return "left";
		}
	}
	return "unknown";
}