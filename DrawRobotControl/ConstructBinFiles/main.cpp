#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


typedef enum PenRobotMovementCodes
{
	Move_Up = 0x00,
	Move_Left = 0x01,
	Move_Right = 0x02,
	Move_Down = 0x03,
}PenRobotMovementCodes;

typedef enum PenRobotPenPosition
{
	Pen_Down = 1,
	Pen_Up = 0,
}PenRobotPenPosition;

#pragma pack(push, 1)
typedef struct RobotCommand
{
	uint8_t motorDirection : 2; // up,down,left,right 
	uint8_t Transition : 1; // raise pen, swap paper or reposition head. Seems to depend on the sequence
	uint8_t penIsMoving : 1; // only zero when writing pauses ( transition )
	uint8_t alwaysZero_2 : 1;
	uint8_t penPosition : 1;
	uint8_t motor1TriggerMovement : 1; // affects vertical movement of the pen
	uint8_t motor2TriggerMovement : 1; // affects horizontal movement of the pen
}RobotCommand;
#pragma pack(pop)

#define VerticalStepsPerInch (213.0*2.0*2.0)
#define HorizontalStepsPerInch (213.0*2.0*2.0)

int WriteMovement(FILE* f, uint32_t& writePos, PenRobotPenPosition PenPosition, PenRobotMovementCodes movement, double inches, double xRatio = 1, double yRatio = 1)
{
	RobotCommand command;
	command.Transition = 0;
	command.penIsMoving = 1;
	command.alwaysZero_2 = 0;
	command.motorDirection = movement;
	command.penPosition = PenPosition;

	// this needs to be rewritten to compensate for a non liniar plane
	size_t stepsRequired;
	if (movement == Move_Left || movement == Move_Right)
	{
		stepsRequired = (size_t)(inches * VerticalStepsPerInch);
	}
	else
	{
		stepsRequired = (size_t)(inches * HorizontalStepsPerInch);
	}

	for (int i = 0; i < stepsRequired; i++)
	{
		double prevStep = (double)(i - 1);
		double curStep = (double)(i + 0);
		int xPositionPrev = (int)(xRatio * prevStep);
		int yPositionPrev = (int)(yRatio * prevStep);
		int xPositionCur = (int)(xRatio * curStep);
		int yPositionCur = (int)(yRatio * curStep);
		if (xPositionCur != xPositionPrev)
		{
			command.motor1TriggerMovement = 1 - command.motor1TriggerMovement;
		}
		if (yPositionCur != yPositionPrev)
		{
			command.motor2TriggerMovement = 1 - command.motor2TriggerMovement;
		}
		fwrite(&command, 1, 1, f);
	}

	return 0;
}

//start of a segment. Ex : page1, page2
void WriteHeader(FILE* f, uint32_t& writePos)
{
	for (size_t i = 0; i < 10; i++)
	{
		uint8_t byte = 0x08;
		fwrite(&byte, 1, 1, f);
	}
}

void WriteFooter(FILE* f, uint32_t& writePos)
{
	for (size_t i = 0; i < 10; i++)
	{
		uint8_t byte = 0x08;
		fwrite(&byte, 1, 1, f);
	}
	for (size_t i = 0; i < 10; i++)
	{
		uint8_t byte = 0x00;
		fwrite(&byte, 1, 1, f);
	}
}

// raise pen
// move back to origin
// swap paper
// auto resume drawing
void WritePaperSwap(FILE* f, uint32_t& writePos)
{
	uint8_t byte = 0x88; // does not seem to matter ?
	fwrite(&byte, 1, 1, f);
	for (size_t i = 0; i < 20; i++)
	{
		uint8_t byte = 0x08 | 0x04;
		fwrite(&byte, 1, 1, f);
	}
	// only part that does not have "always 1" bit 4 set
	// probably enough for a transition
	for (size_t i = 0; i < 20; i++)
	{
		uint8_t byte = 0x04;
		fwrite(&byte, 1, 1, f);
	}
	//seems like this part generates the paper swap
	for (size_t i = 0; i < 4; i++)
	{
		uint8_t byte = 0x08;
		fwrite(&byte, 1, 1, f);
	}
}


void WriteDrawPause(FILE* f, uint32_t& writePos)
{
	uint8_t byte = 0x88; // does not seem to matter ?
	fwrite(&byte, 1, 1, f);
	for (size_t i = 0; i < 20; i++)
	{
		uint8_t byte = 0x08 | 0x04;
		fwrite(&byte, 1, 1, f);
	}
	// only part that does not have "always 1" bit 4 set
	// probably enough for a transition
	for (size_t i = 0; i < 20; i++)
	{
		uint8_t byte = 0x04;
		fwrite(&byte, 1, 1, f);
	}
}


void GenClockFile(FILE* f)
{
	uint32_t writePos = 0;
	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 1, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Up, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Right, 0.1);

	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 0.8, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Up, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Right, 0.1);

	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 0.6, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Up, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Right, 0.1);

	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 0.4, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Up, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Right, 0.1);

	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 0.2, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Up, 1);
	WriteMovement(f, writePos, Pen_Up, Move_Right, 0.1);
}

int main()
{
//	const char* fileName = "./BinFiles/2VerticalLines_EqualHighBits.bin";
//	const char* fileName = "./BinFiles/2VerticalLines_EqualHighBitsSlow.bin";
//	const char* fileName = "./BinFiles/2VerticalLines_EqualHighBitsSlow2.bin";
//	const char* fileName = "./BinFiles/2VerticalLines_Curvature.bin";
//	const char* fileName = "./BinFiles/2VerticalLines_Orientation1.bin";
//	const char* fileName = "./BinFiles/2VerticalLines_Orientation2.bin";
//	const char* fileName = "./BinFiles/2HorizontalLines_EqualHighBits.bin";
//	const char* fileName = "./BinFiles/Really1Inch_2Lines_total5inch.bin";
//	const char* fileName = "./BinFiles/StraightLine5inch_00_11.bin";
//	const char* fileName = "./BinFiles/LineTransitionLine.bin";
//	const char* fileName = "./BinFiles/CheckMotorAngle_00.bin";
//	const char* fileName = "./BinFiles/CheckMotorAngle_01.bin";
//	const char* fileName = "./BinFiles/CheckMotorAngle_10.bin";
//	const char* fileName = "./BinFiles/CheckMotorAngle_11.bin";
	const char* fileName = "./BinFiles/CheckArmAngle_11_0_20_40_60_80.bin";
//	const char* fileName = "./BinFiles/TransitionTry2.bin";
	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, fileName, "wb");
	if (f == NULL)
	{
		return 1;
	}
	uint32_t writePos = 0;
	WriteHeader(f, writePos);

//	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 1, 0.2);
//	WriteDrawPause(f, writePos);
//	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 1, 0.2);

	GenClockFile(f);

//	WriteMovement(f, writePos, Pen_Down, Move_Down, 1, 1, 1);
	//	WritePaperSwap(f, writePos);
//	WriteMovement(f, writePos, Pen_Down, Move_Right, 0.5);
	//	WriteMovement(f, writePos, Pen_Down, Move_Right, 0.5);
//	WriteMovement(f, writePos, Pen_Down, Move_Down, 1);
	WriteFooter(f, writePos);

	fclose(f);

	return 0;
}