#include "StdAfx.h"

void drawClockPNG(FIBITMAP* dib, RobotDrawSession& robotSession)
{
	RelativePointsLine* line = NULL;
	RelativePointsLine::setStartingPosition(&line, robotSession.curx, robotSession.cury);
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.5) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.433) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.25) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.5) * PIXELS_IN_INCH), robotSession.cury + (float)((0) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((0.433) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((0.25) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0) * PIXELS_IN_INCH), robotSession.cury + (float)((0.5) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((0.433) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((0.25) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.5) * PIXELS_IN_INCH), robotSession.cury + (float)((0) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.433) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;

	robotSession.curx = robotSession.startx;
	robotSession.cury = robotSession.starty;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.25) * PIXELS_IN_INCH), &line);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);
	free(line); line = NULL;
}

void MovePenToLineStart_DrawLineInFile(FILE* f, RobotDrawSession *robotSession, float sx, float sy, float ex, float ey)
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

void drawClockBin()
{
	RobotDrawSession robotSession;
	RobotSession_Constructor(&robotSession);

	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, "clock.bin", "wb");
	if (f == NULL)
	{
		return;
	}

	WriteBinHeader(f, &robotSession);

	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0) * PIXELS_IN_INCH), (float)((-0.5) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.25) * PIXELS_IN_INCH), (float)((-0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.433) * PIXELS_IN_INCH), (float)((-0.25) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.5) * PIXELS_IN_INCH), (float)((0) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.25) * PIXELS_IN_INCH), (float)((0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.433) * PIXELS_IN_INCH), (float)((0.25) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0) * PIXELS_IN_INCH), (float)((0.5) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.25) * PIXELS_IN_INCH), (float)((0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.433) * PIXELS_IN_INCH), (float)((0.25) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.5) * PIXELS_IN_INCH), (float)((0) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.25) * PIXELS_IN_INCH), (float)((-0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.433) * PIXELS_IN_INCH), (float)((-0.25) * PIXELS_IN_INCH));

	WriteBinFooter(f, &robotSession);
	fclose(f);
}

void Test_DrawClock()
{
	drawClockBin();
}