#include "StdAfx.h"

// managed to draw max, non stretched, 2900 commands. Non adjusted that is about 5 inches
#define NUM_COMMANDS_PER_UNIT	25
#define TEAR_MULTIPLIER 1.0f

int isLineWithinTear(int sx, int sy, int ex, int ey)
{
	static FIBITMAP* tearImg = NULL;
	static int tearWidth = 0;
	static int tearHeight = 0;
	static int tearStride = 0;
	static BYTE* tearBytes = NULL;
	if (tearImg == NULL)
	{
		tearImg = LoadImage_("SA_2_Tear_filled_11.bmp");
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
		float pixelToInch_sx = sx * TEAR_MULTIPLIER;
		float pixelToInch_sy = sy * TEAR_MULTIPLIER;
		float pixelToInch_ex = ex * TEAR_MULTIPLIER;
		float pixelToInch_ey = ey * TEAR_MULTIPLIER;

		// origin is in the middle
		pixelToInch_sx += tearWidth / 2.0f;
		pixelToInch_sy += tearHeight / 2.0f;
		pixelToInch_ex += tearWidth / 2.0f;
		pixelToInch_ey += tearHeight / 2.0f;
		// scale inches to "tear image" pixels
		int img_sx = (int)(pixelToInch_sx);
		int img_sy = (int)(pixelToInch_sy);
		int img_ex = (int)(pixelToInch_ex);
		int img_ey = (int)(pixelToInch_ey);

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

#if 0
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
#endif

const int printLinesForSIG = 0;
void DrawLineAntidistortedAndAddToSIG(BinFileWriter *bfw, float sx, float sy, float ex, float ey)
{
	if (printLinesForSIG)
	{
		printf("PLINESTART\n");
		printf("%lf,%lf\n", sx / PIXELS_IN_INCH, sy / PIXELS_IN_INCH);
		printf("%lf,%lf\n", ex / PIXELS_IN_INCH, ey / PIXELS_IN_INCH);
		printf("PLINEEND\n");
	}
	else
	{
		bfw->AddLineAntiDistorted(sx, sy, ex, ey);
	}
}

//#define USE_LINE_DRAW_FUNC(sx,sy,ex,ey) bfw.AddLineAntiDistorted(sx/PIXELS_IN_INCH,sy/PIXELS_IN_INCH,ex/PIXELS_IN_INCH,ey/PIXELS_IN_INCH)
//#define USE_LINE_DRAW_FUNC(sx,sy,ex,ey) bfw.AddLineAntiDistorted(sx,sy,ex,ey)
#define USE_LINE_DRAW_FUNC(sx,sy,ex,ey) DrawLineAntidistortedAndAddToSIG(&bfw,sx,sy,ex,ey);
//#define USE_LINE_DRAW_FUNC(sx,sy,ex,ey) bfw.AddLine(sx,sy,ex,ey)

void drawMeasurementFullLines(int lines, int isHorizontal)
{
	SOFT_ASSERT((lines % 2) == 0, "There needs to be a reference line at the origin");

	char fileName[500];
	if (isHorizontal == 1)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_H_%d_%d_FL_06_01.bin", lines, NUM_COMMANDS_PER_UNIT);
	}
	else if (isHorizontal == 0)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_V_%d_%d_FL_06_01.bin", lines, NUM_COMMANDS_PER_UNIT);
	}
	else if (isHorizontal == 3)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_HV_%d_%d_FL_06_01.bin", lines, NUM_COMMANDS_PER_UNIT);
	}
	else if (isHorizontal == 4)
	{
		sprintf_s(fileName, sizeof(fileName), "HearthForm_%d_%d.sig", lines, NUM_COMMANDS_PER_UNIT);
		printf("PLINESTART\n");
	}

	BinFileWriter bfw(fileName);
	bfw.SetDrawSpeedPCT(50);

	// in a horizontal file, draw a vertical line to outline the position of origin
	if (isHorizontal == 1)
	{
		USE_LINE_DRAW_FUNC(0, 0, 0, NUM_COMMANDS_PER_UNIT);
	}
	else if (isHorizontal == 0)
	{
		USE_LINE_DRAW_FUNC(0, 0, NUM_COMMANDS_PER_UNIT, 0);
	}
	else if (isHorizontal == 3)
	{
		USE_LINE_DRAW_FUNC(0, 0, NUM_COMMANDS_PER_UNIT, NUM_COMMANDS_PER_UNIT);
	}

	double stashedX[1000], stashedY[1000];
	int stashCount = 0;

	// fill the tear with same distance, same length perfectly horrizontal or vertical lines
	float prev_minStart = 100000;
	float prev_maxEnd = -100000;
	for (int line2 = -lines; line2 <= lines; line2 += 2)
	{
		float XorY = (float)((line2 + 0) * NUM_COMMANDS_PER_UNIT);
		// horizontal line
		float minStart = 100000;
		float maxEnd = -100000;
		for (int line = -lines; line <= lines; line += 2)
		{
			float startAt = (float)((line + 0) * NUM_COMMANDS_PER_UNIT);
			float endAt = (float)((line + 1) * NUM_COMMANDS_PER_UNIT);
			int canUseValues = 0;
			if (isHorizontal == 1)
			{
				if (isLineWithinTear((int)startAt, (int)XorY, (int)endAt, (int)XorY)
					&& sLineAdjuster2.GetAdjustedPos(startAt, XorY).HasValues
					&& sLineAdjuster2.GetAdjustedPos(endAt, XorY).HasValues
					)
				{
					canUseValues = 1;
				}
			}
			else if (isHorizontal == 0)
			{
				if (isLineWithinTear((int)XorY, (int)startAt, (int)XorY, (int)endAt) 
					&& sLineAdjuster2.GetAdjustedPos(XorY, startAt).HasValues
					&& sLineAdjuster2.GetAdjustedPos(XorY, endAt).HasValues
					)
				{
					canUseValues = 1;
				}
			}
			else if (isHorizontal == 3)
			{
				if (isLineWithinTear((int)XorY, (int)startAt, (int)XorY, (int)startAt + NUM_COMMANDS_PER_UNIT) &&
					isLineWithinTear((int)XorY, (int)startAt, (int)XorY + NUM_COMMANDS_PER_UNIT, (int)startAt) &&
					isLineWithinTear((int)XorY, (int)startAt, (int)XorY + NUM_COMMANDS_PER_UNIT, (int)startAt + NUM_COMMANDS_PER_UNIT))
				{
					canUseValues = 1;
				}
			}
			else if (isHorizontal == 4 && isLineWithinTear((int)XorY, (int)startAt, (int)XorY, (int)endAt))
			{
				Adjusted2DPos2 ap1 = sLineAdjuster2.GetAdjustedPos(XorY, startAt);
				Adjusted2DPos2 ap2 = sLineAdjuster2.GetAdjustedPos(XorY, endAt);
				if (ap1.HasValues && ap2.HasValues)
				{
					double tx = 0, ty = 0;
					RelativePointsLine rpl;
//					if((prev_minStart == 100000 || sLineAdjuster2.DrawLine(XorY, prev_minStart, XorY, startAt, &rpl, tx, ty ) == 0)
					Adjusted2DPos2 ap1 = sLineAdjuster2.GetAdjustedPos(((float)(XorY / PIXELS_IN_INCH)) * PIXELS_IN_INCH, ((float)(startAt / PIXELS_IN_INCH)) * PIXELS_IN_INCH);
					Adjusted2DPos2 ap2 = sLineAdjuster2.GetAdjustedPos(((float)(XorY / PIXELS_IN_INCH)) * PIXELS_IN_INCH, ((float)(endAt / PIXELS_IN_INCH)) * PIXELS_IN_INCH);
					if (ap1.HasValues && ap2.HasValues)
					{
						canUseValues = 1;
					}
					else
					{
						printf("Rounding error detected\n");
					}
				}
			}
			if (canUseValues)
			{
				if (startAt < minStart)
				{
					minStart = startAt;
				}
				if (endAt > maxEnd)
				{
					maxEnd = endAt;
				}
			}
		}
		if (minStart != 100000)
		{
			if (isHorizontal == 1)
			{
				Adjusted2DPos2 ap1 = sLineAdjuster2.GetAdjustedPos(minStart, XorY);
				Adjusted2DPos2 ap2 = sLineAdjuster2.GetAdjustedPos(maxEnd, XorY);
				if (ap1.HasValues != 1 || ap2.HasValues != 1) printf("need to have values\n");
				USE_LINE_DRAW_FUNC(minStart, XorY, maxEnd, XorY);
			}
			else if (isHorizontal == 0)
			{
				Adjusted2DPos2 ap1 = sLineAdjuster2.GetAdjustedPos(XorY, minStart);
				Adjusted2DPos2 ap2 = sLineAdjuster2.GetAdjustedPos(XorY, maxEnd);
				if (ap1.HasValues != 1 || ap2.HasValues != 1) printf("need to have values\n");
				USE_LINE_DRAW_FUNC(XorY, minStart, XorY, maxEnd);
			}
			else if (isHorizontal == 3)
			{
				USE_LINE_DRAW_FUNC(XorY, minStart, XorY, minStart + NUM_COMMANDS_PER_UNIT);
				USE_LINE_DRAW_FUNC(XorY, minStart, XorY + NUM_COMMANDS_PER_UNIT, minStart);
				USE_LINE_DRAW_FUNC(XorY, minStart, XorY + NUM_COMMANDS_PER_UNIT, minStart + NUM_COMMANDS_PER_UNIT);
			}
			else if (isHorizontal == 4)
			{
				// because of rounding errors, these might point outside the actual teardrop
#define ROUNDING_ERR_DECREASE 0
				double XorY2, minStart2, maxEnd2;
				if (XorY < 0)
					XorY2 = XorY + ROUNDING_ERR_DECREASE * NUM_COMMANDS_PER_UNIT;
				else
					XorY2 = XorY - ROUNDING_ERR_DECREASE * NUM_COMMANDS_PER_UNIT;
				XorY2 = XorY;
				minStart2 = minStart + ROUNDING_ERR_DECREASE * NUM_COMMANDS_PER_UNIT;
				maxEnd2 = maxEnd - ROUNDING_ERR_DECREASE * NUM_COMMANDS_PER_UNIT;

				Adjusted2DPos2 ap = sLineAdjuster2.GetAdjustedPos(XorY2, minStart2);
				if (ap.HasValues)
				{
					printf("%lf,%lf\n", XorY2 / PIXELS_IN_INCH, minStart2 / PIXELS_IN_INCH);
//					printf("%lf,%lf\n", XorY2, minStart2);
//					printf("%lf,%lf\n", ap.x / PIXELS_IN_INCH, ap.y / PIXELS_IN_INCH);
				}
				else
				{
					printf("!! Drawing outside calibrated area 1: %lf,%lf\n", XorY2, minStart2);
				}
				ap = sLineAdjuster2.GetAdjustedPos(XorY2, maxEnd2);
				if (ap.HasValues)
				{
					stashedX[stashCount] = XorY2;
					stashedY[stashCount] = maxEnd2;
//					stashedX[stashCount] = ap.x;
//					stashedY[stashCount] = ap.y;
					stashCount++;
				}
				else
				{
					printf("!! Drawing outside calibrated area 2 : %lf,%lf\n", XorY2, maxEnd2);
				}
			}
		}
	}
	
	if (isHorizontal == 4)
	{
		for (int i = stashCount - 1; i >= 0; i--)
		{
			printf("%lf,%lf\n", stashedX[i] / PIXELS_IN_INCH, stashedY[i] / PIXELS_IN_INCH);
//			printf("%lf,%lf\n", stashedX[i], stashedY[i]);
		}
		printf("PLINEEND\nSetting\nSetting\n0, 0, 0, 11\n34082, 0, 0, 0, 1\n");
	}

	bfw.CloseFile();
}

// 9 inches, about 600 moves / inch ... 
void Test_DrawUnitsOfMeasurement()
{
//	drawMeasurementLines(30, 1);
//	drawMeasurementLines(30, 0);
//	drawMeasurementLines(30, 3);

	drawMeasurementFullLines(94, 1);
	if (printLinesForSIG)		printf("PLINEEND\nSetting\nSetting\n0, 0, 0, 11\n34082, 0, 0, 0, 1\n\n\n");
	drawMeasurementFullLines(94, 0);
	if (printLinesForSIG)		printf("PLINEEND\nSetting\nSetting\n0, 0, 0, 11\n34082, 0, 0, 0, 1\n\n\n");

//	drawMeasurementFullLines(76, 4); // draw current tear as sig
}