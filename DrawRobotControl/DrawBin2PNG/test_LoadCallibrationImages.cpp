#include "StdAfx.h"

#define ORIGIN_R 237
#define ORIGIN_G 28
#define ORIGIN_B 36
#define EXPECTED_NORMAL_LINE_GAP 4 // counted in pixels

int CheckLinePresentNearby(FIBITMAP* Img, int x, int y, int radius, int lineId, int &atx, int &aty)
{
	if (x < radius || y < radius)
	{
		return 0;
	}
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	if (x + radius >= Width || y + radius >= Height)
	{
		return 0;
	}

	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	for (int y2 = -radius; y2 <= radius; y2++)
	{
		for (int x2 = -radius; x2 <= radius; x2++)
		{
			if (y2 == 0 && x2 == 0)
			{
				continue;
			}
			// black pixel means there is a line pixel
//			if(*(short*)&Bytes[(y + y2) * Stride + (x + x2) * Bytespp] == lineId)
			if (IsCallibrationLinePixel(Img, x + x2, y + y2))
			{
				atx = x + x2;
				aty = y + y2;
				return 1;
			}
		}
	}

	return 0;
}

// Find up to 8 neighbour shapes around the center shape
// expecting neighbour shapes to be similar to center shape ( size )
int FindNeighbourShapes(FIBITMAP* Img, ShapeStore out_neighbours[3][3])
{
	typedef struct ShapeSearchParams
	{
		int expectedX, expectedY;
		int startX, startY;
		int storeX, storeY;
	}ShapeSearchParams;

	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	int shapeWidth = out_neighbours[1][1].maxX - out_neighbours[1][1].minX;
	int shapeHeight = out_neighbours[1][1].maxY - out_neighbours[1][1].minY;
	int shapeWidthMin = shapeWidth * 80 / 100;
	int shapeWidthMax = shapeWidth * 120 / 100;
	int shapeHeightMin = shapeHeight * 80 / 100;
	int shapeHeightMax = shapeHeight * 120 / 100;
	int shapeGapSize = MAX(shapeWidth, shapeHeight); // expecting to find this gap between shapes
	int shapeSizeSearchJumpX = shapeGapSize * 2;
	int shapeSizeSearchJumpY = shapeGapSize * 2;
	int neiborsFound = 0;

	ShapeSearchParams sp[8] = {
		{ out_neighbours[1][1].centerx - shapeSizeSearchJumpX , out_neighbours[1][1].centery, 0, 0, -1, 0}, // left to center
		{ out_neighbours[1][1].centerx + shapeSizeSearchJumpX, out_neighbours[1][1].centery, 0, 0, 1, 0}, // right to center
		{ out_neighbours[1][1].centerx, out_neighbours[1][1].centery - shapeSizeSearchJumpY, 0, 0, 0, -1}, // up to center
		{ out_neighbours[1][1].centerx, out_neighbours[1][1].centery + shapeSizeSearchJumpY, 0, 0, 0, 1}, // down to center

		{ out_neighbours[1][1].centerx - shapeSizeSearchJumpX, out_neighbours[1][1].centery - shapeSizeSearchJumpY, 0, 0, -1, -1}, // up - left to center
		{ out_neighbours[1][1].centerx + shapeSizeSearchJumpX, out_neighbours[1][1].centery - shapeSizeSearchJumpY, 0, 0, 1, -1}, // up - right to center
		{ out_neighbours[1][1].centerx - shapeSizeSearchJumpX, out_neighbours[1][1].centery + shapeSizeSearchJumpY, 0, 0, -1, 1}, // down - left to center
		{ out_neighbours[1][1].centerx + shapeSizeSearchJumpX, out_neighbours[1][1].centery + shapeSizeSearchJumpY, 0, 0, 1, 1}, // down - right to center
	};

	//for every searched location, find the closest valid line
	for (size_t spInd = 0; spInd < _countof(sp); spInd++)
	{
		__int64 sumX = 0;
		__int64 sumY = 0;
		__int64 sumCount = 0;
		__int64 bestDist = 0x7FFFFFFFFFFFFF;
		// search around the expected location. Due to distorsion, the actual location could be very far from expected
		for (int y = sp[spInd].expectedY - shapeGapSize / 2; y <= sp[spInd].expectedY + shapeGapSize / 2; y++)
		{
			for (int x = sp[spInd].expectedX - shapeGapSize / 2; x <= sp[spInd].expectedX + shapeGapSize / 2; x++)
			{
				if (IsCallibrationLinePixel(Img, x, y))
				{
					sumX += x;
					sumY += y;
					sumCount++;
					int distx = x - sp[spInd].expectedX;
					int disty = y - sp[spInd].expectedY;
					int dist = distx * distx + disty * disty;
					if (dist < bestDist)
					{
						bestDist = dist;
						sp[spInd].startX = x;
						sp[spInd].startY = y;
					}
				}
			}
		}

		// can't extract a shape at this locatin ?
		if (sumCount < MAX(shapeWidthMin, shapeHeightMin))
		{
			continue;
		}

		ShapeStore ssNow = ExtractShapeAtLoc(Img, sp[spInd].startX, sp[spInd].startY, EXPECTED_NORMAL_LINE_GAP, shapeWidthMax, shapeHeightMax);
		int curShapeWidth = ssNow.maxX - ssNow.minX;
		int scurSapeHeight = ssNow.maxY - ssNow.minY;
		int shapeSizeUnexpected = 0;
		// line was found, but it's size is strange size
/*		if (curShapeWidth < shapeHeightMin || curShapeWidth > shapeHeightMax)
		{
			shapeSizeUnexpected = 1;
		}
		if (scurSapeHeight < shapeHeightMin || scurSapeHeight > shapeHeightMax)
		{
			shapeSizeUnexpected = 1;
		}*/
		if (shapeSizeUnexpected == 0)
		{
			ssNow.expectedx = sp[spInd].expectedX;
			ssNow.expectedy = sp[spInd].expectedY;
			printf("%d next shape width=%d, height=%d, x=%d-%d, y=%d-%d, x=%d y=%d, xs=%d ys=%d, xexp=%d yexp=%d, xdiffexpected=%d ydiffexpected=%d\n", (int)spInd,
				ssNow.maxX - ssNow.minX, ssNow.maxY - ssNow.minY,
				ssNow.minX, ssNow.maxX, ssNow.minY, ssNow.maxY, ssNow.centerx, ssNow.centery, ssNow.startX, ssNow.startY,
				ssNow.expectedx, ssNow.expectedy, ssNow.centerx - ssNow.expectedx, ssNow.centery - ssNow.expectedy);
			out_neighbours[1 + sp[spInd].storeX][1 + sp[spInd].storeY] = ssNow;
			neiborsFound++;
		}
	}

	return neiborsFound;
}

void FindManuallyMarkedOrigin(FIBITMAP* Img, int& manualMarkedOriginX, int& manualMarkedOriginY)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	for (size_t y = 0; y < Height; y++)
	{
		for (size_t x = 0; x < Width; x++)
		{
			BYTE* bNow = &Bytes[y * Stride + x * Bytespp];
			if (bNow[0] == ORIGIN_B && bNow[1] == ORIGIN_G && bNow[2] == ORIGIN_R)
			{
				manualMarkedOriginX = (int)x;
				manualMarkedOriginY = (int)y;
				return;
			}
		}
	}
}

void MarkCalibrationChangesOnImg(FIBITMAP* Img, ShapeStore *s)
{
	// sanity check. Should not happen
	int xAdjust = s->centerx - s->expectedx;
	int yAdjust = s->centery - s->expectedy;
	if (abs(xAdjust) > 500 || abs(yAdjust) > 500)
	{
		return;
	}

	DrawLineColor(Img, (float)s->expectedx, (float)s->expectedy, (float)s->centerx, (float)s->centery, 255, 0, 255);

	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int x = s->expectedx;
	int y = s->expectedy;
	Bytes[y * Stride + x * Bytespp + 0] = 0;
	Bytes[y * Stride + x * Bytespp + 1] = 255;
	Bytes[y * Stride + x * Bytespp + 2] = 0;

	x = s->centerx;
	y = s->centery;
	Bytes[y * Stride + x * Bytespp + 0] = 0;
	Bytes[y * Stride + x * Bytespp + 1] = 0;
	Bytes[y * Stride + x * Bytespp + 2] = 255;
}

void SearchAndMarkAllNeighbours(FIBITMAP* Img, ShapeStore * ssCenter, ShapeStore *out_ss)
{
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	ShapeStore* ssQueue = (ShapeStore*)malloc(Width * Height * sizeof(ShapeStore));
	if (ssQueue == NULL)
	{
		return;
	}
	memset(ssQueue, 0, Width * Height * sizeof(ShapeStore));

	int ssQueueSize = 1;
	int ssQueueReadIndex = 0; // don't make it LIFO. Does not work due to increasing distortion values
	ssQueue[0] = *ssCenter;

	while (ssQueueSize > ssQueueReadIndex)
	{
		ShapeStore neighbours[3][3];
		memset(neighbours, 0, sizeof(neighbours));
		neighbours[1][1] = ssQueue[ssQueueReadIndex];
		ssQueueReadIndex++;

		FindNeighbourShapes(Img, neighbours);
		neighbours[1][1].checkedNeighbours = 1;

		for (int y = 0; y < 3; y++)
		{
			for (int x = 0; x < 3; x++)
			{
				// we searched around the "center" and we do not need to search that pos again
				if (x == 1 && y == 1)
				{
					continue;
				}
				// not a valid shape
				if (neighbours[x][y].minX == neighbours[x][y].maxX)
				{
					continue;
				}
				// valid shape and should search if it has neighbours
				out_ss[neighbours[x][y].centery * Width + neighbours[x][y].centerx] = neighbours[x][y];
//				if (x == 1 && y == 0)
//				if ((x == 0 && y == 1) || (x == 2 && y == 1))
				{
					ssQueue[ssQueueSize] = neighbours[x][y];
					ssQueueSize++;
				}
				MarkCalibrationChangesOnImg(Img, &neighbours[x][y]);
			}
		}
	}

	free(ssQueue);
}

void RemoveManuallyMarkedOrigin(FIBITMAP* Img, int manualMarkedOriginX, int manualMarkedOriginY)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	for (int y = manualMarkedOriginY - 30; y < manualMarkedOriginY + 30; y++)
	{
		for (int x = manualMarkedOriginX - 30; x < manualMarkedOriginX + 30; x++)
		{
			BYTE* bNow = &Bytes[y * Stride + x * Bytespp];
			if (bNow[0] == ORIGIN_B && bNow[1] == ORIGIN_G && bNow[2] == ORIGIN_R)
			{
				bNow[0] = 0;
				bNow[1] = 0;
				bNow[2] = 0;
			}
		}
	}
}

void LoadSpecificCallibrationImage(const char* fileName, int isInitial, int commandCountRef)
{
	FIBITMAP* Img = LoadImage_(fileName);
	if (Img == NULL)
	{
		printf("Missing callibration image %s\n", fileName);
		return;
	}
	int bpp = FreeImage_GetBPP(Img);
	if (bpp != 24)
	{
		printf("Expecting 24 bpp image %s\n", fileName);
		return;
	}
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);

	int manualMarkedOriginX = 0;
	int manualMarkedOriginY = 0;
	FindManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	if (manualMarkedOriginX == 0 && manualMarkedOriginY == 0)
	{
		printf("Could not find manually marked origin in callibration image %s\n", fileName);
		return;
	}
	printf("Origin is at %d,%d for image %s\n", manualMarkedOriginX, manualMarkedOriginY, fileName);

	// delete this marked origin
	RemoveManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	ShapeStore* ss = (ShapeStore*)malloc(Width * Height * sizeof(ShapeStore));
	if (ss == NULL)
	{
		return;
	}
	memset(ss, 0, Width * Height * sizeof(ShapeStore));

	ShapeStore ssCenter = ExtractShapeAtLoc(Img, manualMarkedOriginX, manualMarkedOriginY, EXPECTED_NORMAL_LINE_GAP, 100, 100);
	printf("reference shape width=%d, height=%d, x=%d-%d, y=%d-%d\n", ssCenter.maxX - ssCenter.minX, ssCenter.maxY - ssCenter.minY,
		ssCenter.minX, ssCenter.maxX, ssCenter.minY, ssCenter.maxY);
	if (ssCenter.minX == ssCenter.maxX)
	{
		printf("Failed to extract 'shape' at the origin\n");
		return;
	}

	// note down progress so far
	ss[ssCenter.centery * Width + ssCenter.centerx] = ssCenter;
	MarkCalibrationChangesOnImg(Img, &ssCenter);

	// search all possible shapes using relative positioning
	SearchAndMarkAllNeighbours(Img, &ssCenter, ss);

#ifdef IMAGE_IS_NOT_BLACK_AND_WHITTE
	// convert image to black and white
	for (size_t y = 0; y < Height; y++)
	{
		for (size_t x = 0; x < Width; x++)
		{
			BYTE* bNow = &Bytes[y * Stride + x * Bytespp];
			if (bNow[0] >= 64 || bNow[1] >= 64 || bNow[2] >= 64)
			{
				bNow[0] = 255;
				bNow[1] = 255;
				bNow[2] = 255;
			}
			else
			{
				bNow[0] = 0;
				bNow[1] = 0;
				bNow[2] = 0;
			}
		}
	}
#endif
//#define USE_LINE_ANTIGAP_ON_CALIBRATION_MAP
#ifdef USE_LINE_ANTIGAP_ON_CALIBRATION_MAP
	// merge disconnected lines
#define LINE_GAP_SIZE 2
	for (size_t y = LINE_GAP_SIZE; y < Height - LINE_GAP_SIZE; y++)
	{
		for (size_t x = LINE_GAP_SIZE; x < Width - LINE_GAP_SIZE; x++)
		{
			BYTE* b = &Bytes[y * Stride + x * Bytespp + 0];
			// we are on a line, nothing to correct here
			if (b[0] == 0)
			{
				continue;
			}
			// is this point between a disjoint line ?
			for (int dist = -LINE_GAP_SIZE; dist <= LINE_GAP_SIZE; dist++)
			{
				for (int gap1 = 1; gap1 <= LINE_GAP_SIZE; gap1++)
				{
					for (int gap2 = 1; gap2 <= LINE_GAP_SIZE; gap2++)
					{
						BYTE* b1, * b2;
						b1 = &Bytes[(y - gap1) * Stride + (x + dist) * Bytespp];
						b2 = &Bytes[(y + gap2) * Stride + (x - dist) * Bytespp];
						if (b1[0] == 0 && b2[0] == 0)
						{
							b[0] = 0;
							b[1] = 0;
							b[2] = 0;
							goto Pixel_Is_A_Gap;
						}
						b1 = &Bytes[(y + dist) * Stride + (x - gap1) * Bytespp];
						b2 = &Bytes[(y - dist) * Stride + (x + gap2) * Bytespp];
						if (b1[0] == 0 && b2[0] == 0)
						{
							b[0] = 0;
							b[1] = 0;
							b[2] = 0;
							goto Pixel_Is_A_Gap;
						}
					}
				}
			}
		Pixel_Is_A_Gap:
			continue;
		}
	}
#endif

#ifdef USE_ERODE_ON_CALIBRATION_MAP
	// errode small "errors"
	BYTE* OriImg = (BYTE*)malloc(Stride * Height);
	if (OriImg == NULL)
	{
		return;
	}
	memcpy(OriImg, Bytes, Stride * Height);

	for (size_t y = 1; y < Height - 1; y++)
	{
		for (size_t x = 1; x < Width - 1; x++)
		{
			int drawnNeighbourCount = 0;
			for (size_t y2 = y - 1; y2 <= y + 1; y2++)
			{
				for (size_t x2 = x - 1; x2 <= x + 1; x2++)
				{
					BYTE* bNow = &OriImg[y2 * Stride + x2 * Bytespp];
					if (bNow[0] != 0)
					{
						drawnNeighbourCount++;
					}
				}
			}
			// if it has less than 5 neighbours, it is considered that this is a noise and we remove it
			if (drawnNeighbourCount < 5 + 1)
			{
				BYTE* bNow = &OriImg[y * Stride + x * Bytespp];
				bNow[0] = 0;
				bNow[1] = 0;
				bNow[2] = 0;
			}
		}
	}
	free(OriImg); OriImg = NULL;
#endif

	free(ss);
	SaveImagePNG(Img, "t.png");
}

void GetGapSizePixelsAtOrigin_V(int Width, ShapeStore*ss, int manualMarkedOriginX, int manualMarkedOriginY, int &avgGapSize, int& centerStartX)
{
	ShapeStore* sCenter = NULL, * sLeft = NULL, * sRight = NULL;
	for (int x = manualMarkedOriginX - 10; x < manualMarkedOriginX + 10; x++)
	{
		if (ss[x].HasValues())
		{
			sCenter = &ss[x];
			break;
		}
	}
	if (sCenter == NULL)
	{
		printf("Unexpected : Could not find at least 1 line \n");
		return;
	}
	for (int x = sCenter->startX - 1; x > 0; x--)
	{
		if (ss[x].HasValues())
		{
			sLeft = &ss[x];
			break;
		}
	}
	for (int x = sCenter->startX + 1; x < Width; x++)
	{
		if (ss[x].HasValues())
		{
			sRight = &ss[x];
			break;
		}
	}
	if (sLeft == NULL || sRight == NULL)
	{
		printf("Unexpected : Could not find at least 3 lines \n");
		return;
	}

	// expected gap size, so we can know how skewed ar the lines to left or right
	avgGapSize = ((sCenter->startX - sLeft->startX) + (sRight->startX - sCenter->startX)) / 2;
	centerStartX = sCenter->startX;
}

void GetGapSizePixelsAtOrigin_H(int Height, ShapeStore* ss, int manualMarkedOriginX, int manualMarkedOriginY, int& avgGapSize, int& centerStartY)
{
	ShapeStore* sCenter = NULL, * sLeft = NULL, * sRight = NULL;
	for (int y = manualMarkedOriginY - 10; y < manualMarkedOriginY + 10; y++)
	{
		if (ss[y].HasValues())
		{
			sCenter = &ss[y];
			break;
		}
	}
	if (sCenter == NULL)
	{
		printf("Unexpected : Could not find at least 1 line \n");
		return;
	}
	for (int y = sCenter->startY - 1; y > 0; y--)
	{
		if (ss[y].HasValues())
		{
			sLeft = &ss[y];
			break;
		}
	}
	for (int y = sCenter->startY + 1; y < Height; y++)
	{
		if (ss[y].HasValues())
		{
			sRight = &ss[y];
			break;
		}
	}
	if (sLeft == NULL || sRight == NULL)
	{
		printf("Unexpected : Could not find at least 3 lines \n");
		return;
	}

	// expected gap size, so we can know how skewed ar the lines to left or right
	avgGapSize = ((sCenter->startY - sLeft->startY) + (sRight->startY - sCenter->startY)) / 2;
	centerStartY = sCenter->startY;
}

void SetExpectedXForVerticalLinesAtY0(int Width, ShapeStore* ss, int avgGapSize, int centerStartX)
{
	for (int moveDirection : {-1, 1})
	{
		int lineCounter = 1;
		for (int x = centerStartX + moveDirection; x != 0 && x != Width; x = x + moveDirection )
		{
			if (!ss[x].HasValues())
			{
				continue;
			}
			ss[x].expectedx = centerStartX + moveDirection * lineCounter * avgGapSize;
			ss[x].lineIndex = moveDirection * lineCounter;
			printf("Should move vertical line %d to %d from %d\n", moveDirection * lineCounter, ss[x].expectedx, ss[x].startX);
			lineCounter++;
		}
	}
	// line at the center is getting skipped
	ss[centerStartX].expectedx = centerStartX;
	ss[centerStartX].lineIndex = 0;
}

void SetExpectedYForHorizontalLinesAtX0(int Height, ShapeStore* ss, int avgGapSize, int centerStartY)
{
	for (int moveDirection : {-1, 1})
	{
		int lineCounter = 1;
		for (int y = centerStartY + moveDirection; y != 0 && y != Height; y = y + moveDirection)
		{
			if (!ss[y].HasValues())
			{
				continue;
			}
			ss[y].expectedy = centerStartY + moveDirection * lineCounter * avgGapSize;
			ss[y].lineIndex = moveDirection * lineCounter;
			printf("Should move horizontal line %d to %d from %d\n", moveDirection * lineCounter, ss[y].expectedy, ss[y].startY);
			lineCounter++;
		}
	}
	// line at the center is getting skipped
	ss[centerStartY].expectedy = centerStartY;
	ss[centerStartY].lineIndex = 0;
}

// incomming coordinates are in pixels. We need to translate pixels into commands. Commands into inches
void UpdateCallibrationAtSpecificLocationX(int centerX, int centerY, int atx, int aty, int shouldBeX, int gapSizePixels, int commandsIn1Gap)
{
	int dist_atx_shouldbeX = atx - shouldBeX; // need to add this value to "shouldbeX" in order to actually arive to it
	// coordinates are given in pixels
	int xRelPicOrigin = atx - centerX;
	int yRelPicOrigin = aty - centerY; // as normally read from file
//	int yRelPicOrigin = centerY - aty; // vertical flip
	int xRelPicOriginShouldBe = shouldBeX - centerX;
	int xRelPicOriginShouldBeCorrected = shouldBeX - (atx - shouldBeX) - centerX;
	// scale the coordinates from the image to size of the calibration map
	float commandsIn1Pixel = (float)commandsIn1Gap / (float)gapSizePixels;
	float xRelPicOriginComm = (xRelPicOrigin * commandsIn1Pixel);
	float yRelPicOriginComm = (yRelPicOrigin * commandsIn1Pixel);
	float xRelPicOriginShouldBeComm = (xRelPicOriginShouldBe * commandsIn1Pixel);
	float xRelPicOriginShouldBeCorrectedComm = (xRelPicOriginShouldBeCorrected * commandsIn1Pixel);
	double xRelPicOriginInch = (xRelPicOriginComm / PIXELS_IN_INCH);
	double yRelPicOriginInch = (yRelPicOriginComm / PIXELS_IN_INCH);
	double xRelPicOriginShouldBeInch = (xRelPicOriginShouldBeComm / PIXELS_IN_INCH);

	static int adjustmentsMade = 0;
	printf("%d) X correction at %d,%d. Reposition to %d,%d. Rel %d, %d. In inches %0.2f,%.02f to %.02f\n", 
		adjustmentsMade, atx, aty, shouldBeX, aty, xRelPicOrigin, yRelPicOrigin, xRelPicOriginInch, yRelPicOriginInch, xRelPicOriginShouldBeInch);
	adjustmentsMade++;

//	sLineAdjuster.AdjustPositionX((int)xRelPicOriginComm, (int)yRelPicOriginComm, (int)xRelPicOriginShouldBeComm);
	sLineAdjuster.AdjustPositionX((int)xRelPicOriginShouldBeComm, (int)yRelPicOriginComm, (int)xRelPicOriginShouldBeCorrectedComm);
//	sLineAdjuster.AdjustPositionX(xRelPicOriginInch, yRelPicOriginInch, xRelPicOriginShouldBeInch);
}

void UpdateCallibrationForEveryVerticalLineEveryRow(FIBITMAP* Img, ShapeStore* ss, int centerStartX, int gapSizePixels, int commandsIn1Gap, int centerX, int centerY)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);

#define LINE_CONTINUATION_SEARCH_RANGE 50
	for (int x = 0; x < Width; x++) // iterate through every vertical line starting left, going right
	{
		if (!ss[x].HasValues())
		{
			continue;
		}

//		if (ss[x].lineIndex != 15) continue;

		// for every row above the current row
		for (int progressDirection : {-1, 1})
		{
			int prevStartX = ss[x].startX;
			// from middle, go up and down
			for (int y = ss[x].startY; y != 0 && y != Height; y = y + progressDirection)
			{
				// find the startX on this row for this specific line
				int foundLine = 0;
				for (int sr = 0; sr < LINE_CONTINUATION_SEARCH_RANGE; sr++)
				{
					for (int x2 : {prevStartX - sr, prevStartX + sr} )
					{
						if (x2 < 0 || x2 >= Width)
						{
							continue;
						}
						short lineIdAt = *(short*)&Bytes[y * Stride + x2 * Bytespp];
						if (lineIdAt == *(short*)&ss[x].lineId)
						{
							prevStartX = x2;
							UpdateCallibrationAtSpecificLocationX(centerX, centerY, x2, y, ss[x].expectedx, gapSizePixels, commandsIn1Gap);
							// debug set expected location
//							if(x2< ss[x].expectedx)	DrawLineColor(Img, x2 + 1, y, ss[x].expectedx, y, 255, 0, 0);
//							else DrawLineColor(Img, x2 - 1, y, ss[x].expectedx, y, 255, 0, 0);
// 							// temp swap orientation. Should be bad
//							UpdateCallibrationAtSpecificLocationX(centerX, centerY, ss[x].expectedx, y, x2, gapSizePixels, commandsIn1Gap);
							foundLine = 1;
							goto found_line;
						}
					}
				}
found_line:
				if (foundLine == 0)
				{
					printf("line continuation not found at x=%d y=%d\n", ss[x].startX, y); // happens at every end of line
					break;
				}
			}
		}
	}
}


// incomming coordinates are in pixels. We need to translate pixels into commands. Commands into inches
void UpdateCallibrationAtSpecificLocationY(int centerX, int centerY, int atx, int aty, int shouldBeY, int gapSizePixels, int commandsIn1Gap)
{
	int dist_aty_shouldbeY = aty - shouldBeY; // need to add this value to "shouldbeX" in order to actually arive to it
	// coordinates are given in pixels
	int xRelPicOrigin = atx - centerX;
	int yRelPicOrigin = aty - centerY; // as normally read from file
	//	int yRelPicOrigin = centerY - aty; // vertical flip
	int yRelPicOriginShouldBe = shouldBeY - centerY;
	int yRelPicOriginShouldBeCorrected = shouldBeY - (aty - shouldBeY) - centerY;
	// scale the coordinates from the image to size of the calibration map
	float commandsIn1Pixel = (float)commandsIn1Gap / (float)gapSizePixels;
	float xRelPicOriginComm = (xRelPicOrigin * commandsIn1Pixel);
	float yRelPicOriginComm = (yRelPicOrigin * commandsIn1Pixel);
	float yRelPicOriginShouldBeComm = (yRelPicOriginShouldBe * commandsIn1Pixel);
	float yRelPicOriginShouldBeCorrectedComm = (yRelPicOriginShouldBeCorrected * commandsIn1Pixel);
	double xRelPicOriginInch = (xRelPicOriginComm / PIXELS_IN_INCH);
	double yRelPicOriginInch = (yRelPicOriginComm / PIXELS_IN_INCH);
	double yRelPicOriginShouldBeInch = (yRelPicOriginShouldBeComm / PIXELS_IN_INCH);

	static int adjustmentsMadeY = 0;
	printf("%d) Y correction at %d,%d. Reposition to %d,%d. Rel %d, %d. In inches %0.2f,%.02f to %.02f\n",
		adjustmentsMadeY, atx, aty, atx, shouldBeY , xRelPicOrigin, yRelPicOrigin, xRelPicOriginInch, yRelPicOriginInch, yRelPicOriginShouldBeInch);
	adjustmentsMadeY++;

	sLineAdjuster.AdjustPositionY((int)xRelPicOriginComm, (int)yRelPicOriginShouldBeComm, (int)yRelPicOriginShouldBeCorrectedComm);
}

void UpdateCallibrationForEveryHorizontalLineEveryCol(FIBITMAP* Img, ShapeStore* ss, int centerStartY, int gapSizePixels, int commandsIn1Gap, int centerX, int centerY)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);

#define LINE_CONTINUATION_SEARCH_RANGE 50
	for (int y = 0; y < Height; y++) // iterate through every horizontal line starting up, going down
	{
		if (!ss[y].HasValues())
		{
			continue;
		}

//		if (ss[y].lineIndex != 15) continue;

				// for every row above the current row
		for (int progressDirection : {-1, 1})
		{
			int prevStartY = ss[y].startY;
			// from middle, go left and right
			for (int x = ss[y].startX; x != 0 && x != Width; x = x + progressDirection)
			{
				// find the startY on this column for this specific line
				int foundLine = 0;
				for (int sr = 0; sr < LINE_CONTINUATION_SEARCH_RANGE; sr++)
				{
					for (int y2 : {prevStartY - sr, prevStartY + sr})
					{
						if (y2 < 0 || y2 >= Height)
						{
							continue;
						}
						short lineIdAt = *(short*)&Bytes[y2 * Stride + x * Bytespp];
						if (lineIdAt == *(short*)&ss[y].lineId)
						{
							prevStartY = y2;
							UpdateCallibrationAtSpecificLocationY(centerX, centerY, x, y2, ss[y].expectedy, gapSizePixels, commandsIn1Gap);
							// debug set expected location
//							if(y2< ss[y].expectedy)	DrawLineColor(Img, x, y2 + 1, x, ss[y].expectedy, 255, 0, 0);
//							else DrawLineColor(Img, x, y2 - 1, x, ss[y].expectedy, 255, 0, 0);
							foundLine = 1;
							goto found_lineHor;
						}
					}
				}
found_lineHor:
				if (foundLine == 0)
				{
					printf("line continuation not found at x=%d y=%d\n", ss[y].startY, x); // happens at every end of line
					break;
				}
			}
		}
	}
}

ShapeStore* GetAllVerticalLinesFromImage(FIBITMAP* Img, int manualMarkedOriginY, int commandCountRef)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	ShapeStore* ss = (ShapeStore*)malloc(Width * Height * sizeof(ShapeStore));
	if (ss == NULL)
	{
		return NULL;
	}
	memset(ss, 0, Width * Height * sizeof(ShapeStore));

	// search all lines on the Y of the center
	int lineCount = 0;
	for (int x = 0; x < Width; x++)
	{
		if (IsCallibrationLinePixel(Img, x, manualMarkedOriginY))
		{
			ss[x] = ExtractShapeAtLoc(Img, x, manualMarkedOriginY, EXPECTED_NORMAL_LINE_GAP, Width, Height);
			if (!ss[x].HasValues())
			{
				printf("Failed to extract shape at col %d\n", x);
			}
			else
			{
				lineCount++;
			}
		}
	}

	printf("Image contained %d vertical lines. Each gap contained %d commands. Total image width in commands :%d\n", lineCount, commandCountRef, (lineCount - 1) * commandCountRef);

	return ss;
}

ShapeStore* GetAllHorizontalLinesFromImage(FIBITMAP* Img, int manualMarkedOriginX, int commandCountRef)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	ShapeStore* ss = (ShapeStore*)malloc(Width * Height * sizeof(ShapeStore));
	if (ss == NULL)
	{
		return NULL;
	}
	memset(ss, 0, Width * Height * sizeof(ShapeStore));

	// search all lines on the Y of the center
	int lineCount = 0;
	for (int y = 0; y < Height; y++)
	{
		if (IsCallibrationLinePixel(Img, manualMarkedOriginX, y))
		{
			ss[y] = ExtractShapeAtLoc(Img, manualMarkedOriginX, y, EXPECTED_NORMAL_LINE_GAP, Width, Height);
			if (!ss[y].HasValues())
			{
				printf("Failed to extract shape at row %d\n", y);
			}
			else
			{
				lineCount++;
			}
		}
	}

	printf("Image contained %d horizontal lines. Each gap contained %d commands. Total image height in commands :%d\n", lineCount, commandCountRef, (lineCount - 1) * commandCountRef);

	return ss;
}

void LoadSpecificFull_V_LineCallibrationImage(const char* fileName, int isInitial, int commandCountRef)
{
	FIBITMAP* Img = LoadImage_(fileName);
	if (Img == NULL)
	{
		printf("Missing callibration image %s\n", fileName);
		return;
	}
	int bpp = FreeImage_GetBPP(Img);
	if (bpp != 24)
	{
		printf("Expecting 24 bpp image %s\n", fileName);
		return;
	}
	int Width = FreeImage_GetWidth(Img);

	int manualMarkedOriginX = 0;
	int manualMarkedOriginY = 0;
	FindManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	if (manualMarkedOriginX == 0 && manualMarkedOriginY == 0)
	{
		printf("Could not find manually marked origin in callibration image %s\n", fileName);
		return;
	}
	printf("Origin is at %d,%d for image %s\n", manualMarkedOriginX, manualMarkedOriginY, fileName);

	// delete this marked origin
	RemoveManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	ShapeStore* ss = GetAllVerticalLinesFromImage(Img, manualMarkedOriginY, commandCountRef);

	// estimate normal gap size
	int avgGapSize = 0;
	int centerStartX = 0;
	GetGapSizePixelsAtOrigin_V(Width, ss, manualMarkedOriginX, manualMarkedOriginY, avgGapSize, centerStartX);

	// for every column, update where we expect the line to be located
	SetExpectedXForVerticalLinesAtY0(Width, ss, avgGapSize, centerStartX);

	// follow up every shape and at every row, calculate how far off are we from expected
	int commandsIn1Gap = commandCountRef * 2;
	UpdateCallibrationForEveryVerticalLineEveryRow(Img, ss, centerStartX, avgGapSize, commandsIn1Gap, manualMarkedOriginX, manualMarkedOriginY);

	free(ss);
	SaveImagePNG(Img, "tv.png");
}

void LoadSpecificFull_H_LineCallibrationImage(const char* fileName, int isInitial, int commandCountRef)
{
	FIBITMAP* Img = LoadImage_(fileName);
	if (Img == NULL)
	{
		printf("Missing callibration image %s\n", fileName);
		return;
	}
	int bpp = FreeImage_GetBPP(Img);
	if (bpp != 24)
	{
		printf("Expecting 24 bpp image %s\n", fileName);
		return;
	}
	int Height = FreeImage_GetHeight(Img);

	int manualMarkedOriginX = 0;
	int manualMarkedOriginY = 0;
	FindManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	if (manualMarkedOriginX == 0 && manualMarkedOriginY == 0)
	{
		printf("Could not find manually marked origin in callibration image %s\n", fileName);
		return;
	}
	printf("Origin is at %d,%d for image %s\n", manualMarkedOriginX, manualMarkedOriginY, fileName);

	// delete this marked origin
	RemoveManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	ShapeStore* ss = GetAllHorizontalLinesFromImage(Img, manualMarkedOriginX, commandCountRef);

	// estimate normal gap size
	int avgGapSize = 0;
	int centerStartY = 0;
	GetGapSizePixelsAtOrigin_H(Height, ss, manualMarkedOriginX, manualMarkedOriginY, avgGapSize, centerStartY);

	// for every column, update where we expect the line to be located
	SetExpectedYForHorizontalLinesAtX0(Height, ss, avgGapSize, centerStartY);

	// follow up every shape and at every row, calculate how far off are we from expected
	int commandsIn1Gap = commandCountRef * 2;
	UpdateCallibrationForEveryHorizontalLineEveryCol(Img, ss, centerStartY, avgGapSize, commandsIn1Gap, manualMarkedOriginX, manualMarkedOriginY);

	free(ss);
	SaveImagePNG(Img, "th.png");
}

void Test_loadCallibrationImages()
{
//	LoadSpecificFull_V_LineCallibrationImage("../ver_60_25_FL_03_23_stretched.bmp", 1, 25);
	LoadSpecificFull_V_LineCallibrationImage("../ver_60_25_FL_03_23.bmp", 1, 25);
	LoadSpecificFull_H_LineCallibrationImage("../hor_60_25_FL_03_23.bmp", 1, 25);
	//	LoadSpecificFull_V_LineCallibrationImage("../ver_30_50_FL_03_17.bmp", 1, 50);
//	LoadSpecificFull_H_LineCallibrationImage("../hor_30_50_FL_03_17.bmp", 1, 50);
//	LoadSpecificCallibrationImage("../ver_30_50_03_17.bmp", 1, 50);
//	LoadSpecificCallibrationImage("../hor_15_03_16.bmp", 1, 100);
}

