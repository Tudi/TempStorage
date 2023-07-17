#include "StdAfx.h"

void GetLineCenterAndWidth(BYTE* Ori, BYTE* Dst, int x, int y, int width, int height, int stride, int& out_x, int& out_y, int& out_width)
{
	int lineColor = *(int*)&Ori[y * stride + x * 3];
	lineColor = lineColor & 0x00FFFFFF;
	// find the minx,maxx
	int minX = x;
	BYTE* row = &Ori[y * stride];
	for (; minX > 0; minX--)
	{
		if (!ColorMatch(*(int*)&row[minX * 3], lineColor))
		{
			minX++;
			break;
		}
	}

	int maxX = x;
	row = &Ori[y * stride];
	for (; maxX < width; maxX++)
	{
		if (!ColorMatch(*(int*)&row[maxX * 3], lineColor))
		{
			maxX--;
			break;
		}
	}

	int minY = y;
	for (; minY > 0; minY--)
	{
		if (!ColorMatch(*(int*)&Ori[minY * stride + x * 3], lineColor))
		{
			minY++;
			break;
		}
	}

	int maxY = y;
	for (; maxY < height; maxY++)
	{
		if (!ColorMatch(*(int*)&Ori[maxY * stride + x * 3], lineColor))
		{
			maxY--;
			break;
		}
	}

	int lineWidth = maxX - minX + 1;
	int lineHeight = maxY - minY + 1;
	out_width = MIN(lineWidth, lineHeight);

	int minX2 = MAX(x - out_width, minX);
	int maxX2 = MIN(x + out_width, maxX);
	int minY2 = MAX(y - out_width, minY);
	int maxY2 = MIN(y + out_width, maxY);

	int lineCenterX = (minX2 + maxX2) / 2;
	int lineCenterY = (minY2 + maxY2) / 2;

	out_x = lineCenterX;
	out_y = lineCenterY;
}

void MarkLineCenter(BYTE* Ori, BYTE* Dst, int x, int y, int width, int height, int stride, int markColor)
{
	int lineColor = *(int*)&Ori[y * stride + x * 3];
	lineColor = lineColor & 0x00FFFFFF;

	// find the aprox center
	int lineWidth, x2, y2;
	GetLineCenterAndWidth(Ori, Dst, x, y, width, height, stride, x2, y2, lineWidth);

	// probably just noise
	if (lineWidth == 1)
	{
		//		return;
	}

	// find the weighted center
	int minX = MAX(x2 - lineWidth, 0);
	int maxX = MIN(x2 + lineWidth, width);
	int minY = MAX(y2 - lineWidth, 0);
	int maxY = MIN(y2 + lineWidth, height);
	__int64 sumX = 0, sumY = 0, count = 0;
	for (int y2 = minY; y2 < maxY; y2++)
	{
		for (int x2 = minX; x2 < maxX; x2++)
		{
			if (ColorMatch(*(int*)&Ori[y2 * stride + x2 * 3], lineColor))
			{
				sumX += x2;
				sumY += y2;
				count++;
			}
		}
	}

	// mark the center
	int newB = markColor & 0xFF;
	int newG = (markColor >> 8) & 0xFF;
	int newR = (markColor >> 16) & 0xFF;

	int weightedX = (int)(sumX / count);
	int weightedY = (int)(sumY / count);

	Dst[weightedY * stride + weightedX * 3 + 0] = newB;
	Dst[weightedY * stride + weightedX * 3 + 1] = newG;
	Dst[weightedY * stride + weightedX * 3 + 2] = newR;
}

//void ThinLine(FIBITMAP* img, BYTE lineR, BYTE lineG, BYTE lineB, BYTE newR, BYTE newG, BYTE newB)
void ThinLine(FIBITMAP* img, BYTE newR, BYTE newG, BYTE newB)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	// duplicate original image
	BYTE* bytesOri = (BYTE*)malloc(height * stride + 32);
	if (bytesOri == NULL)
	{
		return;
	}
	memcpy(bytesOri, bytes, height * stride);

	//	int lineColor = (int)lineR | ((int)lineG << 8) | ((int)lineB << 16);
	int bgColor = 0x00FFFFFF;
	int newColor = (int)newR | ((int)newG << 8) | ((int)newB << 16);
	for (size_t y = 0; y < height; y++)
	{
		BYTE* row = &bytesOri[y * stride];
		for (size_t x = 0; x < width; x++)
		{
			//			if (ColorMatch(*(int*)&row[x*3], lineColor))
			if (!ColorMatch(*(int*)&row[x * 3], bgColor))
			{
				MarkLineCenter(bytesOri, bytes, (int)x, (int)y, width, height, stride, newColor);
			}
		}
	}

	free(bytesOri);
}

void ProcessInputUsingSimpleWeights(struct FIBITMAP* img)
{
	printf("Thin lines based on weights \n");
	ThinLine(img, 0, 0, 255);

	printf("Remove non selected points \n");
	RemoveBlackColor(img);

	printf("Remove small aberations \n");
	RemoveSmallDots(img);

	printf("Save the image\n");
	SaveImage(img, "processed_stage1.bmp");
}