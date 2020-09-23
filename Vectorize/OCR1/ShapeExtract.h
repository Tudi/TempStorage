#pragma once

struct FontExtracted
{
	BYTE* Pixels;
	int x, y;
	int Width, Height;
	int ShapeID;
	int PixelCount;
	char* AssignedString;
	FontExtracted *MergedNext;
	FontExtracted* MergedPrev;
	int LastX, LastY; // when merged multiple characters check if we can add another one to the last
};

FontExtracted* GetShapeBoundaries(FIBITMAP* dib, int x, int y, int IDMarker);
FontExtracted* GetGradientShapeBoundaries(FIBITMAP* dib, int x, int y, int *ExtractedMap, int RGrad, int GGrad, int BGrad);
void CopySourceToShape(FIBITMAP* dib, FontExtracted* sh);
void CopySourceToShape(FIBITMAP* dib, FontExtracted* sh, int* ExtractedMap);
void SaveShapeToPNGFile(FIBITMAP* dib, FontExtracted* sh, const char* FileName);