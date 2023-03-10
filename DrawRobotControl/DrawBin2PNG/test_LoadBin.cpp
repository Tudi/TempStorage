#include "StdAfx.h"

#define ImageSizeMultiplier 2

void Test_LoadBinFileGeneric(const char* fileName)
{
	FIBITMAP* dib = CreateNewImage(2048 * ImageSizeMultiplier, 2048 * ImageSizeMultiplier);
	RobotDrawSession robotSession;
	robotSession.curx = robotSession.startx = 2048 * ImageSizeMultiplier / 2;
	robotSession.cury = robotSession.starty = 2048 * ImageSizeMultiplier / 2;

	DrawCircleAt(dib, robotSession.curx, robotSession.cury, 300);

	uint32_t readPos = 0;
	size_t fileSize = 0;

	uint8_t* f = OpenBinFile(fileName, readPos, fileSize);
	ReadBinHeader(f, readPos, &robotSession);

	for (size_t i = 0; i < 60; i++)
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
}

void Test_LoadBinFile()
{
//	Test_LoadBinFileGeneric("../BinFiles/0002 Three Half Inch Vertical Lines Half Inch Apart.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0004 Three Half Inch Horizontal Lines Half Inch Apart.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0009 One Inch Square Centered on 0,0.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0011 Five One Inch Squares from 006.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0010 Five One Inch Squares from 004.bin");
	Test_LoadBinFileGeneric("clock.bin");
}