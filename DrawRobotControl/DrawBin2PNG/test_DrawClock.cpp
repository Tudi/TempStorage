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

#if 0
void drawClockReverseBin()
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
#endif

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

#ifdef TEST_8
	for (size_t i = 0; i < 4; i++)
	{
		uint8_t byte = 0x08;
		fwrite(&byte, 1, 1, f);
	}
#endif

	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0) * PIXELS_IN_INCH), (float)((0.5) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.25) * PIXELS_IN_INCH), (float)((0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.433) * PIXELS_IN_INCH), (float)((0.25) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.5) * PIXELS_IN_INCH), (float)((0) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.25) * PIXELS_IN_INCH), (float)((-0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0.433) * PIXELS_IN_INCH), (float)((-0.25) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((0) * PIXELS_IN_INCH), (float)((-0.5) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.25) * PIXELS_IN_INCH), (float)((-0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.433) * PIXELS_IN_INCH), (float)((-0.25) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.5) * PIXELS_IN_INCH), (float)((0) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.25) * PIXELS_IN_INCH), (float)((0.433) * PIXELS_IN_INCH));
	MovePenToLineStart_DrawLineInFile(f, &robotSession, 0, 0, (float)((-0.433) * PIXELS_IN_INCH), (float)((0.25) * PIXELS_IN_INCH));

	WriteBinFooter(f, &robotSession);
	fclose(f);
}

void Test_DrawClock()
{
#ifdef TEST_12
	drawClockBin();
#endif
}