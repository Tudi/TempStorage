#include "StdAfx.h"

void WriteBinHeader(FILE* f, RobotDrawSession* robotSession)
{
	for (size_t i = 0; i < BIN_HEADER_BYTE_COUNT; i++)
	{
		uint8_t byte = BIN_HEADER_BYTE;
		fwrite(&byte, 1, 1, f);
	}
	// temp test to see if this caused the head to not move at all
	for (size_t i = 0; i < 4; i++)
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

BinFileWriter::BinFileWriter(const char* fileName)
{
	bHeaderWritten = false;
	bFooterWritten = false;
	RobotSession_Constructor(&robotSession);
	sFileName = fileName;
	fBinFile = NULL;
}

void BinFileWriter::OpenBinFile()
{
	if (fBinFile != NULL)
	{
		SOFT_ASSERT(false, "Bin file already open");
		return;
	}
	errno_t openerr = fopen_s(&fBinFile, sFileName.c_str(), "wb");
	if (fBinFile == NULL)
	{
		SOFT_ASSERT(false, "Failed to open bin file");
		return;
	}
}

void BinFileWriter::AddLine(float sx, float sy, float ex, float ey)
{
	if (fBinFile == NULL)
	{
		OpenBinFile();
	}
	if (bHeaderWritten == false)
	{
		WriteBinHeader(fBinFile, &robotSession);
		bHeaderWritten = true;
	}

	if ((int)robotSession.curx != (int)sx || (int)robotSession.cury != (int)sy)
	{
		RelativePointsLine line;
		printf("Moving NODRAW pen from %.02f,%.02f to %.02f,%.02f\n", robotSession.curx, robotSession.cury, sx, sy);
		DrawLineRelativeInMem(robotSession.curx, robotSession.cury, sx, sy, &line);
		if (line.GetPointsCount() > 0)
		{
			line.setPenPosition( Pen_Up);
			WriteBinLine(fBinFile, &line, &robotSession);
		}
	}

	RelativePointsLine line;
	printf("Draw line from %.02f,%.02f to %.02f,%.02f\n", sx, sy, ex, ey);
	line.setStartingPosition(robotSession.curx, robotSession.cury);
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, ex, ey, &line);
	if (line.GetPointsCount() > 0)
	{
		line.setPenPosition( Pen_Down );
		WriteBinLine(fBinFile, &line, &robotSession);
	}
}

void BinFileWriter::AddLineAntiDistorted(float sx, float sy, float ex, float ey)
{
	if (fBinFile == NULL)
	{
		OpenBinFile();
	}
	if (bHeaderWritten == false)
	{
		WriteBinHeader(fBinFile, &robotSession);
		bHeaderWritten = true;
	}

	if ((int)robotSession.curx != (int)sx || (int)robotSession.cury != (int)sy)
	{
		RelativePointsLine line;
		printf("Moving NODRAW pen from %.02f,%.02f to %.02f,%.02f\n", robotSession.curx, robotSession.cury, sx, sy);
		sLineAdjuster.DrawLine(robotSession.curx, robotSession.cury, sx, sy, &line);
		if (line.GetPointsCount() > 0)
		{
			line.setPenPosition(Pen_Up);
			WriteBinLine(fBinFile, &line, &robotSession);
		}
	}

	RelativePointsLine line;
	printf("Draw line from %.02f,%.02f to %.02f,%.02f\n", sx, sy, ex, ey);
	line.setStartingPosition(robotSession.curx, robotSession.cury);
	sLineAdjuster.DrawLine(robotSession.curx, robotSession.cury, ex, ey, &line);
	if (line.GetPointsCount() > 0)
	{
		line.setPenPosition(Pen_Down);
		WriteBinLine(fBinFile, &line, &robotSession);
	}
}

void BinFileWriter::CloseFile()
{
	if (fBinFile == NULL)
	{
		return;
	}
	if (bHeaderWritten == false)
	{
		return;
	}
	if (bFooterWritten == true)
	{
		return;
	}
	WriteBinFooter(fBinFile, &robotSession);
	bFooterWritten = true;
	fclose(fBinFile);
	fBinFile = NULL;
}

int WriteBinLine(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession)
{
	RobotCommand CMD = robotSession->prevCMD;
	uint8_t prevPrimaryDirection = Move1_Uninitialized;

	CMD.alwaysZero = 0;
	CMD.penIsMoving = 1;
	CMD.penPosition = line->getPenPosition();
	CMD.Transition = 0;

	for (int i = 0; i < line->GetPointsCount(); i++)
	{
		int shouldMoveAgain = 1;
		int xConsumed = 0;
		int yConsumed = 0;
		while (shouldMoveAgain)
		{
			shouldMoveAgain = 0;
			PenRobotMovementCodesPrimary primaryDirection = Move1_Uninitialized;

			if (line->GetDX(i) < 0)
			{
				if (line->GetDX(i) < xConsumed)
				{
					primaryDirection = Move1_Left;
					xConsumed += -1;
					shouldMoveAgain += (line->GetDX(i) < xConsumed);
				}
			}
			else if (line->GetDX(i) > 0)
			{
				if (line->GetDX(i) > xConsumed)
				{
					primaryDirection = Move1_Right;
					xConsumed += 1;
					shouldMoveAgain += (line->GetDX(i) > xConsumed);
				}
			}

			if (primaryDirection != Move1_Uninitialized)
			{
				// write the same command as a delay. Allow the robot to stop the pen movement and head into a new direction without shaking
				if (prevPrimaryDirection != Move1_Uninitialized && prevPrimaryDirection != primaryDirection)
				{
					fwrite(&CMD, 1, 1, f);
				}
				prevPrimaryDirection = primaryDirection;

				CMD.primaryDirection = primaryDirection;
				CMD.secondaryDirection = ~CMD.secondaryDirection;

				fwrite(&CMD, 1, 1, f);
				primaryDirection = Move1_Uninitialized;
			}

			if (line->GetDY(i) < 0)
			{
				if (line->GetDY(i) < yConsumed)
				{
					primaryDirection = Move1_Down;
					yConsumed += -1;
					shouldMoveAgain += (line->GetDY(i) < yConsumed);
				}
			}
			else if (line->GetDY(i) > 0)
			{
				if (line->GetDY(i) > yConsumed)
				{
					primaryDirection = Move1_Up;
					yConsumed += 1;
					shouldMoveAgain += (line->GetDY(i) > yConsumed);
				}
			}

			// this can happen after adjusting the line to be drawn correctly
//			SOFT_ASSERT((primaryDirection != Move1_Uninitialized) || (abs(xConsumed) + abs(yConsumed) > 0), "Empty line move segment");

			if (primaryDirection != Move1_Uninitialized)
			{
				// write the same command as a delay. Allow the robot to stop the pen movement and head into a new direction without shaking
				if (prevPrimaryDirection != Move1_Uninitialized && prevPrimaryDirection != primaryDirection)
				{
					fwrite(&CMD, 1, 1, f);
				}
				prevPrimaryDirection = primaryDirection;

				CMD.primaryDirection = primaryDirection;
				CMD.secondaryDirection = ~CMD.secondaryDirection;

				fwrite(&CMD, 1, 1, f);
			}
		}
	}

	// update robot session as it arrived to the destination
	robotSession->curx = line->GetEndX();
	robotSession->cury = line->GetEndY();
	robotSession->prevCMD = CMD;

	return 0;
}

void MovePenToLineStart_DrawLineInFile(FILE* f, RobotDrawSession* robotSession, float sx, float sy, float ex, float ey)
{

	if (robotSession->curx != sx || robotSession->cury != sy)
	{
		RelativePointsLine line;
		DrawLineRelativeInMem(robotSession->curx, robotSession->cury, sx, sy, &line);
		line.setPenPosition( Pen_Up );
		WriteBinLine(f, &line, robotSession);
	}

	RelativePointsLine line;
	line.setStartingPosition(robotSession->curx, robotSession->cury);
	DrawLineRelativeInMem(sx, sy, ex, ey, &line);
	line.setPenPosition( Pen_Down );
	WriteBinLine(f, &line, robotSession);
}