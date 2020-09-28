#pragma once

struct FIBITMAP;

struct FontImg
{
	FIBITMAP* dib;
	int LoadedFromFile;
	float Angle;
	float Scale;
	int Width, Height, pitch;
	char *AssignedString;
	int UID;
	BYTE* Pixels;
	BYTE* PixelsBlurred;
};

struct FontSimilarityScore
{
	__int64 SAD;
	FontImg* fi;
	int AdjustedX;
	int Adjustedy;
};

struct ShapeMorphStatus
{
	int Distances;
	int PixelsMigrated;
};

struct FontExtracted;

void OCR_LoadFontsFromDir(const char* Path);
void OCR_LoadFont(const char* Path, const char* AssignedString);
void OCR_GenMultiScaleFonts();
void OCR_GenBlurredFonts(int KernelSize);
void OCR_SaveFontVisualDebug();
int OCR_GetNumbersAvgFontHeight();
FontSimilarityScore* GetBestMatchedFont(FIBITMAP* dib, FontExtracted*f);
FontSimilarityScore* GetBestMatchedFontAtArea(FIBITMAP* dib, int x, int y, int SearchAreaSize);
FontImg* GetFontImgForChar(char*);
//in order for SAD to give reliable values without normalization, we should have all fonts have same width / height
void OCR_ResizeFontsStaticSize(int Width, int Height);
//remove grayscale and force them all to be black and white
void OCR_BinarizeFonts(int ToBlackAnythingBelow);
//how much do pixels need to travel to find their place in destination locations ?
void OCR_GetMorphCost(BYTE *Pixels1, int Width, int Height, int pitch, BYTE *Pixels2, int pitch2, ShapeMorphStatus *ret);
void OCR_GenRotatedFonts();