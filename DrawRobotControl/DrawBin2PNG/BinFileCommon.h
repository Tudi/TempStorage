#pragma once

#define BIN_HEADER_BYTE_COUNT 10
#define BIN_HEADER_BYTE 0x08

#define BIN_FOOTER_BYTE_COUNT1 10
#define BIN_FOOTER_BYTE1 0x08
#define BIN_FOOTER_BYTE_COUNT2 10
#define BIN_FOOTER_BYTE2 0x00

#define UNINITIALIZED_VALUE_32		0xBADBEEF
#define UNINITIALIZED_VALUE_8		0x7F

typedef enum PenRobotMovementCodesPrimary
{
	Move1_Down = 0x03, // negative value in SIG file. Pen moves further from head						
	Move1_Left = 0x01, // negative value in SIG file
	Move1_Right = 0x02, // positive value in SIG file
	Move1_Up = 0x00, // positive value in a SIG file. Pen moves closer to head
	Move1_Uninitialized = 4,
	Move1_Values_Count = 4,
}PenRobotMovementCodesPrimary;

typedef enum PenRobotMovementCodesRelative
{
	Move2_RelativeForward = 0x03,
	Move2_RelativeLeft = 0x01,
	Move2_RelativeRight = 0x02,
	Move2_RelativeNoChange = 0x00,
	Move2_Max_Value = 3,
	Move2_AssertError = 4,
}PenRobotMovementCodesRelative;

typedef enum PenRobotPenPosition
{
	Pen_Down = 1,
	Pen_Up = 0,
}PenRobotPenPosition;

#pragma pack(push, 1)
typedef struct RobotCommand
{
	uint8_t primaryDirection : 2; // up,down,left,right 
	uint8_t Transition : 1; // raise pen, swap paper or reposition head. Seems to depend on the sequence
	uint8_t penIsMoving : 1; // only zero when writing pauses ( transition )
	// higher bits
	uint8_t alwaysZero : 1; // have not seen anything else than 0
	uint8_t penPosition : 1; // 0 is in the air. 1 is lowered on paper
	uint8_t secondaryDirection : 2; // up,down,left,right relative to main direction
}RobotCommand;

// lines move relatively, need to track pen position, reset pen position on specific commands
typedef struct RobotDrawSession
{
	float startx, starty; // should be the same as origo ? 0,0 ?
	float curx, cury;
	double roundingX, roundingY; // less then a full movement command at the end of the line
	int linesDrawn;
	int linesNotDrawn; // line while the pen is in the air
	RobotCommand prevCMD;
	PenRobotMovementCodesPrimary prevMoveDir;
}RobotDrawSession;
#pragma pack(pop)