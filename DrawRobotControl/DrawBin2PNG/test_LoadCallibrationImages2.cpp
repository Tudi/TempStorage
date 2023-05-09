#include "StdAfx.h"

#define ORIGIN_R 237
#define ORIGIN_G 28
#define ORIGIN_B 36
#define EXPECTED_NORMAL_LINE_GAP 4 // counted in pixels
#define INTERSECTION_SIZE 6 // 25 pixels distance, but 4+4 pixel line width
#define MAX_LINE_WIDTH 4
#define MAX_INTERSECTION_DISTANCE 25 // stop searching for next intersection

#define VertColor ((252)|(110<<8)|(107<<16))
#define HorColor ((7)|(251<<8)|(1<<16))
#define IntersectionColor ((255)|(0<<8)|(255<<16))

static void FindManuallyMarkedOrigin(FIBITMAP* Img, int& manualMarkedOriginX, int& manualMarkedOriginY)
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

static void RemoveManuallyMarkedOrigin(FIBITMAP* Img, int manualMarkedOriginX, int manualMarkedOriginY)
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

typedef struct IntersectionData
{
	int picX, picY;
	int x, y; // relativ to our origin
}IntersectionData;

int queuedICount = 0;
IntersectionData queuedI[100000];
int ICount = 0;
IntersectionData intersections[100000];
FIBITMAP* Img = NULL;
int width;
int height;
int stride;
BYTE* bytes;
int manualMarkedOriginX = 0;
int manualMarkedOriginY = 0;
#define InterSectionsOrigin 60
IntersectionData intersectionsMat[100][100];
#define lineToLineDistance 17 // based on the origin left/right/up/down distance estimate the scaling of the scanned image. This is measured in pixels
#define expectedLineToLineDistance 50 // scale the whole image up so that lineToLineDistance becomes expectedLineToLineDistance. Measured in movement units

#define DELETE_PIXEL(sx,sy) { bytes[(sy) * stride + (sx) * 3 + 0] = 255; bytes[(sy) * stride + (sx) * 3 + 1] = 255; bytes[(sy) * stride + (sx) * 3 + 2] = 255;}
#define SetPixelColor(sx,sy,r,g,b) { bytes[(sy) * stride + (sx) * 3 + 0] = b; bytes[(sy) * stride + (sx) * 3 + 1] = g; bytes[(sy) * stride + (sx) * 3 + 2] = r;}
#define ColorMatch(a,b) (((a)&0x00FFFFFF)==((b)&0x00FFFFFF))
#define SNAP_TO_LINE_DIST 5

// need to find both vertical and horizontal line nearby
int IsIntersectionNearby(int sx, int sy, int scanRadius = 3)
{
	int minY = MAX(sy - scanRadius, 0), maxY = MIN(sy + scanRadius, height - 1);
	int minX = MAX(sx - scanRadius, 0), maxX = MIN(sx + scanRadius, width - 1);
	int foundHorLine = 0, foundVerLine = 0;
	for (int y = minY; y <= maxY; y++)
	{
		for (int x = minX; x <= maxX; x++)
		{
			// skip counting distance if pixel is white
			if (ColorMatch(*(int*)&bytes[y * stride + x * 3], VertColor))
			{
				foundVerLine = 1;
			}
			if (ColorMatch(*(int*)&bytes[y * stride + x * 3], HorColor))
			{
				foundHorLine = 1;
			}
		}
	}
	return (foundHorLine == 1) && (foundVerLine == 1);
}

void ScanIntersection3(int sx, int sy, int xdir, int ydir, int& xFound, int& yFound)
{
	xFound = yFound = -1;
	int stepsMade = 0;
	int nextsx = sx;
	int nextsy = sy;
	int walkLineColor;
	int stepColor = 255;
	if (xdir != 0)
	{
		walkLineColor = HorColor;
	}
	else
	{
		walkLineColor = VertColor;
	}

	while (stepsMade < MAX_INTERSECTION_DISTANCE)
	{
		// check for out of bounds
		if (sx + xdir < 0 || sx + xdir >= width)
		{
			return;
		}
		if (sy + ydir < 0 || sy + ydir >= height)
		{
			return;
		}

		// delete our track so that we do not detect this same intersection more than once
//		DELETE_PIXEL(sx, sy);

		// check if this is an intersection
//		if (stepsMade >= INTERSECTION_SIZE)
		{
			size_t minY = MAX(sy - MAX_LINE_WIDTH, 0), maxY = MIN(sy + MAX_LINE_WIDTH, height - 1);
			size_t minX = MAX(sx - MAX_LINE_WIDTH, 0), maxX = MIN(sx + MAX_LINE_WIDTH, width - 1);
			for (size_t y2 = minY; y2 <= maxY; y2++)
			{
				for (size_t x2 = minX; x2 <= maxX; x2++)
				{
					if (ColorMatch(*(int*)&bytes[y2 * stride + x2 * 3], IntersectionColor))
					{
#if 0
						size_t minY2 = MAX(y2 - MAX_LINE_WIDTH, 0), maxY2 = MIN(y2 + MAX_LINE_WIDTH, height - 1);
						size_t minX2 = MAX(x2 - MAX_LINE_WIDTH, 0), maxX2 = MIN(x2 + MAX_LINE_WIDTH, width - 1);
						for (size_t y3 = minY2; y3 <= maxY2; y3++)
						{
							for (size_t x3 = minX2; x3 <= maxX2; x3++)
							{
								SetPixelColor(x3, y3, 255, 255, 255);
							}
						}
#endif
						SetPixelColor(x2, y2, 0, 0, 255);
						xFound = (int)x2;
						yFound = (int)y2;
						return;
					}
				}
			}
		}
		stepsMade++;

		// move "forward"
		int nextsx = sx + xdir;
		int nextsy = sy + ydir;

		// try to walk the line that connects 2 intersections
		if (!ColorMatch(*(int*)&bytes[nextsy * stride + nextsx * 3], walkLineColor))
		{
			if (xdir != 0)
			{
				for (int radius = 0; radius < SNAP_TO_LINE_DIST; radius++)
				{
					if (nextsy + radius >= 0 && nextsy + radius < height && ColorMatch(*(int*)&bytes[(nextsy + radius) * stride + nextsx * 3], walkLineColor))
					{
						nextsy = nextsy + radius;
						break;
					}
					if (nextsy - radius >= 0 && nextsy - radius < height && ColorMatch(*(int*)&bytes[(nextsy - radius) * stride + nextsx * 3], walkLineColor))
					{
						nextsy = nextsy - radius;
						break;
					}
				}
			}
			if (ydir != 0)
			{
				for (int radius = 0; radius < SNAP_TO_LINE_DIST; radius++)
				{
					if (nextsx + radius >= 0 && nextsx + radius < width && ColorMatch(*(int*)&bytes[nextsy * stride + (nextsx + radius) * 3], walkLineColor))
					{
						nextsx = nextsx + radius;
						break;
					}
					if (nextsx - radius >= 0 && nextsx - radius < width && ColorMatch(*(int*)&bytes[nextsy * stride + (nextsx - radius) * 3], walkLineColor))
					{
						nextsx = nextsx - radius;
						break;
					}
				}
			}
		}
		sx = nextsx;
		sy = nextsy;
//		SetPixelColor(sx, sy, stepColor, 0, 0); stepColor -= 15;
	}
}

int CheckScanIntersection2(IntersectionData* si, int xdir, int ydir)
{
	// can't add more intersections
	if (ICount >= _countof(intersections))
	{
		printf("Found more intersections then we can store. Unexpected\n");
		return -1;
	}
	int xExpected = si->x + xdir;
	int yExpected = si->y + ydir;
	// check if this position has already been scanned
	for (size_t i = 0; i < ICount; i++)
	{
		if (intersections[i].x == xExpected && intersections[i].y == yExpected)
		{
			// position has already been scanned. Nothing to do
			return -1;
		}
	}
	int xFound, yFound;
	ScanIntersection3(si->picX, si->picY, xdir, ydir, xFound, yFound);
	if (xFound > 0)
	{
		intersections[ICount].picX = xFound;
		intersections[ICount].picY = yFound;
		intersections[ICount].x = xExpected;
		intersections[ICount].y = yExpected;
		ICount++;
	}

	return 0;
}

void ScanNeighbourIntersections(int index)
{
	// go to left, right, up, down and try to find 4 intersections
	while( index < ICount && CheckScanIntersection2(&intersections[index], -1, 0) == 0)
		index++;
	while (index < ICount && CheckScanIntersection2(&intersections[index], 1, 0) == 0)
		index++;
	while (index < ICount && CheckScanIntersection2(&intersections[index], 0, -1) == 0)
		index++;
	while (index < ICount && CheckScanIntersection2(&intersections[index], 0, 1) == 0)
		index++;
}

void MarkAllIntersections()
{
	BYTE* outb = (BYTE * )malloc(height * stride);
	if (outb == NULL)
	{
		return;
	}
	memset(outb, 0, height * stride);
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			if (IsIntersectionNearby((int)x, (int)y, 1) == 0)
			{
				continue;
			}
			size_t sy = y;
			size_t sx = x;
			outb[sy * stride + sx * 3 + 0] = 255;
			outb[sy * stride + sx * 3 + 1] = 0;
			outb[sy * stride + sx * 3 + 2] = 0;
		}
	}
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			if (!ColorMatch(*(int*)&outb[y * stride + x * 3], 255))
			{
				continue;
			}
			int count = 0;
			size_t sy = 0;
			size_t sx = 0;
			size_t minY = MAX(y - INTERSECTION_SIZE, 0), maxY = MIN(y + INTERSECTION_SIZE, height - 1);
			size_t minX = MAX(x - INTERSECTION_SIZE, 0), maxX = MIN(x + INTERSECTION_SIZE, width - 1);
			for (size_t y2 = minY; y2 <= maxY; y2++)
			{
				for (size_t x2 = minX; x2 <= maxX; x2++)
				{
					if (ColorMatch(*(int*)&outb[y2 * stride + x2 * 3], 255))
					{
						outb[y2 * stride + x2 * 3 + 0] = 0;
						sy += y2;
						sx += x2;
						count++;
					}
				}
			}
			sy = sy / count;
			sx = sx / count;
			SetPixelColor(sx, sy, 255, 0, 255);
		}
	}

	free(outb);
}

void DrawMotionVectors()
{
	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		int xFound = intersections[i].picX;
		int yFound = intersections[i].picY;
//		if (abs(xExpected) == 15 || abs(yExpected) == 15)
		{
			DrawLineColor(Img, (float)(manualMarkedOriginX + xExpected * lineToLineDistance), (float)(manualMarkedOriginY + yExpected * lineToLineDistance), (float)xFound, (float)yFound, 255, 0, 0);
		}
	}
}


void PrintMotionVectors()
{
	char outx[100000];
	char outy[100000];
	char outz1[100000];
	char outz2[100000];
	size_t bwx = 0, bwy = 0, bwz1 = 0, bwz2 = 0;
	bwx += sprintf_s(outx + bwx, sizeof(outx) - bwx, "xData = numpy.array([");
	bwy += sprintf_s(outy + bwy, sizeof(outy) - bwy, "yData = numpy.array([");
	bwz1 += sprintf_s(outz1 + bwz1, sizeof(outz1) - bwz1, "zData = numpy.array([");
	bwz2 += sprintf_s(outz2 + bwz2, sizeof(outz2) - bwz2, "zData = numpy.array([");

	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		double sx = xExpected * 17;
		double sy = yExpected * 17;
		//		if (!(sx >= 0 && sy >= 0))
		//		if (!(sx <= 0 && sy >= 0))
		//		if (!(sx <= 0 && sy <= 0))
		if (!(sx >= 0 && sy <= 0))
		{
			//			continue;
		}
		double ex = intersections[i].picX - manualMarkedOriginX;
		double ey = intersections[i].picY - manualMarkedOriginY;
		// scale to normalized size. We intended to draw lines with distance 25*2, but the scanned image contained 17 pixels
		// the scanned image is almost 3 times smaller than we intended to have
		sx = sx * 50.0 / 17.0;
		sy = sy * 50.0 / 17.0;
		ex = ex * 50.0 / 17.0;
		ey = ey * 50.0 / 17.0;
		// we need to flip the position of the destination relativ to the position where we intended it to be
		// required to obtain a compensation value instead of the obtained value
		double dx = ex - sx;
		double dy = ey - sy;
		ex = sx - dx;
		ey = sy - dy;

		bwx += sprintf_s(outx + bwx, sizeof(outx) - bwx, "%.01f,", sx);
		bwy += sprintf_s(outy + bwy, sizeof(outy) - bwy, "%.01f,", sy);
		bwz1 += sprintf_s(outz1 + bwz1, sizeof(outz1) - bwz1, "%.05f,", ex);
		bwz2 += sprintf_s(outz2 + bwz2, sizeof(outz2) - bwz2, "%.05f,", ey);
	}
	bwx += sprintf_s(outx + bwx - 1, sizeof(outx) - bwx, "])");
	bwy += sprintf_s(outy + bwy - 1, sizeof(outy) - bwy, "])");
	bwz1 += sprintf_s(outz1 + bwz1 - 1, sizeof(outz1) - bwz1, "])");
	bwz2 += sprintf_s(outz2 + bwz2 - 1, sizeof(outz2) - bwz2, "])");

	printf("%s\n", outx);
	printf("%s\n", outy);
	printf("%s\n", outz1);
	printf("%s\n", outz2);
}

void UpdateAdjustmentMap()
{
	// build a calibration map out of the obtained data
	PositionAdjustInfoHeader2 paih;
	paih.scaleX = 1.0f / (float)expectedLineToLineDistance;
	paih.scaleY = 1.0f / (float)expectedLineToLineDistance;
	paih.width = InterSectionsOrigin * 2;
	paih.height = InterSectionsOrigin * 2;
	paih.originX = paih.width / 2;
	paih.originY = paih.height / 2;
	sLineAdjuster2.CreateNewMap(&paih);
#define IMG_SIZE 4000
	FIBITMAP* dib = CreateNewImage(IMG_SIZE, IMG_SIZE);
	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		double sx = xExpected * lineToLineDistance;
		double sy = yExpected * lineToLineDistance;
		double ex = intersections[i].picX - manualMarkedOriginX;
		double ey = intersections[i].picY - manualMarkedOriginY;
		// scale to normalized size. We intended to draw lines with distance 25*2, but the scanned image contained 17 pixels
		// the scanned image is almost 3 times smaller than we intended to have
		sx = sx * expectedLineToLineDistance / lineToLineDistance;
		sy = sy * expectedLineToLineDistance / lineToLineDistance;
		ex = ex * expectedLineToLineDistance / lineToLineDistance;
		ey = ey * expectedLineToLineDistance / lineToLineDistance;
		// we need to flip the position of the destination relativ to the position where we intended it to be
		// required to obtain a compensation value instead of the obtained value
		double dx = ex - sx;
		double dy = ey - sy;
		ex = sx - dx;
		ey = sy - dy;
		// 
		sLineAdjuster2.AdjustPosition((int)sx, (int)sy, ex, ey);
		DrawLineColorFade(dib, (float)sx + IMG_SIZE / 2, (float)sy + IMG_SIZE / 2, (float)ex + IMG_SIZE / 2, (float)ey + IMG_SIZE / 2, 255, 255, 255);
	}
	BYTE* bytes = FreeImage_GetBits(dib);
	int stride = FreeImage_GetPitch(dib);
	for (size_t y = IMG_SIZE / 2 - 5; y < IMG_SIZE / 2 + 5; y++)
	{
		for (size_t x = IMG_SIZE / 2 - 5; x < IMG_SIZE / 2 + 5; x++)
		{
			bytes[(y)*stride + (x) * 3 + 0] = 0; 
			bytes[(y)*stride + (x) * 3 + 1] = 0; 
			bytes[(y)*stride + (x) * 3 + 2] = 255; 
		}
	}
	SaveImagePNG(dib, "CorrectionsVisualized.png");
	FreeImage_Unload(dib);
}

static void LoadSpecificCallibrationImage(const char* fileName, int isInitial, int commandCountRef)
{
	Img = LoadImage_(fileName);
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
	width = FreeImage_GetWidth(Img);
	height = FreeImage_GetHeight(Img);
	stride = FreeImage_GetPitch(Img);
	bytes = FreeImage_GetBits(Img);

	FindManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	if (manualMarkedOriginX == 0 && manualMarkedOriginY == 0)
	{
		printf("Could not find manually marked origin in callibration image %s\n", fileName);
		return;
	}
	printf("Origin is at %d,%d for image %s\n", manualMarkedOriginX, manualMarkedOriginY, fileName);

	// delete this marked origin
	RemoveManuallyMarkedOrigin(Img, manualMarkedOriginX, manualMarkedOriginY);

	// so we can jump to these locations
	MarkAllIntersections();

	// remove intersection at 0,0 to avoid finding it multiple times
	for (int y = manualMarkedOriginY - 4; y <= manualMarkedOriginY + 4; y++)
		for (int x = manualMarkedOriginX - 4; x <= manualMarkedOriginX + 4; x++)
			SetPixelColor(x, y, 0, 0, 255);

	intersections[0].picX = manualMarkedOriginX;
	intersections[0].picY = manualMarkedOriginY;
	intersections[0].x = 0;
	intersections[0].y = 0;
	ICount = 1;
	int readIndex = 0;
	while (readIndex < ICount)
	{
		ScanNeighbourIntersections(readIndex);
		readIndex++;
	}
	printf("Found %d intersections \n", ICount);

#if 1
	// some formatting and sanity checks
	memset(intersectionsMat, 0, sizeof(intersectionsMat));
	int minX = 10000, maxX = -10000, minY = 10000, maxY = -10000;
	for (size_t i = 0; i < ICount; i++)
	{
		int x = intersections[i].x + InterSectionsOrigin;
		int y = intersections[i].y + InterSectionsOrigin;
		if (x < 0 || x > 100)
		{
			printf("Mat is not large enough\n");
			continue;
		}
		if (y < 0 || y > 100)
		{
			printf("Mat is not large enough\n");
			continue;
		}
		if (intersectionsMat[y][x].picX != 0)
		{
			printf("Intersection %d,%d was detected more than once\n", y, x);
		}
		intersectionsMat[y][x] = intersections[i];
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
	}
	for (size_t y = 0; y < 100-3; y++)
	{
		for (size_t x = 0; x < 100-3; x++)
		{
			if (intersectionsMat[y][x].picX != 0 && intersectionsMat[y][x + 1].picX == 0 && intersectionsMat[y][x + 2].picX != 0)
			{
				printf("Gap in row %zd %zd detected\n", y, x);
			}
			if (intersectionsMat[y][x].picX != 0 && intersectionsMat[y + 1][x].picX == 0 && intersectionsMat[y + 2][x].picX != 0)
			{
				printf("Gap in col %zd %zd detected\n", y, x);
			}
		}
	}
	printf("minX=%d,maxX=%d,minY=%d,maxY=%d\n", minX, maxX, minY, maxY);
#endif
	// debugging visually
//	DrawMotionVectors();
//	PrintMotionVectors();
	UpdateAdjustmentMap();
	sLineAdjuster2.FillMissingInfo();

	SaveImagePNG(Img, "tv.png");

}

void Test_loadCallibrationImages2()
{
	LoadSpecificCallibrationImage("./stretch_maps/horver2.bmp", 1, 25);
}
