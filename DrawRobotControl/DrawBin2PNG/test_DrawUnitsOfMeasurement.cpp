#include "StdAfx.h"

#define NUM_COMMANDS_PER_UNIT	100

void drawMeasurementLines(int lines, int isHorizontal)
{
	char fileName[500];
	if (isHorizontal)
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_H_%d.bin", lines);
	}
	else
	{
		sprintf_s(fileName, sizeof(fileName), "UnitsOfMeasurement_V_%d.bin", lines);
	}

	BinFileWriter bfw(fileName);
	for (int line2 = -lines; line2 <= lines; line2++)
	{
		float startAt2 = (float)((line2 + 0) * NUM_COMMANDS_PER_UNIT);
		// horizontal line
		for (int line = -lines; line <= lines; line++)
		{
			if ((abs(line) % 2) == 0)
			{
				continue;
			}
			float startAt = (float)((line + 0) * NUM_COMMANDS_PER_UNIT);
			float endAt = (float)((line + 1) * NUM_COMMANDS_PER_UNIT);
			if (isHorizontal)
			{
				bfw.AddLine(startAt, startAt2, endAt, startAt2);
			}
			else
			{
				bfw.AddLine(startAt2, startAt, startAt2, endAt);
			}
		}
	}

	bfw.CloseFile();
}

// 9 inches, about 600 moves / inch ... 
void Test_DrawUnitsOfMeasurement()
{
	drawMeasurementLines(20, 1);
	drawMeasurementLines(20, 0);
	drawMeasurementLines(30, 1);
	drawMeasurementLines(30, 0);
}