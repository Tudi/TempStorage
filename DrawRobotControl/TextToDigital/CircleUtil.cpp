#include "StdAfx.h"

int circlesFoundCount = 0;
int maxRadius = 0;
CircleStore circlesFound[1000 * 1000];

int circle_rad1[3][3] = {	{1,1,1},
							{1,0,1},
							{1,1,1} };
int circle_rad2[5][5] = {	{0,1,1,1,0},
							{1,0,0,0,1},
							{1,0,0,0,1},
							{1,0,0,0,1},
							{0,1,1,1,0} };
int circle_rad3[7][7] = {	{0,0,1,1,1,0,0},
							{0,1,0,0,0,1,0},
							{1,0,0,0,0,0,1},
							{1,0,0,0,0,0,1},
							{1,0,0,0,0,0,1},
							{0,1,0,0,0,1,0},
							{0,0,1,1,1,0,0} };
int circle_rad4[9][9] = {	{0,0,0,1,1,1,0,0,0},
							{0,0,1,0,0,0,1,0,0},
							{0,1,0,0,0,0,0,1,1},
							{1,0,0,0,0,0,0,0,1},
							{1,0,0,0,0,0,0,0,1},
							{1,0,0,0,0,0,0,0,1},
							{0,1,0,0,0,0,0,1,0},
							{0,0,1,0,0,0,1,0,0},
							{0,0,0,1,1,1,0,0,0} };


template<int rad>
int CircleIsCoveredHandMade(FIBITMAP* img, int cx, int cy, int circle_points[rad * 2 + 1][rad * 2 + 1])
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	int neededLocationsCovered = 0;
	int locationsCovered = 0;
	for (int y = -rad; y <= rad; y++)
	{
		for (int x = -rad; x <= rad; x++)
		{
			if (circle_points[y + rad][x + rad] == 0)
			{
				continue;
			}
			neededLocationsCovered++;
			if (isBGColor(bytes, cx + x, cy + y, stride, width, height) == 1)
			{
				//				return 0;
				continue;
			}
			locationsCovered++;
		}
	}
	if (rad == 1 || rad == 2 || rad == 3 || rad == 4)
	{
		if (neededLocationsCovered - rad > locationsCovered)
		{
			return 0;
		}
	}
	else if (neededLocationsCovered * 90 / 100 > locationsCovered)
	{
		return 0;
	}
	return 1;
}

// draw a circle at this specific center and check if every location falls on a line
int CircleIsCovered(FIBITMAP* img, int cx, int cy, int r)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	if (cx - r < 0 || cx + r >= width)
	{
		return -1;
	}
	if (cy - r < 0 || cy + r >= height)
	{
		return -2;
	}
	if (r == 1)
	{
		return CircleIsCoveredHandMade<1>(img, cx, cy, circle_rad1);
	}
	if (r == 2)
	{
		return CircleIsCoveredHandMade<2>(img, cx, cy, circle_rad2);
	}
	if (r == 3)
	{
		return CircleIsCoveredHandMade<3>(img, cx, cy, circle_rad3);
	}
	if (r == 4)
	{
		return CircleIsCoveredHandMade<4>(img, cx, cy, circle_rad4);
	}


	// angle speed should depend on radius, but we do not have time for that now
	double radius = r;
	for (double angle = 0; angle < 360; angle += 0.05)
	{
		double tx1 = radius * cos(angle);
		if (tx1 > 0)
		{
			tx1 += 0.9;
		}
		else if (tx1 < 0)
		{
			tx1 -= 0.9;
		}
		double ty1 = radius * sin(angle);
		if (ty1 > 0)
		{
			ty1 += 0.9;
		}
		else if (ty1 < 0)
		{
			ty1 -= 0.9;
		}
		int px = (int)(cx + tx1);
		int py = (int)(cy + ty1);
		// radius is so small the endge of the circle falls on the center ?
		if (px == cx && py == cy)
		{
			continue;
		}
#define PIXEL_SIZE (1-1) // substract 1
		for (int y2 = PIXEL_SIZE; y2 <= PIXEL_SIZE; y2++)
		{
			for (int x2 = PIXEL_SIZE; x2 <= PIXEL_SIZE; x2++)
			{
				if (isBGColor(bytes, px + x2, py + y2, stride, width, height) == 1)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}

// at a certain location, check how large circle we could draw that is still covered by a line
int GetLargestAreaCoverageAt(FIBITMAP* img, int cx, int cy)
{
	int r = 1;
	while (CircleIsCovered(img, cx, cy, r) == 1)
	{
		r++;
	}
	if (r > maxRadius)
	{
		maxRadius = r;
	}
	return r;
}

// convert drawn lines into an array of points with sizes
void ExtractCircles(FIBITMAP* img)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	int bgColor = 0x00FFFFFF;
	for (size_t y = 0; y < height; y++)
	{
		BYTE* row = &bytes[y * stride];
		for (size_t x = 0; x < width; x++)
		{
			if (!ColorMatch(*(int*)&row[x * 3], bgColor))
			{
				int r = GetLargestAreaCoverageAt(img, (int)x, (int)y);
				circlesFound[circlesFoundCount].cx = (short)x;
				circlesFound[circlesFoundCount].cy = (short)y;
				circlesFound[circlesFoundCount].r = (short)r;
				circlesFoundCount++;
				if (circlesFoundCount == _countof(circlesFound))
				{
					printf("Too many circles to store. Early exiting \n");
					return;
				}
			}
		}
	}
}

// eliminate redundant circles. If a circle is covered by a larger circle, erase it
void EliminateCircles(FIBITMAP* img)
{
	for (int checkRCoverage = 1; checkRCoverage <= maxRadius; checkRCoverage++)
	{
		int eliminatedCount = 0;
		for (size_t indIsCovered = 0; indIsCovered < circlesFoundCount; indIsCovered++)
		{
			for (size_t indIsCovering = 0; indIsCovering < circlesFoundCount; indIsCovering++)
			{
				// merge small circles into larger ones
				if (circlesFound[indIsCovering].r != checkRCoverage)
				{
					continue;
				}
				// no self check
				if (indIsCovered == indIsCovering)
				{
					continue;
				}
				// this circle has been eliminated and no longer covering others
//				if (circlesFound[indIsCovering].r == 0)
				{
					//					continue;
				}
				// the circle that is covering us is smaller than us, we do not want to eliminate the larger circle
				if (circlesFound[indIsCovering].r < circlesFound[indIsCovered].r)
				{
					continue;
				}
				// do not chain eliminate points. Leave some of them
				// ex : B covers A, C covers B, then do not eliminate B. Instead try to eliminate A and C
				if (circlesFound[indIsCovering].r == circlesFound[indIsCovered].r &&
					circlesFound[indIsCovered].isCovering == 1
					)
				{
					continue;
				}
				// check if this circle is actually covered by the larger circle
				if (circlesFound[indIsCovered].cx <= circlesFound[indIsCovering].cx + circlesFound[indIsCovering].r &&
					circlesFound[indIsCovered].cx >= circlesFound[indIsCovering].cx - circlesFound[indIsCovering].r &&
					circlesFound[indIsCovered].cy <= circlesFound[indIsCovering].cy + circlesFound[indIsCovering].r &&
					circlesFound[indIsCovered].cy >= circlesFound[indIsCovering].cy - circlesFound[indIsCovering].r
					)
				{
					circlesFound[indIsCovered].r = 0;
					circlesFound[indIsCovering].isCovering = 1;
					eliminatedCount++;
					break; // this circle has been eliminated. No more need to check
				}
			}
		}
		printf("\t Weight %d eliminated %d points\n", checkRCoverage, eliminatedCount);
	}
}

int IsInCircle(int cx, int cy, int rad, int tx, int ty)
{
	double dx = cx - tx;
	double dy = cy - ty;
	double r2 = sqrt(dx * dx + dy * dy);
	return rad >= r2;
}

void EliminateCirclesWeaker(FIBITMAP* img)
{
	for (int checkRCoverage = maxRadius; checkRCoverage > 0; checkRCoverage--)
	{
		int eliminatedCount = 0;
		for (size_t indIsCovered = 0; indIsCovered < circlesFoundCount; indIsCovered++)
		{
			for (size_t indIsCovering = 0; indIsCovering < circlesFoundCount; indIsCovering++)
			{
				// merge small circles into larger ones
				if (circlesFound[indIsCovering].r != checkRCoverage)
				{
					continue;
				}
				// no self check
				if (indIsCovered == indIsCovering)
				{
					continue;
				}
				// the circle that is covering us is smaller than us, we do not want to eliminate the larger circle
				if (circlesFound[indIsCovering].r <= circlesFound[indIsCovered].r)
				{
					continue;
				}
				// check if this circle is actually covered by the larger circle
				if (IsInCircle(circlesFound[indIsCovering].cx, circlesFound[indIsCovering].cy, circlesFound[indIsCovering].r, circlesFound[indIsCovered].cx, circlesFound[indIsCovered].cy))
				{
					circlesFound[indIsCovered].r = 0;
					circlesFound[indIsCovering].isCovering = 1;
					eliminatedCount++;
					break; // this circle has been eliminated. No more need to check
				}
			}
		}
		printf("\t Weight %d eliminated %d points\n", checkRCoverage, eliminatedCount);
	}
}

void MarkCircles(FIBITMAP* img, BYTE newR, BYTE newG, BYTE newB)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	int markedCount = 0;
	for (size_t i = 0; i < circlesFoundCount; i++)
	{
		// skip eliminated circles
		if (circlesFound[i].r == 0)
		{
			continue;
		}

		// for the sake of debugging. Only draw specific circles
//		if (circlesFound[i].r != 4)
		{
			//			continue;
		}

		int weightedX = circlesFound[i].cx;
		int weightedY = circlesFound[i].cy;
		bytes[weightedY * stride + weightedX * BYTESPP + 0] = newB;
		bytes[weightedY * stride + weightedX * BYTESPP + 1] = newG;
		bytes[weightedY * stride + weightedX * BYTESPP + 2] = newR;
		markedCount++;
	}
	printf("\t Marked %d points\n", markedCount);
}

void MarkCirclesGradient(FIBITMAP* img)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	int markedCount = 0;
	for (size_t r = 1; r <= maxRadius; r++)
	{
		for (size_t i = 0; i < circlesFoundCount; i++)
		{
			if (circlesFound[i].r != r)
			{
				continue;
			}

			int weightedX = circlesFound[i].cx;
			int weightedY = circlesFound[i].cy;
			bytes[weightedY * stride + weightedX * BYTESPP + 0] = 0;
			bytes[weightedY * stride + weightedX * BYTESPP + 1] = 0;
			bytes[weightedY * stride + weightedX * BYTESPP + 2] = 255 - circlesFound[i].r * 30; // R
			markedCount++;
		}
	}
	printf("\t Marked %d points\n", markedCount);
}

void ProcessInputUsingWeights(struct FIBITMAP* img)
{
	printf("Detect line points and weights \n");
	ExtractCircles(img);

	printf("Eliminate covered points. Total %d \n", circlesFoundCount);
	EliminateCirclesWeaker(img);

	printf("Mark selected points \n");
	MarkCirclesGradient(img);

	//	printf("Remove non selected points \n");
	//	RemoveBlackColor(img);

	//	printf("Remove small aberations \n");
	//	RemoveSmallDots(img);

	printf("Save the image\n");
	SaveImage(img, "processed_stage1.bmp");
}