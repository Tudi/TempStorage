#include "StdAfx.h"
#include "Strategy_ShapeExtractMatchFont.h"
#include "Strategy_ResizeFontSearchImage.h"
#include "Strategy_ShapeExtractMatchOriginalToFont.h"
#include "FontLib.h"

void TrySimplifyImage(FIBITMAP* dib)
{
}

void ProcessImage(FIBITMAP* dib, int BinarizationStrength)
{
	ExtractShapesThanMatchFontsToShapes(dib, BinarizationStrength);
//	ResizeFontsMatchOnImage(dib);
//	ExtractShapesThanMatchFontsToOriginal(dib);
}

int main(int argc, char** argv)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
//	const char* InputFileName = "../FilesReceived/311-010000007001-B.PNG";
	const char* InputFileName = "../FilesReceived/41-14-08-252-024.PNG";
	//	const char* InputFileName = "../FilesReceived/6725714110030.PNG";
//	const char* InputFileName = "../FilesReceived/6a (377491).PNG";
//	const char* InputFileName = "Fonts_Hairline.png";
//	const char* InputFileName = "FontLib/A 8.png";
//	const char* InputFileName = "JustACorner.png";
	if (argc == 1)
		printf("Usage: ocr1.exe <inputFilename> [BinarizationStrength]");
	if (argc > 1)
		InputFileName = argv[1];
	int BinarizationStrength = -1;
	if (argc > 2)
		BinarizationStrength = atoi(argv[2]);
	dib = LoadImage_(InputFileName);

	if (dib == NULL)
	{
		printf("Could not open input file\n");
		return 1;
	}

	int bpp = FreeImage_GetBPP(dib);
	if (bpp != 24)
		dib = FreeImage_ConvertTo24Bits(dib);
	bpp = FreeImage_GetBPP(dib);
	if (bpp != 24)
	{
		printf("!!!!Only support 24 bpp input. Upgrade software or convert input from %d to 24\n", bpp);
		return 1;
	}

	OCR_LoadFontsFromDir("FontLib");
	//OCR_GenMultiScaleFonts();

	InitStatusStore(dib);

	TrySimplifyImage(dib);

	ProcessImage(dib, BinarizationStrength);

	// free the dib
	FreeImage_Unload(dib);

	FreeImage_DeInitialise();

	return 0;
}