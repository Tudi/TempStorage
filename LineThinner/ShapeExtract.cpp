#include <stdlib.h>
#include "ShapeExtract.h"


#define ColorMatch(a,b) (((a)&0x00FFFFFF)==((b)&0x00FFFFFF))
static signed short LineExtractUsedID = 1;
#define Bytespp 3
#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

ShapeStore ExtractShapeAtLoc(FIBITMAP* Img, int x, int y, int jumpGap, int searchRadiusX, int searchRadiusY, ShapeExtractFlags sef)
{
	ShapeStore ss;
	ss.Constructor();
	ss.startX = x;
	ss.startY = y;
	ss.minX = x;
	ss.maxX = x;
	ss.minY = y;
	ss.maxY = y;
	BYTE* BytesOri = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	if (x < 0 || x >= Width)
	{
		return ss;
	}
	if (y <= 0 || y >= Height)
	{
		return ss;
	}
	BYTE* Bytes = (BYTE*)malloc(Height * Stride);
	if (Bytes == NULL)
	{
		return ss;
	}
	memcpy(Bytes, BytesOri, Height * Stride);

	if (searchRadiusX <= 0)
	{
		searchRadiusX = Width;
	}
	if (searchRadiusY <= 0)
	{
		searchRadiusY = Height;
	}

	int followColor = *(int*)(&Bytes[y * Stride + x * Bytespp]);

	// mark starting position
	if (LineExtractUsedID == followColor)
	{
		LineExtractUsedID = LineExtractUsedID + 1;
	}
	short lineId = LineExtractUsedID;
	LineExtractUsedID = LineExtractUsedID + 1;
	*(short*)(&Bytes[y * Stride + x * Bytespp]) = lineId;

	// search nearby as long as we find a pixel
	int foundShapeAtRadius = 0;

	int locReadIndex = 0;
	ShapeLoc* locationsFound = (ShapeLoc*)malloc(Width * Height * sizeof(ShapeLoc));
	if (locationsFound == NULL)
	{
		return ss;
	}
	ss.points = locationsFound;
	locationsFound[0].x = x;
	locationsFound[0].y = y;
	ss.shapePointCount = 1;
	__int64 sumX = 0;
	__int64 sumY = 0;
	__int64 sumCount = 0;

	while (locReadIndex < ss.shapePointCount)
	{
		int investigateX = locationsFound[locReadIndex].x;
		int investigateY = locationsFound[locReadIndex].y;
		locReadIndex++;
		for (int y2 = -jumpGap; y2 <= jumpGap; y2++)
		{
			for (int x2 = -jumpGap; x2 <= jumpGap; x2++)
			{
				if (y2 == 0 && x2 == 0)
				{
					continue;
				}
				int atx = investigateX + x2;
				int aty = investigateY + y2;
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
				if (ColorMatch(*(int*)&Bytes[aty * Stride + atx * Bytespp], followColor))
				{
					locationsFound[ss.shapePointCount].x = atx;
					locationsFound[ss.shapePointCount].y = aty;
					if (ss.shapePointCount < Width * Height)
					{
						ss.shapePointCount++;
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

	free(Bytes);

	ss.centerx = (ss.minX + ss.maxX) / 2;
	ss.centery = (ss.minY + ss.maxY) / 2;
	ss.centerx2 = (int)(sumX / sumCount);
	ss.centery2 = (int)(sumY / sumCount);
	ss.lineId = lineId;

	return ss;
}

void SetShapeColor(FIBITMAP* Img, ShapeStore* ss, BYTE R, BYTE G, BYTE B)
{
	BYTE* BytesOri = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	for (size_t i = 0; i < ss->shapePointCount; i++)
	{
		int index = ss->points[i].y * Stride + ss->points[i].x * Bytespp;
		if (index >= Height * Stride || index < 0)
		{
			continue;
		}
		BytesOri[index + 0] = B;
		BytesOri[index + 1] = G;
		BytesOri[index + 2] = R;
	}
}