#include "StdAfx.h"

// would we step on the edge of a font ? Or maybe at least inside of the font ?
int IsLineFrontNearColor(BYTE* bytes, int x, int y, int stride, int width, int height)
{
	if (x < 0 || x >= width)
	{
		return 2;
	}
	if (y < 0 || y >= height)
	{
		return 2;
	}
//	if (ColorMatch(*(int*)&bytes[y * stride + x * 3], FONT_COLOR))
	{
//		return 1;
	}
	if (ColorMatch(*(int*)&bytes[y * stride + x * 3], FONT_EDGE_COLOR))
	{
		return 1;
	}
	return 0;
}

int IsLineCovered(FIBITMAP* img, int sx, int sy, int ex, int ey)
{
	if (sx < 0)return 0;
	if (ex < 0)return 0;
	if (sy < 0)return 0;
	if (ey < 0)return 0;
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	if (sx >= width)return 0;
	if (ex >= width)return 0;
	if (sy >= height)return 0;
	if (ey >= height)return 0;

	double dx = ex - sx;
	double dy = ey - sy;
	if (dx == dy && dx == 0)
	{
		return 0 ;
	}

	// just to increase the draw accuracy. More points, more smoothness
	double lineDrawSteps;
	if (abs(dy) > abs(dx))
	{
		lineDrawSteps = abs(dy);
	}
	else
	{
		lineDrawSteps = abs(dx);
	}

	double xIncForStep = dx / lineDrawSteps;
	double yIncForStep = dy / lineDrawSteps;
	double writtenX = 0;
	double writtenY = 0;
	double step = 0;
	do
	{
		step += 1;
		if (step > lineDrawSteps)
		{
			step = lineDrawSteps;
		}
		double curXPos = step * xIncForStep;
		double curYPos = step * yIncForStep;
		double xdiff = curXPos - writtenX;
		double ydiff = curYPos - writtenY;

		if (xdiff <= -1.0)
		{
			writtenX += -1.0;
			if (IsLineFrontNearColor(bytes, sx + writtenX, sy + writtenY, stride, width, height) != 1)
			{
				return 0;
			}
		}
		else if (xdiff >= 1.0)
		{
			writtenX += 1.0;
			if (IsLineFrontNearColor(bytes, sx + writtenX, sy + writtenY, stride, width, height) != 1)
			{
				return 0;
			}
		}
		if (ydiff <= -1.0)
		{
			writtenY += -1.0;
			if (IsLineFrontNearColor(bytes, sx + writtenX, sy + writtenY, stride, width, height) != 1)
			{
				return 0;
			}
		}
		else if (ydiff >= 1.0)
		{
			writtenY += 1.0;
			if (IsLineFrontNearColor(bytes, sx + writtenX, sy + writtenY, stride, width, height) != 1)
			{
				return 0;
			}
		}
	} while (step != lineDrawSteps);

	return 1;
}

void ExtractLineAt(struct FIBITMAP* img, int sx, int sy, LineStore* res)
{
	int radius = 0;
	int foundLine = 0;
	res->len = 0;
	res->sx = sx;
	res->sy = sy;
	do {
		foundLine = 0;
		radius++;
		for (int y = -radius; y <= radius; y++)
		{
			if (foundLine == 1)
			{
				break;
			}
			for (int x = -radius; x <= radius; x++)
			{
				if (x == 0 && y == y)
				{
					continue;
				}

				int ey = y + sy;
				int ex = x + sx;
				if (IsLineCovered(img, sx, sy, ex, ey) == 1)
				{
					double newLen = GetLineLen(sx, sy, ex, ey);
					if (newLen > res->len)
					{
						foundLine = 1;
						res->len = newLen;
						res->ex = ex;
						res->ey = ey;
						break;
					}
				}
			}
		}
	} while (foundLine != 0);
}

int linesExtractedCount = 0;
LineStore linesExtracted[1000 * 1000];
double longestLine = 0;

void ExtractAllLines(struct FIBITMAP* img)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (ColorMatch(*(int*)&bytes[y * stride + x * BYTESPP], FONT_EDGE_COLOR))
			{
				if (linesExtractedCount >= _countof(linesExtracted))
				{
					printf("There are more lines present than can be extracted. Aborting \n");
				}
				ExtractLineAt(img, x, y, &linesExtracted[linesExtractedCount]);
				if (linesExtracted[linesExtractedCount].len > longestLine)
				{
					longestLine = linesExtracted[linesExtractedCount].len;
				}
				linesExtractedCount++;
				printf("\r extracting line %d", linesExtractedCount);
			}
		}
	}
	printf("\rGenerated %d lines. Longest %f \n", linesExtractedCount, longestLine);
}

template<int pixelWidth>
void MarkPixel(BYTE* dummyImg, int x, int y, int stride)
{
	for (int py = -pixelWidth; py <= pixelWidth; py++)
	{
		for (int px = -pixelWidth; px <= pixelWidth; px++)
		{
			if (y + py < 0)
			{
				continue;
			}
			dummyImg[(y + py) * stride + x + px] = 1;
		}
	}
}
int CheckLineCovered(BYTE *dummyImg, int lineIndex, int stride, int markLine)
{
	int sx = linesExtracted[lineIndex].sx;
	int sy = linesExtracted[lineIndex].sy;
	double dx = linesExtracted[lineIndex].ex - linesExtracted[lineIndex].sx;
	double dy = linesExtracted[lineIndex].ey - linesExtracted[lineIndex].sy;
	if (dx == dy && dx == 0)
	{
		return 0;
	}

	// just to increase the draw accuracy. More points, more smoothness
	double lineDrawSteps;
	if (abs(dy) > abs(dx))
	{
		lineDrawSteps = abs(dy);
	}
	else
	{
		lineDrawSteps = abs(dx);
	}

	double xIncForStep = dx / lineDrawSteps;
	double yIncForStep = dy / lineDrawSteps;
	int writtenX = 0;
	int writtenY = 0;
	double step = 0;
	do
	{
		step += 1;
		if (step > lineDrawSteps)
		{
			step = lineDrawSteps;
		}
		double curXPos = step * xIncForStep;
		double curYPos = step * yIncForStep;
		double xdiff = curXPos - writtenX;
		double ydiff = curYPos - writtenY;

		if (xdiff <= -1.0)
		{
			writtenX += -1;
			if (markLine)
			{
//				dummyImg[(sy + writtenY) * stride + sx + writtenX] = 1;
				MarkPixel<0>(dummyImg, sx + writtenX, sy + writtenY, stride);
			}
			else if (dummyImg[(sy + writtenY )*stride + sx + writtenX] == 0)
			{
				return 0;
			}
		}
		else if (xdiff >= 1.0)
		{
			writtenX += 1;
			if (markLine)
			{
//				dummyImg[(sy + writtenY) * stride + sx + writtenX] = 1;
				MarkPixel<0>(dummyImg, sx + writtenX, sy + writtenY, stride);
			}
			else if (dummyImg[(sy + writtenY) * stride + sx + writtenX] == 0)
			{
				return 0;
			}
		}
		if (ydiff <= -1.0)
		{
			writtenY += -1;
			if (markLine)
			{
//				dummyImg[(sy + writtenY) * stride + sx + writtenX] = 1;
				MarkPixel<0>(dummyImg, sx + writtenX, sy + writtenY, stride);
			}
			else if (dummyImg[(sy + writtenY) * stride + sx + writtenX] == 0)
			{
				return 0;
			}
		}
		else if (ydiff >= 1.0)
		{
			writtenY += 1;
			if (markLine)
			{
//				dummyImg[(sy + writtenY) * stride + sx + writtenX] = 1;
				MarkPixel<0>(dummyImg, sx + writtenX, sy + writtenY, stride);
			}
			else if (dummyImg[(sy + writtenY) * stride + sx + writtenX] == 0)
			{
				return 0;
			}
		}
	} while (step != lineDrawSteps);

	return 1;
}

void EliminateSmallLines()
{
	const int stride = 5000;
	const int estimatedImageResolution = 5000 * stride;
	BYTE *dummyImg = (BYTE*)malloc(estimatedImageResolution);

	if (dummyImg == NULL)
	{
		return;
	}

	memset(dummyImg, 0, estimatedImageResolution);

	int linesEliminatedTotal = 0;
	for (int selectedLen = (int)longestLine; selectedLen > 0; selectedLen--)
	{
		int eliminatedThisLen = 0;
		int keptThisLen = 0;
		for (int i = 0; i < linesExtractedCount; i++)
		{
			if ((int)linesExtracted[i].len != selectedLen)
			{
				continue;
			}
			if (CheckLineCovered(dummyImg, i, stride, 0) == 1)
			{
				linesExtracted[i].len = 0;
				linesExtracted[i].sx = linesExtracted[i].sy = linesExtracted[i].ex = linesExtracted[i].ey = 0;
				eliminatedThisLen++;
				linesEliminatedTotal++;
			}
			else
			{
				CheckLineCovered(dummyImg, i, stride, 1);
				keptThisLen++;
			}
		}
		if (eliminatedThisLen > 0)
		{
			printf("\tAt len %d eliminated %d . Kept %d\n", selectedLen, eliminatedThisLen, keptThisLen);
		}
	}
	printf("Total lines eliminated %d\n", linesEliminatedTotal);

	free(dummyImg);
}

int DrawSpecificLine(FIBITMAP* img, int lineIndex)
{
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	int sx = linesExtracted[lineIndex].sx;
	int sy = linesExtracted[lineIndex].sy;
	double dx = linesExtracted[lineIndex].ex - linesExtracted[lineIndex].sx;
	double dy = linesExtracted[lineIndex].ey - linesExtracted[lineIndex].sy;
	if (dx == dy && dx == 0)
	{
		return 0;
	}

	// just to increase the draw accuracy. More points, more smoothness
	double lineDrawSteps;
	if (abs(dy) > abs(dx))
	{
		lineDrawSteps = abs(dy);
	}
	else
	{
		lineDrawSteps = abs(dx);
	}

	double xIncForStep = dx / lineDrawSteps;
	double yIncForStep = dy / lineDrawSteps;
	int writtenX = 0;
	int writtenY = 0;
	double step = 0;
	do
	{
		step += 1;
		if (step > lineDrawSteps)
		{
			step = lineDrawSteps;
		}
		double curXPos = step * xIncForStep;
		double curYPos = step * yIncForStep;
		double xdiff = curXPos - writtenX;
		double ydiff = curYPos - writtenY;

		if (xdiff <= -1.0)
		{
			writtenX += -1;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 0] = 0;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 1] = 255;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 2] = 0;
		}
		else if (xdiff >= 1.0)
		{
			writtenX += 1;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 0] = 0;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 1] = 255;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 2] = 0;
		}
		if (ydiff <= -1.0)
		{
			writtenY += -1;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 0] = 0;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 1] = 255;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 2] = 0;
		}
		else if (ydiff >= 1.0)
		{
			writtenY += 1;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 0] = 0;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 1] = 255;
			bytes[(sy + writtenY) * stride + (sx + writtenX) * BYTESPP + 2] = 0;
		}
	} while (step != lineDrawSteps);

	return 1;
}

void DrawLines(FIBITMAP* img)
{
	for (int i = 0; i < linesExtractedCount; i++)
	{
		if ((int)linesExtracted[i].len == 0)
		{
			continue;
		}
		DrawSpecificLine(img, i);
	}
}

void ProcessInputUsingLines(struct FIBITMAP* img)
{
	printf("Mark edges \n");
	MarkEdges(img, 0, 0, 255);

	printf("Convert points to lines\n");
	ExtractAllLines(img);

	EliminateSmallLines();

	DrawLines(img);

	printf("Save the image\n");
	SaveImage(img, "processed_stage1.bmp");

	dumpLinesToSig("edgeLines.sig", linesExtracted, linesExtractedCount);
}