#include "StdAfx.h"

int DetectLineAtVertHor(FIBITMAP* dib, int x, int y, ShapeGeneric* pret)
{
	ShapeLine* ret = &pret->Shapes.sl;
	pret->ShapeType = SHT_LINE;

	BYTE* ExtrMap = GetExtractedMap();

	//starting from a point, scan in half a circle if we could draw a line that will cover the same color
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE BaseB = Pixels[y * pitch + x * Bytespp + 0];
	BYTE BaseG = Pixels[y * pitch + x * Bytespp + 1];
	BYTE BaseR = Pixels[y * pitch + x * Bytespp + 2];

	if (ExtrMap[y * Width + x] != 0)
	{
		pret->ShapeType = SHT_INVALID;
		return 0;
	}

	ret->StartX = x;
	ret->StartY = y;
	pret->R = BaseR;
	pret->G = BaseG;
	pret->B = BaseB;

	//try a vertica line
	int MaxWidth = x;
	for(int i=x;i<Width;i++)
		if (Pixels[(y) * pitch + (i) * 3 + 0] != BaseB
			|| Pixels[(y) * pitch + (i) * 3 + 1] != BaseG
			|| Pixels[(y) * pitch + (i) * 3 + 2] != BaseR
			|| (ExtrMap[(y) * Width + i ] != EXTRACTED_NONE)
//			|| (ExtrMap[(y)*Width + i] != EXTRACTED_NONE && ExtrMap[(y)*Width + i] != EXTRACTED_HOR_LINE)
			)
		{
			MaxWidth = i;
			break;
		}
	int MaxHeight = y;
	for (int i = y; i < Height; i++)
		if (Pixels[(i)*pitch + (x) * 3 + 0] != BaseB
			|| Pixels[(i)*pitch + (x) * 3 + 1] != BaseG
			|| Pixels[(i)*pitch + (x) * 3 + 2] != BaseR
			|| (ExtrMap[(i)*Width + x] != EXTRACTED_NONE)
//			|| (ExtrMap[(i)*Width + x] != EXTRACTED_NONE && ExtrMap[(i)*Width + x] != EXTRACTED_VERT_LINE)
			)
		{
			MaxHeight = i;
			break;
		}
	if (MaxWidth == x || MaxHeight == y)
		pret->ShapeType = SHT_INVALID;
	else if (MaxWidth - x >= MaxHeight - y)
	{
		ret->EndY = y;
		ret->EndX = MaxWidth - 1;
		ret->Angle = 0;
	}
	else 
	{
		ret->EndY = MaxHeight - 1;
		ret->EndX = x;
		ret->Angle = 90;
	}

	return 0;
}


void MarkShapeLineExtracted(FIBITMAP* dib, ShapeGeneric* pret)
{
	BYTE* ExtrMap = GetExtractedMap();

	int sx, sy, ex, ey;
	if (pret->ShapeType == SHT_LINE)
	{
		sx = pret->Shapes.sl.StartX;
		sy = pret->Shapes.sl.StartY;
		ex = pret->Shapes.sl.EndX;
		ey = pret->Shapes.sl.EndY;
	}
	else
		return;

	int Width = FreeImage_GetWidth(dib);
	int MarkValue;
	if (pret->Shapes.sl.Angle == 0)
		MarkValue = EXTRACTED_HOR_LINE;
	else
		MarkValue = EXTRACTED_VERT_LINE;
	for (int y = sy; y <= ey; y++)
		for (int x = sx; x <= ex; x++)
			ExtrMap[y * Width + x] = MarkValue;
}

void PaintShapeLineExtracted(FIBITMAP* dib, ShapeGeneric* pret, int R, int G, int B)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);

	int sx, sy, ex, ey;
	if (pret->ShapeType == SHT_LINE)
	{
		sx = pret->Shapes.sl.StartX;
		sy = pret->Shapes.sl.StartY;
		ex = pret->Shapes.sl.EndX;
		ey = pret->Shapes.sl.EndY;
	}
	else
		return;

	for (int y = sy; y <= ey; y++)
		for (int x = sx; x <= ex; x++)
		{
			Pixels[y * pitch + x * 3 + 0] = B;
			Pixels[y * pitch + x * 3 + 1] = G;
			Pixels[y * pitch + x * 3 + 2] = R;
		}
}

int GetLineLength(ShapeGeneric* ret)
{
	int Len = ret->Shapes.sl.EndX - ret->Shapes.sl.StartX;
	Len += ret->Shapes.sl.EndY - ret->Shapes.sl.StartY;
	return Len;
}