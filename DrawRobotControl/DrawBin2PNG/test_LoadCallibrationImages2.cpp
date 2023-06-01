#include "StdAfx.h"

#define ORIGIN_R 237
#define ORIGIN_G 28
#define ORIGIN_B 36
#define EXPECTED_NORMAL_LINE_GAP 4 // counted in pixels
#define INTERSECTION_SIZE 7 // 25 pixels distance, but 4+4 pixel line width
#define lineToLineDistance1 50 // based on the origin left/right/up/down distance estimate the scaling of the scanned image. This is measured in pixels
#define lineToLineDistance2 36 // based on the origin left/right/up/down distance estimate the scaling of the scanned image. This is measured in pixels
#define expectedLineToLineDistance 50 // scale the whole image up so that lineToLineDistance becomes expectedLineToLineDistance. Measured in movement units
#define MAX_LINE_WIDTH 4
#define MAX_INTERSECTION_DISTANCE (lineToLineDistanceX*7/4) // stop searching for next intersection

#define VertColor ((252)|(110<<8)|(107<<16))
#define HorColor ((7)|(251<<8)|(1<<16))
#define IntersectionColor ((255)|(0<<8)|(255<<16))

#define IMG_SIZE_DEBUG_VECTORS 6000

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
				ShapeStore ss = ExtractShapeAtLoc2(Img, x, y, 1, 10, 10);
				manualMarkedOriginX = (int)ss.centerx2;
				manualMarkedOriginY = (int)ss.centery2;
				SetShapeColor(&ss, Img, ORIGIN_B, ORIGIN_G, ORIGIN_R);
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
	byte scannedDirections[3][3]; // from every position, we can branch out to 4 locations
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
#define InterSectionsOrigin 80
IntersectionData intersectionsMat[InterSectionsOrigin * 2][InterSectionsOrigin * 2];
double lineToLineDistanceX = 0;
double lineToLineDistanceY = 0;
#define DELETE_PIXEL(sx,sy) { bytes[(sy) * stride + (sx) * 3 + 0] = 255; bytes[(sy) * stride + (sx) * 3 + 1] = 255; bytes[(sy) * stride + (sx) * 3 + 2] = 255;}
#define SetPixelColor_(sx,sy,r,g,b) { bytes[(sy) * stride + (sx) * 3 + 0] = b; bytes[(sy) * stride + (sx) * 3 + 1] = g; bytes[(sy) * stride + (sx) * 3 + 2] = r;}
#define SNAP_TO_LINE_DIST 5

// need to find both vertical and horizontal line nearby
int IsIntersectionNearby(int sx, int sy, int scanRadius = 1)
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
								SetPixelColor_(x3, y3, 255, 255, 255);
							}
						}
#endif
						SetPixelColor_(x2, y2, 0, 0, 255);
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
//		SetPixelColor_(sx, sy, stepColor, 0, 0); stepColor -= 15;
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
	// we already searched this direction for this location
	if (si->scannedDirections[1 + xdir][1 + ydir] != 0)
	{
		return -1;
	}
	si->scannedDirections[1 + xdir][1 + ydir] = 1;

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

#if 0
void ScanNeighbourIntersections(int index)
{
	// go to left, right, up, down and try to find 4 intersections
	int index2;

	index2 = index;
	while(index2 < ICount && CheckScanIntersection2(&intersections[index2], -1, 0) == 0)
		index2++;
	index2 = index;
	while (index2 < ICount && CheckScanIntersection2(&intersections[index2], 1, 0) == 0)
		index2++;
	index2 = index;
	while (index2 < ICount && CheckScanIntersection2(&intersections[index2], 0, -1) == 0)
		index2++;
	index2 = index;
	while (index2 < ICount && CheckScanIntersection2(&intersections[index2], 0, 1) == 0)
		index2++;
}
#endif
void ScanNeighbourIntersections(int index)
{
	// go to left, right, up, down and try to find 4 intersections
	int index2;

	index2 = index;
	while (index2 < ICount)
	{
		CheckScanIntersection2(&intersections[index2], -1, 0);
		CheckScanIntersection2(&intersections[index2], 1, 0);
		CheckScanIntersection2(&intersections[index2], 0, -1);
		CheckScanIntersection2(&intersections[index2], 0, 1);
		index2++;
	}
}


void MarkAllIntersections()
{
	BYTE* outb = (BYTE * )malloc(height * stride);
	if (outb == NULL)
	{
		return;
	}
	memset(outb, 0, height * stride);

	// mark locations that are nearby an intersection
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

	double bestDistancesX[4] = { 10000, 10000, 10000, 10000 };
	double bestDistancesY[4] = { 10000, 10000, 10000, 10000 };

	// group up nearby intersections and calculate the average location for them
	int markCount = 0;
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			// search for the next pixel that is part of an intersection
			if (!ColorMatch(*(int*)&outb[y * stride + x * 3], 255))
			{
				continue;
			}
			ShapeStore ss = ExtractShapeAtLoc(outb, stride, width, height, (int)x, (int)y, 3, INTERSECTION_SIZE, INTERSECTION_SIZE);
#if 0
			// group nearby intersection pixels into a single intersection blob
			// at this point we know that in the previous line there was no intersection, se we only need to scan the next lines
			int count = 0;
			size_t sy = 0;
			size_t sx = 0;
//			size_t minY = MAX(y - INTERSECTION_SIZE, 0), maxY = MIN(y + INTERSECTION_SIZE, height - 1);
			size_t minY = y, maxY = MIN(y + INTERSECTION_SIZE * 2, height - 1);
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
//SetPixelColor_(x2, y2, 255, 0, 255);
					}
				}
			}
			// the average position of this intersection is ...
			sy = sy / count;
			sx = sx / count;
#else
			int count = ss.pixelsFound;
			int sx = ss.centerx2;
			int sy = ss.centery2;
#endif
			// we are expecting at least 2-9 pixels for an intersection
			if (count > 4)
			{
				// get the distance to the center
				double dx = sx - manualMarkedOriginX;
				double dy = sy - manualMarkedOriginY;
				double dist = sqrt(dx * dx + dy * dy);
				//is this a new "best" distance ?
				if (dist > 4 
					// don't inspect values on diagonals
					&& (abs(dx) < lineToLineDistanceX /2 || abs(dy) < lineToLineDistanceY / 2))
				{
					double *bestDistances;
					if (abs(dx) < lineToLineDistanceX / 2)
					{
						bestDistances = bestDistancesX;
					}
					else
					{
						bestDistances = bestDistancesY;
					}
					// what is the worst distance we memorized so far ? replace it
					int putToIndex = -1;
					double putToDist = -1;
					for (int td = 0; td < _countof(bestDistancesX); td++)
					{
						if (bestDistances[td] > dist)
						{
							double distdist = abs(bestDistances[td] - dist);
							if (distdist > putToDist)
							{
								putToDist = distdist;
								putToIndex = td;
							}
						}
					}
					if(putToIndex != -1)
					{
//						printf("new best distance at %d %d dist %lf %lf. replace %lf with %lf\n", sx, sy, dx, dy, bestDistances[putToIndex], dist);
						bestDistances[putToIndex] = dist;
					}
				}
				// actually mark and intersection
				// this mark will be used by later steps !
				SetPixelColor_(sx, sy, 255, 0, 255);
//				SetPixelColor(Img, sx, sy, 3, 255, 0, 255);
				markCount++;
			}
		}
	}
	printf("Marked %d intersections for later search\n", markCount);
	double avgDistanceX = 0,avgDistanceY = 0;
	for (int i = 0; i < _countof(bestDistancesX); i++)
	{
		avgDistanceX += bestDistancesX[i];
		avgDistanceY += bestDistancesY[i];
	}
	avgDistanceX = avgDistanceX / (_countof(bestDistancesX) + _countof(bestDistancesX) / 2);
	avgDistanceY = avgDistanceY / (_countof(bestDistancesX) + _countof(bestDistancesX) / 2);
	printf("Average line to line distance in pixels : %lf %lf\n", avgDistanceX, avgDistanceY);
	// we want to have same amount of lines on both X an Y. So we will force one of the axis to stretch to the other one
	lineToLineDistanceX = MAX(avgDistanceX, avgDistanceY);
	lineToLineDistanceY = MAX(avgDistanceX, avgDistanceY);

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
//		if (abs(xExpected) == 19 || abs(yExpected) == 19)
//		if (xExpected > 15 && yExpected > 15)
//		if (abs(yExpected) == 24)
		{
			DrawLineColor(Img, (float)(manualMarkedOriginX + xExpected * lineToLineDistanceX), (float)(manualMarkedOriginY + yExpected * lineToLineDistanceY), (float)xFound, (float)yFound, 255, 0, 0);
		}
	}
}

void VisualMarkIntersections()
{
	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		int xFound = intersections[i].picX;
		int yFound = intersections[i].picY;
		for (int y = yFound - 2; y < yFound + 2; y++)
		{
			for (int x = xFound - 2; x < xFound + 2; x++)
			{
				if (x >= 0 && x < width && y >= 0 && y < height)
				{
					SetPixelColor_(x, y, 255, 0, 0);
				}
			}
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

void InitAdjustmentMap()
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
	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		double sx = xExpected * lineToLineDistanceX;
		double sy = yExpected * lineToLineDistanceY;
		double ex = intersections[i].picX - manualMarkedOriginX;
		double ey = intersections[i].picY - manualMarkedOriginY;
		// scale to normalized size. We intended to draw lines with distance 25*2, but the scanned image contained 17 pixels
		// the scanned image is almost 3 times smaller than we intended to have
		sx = sx * expectedLineToLineDistance / lineToLineDistanceX;
		sy = sy * expectedLineToLineDistance / lineToLineDistanceY;
		ex = ex * expectedLineToLineDistance / lineToLineDistanceX;
		ey = ey * expectedLineToLineDistance / lineToLineDistanceY;
		// we need to flip the position of the destination relativ to the position where we intended it to be
		// required to obtain a compensation value instead of the obtained value
		double dx = ex - sx;
		double dy = ey - sy;
		ex = sx - dx;
		ey = sy - dy;
//		sLineAdjuster2.AdjustPosition((int)sx, (int)sy, ex, ey, 1);
		sLineAdjuster2.AdjustPosition(intersections[i].x, intersections[i].y, ex, ey, 1, 1);
	}
}

void VisualizeCorrections()
{
	// build a calibration map out of the obtained data
	FIBITMAP* dib = CreateNewImage(IMG_SIZE_DEBUG_VECTORS, IMG_SIZE_DEBUG_VECTORS);
	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		double sx = xExpected * lineToLineDistanceX;
		double sy = yExpected * lineToLineDistanceY;
		double ex = intersections[i].picX - manualMarkedOriginX;
		double ey = intersections[i].picY - manualMarkedOriginY;
		// scale to normalized size. We intended to draw lines with distance 25*2, but the scanned image contained 17 pixels
		// the scanned image is almost 3 times smaller than we intended to have
		sx = sx * expectedLineToLineDistance / lineToLineDistanceX;
		sy = sy * expectedLineToLineDistance / lineToLineDistanceY;
		ex = ex * expectedLineToLineDistance / lineToLineDistanceX;
		ey = ey * expectedLineToLineDistance / lineToLineDistanceY;
		// we need to flip the position of the destination relativ to the position where we intended it to be
		// required to obtain a compensation value instead of the obtained value
		double dx = ex - sx;
		double dy = ey - sy;
		ex = sx - dx;
		ey = sy - dy;
		DrawLineColorFade(dib, (float)sx + IMG_SIZE_DEBUG_VECTORS / 2, (float)sy + IMG_SIZE_DEBUG_VECTORS / 2, (float)ex + IMG_SIZE_DEBUG_VECTORS / 2, (float)ey + IMG_SIZE_DEBUG_VECTORS / 2, 255, 255, 255);
	}
	BYTE* bytes = FreeImage_GetBits(dib);
	int stride = FreeImage_GetPitch(dib);
	for (size_t y = IMG_SIZE_DEBUG_VECTORS / 2 - 5; y < IMG_SIZE_DEBUG_VECTORS / 2 + 5; y++)
	{
		for (size_t x = IMG_SIZE_DEBUG_VECTORS / 2 - 5; x < IMG_SIZE_DEBUG_VECTORS / 2 + 5; x++)
		{
			bytes[(y)*stride + (x) * 3 + 0] = 0;
			bytes[(y)*stride + (x) * 3 + 1] = 0;
			bytes[(y)*stride + (x) * 3 + 2] = 255;
		}
	}
	SaveImagePNG(dib, "CorrectionsVisualized1.png");
	FreeImage_Unload(dib);
}

void UpdateAdjustmentMap(double ImpactCoeff = 1.0)
{
	// build a calibration map out of the obtained data
	for (size_t i = 0; i < ICount; i++)
	{
		int xExpected = intersections[i].x;
		int yExpected = intersections[i].y;
		double sx = xExpected * lineToLineDistanceX;
		double sy = yExpected * lineToLineDistanceY;
		double ex = intersections[i].picX - manualMarkedOriginX;
		double ey = intersections[i].picY - manualMarkedOriginY;
		// scale to normalized size. We intended to draw lines with distance 25*2, but the scanned image contained 17 pixels
		// the scanned image is almost 3 times smaller than we intended to have
		sx = sx * expectedLineToLineDistance / lineToLineDistanceX;
		sy = sy * expectedLineToLineDistance / lineToLineDistanceY;
		ex = ex * expectedLineToLineDistance / lineToLineDistanceX;
		ey = ey * expectedLineToLineDistance / lineToLineDistanceY;
		// we need to flip the position of the destination relativ to the position where we intended it to be
		// required to obtain a compensation value instead of the obtained value
		double dx = ex - sx;
		double dy = ey - sy;
//		if (abs(dx) > lineToLineDistanceX)
		{
//			printf("Large adjustment x = %lf y = %lf\n", dx, dy);
		}
//		if (abs(dy) > lineToLineDistanceY)
		{
//			printf("Large adjustment x = %lf y = %lf\n", dx, dy);
		}
//		sLineAdjuster2.AdjustPosition((int)sx, (int)sy, -dx * ImpactCoeff, -dy * ImpactCoeff, 0, 0);
		sLineAdjuster2.AdjustPosition(intersections[i].x, intersections[i].y, -dx * ImpactCoeff, -dy * ImpactCoeff, 0, 1);
	}
}

void RunSafetyChecks()
{
#if 1
	// some formatting and sanity checks
	memset(intersectionsMat, 0, sizeof(intersectionsMat));
	int minX = 10000, maxX = -10000, minY = 10000, maxY = -10000;
	for (size_t i = 0; i < ICount; i++)
	{
		int x = intersections[i].x + InterSectionsOrigin;
		int y = intersections[i].y + InterSectionsOrigin;
		if (x < 0 || x >= InterSectionsOrigin * 2)
		{
			printf("Mat is not large enough x= %d y=%d\n", x, y);
			continue;
		}
		if (y < 0 || y >= InterSectionsOrigin * 2)
		{
			printf("Mat is not large enough x= %d y=%d\n", x, y);
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
	for (size_t y = 0; y < InterSectionsOrigin * 2; y++)
	{
		for (size_t x = 0; x < InterSectionsOrigin * 2; x++)
		{
			if (intersectionsMat[y][x].picX != 0 && intersectionsMat[y][x + 1].picX == 0 && intersectionsMat[y][x + 2].picX != 0)
			{
				printf("Gap in row %zd col %zd detected\n", y, x);
				SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
			}
			if (intersectionsMat[y][x].picX != 0 && intersectionsMat[y + 1][x].picX == 0 && intersectionsMat[y + 2][x].picX != 0)
			{
				printf("Gap in col %zd col %zd detected\n", y, x);
				SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
			}
			if (intersectionsMat[y][x].picY != 0 && intersectionsMat[y][x + 1].picY == 0 && intersectionsMat[y][x + 2].picY != 0)
			{
				printf("Gap in row %zd col %zd detected\n", y, x);
				SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
			}
			if (intersectionsMat[y][x].picY != 0 && intersectionsMat[y + 1][x].picY == 0 && intersectionsMat[y + 2][x].picY != 0)
			{
				printf("Gap in col %zd col %zd detected\n", y, x);
				SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
			}
			if (intersectionsMat[y][x].picX != 0 && intersectionsMat[y][x + 1].picX != 0 && intersectionsMat[y][x].picX >= intersectionsMat[y][x + 1].picX)
			{
				printf("Column should be increasing %zd col %zd detected\n", y, x);
				SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
			}
			if (intersectionsMat[y][x].picY != 0 && intersectionsMat[y+1][x].picY != 0 && intersectionsMat[y][x].picY >= intersectionsMat[y+1][x].picY)
			{
				printf("Row should be increasing %zd col %zd detected\n", y, x);
				SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
			}
			// values should be between neighbours
			if (x > 0 && intersectionsMat[y][x].picX != 0 && intersectionsMat[y][x - 1].picX != 0 && intersectionsMat[y][x + 1].picX != 0)
			{
				if (!(intersectionsMat[y][x - 1].picX < intersectionsMat[y][x].picX && intersectionsMat[y][x].picX < intersectionsMat[y][x + 1].picX))
				{
					printf("Col value is not between neighbours %zd col %zd detected\n", y, x);
					SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
				}
				double distLeft = abs(intersectionsMat[y][x - 1].picX - intersectionsMat[y][x].picX);
				double distRight = abs(intersectionsMat[y][x + 1].picX - intersectionsMat[y][x].picX);
				if (distLeft / distRight > 1.5 || distLeft / distRight < 0.5)
				{
//					printf("Big distance diff in X %zd col %zd detected %lf %lf\n", y, x, distLeft, distRight);
					SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
				}
			}
			if (y > 0 && intersectionsMat[y - 1][x].picY != 0 && intersectionsMat[y][x].picY != 0 && intersectionsMat[y + 1][x].picY != 0)
			{
				if (!(intersectionsMat[y - 1][x].picY < intersectionsMat[y][x].picY && intersectionsMat[y][x].picY < intersectionsMat[y + 1][x].picY))
				{
					printf("Row value is not between neighbours %zd col %zd detected\n", y, x);
					SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
				}
				double distUp = abs(intersectionsMat[y-1][x].picY - intersectionsMat[y][x].picY);
				double distDown = abs(intersectionsMat[y+1][x].picY - intersectionsMat[y][x].picY);
				if (distUp / distDown > 1.5 || distUp / distDown < 0.5)
				{
					printf("Big distance diff in Y %zd col %zd detected %lf %lf\n", y, x, distUp, distDown);
//					SetPixelColor(Img, intersectionsMat[y][x].picX, intersectionsMat[y][x].picY, 7, 0, 0, 0);
				}
			}
		}
	}
	printf("minX=%d,maxX=%d,minY=%d,maxY=%d\n", minX, maxX, minY, maxY);
#endif

	// for every row, get min, max, check if all values exist between them
	for (int y = -InterSectionsOrigin; y < InterSectionsOrigin; y++)
	{
		int minX = InterSectionsOrigin, maxX = -InterSectionsOrigin;
		int valuesSet[InterSectionsOrigin * 2] = { 0 };
		for (int i = 0; i < ICount; i++)
		{
			if (intersections[i].y != y)
			{
				continue;
			}
			if (intersections[i].x > maxX)
			{
				maxX = intersections[i].x;
			}
			if (intersections[i].x < minX)
			{
				minX = intersections[i].x;
			}
			valuesSet[InterSectionsOrigin + intersections[i].x] = i + 1;
		}
		// this row dows not have values set
		if (minX == InterSectionsOrigin)
		{
			continue;
		}
		// check if all values are set
		for (int i = minX + 1; i < maxX; i++)
		{
			if (valuesSet[InterSectionsOrigin + i] == 0)
			{
				printf("Value is not set for row %d col %d. MinX %d maxX %d\n", y, i, minX, maxX);
				int indAtPreMissing = valuesSet[InterSectionsOrigin + i - 1] - 1;
				for (int y3 = intersections[indAtPreMissing].picY - 10; y3 < intersections[indAtPreMissing].picY + 10; y3++)
				{
					for (int x3 = intersections[indAtPreMissing].picX - 10; x3 < intersections[indAtPreMissing].picX + 10; x3++)
					{
						if (x3 >= 0 && x3 < width && y3 >= 0 && y3 < height)
						{
							SetPixelColor_(x3, y3, 0, 0, 0);
						}
					}
				}
			}
			int indAtPre = valuesSet[InterSectionsOrigin + i - 1] - 1;
			int indAt = valuesSet[InterSectionsOrigin + i] - 1;
			if (intersections[indAtPre].picX >= intersections[indAt].picX)
			{
				printf("Unexpected out of order column value at line %d col %d\n", y, i);
			}
		}
	}
	for (int x = -InterSectionsOrigin; x < InterSectionsOrigin; x++)
	{
		int minY = InterSectionsOrigin, maxY = -InterSectionsOrigin;
		int valuesSet[InterSectionsOrigin * 2] = { 0 };
		for (int i = 0; i < ICount; i++)
		{
			if (intersections[i].x != x)
			{
				continue;
			}
			if (intersections[i].y > maxY)
			{
				maxY = intersections[i].y;
			}
			if (intersections[i].y < minY)
			{
				minY = intersections[i].y;
			}
			valuesSet[InterSectionsOrigin + intersections[i].y] = i + 1;
		}
		// this row dows not have values set
		if (minY == InterSectionsOrigin)
		{
			continue;
		}
		// check if all values are set
		for (int i = minY + 1; i < maxY; i++)
		{
			if (valuesSet[InterSectionsOrigin + i] == 0 && valuesSet[InterSectionsOrigin + i - 1] - 1 >= 0)
			{
				printf("Value is not set for col %d row %d. MinX %d maxX %d\n", x, i, minY, maxY);
				int indAtPreMissing = valuesSet[InterSectionsOrigin + i - 1] - 1;
				for (int y3 = intersections[indAtPreMissing].picY - 10; y3 < intersections[indAtPreMissing].picY + 10; y3++)
				{
					for (int x3 = intersections[indAtPreMissing].picX - 10; x3 < intersections[indAtPreMissing].picX + 10; x3++)
					{
						SetPixelColor_(x3, y3, 0, 0, 0);
					}
				}
			}
			int indAtPre = valuesSet[InterSectionsOrigin + i - 1] - 1;
			int indAt = valuesSet[InterSectionsOrigin + i] - 1;
			if (intersections[indAtPre].picY >= intersections[indAt].picY)
			{
				printf("Unexpected out of order column value at col %d line %d\n", x, i);
			}
		}
	}
}

static void LoadSpecificCallibrationImage(const char* fileName, int isInitial, int commandCountRef, int isInitialImg)
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
	if (isInitialImg)
	{
		lineToLineDistanceX = lineToLineDistance1;
		lineToLineDistanceY = lineToLineDistance1;
	}
	else
	{
		lineToLineDistanceX = lineToLineDistance2;
		lineToLineDistanceY = lineToLineDistance2;
	}

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
			SetPixelColor_(x, y, 0, 0, 255);

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

	VisualMarkIntersections(); // debugging visually
	VisualizeCorrections(); // create a separate image without background for motion vectors only
	RunSafetyChecks(); // make sure lines are lines and columns are columns. See if values are missing inbetween
	DrawMotionVectors(); // draw a line from expected to actual position
//	PrintMotionVectors(); // once uppon a time when python was used manually ..
	if (isInitialImg)
	{
		InitAdjustmentMap();
//		sLineAdjuster2.FillMissingInfo();
		SaveImagePNG(Img, "tv1.png");
	}
	else
	{
		UpdateAdjustmentMap(1);
//		sLineAdjuster2.FillMissingInfo2();
		SaveImagePNG(Img, "tv2.png");
	}

	FreeImage_Unload(Img);
	Img = NULL;

	sLineAdjuster2.DebugDumpMapToImage(X_IS_SET | Y_IS_SET, "map_locs_set.png");
	sLineAdjuster2.DebugDumpMapToImage(X_IS_MEASURED | Y_IS_MEASURED, "map_locs_measured.png");
	sLineAdjuster2.DebugDumpMapToImage(X_IS_UPDATED | Y_IS_UPDATED, "map_locs_updated.png");
}

void Test_loadCallibrationImages2()
{
//	LoadSpecificCallibrationImage("./stretch_maps/horver6_05_31_1.bmp", 1, 25, 1);
	LoadSpecificCallibrationImage("./stretch_maps/horver6_05_31.bmp", 1, 25, 0);

//	sLineAdjuster2.FillMissingInfo();
}
