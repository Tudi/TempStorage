#include "StdAfx.h"

FIBITMAP* openImage(const char* fileName)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
	const char* InputFileName = fileName;
	if (InputFileName == NULL)
	{
		return NULL;
	}
	dib = LoadImage_(InputFileName);

	if (dib == NULL)
	{
		printf("Could not open input file %s\n", InputFileName);
		return NULL;
	}

	int bpp = FreeImage_GetBPP(dib);
	if (bpp != Bytespp * 8)
		dib = FreeImage_ConvertTo24Bits(dib);
	bpp = FreeImage_GetBPP(dib);
	if (bpp != Bytespp * 8)
	{
		printf("!!!!Only support 24 bpp input. Upgrade software or convert input from %d to 24\n", bpp);
		return NULL;
	}

	return dib;
}

void drawClock(FIBITMAP* dib, RobotDrawSession &robotSession)
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

#define ImageSizeMultiplier 2
int main()
{
	LoadImage_("NONE"); // initialize lib

	FIBITMAP* dib = CreateNewImage(2048 * ImageSizeMultiplier,2048 * ImageSizeMultiplier);
	RobotDrawSession robotSession;
	robotSession.curx = robotSession.startx = 2048 * ImageSizeMultiplier / 2;
	robotSession.cury = robotSession.starty = 2048 * ImageSizeMultiplier / 2;

	DrawCircleAt(dib, robotSession.curx, robotSession.cury, 300);
	
//	drawClock(dib, robotSession);

	uint32_t readPos = 0;
	size_t fileSize = 0;

//	uint8_t* f = OpenBinFile("../BinFiles/0002 Three Half Inch Vertical Lines Half Inch Apart.bin", readPos, fileSize);
//	uint8_t* f = OpenBinFile("../BinFiles/0004 Three Half Inch Horizontal Lines Half Inch Apart.bin", readPos, fileSize);
//	uint8_t* f = OpenBinFile("../BinFiles/0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.bin", readPos, fileSize);
//	uint8_t* f = OpenBinFile("../BinFiles/0009 One Inch Square Centered on 0,0.bin", readPos, fileSize);
	uint8_t* f = OpenBinFile("../BinFiles/0011 Five One Inch Squares from 006.bin", readPos, fileSize);
//	uint8_t* f = OpenBinFile("../ConstructBinFiles/BinFiles/CheckArmAngle_11_0_20_40_60_80.bin", readPos, fileSize);
	ReadBinHeader(f, readPos, &robotSession);

	for (size_t i = 0; i < 0; i++)
	{
		RelativePointsLine* line = NULL;
		RelativePointsLine::setStartingPosition(&line, robotSession.curx, robotSession.cury);
		int ret = ReadBinLine(f, readPos, fileSize, &line, &robotSession);
		if (ret != 0)
		{
			free(line);
			break;
		}

		// draw line on PNG
		DrawBinLineOnPNG(dib, robotSession.curx, robotSession.cury, line);

		free(line);
	}
	ReadBinFooter(f, readPos, &robotSession);

	free(f);

	SaveImagePNG(dib, "bin2PNG.png");
	FreeImage_Unload(dib);

	FreeImage_DeInitialise();
}