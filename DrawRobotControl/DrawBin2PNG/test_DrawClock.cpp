#include "StdAfx.h"

void drawClock(FIBITMAP* dib, RobotDrawSession& robotSession)
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