#include "StdAfx.h"

#define TEAR_MAX_COMMANDS_QAURTER 3000
#define IMG_SIZE 6000
#define IMG_CENTER ( IMG_SIZE/2)

void Test_VisualizeCallibrationMap()
{
	sLineAdjuster2.DebugDumpMapToTear();
//	sLineAdjuster2.DebugDumpMapToImage(X_IS_SET | Y_IS_SET, "map_locs_set.png", 0);
//	sLineAdjuster2.DebugDumpMapToImage(X_IS_MEASURED | Y_IS_MEASURED, "map_locs_measured.png", 0);
	sLineAdjuster2.DebugDumpMapToImage(X_IS_UPDATED | Y_IS_UPDATED, "map_locs_updated_1.png", 1);
	return;
	// tear is [-4.5,4.5] inches in size or [-2700,2700] in commands
	for (int i = 0; i < 1600; i += 100)
	{
		sLineAdjuster2.DebugDumpMapRowColToImage(i);
	}

	FIBITMAP* dib = CreateNewImage(IMG_SIZE, IMG_SIZE);
	for (int y = 0; y < TEAR_MAX_COMMANDS_QAURTER * 2; y++)
	{
		int xMiddleCentered = y - TEAR_MAX_COMMANDS_QAURTER;
		int yMiddleCentered = y - TEAR_MAX_COMMANDS_QAURTER;
		PositionAdjustInfo2* ai = sLineAdjuster2.GetAdjustInfo(xMiddleCentered, yMiddleCentered);
		if (ai == NULL || ai->HasX() == 0)
		{
			continue;
		}
		int correctedX = (int)ai->GetNewX(); // should be middle centered with range between -6000,6000
		printf("Corrected x=%d to %d at row %d\n", xMiddleCentered, correctedX, yMiddleCentered);
		int correctedY = yMiddleCentered;
		int xImgCentered1 = IMG_CENTER + correctedX;
		int xImgCentered2 = IMG_CENTER + xMiddleCentered;
		int yImgCentered = IMG_CENTER + y;
		DrawLineColor(dib, (float)xImgCentered1, (float)yImgCentered, (float)xImgCentered2, (float)yImgCentered, 255, 255, 255);
	}
	SaveImagePNG(dib, "MapVisualized.png");
	FreeImage_Unload(dib);
}