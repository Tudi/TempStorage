#pragma once

#define BIN_HEADER_BYTE_COUNT 10
#define BIN_FOOTER_BYTE_COUNT 20

typedef enum PenRobotMovementCodes
{
	Move_Up = 0x00,
	Move_Left = 0x01,
	Move_Right = 0x02,
	Move_Down = 0x03,
}PenRobotMovementCodes;

typedef enum PenRobotMovementCodesRelative
{
	Move_Forward = 0x03,
	Move_RelativeLeft = 0x01,
	Move_RelativeRight = 0x02,
	Move_RelativeUnknown = 0x00, // would not make sense to move backward
}PenRobotMovementCodesRelative;

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
	uint8_t alwaysZero : 1; // have not seen anything else than 0
	uint8_t penPosition : 1; // 0 is in the air. 1 is lowered on paper
	uint8_t motor1TriggerMovement : 1; // affects vertical movement of the pen
	uint8_t motor2TriggerMovement : 1; // affects horizontal movement of the pen
}RobotCommand;
#pragma pack(pop)

char* OpenBinFile(const char* name, uint32_t& readPos, size_t& fileSize);
void ReadBinHeader(char* f, uint32_t& readPos);
void ReadBinFooter(char* f, uint32_t& readPos);

#define MAX_LINE_NODES 65535 // todo : should make this dynamic ...
int ReadBinLine(char* f, uint32_t& readPos, size_t fileSize, float** line);
const char* GetDirectionString(PenRobotMovementCodes movementCode);
const char* GetDirectionStringRelative(PenRobotMovementCodes movementCode, PenRobotMovementCodesRelative relative);