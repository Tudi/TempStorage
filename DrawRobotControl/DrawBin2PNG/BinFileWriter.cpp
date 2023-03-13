#include "StdAfx.h"

void WriteBinHeader(FILE* f, RobotDrawSession* robotSession)
{
	for (size_t i = 0; i < BIN_HEADER_BYTE_COUNT; i++)
	{
		uint8_t byte = BIN_HEADER_BYTE;
		fwrite(&byte, 1, 1, f);
	}
	RobotCommand_Constructor(&robotSession->prevCMD, BIN_HEADER_BYTE);
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
	RobotCommand_Constructor(&robotSession->prevCMD, BIN_FOOTER_BYTE2);
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
	RobotCommand_Constructor(&robotSession->prevCMD, 0x04);
	//seems like this part generates the paper swap
	if (writePaperSwap)
	{
		for (size_t i = 0; i < 4; i++)
		{
			uint8_t byte = 0x08;
			fwrite(&byte, 1, 1, f);
		}
		RobotCommand_Constructor(&robotSession->prevCMD, 0x08);

	}
}

// I know there is a function for this, but for the sake of readability I added it
static int NumSign(float x)
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

#define CoordMoveSignCount 3
static PenRobotMovementCodesRelative Coord_X_SignToMovementTransalationTable[Move1_Values_Count][CoordMoveSignCount];
static PenRobotMovementCodesRelative Coord_Y_SignToMovementTransalationTable[Move1_Values_Count][CoordMoveSignCount];

// !! up is ment as on a paper. When drawing in memory, up has -1 on row. Line(0,0,0,100)
void InitSignToMovementTranslationTable()
{
	Coord_X_SignToMovementTransalationTable[Move1_Up][1 - 1] = Move2_RelativeLeft;
	Coord_X_SignToMovementTransalationTable[Move1_Up][1 - 0] = Move2_RelativeNoChange;
	Coord_X_SignToMovementTransalationTable[Move1_Up][1 + 1] = Move2_RelativeRight;

	Coord_Y_SignToMovementTransalationTable[Move1_Up][1 - 1] = Move2_AssertError;
	Coord_Y_SignToMovementTransalationTable[Move1_Up][1 - 0] = Move2_RelativeNoChange;
	Coord_Y_SignToMovementTransalationTable[Move1_Up][1 + 1] = Move2_RelativeForward; // positive is up

	// ========================

	Coord_X_SignToMovementTransalationTable[Move1_Left][1 - 1] = Move2_RelativeForward;
	Coord_X_SignToMovementTransalationTable[Move1_Left][1 - 0] = Move2_RelativeNoChange;
	Coord_X_SignToMovementTransalationTable[Move1_Left][1 + 1] = Move2_AssertError;

	Coord_Y_SignToMovementTransalationTable[Move1_Left][1 - 1] = Move2_RelativeLeft;
	Coord_Y_SignToMovementTransalationTable[Move1_Left][1 - 0] = Move2_RelativeNoChange;
	Coord_Y_SignToMovementTransalationTable[Move1_Left][1 + 1] = Move2_RelativeRight;

	// ========================

	Coord_X_SignToMovementTransalationTable[Move1_Right][1 - 1] = Move2_AssertError;
	Coord_X_SignToMovementTransalationTable[Move1_Right][1 - 0] = Move2_RelativeNoChange;
	Coord_X_SignToMovementTransalationTable[Move1_Right][1 + 1] = Move2_RelativeForward;

	Coord_Y_SignToMovementTransalationTable[Move1_Right][1 - 1] = Move2_RelativeRight;
	Coord_Y_SignToMovementTransalationTable[Move1_Right][1 - 0] = Move2_RelativeNoChange;
	Coord_Y_SignToMovementTransalationTable[Move1_Right][1 + 1] = Move2_RelativeLeft;

	// ========================

	Coord_X_SignToMovementTransalationTable[Move1_Down][1 - 1] = Move2_RelativeRight;
	Coord_X_SignToMovementTransalationTable[Move1_Down][1 - 0] = Move2_RelativeNoChange;
	Coord_X_SignToMovementTransalationTable[Move1_Down][1 + 1] = Move2_RelativeLeft;

	Coord_Y_SignToMovementTransalationTable[Move1_Down][1 - 1] = Move2_RelativeForward; // negative is downwards
	Coord_Y_SignToMovementTransalationTable[Move1_Down][1 - 0] = Move2_RelativeNoChange;
	Coord_Y_SignToMovementTransalationTable[Move1_Down][1 + 1] = Move2_AssertError;

}

int GenRelativeMovementOpcode(PenRobotMovementCodesRelative PrevCode, PenRobotMovementCodesRelative move)
{
	int prevBitLeft = PrevCode & 1;
	int prevBitRight = (PrevCode >> 1) & 1;

	if (move == Move2_RelativeForward)
	{
		return (1 - prevBitLeft) | ((1 - prevBitRight) << 1);
	}
	else if (move == Move2_RelativeLeft)
	{
		return (1 - prevBitLeft) | (prevBitRight << 1);
	}
	else if (move == Move2_RelativeRight)
	{
		return (prevBitLeft) | ((1 - prevBitRight) << 1);
	}
	return Move2_AssertError;
}

// we always move to the left of the main direction
int WriteBinLineCloseToMainDirection(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
	// todo : convert this to static once ironed out the correct orientations
	InitSignToMovementTranslationTable();

	RobotCommand CMD = robotSession->prevCMD;

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

	int dx = (int)(line->endx - line->startx);
	int dy = (int)(line->endy - line->starty);

	if (abs(dx) > abs(dy))
	{
		if (line->startx <= line->endx)
		{
			CMD.primaryDirection = Move1_Right;
		}
		else
		{
			CMD.primaryDirection = Move1_Left;
		}
	}
	else
	{
		if (line->starty <= line->endy)
		{
			CMD.primaryDirection = Move1_Up;
		}
		else
		{
			CMD.primaryDirection = Move1_Down;
		}
	}

	CMD.alwaysZero = 0;
	CMD.penIsMoving = 1;
	CMD.penPosition = line->penPosition;
	CMD.Transition = 0;

	for (int i = 0; i < line->numberOfPoints; i++)
	{
		int xSign = NumSign(line->moves[i].dx);
		int ySign = NumSign(line->moves[i].dy);

		SOFT_ASSERT(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] != Move2_AssertError, "Unexpected x movement for main direction");
		SOFT_ASSERT(Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign] != Move2_AssertError, "Unexpected y movement for main direction");
		SOFT_ASSERT((Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] + Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign]) <= Move2_Max_Value, "Unexpected movement combination");
		SOFT_ASSERT((xSign == 0 && (ySign == 1 || ySign == -1)) || (ySign == 0 && (xSign == 1 || xSign == -1)), "Unexpected line movement");
		SOFT_ASSERT(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] == Move2_RelativeNoChange  || Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign] == Move2_RelativeNoChange, "Unexpected movement combination");

		PenRobotMovementCodesRelative relativeMovementType = (PenRobotMovementCodesRelative)(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] | Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign]);
		SOFT_ASSERT(relativeMovementType != Move2_RelativeNoChange, "Unexpected relative movement value : NoChange");

		CMD.secondaryDirection = GenRelativeMovementOpcode((PenRobotMovementCodesRelative)CMD.secondaryDirection, relativeMovementType);

		SOFT_ASSERT(CMD.secondaryDirection != Move2_AssertError, "Unexpected relative movement value : Invalid");

		fwrite(&CMD, 1, 1, f);
	}

	// update robot session as it arrived to the destination
	robotSession->curx = line->endx;
	robotSession->cury = line->endy;
	robotSession->prevCMD = CMD;

	return 0;
}

// we always move to the left of the main direction
int WriteBinLineRightOrLeft(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
	// todo : convert this to static once ironed out the correct orientations
	InitSignToMovementTranslationTable();

	RobotCommand CMD = robotSession->prevCMD;

#ifdef TEST_11
	CMD.secondaryDirection = 0;
#endif

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

#ifdef TEST_9
	for (size_t i = 0; i < 4; i++)
	{
		uint8_t byte = 0xE8;
		fwrite(&byte, 1, 1, f);
	}
#endif

	if (line->startx <= line->endx)
	{
		CMD.primaryDirection = Move1_Right;
	}
	else
	{
		CMD.primaryDirection = Move1_Left;
	}

	CMD.alwaysZero = 0;
	CMD.penIsMoving = 1;
	CMD.penPosition = line->penPosition;
	CMD.Transition = 0;

#ifdef TEST_1
	RobotCommand tCMD = CMD;
	if (robotSession->prevCMD.primaryDirection == Move1_Right)
	{
		tCMD.primaryDirection = Move1_Down;
		tCMD.secondaryDirection = 0;
	}
	else
	{
		tCMD.primaryDirection = Move1_Up;
		tCMD.secondaryDirection = 0;
	}
	fwrite(&tCMD, 1, 1, f);
#endif

	for (int i = 0; i < line->numberOfPoints; i++)
	{
		int xSign = NumSign(line->moves[i].dx);
		int ySign = NumSign(line->moves[i].dy);

		SOFT_ASSERT(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] != Move2_AssertError, "Unexpected x movement for main direction");
		SOFT_ASSERT(Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign] != Move2_AssertError, "Unexpected y movement for main direction");
		SOFT_ASSERT((Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] + Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign]) <= Move2_Max_Value, "Unexpected movement combination");
		SOFT_ASSERT((xSign == 0 && (ySign == 1 || ySign == -1)) || (ySign == 0 && (xSign == 1 || xSign == -1)), "Unexpected line movement");
		SOFT_ASSERT(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] == Move2_RelativeNoChange || Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign] == Move2_RelativeNoChange, "Unexpected movement combination");

		PenRobotMovementCodesRelative relativeMovementType = (PenRobotMovementCodesRelative)(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] | Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign]);
		SOFT_ASSERT(relativeMovementType != Move2_RelativeNoChange, "Unexpected relative movement value : NoChange");

		CMD.secondaryDirection = GenRelativeMovementOpcode((PenRobotMovementCodesRelative)CMD.secondaryDirection, relativeMovementType);

		SOFT_ASSERT(CMD.secondaryDirection != Move2_AssertError, "Unexpected relative movement value : Invalid");

		fwrite(&CMD, 1, 1, f);
	}
#ifdef TEST_7
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
#endif
#ifdef TEST_15
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
#endif
#ifdef TEST_16
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
#endif

	// update robot session as it arrived to the destination
	robotSession->curx = line->endx;
	robotSession->cury = line->endy;
	robotSession->prevCMD = CMD;

	return 0;
}

// we always move to the left of the main direction
int WriteBinLineUpOrDown(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
	// todo : convert this to static once ironed out the correct orientations
	InitSignToMovementTranslationTable();

	RobotCommand CMD = robotSession->prevCMD;

#ifdef TEST_11
	CMD.secondaryDirection = 0;
#endif

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

#ifdef TEST_9
	for (size_t i = 0; i < 4; i++)
	{
		uint8_t byte = 0xE8;
		fwrite(&byte, 1, 1, f);
	}
#endif

	if (line->starty <= line->endy)
	{
		CMD.primaryDirection = Move1_Up;
	}
	else
	{
		CMD.primaryDirection = Move1_Down;
	}

	CMD.alwaysZero = 0;
	CMD.penIsMoving = 1;
	CMD.penPosition = line->penPosition;
	CMD.Transition = 0;

	for (int i = 0; i < line->numberOfPoints; i++)
	{
		int xSign = NumSign(line->moves[i].dx);
		int ySign = NumSign(line->moves[i].dy);

		SOFT_ASSERT(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] != Move2_AssertError, "Unexpected x movement for main direction");
		SOFT_ASSERT(Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign] != Move2_AssertError, "Unexpected y movement for main direction");
		SOFT_ASSERT((Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] + Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign]) <= Move2_Max_Value, "Unexpected movement combination");
		SOFT_ASSERT((xSign == 0 && (ySign == 1 || ySign == -1)) || (ySign == 0 && (xSign == 1 || xSign == -1)), "Unexpected line movement");
		SOFT_ASSERT(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] == Move2_RelativeNoChange || Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign] == Move2_RelativeNoChange, "Unexpected movement combination");

		PenRobotMovementCodesRelative relativeMovementType = (PenRobotMovementCodesRelative)(Coord_X_SignToMovementTransalationTable[CMD.primaryDirection][1 + xSign] | Coord_Y_SignToMovementTransalationTable[CMD.primaryDirection][1 + ySign]);
		SOFT_ASSERT(relativeMovementType != Move2_RelativeNoChange, "Unexpected relative movement value : NoChange");

		CMD.secondaryDirection = GenRelativeMovementOpcode((PenRobotMovementCodesRelative)CMD.secondaryDirection, relativeMovementType);

		SOFT_ASSERT(CMD.secondaryDirection != Move2_AssertError, "Unexpected relative movement value : Invalid");

		fwrite(&CMD, 1, 1, f);
	}
#ifdef TEST_7
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
#endif
#ifdef TEST_15
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
#endif
#ifdef TEST_16
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
	fwrite(&CMD, 1, 1, f); // add useless command at the end of the line
#endif

	// update robot session as it arrived to the destination
	robotSession->curx = line->endx;
	robotSession->cury = line->endy;
	robotSession->prevCMD = CMD;

	return 0;
}

// we always move to the left of the main direction
int WriteBinLineMainDirections(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
	// todo : convert this to static once ironed out the correct orientations
	InitSignToMovementTranslationTable();

	RobotCommand CMD = robotSession->prevCMD;

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

	if (line->startx <= line->endx)
	{
		CMD.primaryDirection = Move1_Right;
	}
	else
	{
		CMD.primaryDirection = Move1_Left;
	}

	CMD.alwaysZero = 0;
	CMD.penIsMoving = 1;
	CMD.penPosition = line->penPosition;
	CMD.Transition = 0;

	for (int i = 0; i < line->numberOfPoints; i++)
	{
		if (line->moves[i].dx < 0)
		{
			CMD.primaryDirection = Move1_Left;
		}
		else if (line->moves[i].dx > 0)
		{
			CMD.primaryDirection = Move1_Right;
		}
		else if (line->moves[i].dy < 0)
		{
			CMD.primaryDirection = Move1_Down;
		}
		else if (line->moves[i].dy > 0)
		{
			CMD.primaryDirection = Move1_Up;
		}

		CMD.secondaryDirection = ~CMD.secondaryDirection;

		fwrite(&CMD, 1, 1, f);
	}

	// update robot session as it arrived to the destination
	robotSession->curx = line->endx;
	robotSession->cury = line->endy;
	robotSession->prevCMD = CMD;

	return 0;
}

int WriteBinLine(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
#ifdef TEST_13
	return WriteBinLineMainDirections(f, line, robotSession);
#elif defined(TEST_2)
	return WriteBinLineUpOrDown(f, line, robotSession);
#elif defined(TEST_4)
	return WriteBinLineCloseToMainDirection(f, line, robotSession);
#else
	return WriteBinLineRightOrLeft(f, line, robotSession);
#endif
}

void MovePenToLineStart_DrawLineInFile(FILE* f, RobotDrawSession* robotSession, float sx, float sy, float ex, float ey)
{
	RelativePointsLine* line = NULL;

	if (robotSession->curx != sx || robotSession->cury != sy)
	{
		DrawLineRelativeInMem(robotSession->curx, robotSession->cury, sx, sy, &line);
		line->penPosition = Pen_Up;
		WriteBinLine(f, line, robotSession);
		FreeAndNULL(line)
	}
	RelativePointsLine::setStartingPosition(&line, robotSession->curx, robotSession->cury);
	DrawLineRelativeInMem(sx, sy, ex, ey, &line);
	line->penPosition = Pen_Down;
	WriteBinLine(f, line, robotSession);
	FreeAndNULL(line)
}