#include "StdAfx.h"

void WriteBinHeader(FILE *f, RobotDrawSession* robotSession)
{
	for (size_t i = 0; i < BIN_HEADER_BYTE_COUNT; i++)
	{
		uint8_t byte = BIN_HEADER_BYTE;
		fwrite(&byte, 1, 1, f);
	}
}

void WriteBinFooter(FILE* f, RobotDrawSession* robotSession)
{
	for (size_t i = 0; i < BIN_FOOTER_BYTE_COUNT1; i++)
	{
		uint8_t byte = BIN_FOOTER_BYTE1;
		fwrite(&byte, 1, 1, f);
	}
	for (size_t i = 0; i < BIN_FOOTER_BYTE_COUNT2; i++)
	{
		uint8_t byte = BIN_FOOTER_BYTE2;
		fwrite(&byte, 1, 1, f);
	}
}

// Need to work on this. I sense some parts of this block is not required
void WriteBinTransition(FILE* f, RobotDrawSession* robotSession, int writePaperSwap)
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
	if (writePaperSwap)
	{
		for (size_t i = 0; i < 4; i++)
		{
			uint8_t byte = 0x08;
			fwrite(&byte, 1, 1, f);
		}
	}
}

// I know there is a function for this, but for the sake of readability I added it
static int NumSign(int x)
{
	if (x > 0)
	{
		return 1;
	}
	if (x < 0)
	{
		return -1;
	}
	return 0;
}

int WriteBinLine(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
	RobotCommand CMD;

	// get the direction of the line
	line->endx = line->startx = robotSession->curx;
	line->endy = line->starty = robotSession->cury;
	// "play" the line to get to the end of it
	for (size_t i = 0; i < line->numberOfPoints; i++)
	{
		line->endx += line->moves[i].dx;
		line->endy += line->moves[i].dy;
	}

	// looks like an empty line
	if (line->startx == line->endx && line->starty == line->endy)
	{
		SOFT_ASSERT(0, "Unexpected 0 length line");
		return 1;
	}

	// select the bigger amount of steps as dominant
	float dx = line->endx - line->startx;
	float dy = line->endy - line->starty;

	// is it a dominant horizontal movement ?
	int expectedXSign = 0;
	int expectedYSign = 0;
	if (abs(dx) >= abs(dy))
	{
		if (line->startx >= line->endx)
		{
			CMD.primaryDirection = Move1_Left;
			expectedXSign = -1;
		}
		else
		{
			CMD.primaryDirection = Move1_Right;
			expectedXSign = 1;
		}
	}
	else
	{
		if (line->starty >= line->endy)
		{
			CMD.primaryDirection = Move1_Up;
			expectedYSign = -1;
		}
		else
		{
			CMD.primaryDirection = Move1_Down;
			expectedYSign = 1;
		}
	}

	CMD.alwaysZero = 0;
	CMD.penIsMoving = 1;
	CMD.penPosition = line->penPosition;
	CMD.Transition = 0;

	for (int i = 0; i < line->numberOfPoints; i++)
	{
		if (CMD.primaryDirection == Move1_Down || CMD.primaryDirection == Move1_Up)
		{
			SOFT_ASSERT(NumSign(line->moves[i].dy) != expectedYSign, "Unexpected backward moving line");
		}
		if (CMD.primaryDirection == Move1_Left || CMD.primaryDirection == Move1_Right)
		{
			SOFT_ASSERT(NumSign(line->moves[i].dx) != expectedXSign, "Unexpected backward moving line");
		}

		fwrite(&CMD, 1, 1, f);
	}

	return 0;
}
