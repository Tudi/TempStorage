#include "StdAfx.h"

// managed to draw max, non stretched, 2900 commands. Non adjusted that is about 5 inches
#define NUM_COMMANDS_PER_UNIT	50
#define PIXELS_IN_INCH_FOR_TEAR (PIXELS_IN_INCH*0.7f) // try to not full fill tear

int isLineWithinTear(int sx, int sy, int ex, int ey)
{
	static FIBITMAP* tearImg = NULL;
	static int tearWidth = 0;
	static int tearHeight = 0;
	static int tearStride = 0;
	static BYTE* tearBytes = NULL;
	if (tearImg == NULL)
	{
		tearImg = LoadImage_("SA_2_Tear_filled.bmp");
		if (tearImg)
		{
			tearStride = FreeImage_GetPitch(tearImg);
			tearBytes = FreeImage_GetBits(tearImg);
			tearWidth = FreeImage_GetWidth(tearImg);
			tearHeight = FreeImage_GetHeight(tearImg);
		}
	}
	int IsWithin = 0;
	if (tearImg)
	{
		// scale the sx,sy,ex,ey so that commands become inches
		float pixelToInch_sx = sx / PIXELS_IN_INCH_FOR_TEAR;
		float pixelToInch_sy = sy / PIXELS_IN_INCH_FOR_TEAR;
		float pixelToInch_ex = ex / PIXELS_IN_INCH_FOR_TEAR;
		float pixelToInch_ey = ey / PIXELS_IN_INCH_FOR_TEAR;

#define TearWidthInInches	9.0f
#define TearHeightInInches	9.0f
		// origin is in the middle
		pixelToInch_sx += TearWidthInInches / 2.0f;
		pixelToInch_sy += TearHeightInInches / 2.0f;
		pixelToInch_ex += TearWidthInInches / 2.0f;
		pixelToInch_ey += TearHeightInInches / 2.0f;
		// scale inches to "tear image" pixels
		int img_sx = (int)((float)tearWidth / TearWidthInInches * pixelToInch_sx);
		int img_sy = (int)((float)tearHeight / TearHeightInInches * pixelToInch_sy);
		int img_ex = (int)((float)tearWidth / TearWidthInInches * pixelToInch_ex);
		int img_ey = (int)((float)tearHeight / TearHeightInInches * pixelToInch_ey);

		if (img_sx > 0 && img_sy > 0 && img_sx < tearWidth && img_sy < tearHeight)
		{
			BYTE B = tearBytes[img_sy * tearStride + img_sx * Bytespp];
			if (B == 0)
			{
				IsWithin++;
			}
		}
		if (img_ex > 0 && img_ey > 0 && img_ex < tearWidth && img_ey < tearHeight)
		{
			BYTE B = tearBytes[img_ey * tearStride + img_ex * Bytespp];
			if (B == 0)
			{
				IsWithin++;
			}
		}
	}
	return IsWithin == 2;
}

void drawMeasurementLines(int lines, int isHorizontal)
{
	SOFT_ASSERT((lines % 2) == 0, "There needs to be a reference line at the origin");

	char fileName[500];
	if (isHorizontal == 1)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_H_%d_50_03_17.bin", lines);
	}
	else if (isHorizontal == 0)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_V_%d_50_03_17.bin", lines);
	}
	else if (isHorizontal == 3)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_HV_%d_50_03_17.bin", lines);
	}

	BinFileWriter bfw(fileName);

	// in a horizontal file, draw a vertical line to outline the position of origin
	if (isHorizontal == 1)
	{
		bfw.AddLine(0, 0, 0, NUM_COMMANDS_PER_UNIT);
	}
	else if (isHorizontal == 0)
	{
		bfw.AddLine(0, 0, NUM_COMMANDS_PER_UNIT, 0);
	}
	else if (isHorizontal == 3)
	{
		bfw.AddLine(0, 0, NUM_COMMANDS_PER_UNIT, NUM_COMMANDS_PER_UNIT);
	}

	// fill the tear with same distance, same length perfectly horrizontal or vertical lines
	for (int line2 = -lines; line2 <= lines; line2+=2)
	{
		float XorY = (float)((line2 + 0) * NUM_COMMANDS_PER_UNIT);
		// horizontal line
		for (int line = -lines; line <= lines; line+=2)
		{
			float startAt = (float)((line + 0) * NUM_COMMANDS_PER_UNIT);
			float endAt = (float)((line + 1) * NUM_COMMANDS_PER_UNIT);
			if (isHorizontal == 1)
			{
				if (isLineWithinTear((int)startAt, (int)XorY, (int)endAt, (int)XorY))
				{
					bfw.AddLine(startAt, XorY, endAt, XorY);
				}
			}
			else if (isHorizontal == 0)
			{
				if (isLineWithinTear((int)XorY, (int)startAt, (int)XorY, (int)endAt))
				{
					bfw.AddLine(XorY, startAt, XorY, endAt);
				}
			}
			else if (isHorizontal == 3)
			{
				if (isLineWithinTear((int)XorY, (int)startAt, (int)XorY, (int)startAt + NUM_COMMANDS_PER_UNIT) &&
					isLineWithinTear((int)XorY, (int)startAt, (int)XorY + NUM_COMMANDS_PER_UNIT, (int)startAt) && 
					isLineWithinTear((int)XorY, (int)startAt, (int)XorY + NUM_COMMANDS_PER_UNIT, (int)startAt + NUM_COMMANDS_PER_UNIT))
				{
					bfw.AddLine(XorY, startAt, XorY, startAt + NUM_COMMANDS_PER_UNIT);
					bfw.AddLine(XorY, startAt, XorY + NUM_COMMANDS_PER_UNIT, startAt);
					bfw.AddLine(XorY, startAt, XorY + NUM_COMMANDS_PER_UNIT, startAt + NUM_COMMANDS_PER_UNIT);
				}
			}
		}
	}

	bfw.CloseFile();
}

// 9 inches, about 600 moves / inch ... 
void Test_DrawUnitsOfMeasurement()
{
	drawMeasurementLines(30, 1);
	drawMeasurementLines(30, 0);
	drawMeasurementLines(30, 3);
}