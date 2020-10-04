#include "StdAfx.h"
#include "ShapeExtract.h"

int IsPixelBlack(BYTE *Pixels, int Height, int pitch, int x, int y)
{
	if (Pixels[y * pitch + x * 3 + 0] != 1 || Pixels[y * pitch + x * 3 + 1] != 1 || Pixels[y * pitch + x * 3 + 2] != 1)
		return 0;
	return 1;
}

void MarkPixelExtracted(BYTE* Pixels, int Height, int pitch, int x, int y, int IDMarker)
{
	Pixels[y * pitch + x * 3 + 0] = (IDMarker >> 0) & 0xFF;
	Pixels[y * pitch + x * 3 + 1] = (IDMarker >> 8) & 0xFF;
	Pixels[y * pitch + x * 3 + 2] = (IDMarker >> 16) & 0xFF;
}

#define PackCoord(x,y) ((((unsigned __int64)x)<<32)|((unsigned __int64)y))
#define UnPackCoord_x(xy) (int(((unsigned __int64)xy)>>32))
#define UnPackCoord_y(xy) ((int)xy)

FontExtracted* GetShapeBoundaries(FIBITMAP* dib, int x, int y, int IDMarker)
{
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);

	//we want to extract a not yet extracted shape
	if (IsPixelBlack(Pixels, Height, pitch, x, y) == 0)
		return NULL;

	if (IDMarker > 0x00FFFFFF)
	{
		printf("ID marker is too large : %d\n", IDMarker);
		return NULL;
	}

	struct Coord
	{
		int x, y;
	};
	std::list<unsigned __int64> EdgeCoords;
	int FoundNew = 0;
	int PixelCount = 1;
	//mark it as extracted
	MarkPixelExtracted(Pixels, Height, pitch, x, y, IDMarker);

	EdgeCoords.push_back(PackCoord(x,y));

	int MinX = x, MinY = y, MaxX = x, MaxY = y;
	do {
		FoundNew = 0;
		std::list<unsigned __int64> EdgeCoords2;
		for (auto itr = EdgeCoords.begin(); itr != EdgeCoords.end(); itr++)
		{
			int tx = UnPackCoord_x(*itr);
			int ty = UnPackCoord_y(*itr);
			//can we extend this pixel to neighbours ?
			for (int y1 = -1; y1 <= 1; y1++)
				for (int x1 = -1; x1 <= 1; x1++)
				{
					if (x1 == 0 && y1 == 0)
						continue;
					if (tx + x1 < 0)
						continue;
					if (tx + x1 >= Width)
						continue;
					if (ty + y1 < 0)
						continue;
					if (ty + y1 >= Height)
						continue;
					if (IsPixelBlack(Pixels, Height, pitch, tx + x1, ty + y1) == 1)
					{
						MarkPixelExtracted(Pixels, Height, pitch, tx + x1, ty + y1, IDMarker);
						EdgeCoords2.push_back(PackCoord(tx + x1, ty + y1));
						FoundNew = 1;
						if (tx + x1 < MinX)
							MinX = tx + x1;
						if (tx + x1 > MaxX)
							MaxX = tx + x1;
						if (ty + y1 < MinY)
							MinY = ty + y1;
						if (ty + y1 > MaxY)
							MaxY = ty + y1;
						PixelCount++;
					}
				}
		}
		EdgeCoords.clear();
		EdgeCoords = EdgeCoords2;
		EdgeCoords2.clear();
	} while (FoundNew == 1);

	FontExtracted* ret = (FontExtracted*)malloc(sizeof(FontExtracted));
	memset(ret, 0, sizeof(FontExtracted));
	ret->x = MinX;
	ret->y = MinY;
	ret->Width = MaxX - MinX + 1;
	ret->Height = MaxY - MinY + 1;
	ret->Pixels = NULL;
	ret->ShapeID = IDMarker;
	ret->PixelCount = PixelCount;
	ret->AssignedString = NULL;
	ret->fss = NULL;

	return ret;
}

void CopySourceToShape(FIBITMAP* dib, FontExtracted* sh)
{
	int pitch2 = sh->Width * Bytespp;
	BYTE* Pixels2;
	if (sh->Pixels != NULL)
		Pixels2 = sh->Pixels;
	else
	{
		sh->Pixels = (BYTE*)malloc(pitch2 * sh->Height);
		Pixels2 = sh->Pixels;
	}

	memset(Pixels2, 255, sh->Height * pitch2);

	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);
	for (int y = sh->y; y < sh->y + sh->Height; y++)
		for (int x = sh->x; x < sh->x + sh->Width; x++)
		{
			int RGBTri = *(int*)&Pixels[y * pitch + x * 3];
			RGBTri &= 0x00FFFFFF;

			if (RGBTri == sh->ShapeID)
			{
				int ty = (y - sh->y);
				int tx = (x - sh->x);
				Pixels2[ty * pitch2 + tx * 3 + 0] = 0;
				Pixels2[ty * pitch2 + tx * 3 + 1] = 0;
				Pixels2[ty * pitch2 + tx * 3 + 2] = 0;
			}
		}
}

void SaveShapeToPNGFile(FIBITMAP* dib, FontExtracted* sh, const char* FileName)
{
	FIBITMAP* dib2 = FreeImage_Allocate(sh->Width, sh->Height, 24);
	int pitch2 = FreeImage_GetPitch(dib2);
	BYTE* Pixels2 = FreeImage_GetBits(dib2);

	memset(Pixels2, 255, (sh->Height) * pitch2);
//	if (sh->Pixels == NULL)
	{
//		sh->Pixels = (BYTE*)malloc(sh->Width * sh->Height * 3);
		int Width = FreeImage_GetWidth(dib);
		int Height = FreeImage_GetHeight(dib);
		int pitch = FreeImage_GetPitch(dib);
		BYTE* Pixels = FreeImage_GetBits(dib);
		for (int y = sh->y; y < sh->y + sh->Height; y++)
			for (int x = sh->x; x < sh->x + sh->Width; x++)
			{
				int RGBTri = *(int*)&Pixels[y * pitch + x * 3];
				RGBTri &= 0x00FFFFFF;

				if (RGBTri == sh->ShapeID)
				{
					int ty = (y - sh->y);
					int tx = (x - sh->x);
					Pixels2[ty * pitch2 + tx * 3 + 0] = 0;
					Pixels2[ty * pitch2 + tx * 3 + 1] = 0;
					Pixels2[ty * pitch2 + tx * 3 + 2] = 0;
				}
			}
	}
	FreeImage_Save(FIF_PNG, dib2, FileName, 0);
}

FontExtracted* GetGradientShapeBoundaries(FIBITMAP* dib, int x, int y, int* ExtractedMap, int RGrad, int GGrad, int BGrad)
{
	static int IDMarker = 2;
	IDMarker = (IDMarker * 0x007007007) & 0x00FFFFFF;

	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);

	if (ExtractedMap[y * Width + x] != 0)
		return NULL;

	struct Coord
	{
		int x, y;
	};
	std::list<unsigned __int64> EdgeCoords;
	int FoundNew = 0;
	int PixelCount = 1;

	//mark it as extracted
	ExtractedMap[y * Width + x] = IDMarker;

	EdgeCoords.push_back(PackCoord(x, y));

	int MinX = x, MinY = y, MaxX = x, MaxY = y;
	do {
		FoundNew = 0;
		std::list<unsigned __int64> EdgeCoords2;
		for (auto itr = EdgeCoords.begin(); itr != EdgeCoords.end(); itr++)
		{
			int tx = UnPackCoord_x(*itr);
			int ty = UnPackCoord_y(*itr);
			int RefB = Pixels[(ty + 0) * pitch + (tx + 0) * Bytespp + 0];
			int RefG = Pixels[(ty + 0) * pitch + (tx + 0) * Bytespp + 1];
			int RefR = Pixels[(ty + 0) * pitch + (tx + 0) * Bytespp + 2];
			//can we extend this pixel to neighbours ?
			for (int y1 = -1; y1 <= 1; y1++)
				for (int x1 = -1; x1 <= 1; x1++)
				{
					if (x1 == 0 && y1 == 0)
						continue;
					if (tx + x1 < 0)
						continue;
					if (tx + x1 >= Width)
						continue;
					if (ty + y1 < 0)
						continue;
					if (ty + y1 >= Height)
						continue;
					if (ExtractedMap[(ty + y1) * Width + (tx + x1)] != 0)
						continue;
					//does it have a gradient we can accept as continuation ?
					int CurB = Pixels[(ty + y1) * pitch + (tx + x1) * Bytespp + 0];
					int CurG = Pixels[(ty + y1) * pitch + (tx + x1) * Bytespp + 0];
					int CurR = Pixels[(ty + y1) * pitch + (tx + x1) * Bytespp + 0];
					if (abs(RefB - CurB) > RGrad)
						continue;
					if (abs(RefG - CurG) > GGrad)
						continue;
					if (abs(RefR - CurR) > BGrad)
						continue;
					ExtractedMap[(ty + y1) * Width + (tx + x1)] = IDMarker;
					EdgeCoords2.push_back(PackCoord(tx + x1, ty + y1));
					FoundNew = 1;
					if (tx + x1 < MinX)
						MinX = tx + x1;
					if (tx + x1 > MaxX)
						MaxX = tx + x1;
					if (ty + y1 < MinY)
						MinY = ty + y1;
					if (ty + y1 > MaxY)
						MaxY = ty + y1;
					PixelCount++;
				}
		}
		EdgeCoords.clear();
		EdgeCoords = EdgeCoords2;
		EdgeCoords2.clear();
	} while (FoundNew == 1);

	FontExtracted* ret = (FontExtracted*)malloc(sizeof(FontExtracted));
	memset(ret, 0, sizeof(FontExtracted));
	ret->x = MinX;
	ret->y = MinY;
	ret->Width = MaxX - MinX + 1;
	ret->Height = MaxY - MinY + 1;
	ret->Pixels = NULL;
	ret->ShapeID = IDMarker;
	ret->PixelCount = PixelCount;
	ret->AssignedString = NULL;

	return ret;
}

void CopySourceToShape(FIBITMAP* dib, FontExtracted* sh, int* ExtractedMap)
{
	int pitch2 = sh->Width * Bytespp;
	BYTE* Pixels2;
	if (sh->Pixels != NULL)
		Pixels2 = sh->Pixels;
	else
	{
		sh->Pixels = (BYTE*)malloc(pitch2 * sh->Height);
		Pixels2 = sh->Pixels;
	}

	memset(Pixels2, 255, sh->Height * pitch2);

	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);
	for (int y = sh->y; y < sh->y + sh->Height; y++)
		for (int x = sh->x; x < sh->x + sh->Width; x++)
		{
			if (ExtractedMap[y * Width + x] == sh->ShapeID)
			{
				int ty = (y - sh->y);
				int tx = (x - sh->x);
				Pixels2[ty * pitch2 + tx * Bytespp + 0] = Pixels[y * pitch + x * Bytespp + 0];
				Pixels2[ty * pitch2 + tx * Bytespp + 1] = Pixels[y * pitch + x * Bytespp + 1];
				Pixels2[ty * pitch2 + tx * Bytespp + 2] = Pixels[y * pitch + x * Bytespp + 2];
			}
		}
}