#include "StdAfx.h"

#define ORIGIN_R 237
#define ORIGIN_G 28
#define ORIGIN_B 36
#define EXPECTED_NORMAL_LINE_GAP 4 // counted in pixels

static signed short LineExtractUsedID = 1;

typedef struct ShapeStore
{
	int startX, starty;
	int minX, maxX, minY, maxY;
	int centerx, centery;
	int expectedx, expectedy;
	int lineId;
}ShapeStore;

typedef struct picLoc
{
	int x, y;
}picLoc;


int IsLinePixel(FIBITMAP* Img, int x, int y)
{
	if (x < 0 || y < 0)
	{
		return 0;
	}
	int Width = FreeImage_GetWidth(Img);
	if (x >= Width)
	{
		return 0;
	}
	int Height = FreeImage_GetHeight(Img);
	if (y >= Height)
	{
		return 0;
	}
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	BYTE* bNow = &Bytes[(y)*Stride + (x)*Bytespp + 0];
	if (bNow[0] == 0 && bNow[1] == 0 && bNow[2] == 0)
	{
		return 1;
	}
	return 0;
}

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
			if (IsLinePixel(Img, x + x2, y + y2))
			{
				atx = x + x2;
				aty = y + y2;
				return 1;
			}
		}
	}

	return 0;
}

ShapeStore ExtractShapeAtLoc(FIBITMAP* Img, int x, int y, int jumpGap, int searchRadiusX, int searchRadiusY)
{
	ShapeStore ss;
	ss.startX = x;
	ss.starty = y;
	ss.minX = x;
	ss.maxX = x;
	ss.minY = y;
	ss.maxY = y;
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	if (x <= 2 || x >= Width - 2)
	{
		return ss;
	}
	if (y <= 2 || y >= Height - 2)
	{
		return ss;
	}
	if (!IsLinePixel(Img, x, y))
	{
		return ss;
	}

	// mark starting position
	short lineId = LineExtractUsedID++;
	*(short*)(&Bytes[y * Stride + x * Bytespp]) = lineId;

	// search nearby as long as we find a pixel
	int foundShapeAtRadius = 0;

	int prevLocsFound = 1;
	int curLocsFound = 0;
	picLoc* prevLocs = (picLoc * )malloc(Width * Height * sizeof(picLoc));
	picLoc* curLocs = (picLoc*)malloc(Width * Height * sizeof(picLoc));
	if (prevLocs == NULL || curLocs == NULL)
	{
		return ss;
	}
	prevLocs[0].x = x;
	prevLocs[0].y = y;

	while (prevLocsFound > 0)
	{
		curLocsFound = 0;
		for (int ind = 0; ind < prevLocsFound; ind++)
		{
//			int atx, aty;
//			if (CheckLinePresentNearby(Img, prevLocs[ind].x, prevLocs[ind].y, searchRadius, lineId, atx, aty))
			for (int y2 = -jumpGap; y2 <= jumpGap; y2++)
			{
				for (int x2 = -jumpGap; x2 <= jumpGap; x2++)
				{
					if (y2 == 0 && x2 == 0)
					{
						continue;
					}
					int atx = prevLocs[ind].x + x2;
					int aty = prevLocs[ind].y + y2;
					// too far from search center ?
					if (atx < x - searchRadiusX || x + searchRadiusX < atx)
					{
						continue;
					}
					if (aty < y - searchRadiusY || y + searchRadiusY < aty)
					{
						continue;
					}
					// black pixel means there is a line pixel
		//			if(*(short*)&Bytes[(y + y2) * Stride + (x + x2) * Bytespp] == lineId)
					if (IsLinePixel(Img, atx, aty))
					{
						curLocs[curLocsFound].x = atx;
						curLocs[curLocsFound].y = aty;
						if (curLocsFound < Width * Height)
						{
							curLocsFound++;
						}
						*(short*)(&Bytes[aty * Stride + atx * Bytespp]) = lineId; // mark this location so that we do not find it next time
						ss.minX = MIN(ss.minX, atx);
						ss.maxX = MAX(ss.maxX, atx);
						ss.minY = MIN(ss.minY, aty);
						ss.maxY = MAX(ss.maxY, aty);
					}
				}
			}
		}
		picLoc* t = prevLocs;
		prevLocs = curLocs;
		curLocs = t;
		prevLocsFound = curLocsFound;
	}

	free(prevLocs);
	free(curLocs);

	ss.centerx = (ss.minX + ss.maxX) / 2;
	ss.centery = (ss.minY + ss.maxY) / 2;
	ss.lineId = lineId;

	return ss;
}

// Find up to 8 neighbour shapes around the center shape
// expecting neighbour shapes to be similar to center shape ( size )
void FindNeighbourShapes(FIBITMAP* Img, ShapeStore *center, ShapeStore out_neighbours[3][3], int isFirst)
{
	typedef struct ShapeSearchParams
	{
		int expectedX, expectedY;
		int startX, startY;
		int storeX, storeY;
	}ShapeSearchParams;

	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	int shapeWidth = center->maxX - center->minX;
	int shapeHeight = center->maxY - center->minY;
	int shapeWidthMin = shapeWidth * 80 / 100;
	int shapeWidthMax = shapeWidth * 120 / 100;
	int shapeHeightMin = shapeHeight * 80 / 100;
	int shapeHeightMax = shapeHeight * 120 / 100;
	int shapeGapSize = MAX(shapeWidth, shapeHeight); // expecting to find this gap between shapes
	int shapeSizeSearchJumpX = shapeGapSize * 2;
	int shapeSizeSearchJumpY = shapeGapSize * 2;

	ShapeSearchParams sp[8] = {
		{ center->centerx - shapeSizeSearchJumpX , center->centery, 0, 0, -1, 0}, // left to center
		{ center->centerx + shapeSizeSearchJumpX, center->centery, 0, 0, 1, 0}, // right to center
		{ center->centerx, center->centery - shapeSizeSearchJumpY, 0, 0, 0, -1}, // up to center
		{ center->centerx, center->centery + shapeSizeSearchJumpY, 0, 0, 0, 1}, // down to center

		{ center->centerx - shapeSizeSearchJumpX, center->centery - shapeSizeSearchJumpY, 0, 0, -1, -1}, // up - left to center
		{ center->centerx + shapeSizeSearchJumpX, center->centery - shapeSizeSearchJumpY, 0, 0, 1, -1}, // up - right to center
		{ center->centerx - shapeSizeSearchJumpX, center->centery + shapeSizeSearchJumpY, 0, 0, -1, 1}, // down - left to center
		{ center->centerx + shapeSizeSearchJumpX, center->centery + shapeSizeSearchJumpY, 0, 0, 1, 1}, // down - right to center
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
				if (IsLinePixel(Img, x, y))
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
		if (isFirst == 0)
		{
			if (curShapeWidth < shapeHeightMin || curShapeWidth > shapeHeightMax)
			{
				shapeSizeUnexpected = 1;
			}
			if (scurSapeHeight < shapeHeightMin || scurSapeHeight > shapeHeightMax)
			{
				shapeSizeUnexpected = 1;
			}
		}
		if (shapeSizeUnexpected == 0)
		{
			ssNow.expectedx = sp[spInd].expectedX;
			ssNow.expectedy = sp[spInd].expectedY;
			printf("next shape width=%d, height=%d, x=%d-%d, y=%d-%d, x=%d y=%d, xs=%d ys=%d, xexp=%d yexp=%d, xdiffexpected=%d ydiffexpected=%d\n", ssNow.maxX - ssNow.minX, ssNow.maxY - ssNow.minY,
				ssNow.minX, ssNow.maxX, ssNow.minY, ssNow.maxY, ssNow.centerx, ssNow.centery, ssNow.startX, ssNow.starty,
				ssNow.expectedx, ssNow.expectedy, ssNow.centerx - ssNow.expectedx, ssNow.centery - ssNow.expectedy);
			out_neighbours[1 + sp[spInd].storeX][1 + sp[spInd].storeY] = ssNow;
		}
	}
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
	{
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

	ShapeStore neighbours[3][3];
	FindNeighbourShapes(Img, &ssCenter, neighbours, 1);

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

//	SaveImagePNG(Img, "t.png");
}

void Test_loadCallibrationImages()
{
	LoadSpecificCallibrationImage("../ver_30_50_03_17.bmp", 1, 50);
	//	LoadSpecificCallibrationImage("../hor_15_03_16.bmp", 1, 100);
}

