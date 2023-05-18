#include "StdAfx.h"

void drawClockPNG(FIBITMAP* dib, RobotDrawSession& robotSession)
{
	RelativePointsLine line;
	line.setStartingPosition(robotSession.curx, robotSession.cury);
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.5) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.433) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.25) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.5) * PIXELS_IN_INCH), robotSession.cury + (float)((0) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((0.433) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((-0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((0.25) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0) * PIXELS_IN_INCH), robotSession.cury + (float)((0.5) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((0.433) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((0.25) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.5) * PIXELS_IN_INCH), robotSession.cury + (float)((0) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.25) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.433) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);

	robotSession.curx = 0;
	robotSession.cury = 0;
	DrawLineRelativeInMem(robotSession.curx, robotSession.cury, robotSession.curx + (float)((0.433) * PIXELS_IN_INCH), robotSession.cury + (float)((-0.25) * PIXELS_IN_INCH), &line, robotSession.roundingX, robotSession.roundingY);
	DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, &line);
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

void DrawClockAnyRadius(double R_inches, double lineCount)
{
	char fileName[500];
	sprintf_s(fileName, sizeof(fileName), "clock_%.02f_%d.bin", R_inches, (int)lineCount);

	BinFileWriter bfw(fileName);
//	bfw.SetDrawSpeedPCT(25);

	double angleIncrement = 360 / lineCount;
	double originX = 0;
	double originY = 0;
	// angle speed should depend on radius, but we do not have time for that now
	for (double angle = 0; angle <= 360; angle += angleIncrement)
	{
		double radians = 3.14 / 180.0 * angle;
		int ex = (int)(originX + (double)R_inches * PIXELS_IN_INCH * cos(radians));
		int ey = (int)(originY + (double)R_inches * PIXELS_IN_INCH * sin(radians));
//		bfw.AddLine(0, 0, (float)ex, (float)ey);
		bfw.AddLineAntiDistorted(0, 0, (float)ex, (float)ey);
	}
	bfw.CloseFile();
}

void Test_DrawClock()
{
//	drawClockBin();
//	DrawClockAnyRadius(1, 10);
	DrawClockAnyRadius(2, 20);
//	DrawClockAnyRadius(2.5, 60);
//	DrawClockAnyRadius(3, 30);
}