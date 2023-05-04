#include "StdAfx.h"

void LogMessage(const char* file, int line, const char* msg)
{ 
	char remsg[5000];
	sprintf_s(remsg, sizeof(remsg), "%s:%d:%s\n", __FILE__, __LINE__, msg);
	printf(remsg);
}

static signed short LineExtractUsedID = 1;

typedef struct picLoc
{
	int x, y;
}picLoc;

int IsCallibrationLinePixel(FIBITMAP* Img, int x, int y)
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

ShapeStore ExtractShapeAtLoc(FIBITMAP* Img, int x, int y, int jumpGap, int searchRadiusX, int searchRadiusY)
{
	ShapeStore ss;
	memset(&ss, 0, sizeof(ss));
	ss.startX = x;
	ss.startY = y;
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
	if (!IsCallibrationLinePixel(Img, x, y))
	{
		return ss;
	}

	// mark starting position
	short lineId = LineExtractUsedID;
	LineExtractUsedID = LineExtractUsedID + 1;
	*(short*)(&Bytes[y * Stride + x * Bytespp]) = lineId;

	// search nearby as long as we find a pixel
	int foundShapeAtRadius = 0;

	int prevLocsFound = 1;
	int curLocsFound = 0;
	picLoc* prevLocs = (picLoc*)malloc(Width * Height * sizeof(picLoc));
	picLoc* curLocs = (picLoc*)malloc(Width * Height * sizeof(picLoc));
	if (prevLocs == NULL || curLocs == NULL)
	{
		return ss;
	}
	prevLocs[0].x = x;
	prevLocs[0].y = y;
	__int64 sumX = 0;
	__int64 sumY = 0;
	__int64 sumCount = 0;

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
					if (IsCallibrationLinePixel(Img, atx, aty))
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

						sumX += atx;
						sumY += aty;
						sumCount++;
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
	ss.centerx2 = (int)(sumX / sumCount);
	ss.centery2 = (int)(sumY / sumCount);
	ss.lineId = lineId;

	return ss;
}

void DrawLineColor(FIBITMAP* Img, float sx, float sy, float ex, float ey, BYTE R, BYTE G, BYTE B)
{
	float dx = ex - sx;
	float dy = ey - sy;
	float steps;
	if (abs(dx) > abs(dy))
	{
		steps = abs(dx);
	}
	else
	{
		steps = abs(dy);
	}
	float xinc = dx / steps;
	float yinc = dy / steps;
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	for (float step = 0; step < steps; step += 1.0f)
	{
		int x = (int)(sx + xinc * step);
		int y = (int)(sy + yinc * step);
		if (x<0 || y<0 || x>=Width || y>=Height)
		{
			continue;
		}
		Bytes[y * Stride + x * Bytespp + 0] = B;
		Bytes[y * Stride + x * Bytespp + 1] = G;
		Bytes[y * Stride + x * Bytespp + 2] = R;
	}
}

void DrawLineColorFade(FIBITMAP* Img, float sx, float sy, float ex, float ey, BYTE R, BYTE G, BYTE B)
{
	float dx = ex - sx;
	float dy = ey - sy;
	float steps;
	if (abs(dx) > abs(dy))
	{
		steps = abs(dx);
	}
	else
	{
		steps = abs(dy);
	}
	float xinc = dx / steps;
	float yinc = dy / steps;
	float colorStep = 0.5 / steps;

	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	for (float step = 0; step < steps; step += 1.0f)
	{
		int x = (int)(sx + xinc * step);
		int y = (int)(sy + yinc * step);
		if (x < 0 || y < 0 || x >= Width || y >= Height)
		{
			continue;
		}
		float colorMul = colorStep * step;
		Bytes[y * Stride + x * Bytespp + 0] = B / 2 + B * colorMul;
		Bytes[y * Stride + x * Bytespp + 1] = G / 2 + G * colorMul;
		Bytes[y * Stride + x * Bytespp + 2] = R / 2 + R * colorMul;
	}
}

int getSign(double a)
{
	if (a == 0) return 0;
	if (a < 0) return -1;
	return 1;
}