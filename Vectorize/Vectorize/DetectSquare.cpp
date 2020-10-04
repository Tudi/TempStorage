#include "StdAfx.h"

int DetectSquareAt(FIBITMAP* dib, int x, int y, ShapeGeneric *pret)
{
	//if this start point is already extracted, nothing to do at this position
	ShapeSquare* ret = &pret->Shapes.sq;
	pret->ShapeType = SHT_SQUARE;

	BYTE* ExtrMap = GetExtractedMap();

	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);

	if (ExtrMap[y * Width + x] != 0)
	{
		pret->ShapeType = SHT_INVALID;
		return 0;
	}

	//scan from left to right until a new color is found
	BYTE BaseB = Pixels[y * pitch + x * Bytespp + 0];
	BYTE BaseG = Pixels[y * pitch + x * Bytespp + 1];
	BYTE BaseR = Pixels[y * pitch + x * Bytespp + 2];
	ret->StartX = x;
	ret->StartY = y;
	ret->Width = 1;
	ret->Height = 1;
	pret->R = BaseR;
	pret->G = BaseG;
	pret->B = BaseB;

	while (y + ret->Height < Height && x + ret->Width < Width)
	{
		int curHeight = ret->Height;
		int curWidth = ret->Width;
		//can we add a new column ?
		int CanAddColumn = 1;
		for (int Row = 0; Row < curHeight; Row++)
			if (Pixels[(y + Row) * pitch + (x + curWidth) * 3 + 0] != BaseB
				|| Pixels[(y + Row) * pitch + (x + curWidth) * 3 + 1] != BaseG
				|| Pixels[(y + Row) * pitch + (x + curWidth) * 3 + 2] != BaseR
				|| ExtrMap[(y + Row) * Width + x + curWidth] != 0
				)
			{
				CanAddColumn = 0;
				break;
			}
		if (CanAddColumn != 1)
			break;
		int CanAddRow = 1;
		for (int Col = 0; Col < curWidth; Col++)
			if(Pixels[ (y + curHeight) * pitch + (x + Col) * 3 + 0] != BaseB
				|| Pixels[(y + curHeight) * pitch + (x + Col) * 3 + 1] != BaseG
				|| Pixels[(y + curHeight) * pitch + (x + Col) * 3 + 2] != BaseR
				|| ExtrMap[(y + curHeight) * Width + (x + Col)] != 0
				)
			{
				CanAddRow = 0;
				break;
			}
		if (CanAddRow != 1)
			break;
		//extra pixel on the diagonal
		if (Pixels[(y + curHeight) * pitch + (x +curWidth) * 3 + 0] != BaseB
			|| Pixels[(y + curHeight) * pitch + (x + curWidth) * 3 + 1] != BaseG
			|| Pixels[(y + curHeight) * pitch + (x + curWidth) * 3 + 2] != BaseR
			|| ExtrMap[(y + curHeight) * Width + (x + curWidth)] != 0
			)
			break;
		ret->Height++;
		ret->Width++;
		if (y + ret->Height == Height || x + ret->Width == Width)
			break;
	}
	return 0;
}

void MarkShapeSquareExtracted(FIBITMAP* dib, ShapeGeneric *pret)
{
	BYTE* ExtrMap = GetExtractedMap();

	int sx, sy, ex, ey;
	if (pret->ShapeType == SHT_SQUARE)
	{
		sx = pret->Shapes.sq.StartX;
		sy = pret->Shapes.sq.StartY;
		ex = sx + pret->Shapes.sq.Width;
		ey = sy + pret->Shapes.sq.Height;
	}
	else
		return;

	int Width = FreeImage_GetWidth(dib);
	for (int y = sy; y < ey; y++)
		for (int x = sx; x < ex; x++)
			ExtrMap[y * Width + x] = 1;
}

void PaintShapeSquareExtracted(FIBITMAP* dib, ShapeGeneric* pret, int R, int G, int B)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);

	int sx, sy, ex, ey;
	if (pret->ShapeType == SHT_SQUARE)
	{
		sx = pret->Shapes.sq.StartX;
		sy = pret->Shapes.sq.StartY;
		ex = sx + pret->Shapes.sq.Width;
		ey = sy + pret->Shapes.sq.Height;
	}
	else
		return;

	for (int y = sy; y < ey; y++)
		for (int x = sx; x < ex; x++)
		{
			Pixels[y * pitch + x * 3 + 0] = B;
			Pixels[y * pitch + x * 3 + 1] = G;
			Pixels[y * pitch + x * 3 + 2] = R;
		}
}