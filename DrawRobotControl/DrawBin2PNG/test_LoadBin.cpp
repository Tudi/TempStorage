#include "StdAfx.h"

#define ImageSizeMultiplier 2

void Test_LoadBinFileGeneric(const char* fileName)
{
	FIBITMAP* dib = CreateNewImage(2048 * ImageSizeMultiplier, 2048 * ImageSizeMultiplier);
	RobotDrawSession robotSession;
	robotSession.curx = robotSession.startx = 2048 * ImageSizeMultiplier / 2;
	robotSession.cury = robotSession.starty = 2048 * ImageSizeMultiplier / 2;

//	DrawCircleAt(dib, robotSession.curx, robotSession.cury, 300);

	uint32_t readPos = 0;
	size_t fileSize = 0;

	uint8_t* f = OpenBinFile(fileName, readPos, fileSize);
	if (f == NULL)
	{
		return;
	}
	ReadBinHeader(f, readPos, &robotSession);

	for (size_t i = 0; i < 6000; i++)
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

	char* sBinFileName = _strdup(fileName);
	if (sBinFileName == NULL)
	{
		SOFT_ASSERT(false, "Failed to copy input SIG file name");
		return;
	}
	size_t uiNameLen = strlen(fileName);
	sBinFileName[uiNameLen - 3] = 'p';
	sBinFileName[uiNameLen - 2] = 'n';
	sBinFileName[uiNameLen - 1] = 'g';

	SaveImagePNG(dib, sBinFileName);
	free(sBinFileName);
	FreeImage_Unload(dib);
}

void Test_LoadBinFile(char** argv, int argc)
{
//	Test_LoadBinFileGeneric("../BinFiles/0002 Three Half Inch Vertical Lines Half Inch Apart.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0004 Three Half Inch Horizontal Lines Half Inch Apart.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0009 One Inch Square Centered on 0,0.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0011 Five One Inch Squares from 006.bin");
//	Test_LoadBinFileGeneric("../BinFiles/0010 Five One Inch Squares from 004.bin");
//	Test_LoadBinFileGeneric("clock.bin");
//	Test_LoadBinFileGeneric("../BinFiles/right_right.bin");
//	Test_LoadBinFileGeneric("../BinFiles/left_left.bin");
//	Test_LoadBinFileGeneric("../BinFiles/up_up.bin");
//	Test_LoadBinFileGeneric("../BinFiles/down_down.bin");
//	Test_LoadBinFileGeneric("../BinFiles/Left_then_clock.bin");
//	Test_LoadBinFileGeneric("../BinFiles/right_then_clock.bin");
//	Test_LoadBinFileGeneric("S005 Five One Inch Squares.bin");
	if (argc < 3)
	{
		printf("expecting array of input SIG file names\n");
		return;
	}
	if (strcmp(argv[1], "-b2i") != 0)
	{
		return;
	}
	for (size_t i = 2; i < argc; i++)
	{
		Test_LoadBinFileGeneric(argv[i]);
	}
}