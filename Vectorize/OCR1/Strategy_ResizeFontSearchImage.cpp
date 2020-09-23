#include "StdAfx.h"
#include "ShapeExtract.h"
#include "FontLib.h"

#define FONT_PIXEL_RATIO				10
#define ALLOWED_FONT_HEIGHT_DEVIATION	1

void ResizeFontsMatchOnImage(FIBITMAP* dib)
{
//	int FontHeight = OCR_GetNumbersAvgFontHeight();
//	OCR_GenMultiScaleFonts();

	OCR_LoadFontsFromDir("FontsRescaled");

	printf("Max accepted for is 36. If image contains fonts with height larger than 34 pixels, please resize input image. See directory 'FontLib' for searched fonts\n");

	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);
	int pitch = FreeImage_GetPitch(dib);
	const int SearchAreaSize = 0; // number of pixels left / right to extract a font
	std::list< FontSimilarityScore*> ExtractedFonts;
	int TotalPixelCount = Width * Height;
	for(int y=0;y<Height;y++)
		for (int x = 0; x < Width; x++)
//	for (int y = 121; y < 218; y++)
//		for (int x = 694; x < 807; x++)
		{
			FontSimilarityScore*fss= GetBestMatchedFontAtArea(dib, x, y, SearchAreaSize);
			if (fss == NULL)
				continue;
			if (fss->fi == NULL)
			{
				free(fss);
				continue;
			}
			//do we accept that we recognized a character here ?
			if (!(fss->SAD < 256 / 10 * 1000000))
			{
				free(fss);
				continue;
			}
			ExtractedFonts.push_back(fss);
			//mark the pixels of this font as extracted

			int ProgressPCT = (y * Width + x) * 100 / TotalPixelCount;
			printf("\rProgress : %d%%", ProgressPCT);
		}
	//create a new image and mark extracted fonts
#define DUMP_GUESSED_CHARS_TO_NEW_IMG
#ifdef DUMP_GUESSED_CHARS_TO_NEW_IMG
	FIBITMAP* dib2 = FreeImage_Allocate(Width, Height, 24);
	int pitch2 = FreeImage_GetPitch(dib2);
	BYTE* Pixels2 = FreeImage_GetBits(dib2);
	memset(Pixels2, 255, Height * pitch2);
/*	for (int y = 121; y < 218; y++)
		for (int x = 694; x < 807; x++)
		{
			int NewB = Pixels[y * pitch + x * Bytespp + 0];
			int NewG = Pixels[y * pitch + x * Bytespp + 1];
			int NewR = Pixels[y * pitch + x * Bytespp + 2];
			Pixels2[y * pitch2 + x * Bytespp + 0] = NewB / 2;
			Pixels2[y * pitch2 + x * Bytespp + 1] = NewG / 2;
			Pixels2[y * pitch2 + x * Bytespp + 2] = NewR / 2;
		} */
	for (auto itr = ExtractedFonts.begin(); itr != ExtractedFonts.end(); itr++)
	{
		FontSimilarityScore* fss = *itr;
		FontImg* fi = fss->fi;
		for (int ty = 0; ty < fi->Height; ty++)
			for (int tx = 0; tx < fi->Width; tx++)
			{
				int NewB = Pixels2[(fss->Adjustedy + ty) * pitch2 + (fss->AdjustedX + tx) * Bytespp + 0] + fi->Pixels[ty * fi->pitch + tx * Bytespp + 0];
				int NewG = Pixels2[(fss->Adjustedy + ty) * pitch2 + (fss->AdjustedX + tx) * Bytespp + 1] + fi->Pixels[ty * fi->pitch + tx * Bytespp + 1];
				int NewR = Pixels2[(fss->Adjustedy + ty) * pitch2 + (fss->AdjustedX + tx) * Bytespp + 2] + fi->Pixels[ty * fi->pitch + tx * Bytespp + 2];
				Pixels2[(fss->Adjustedy + ty) * pitch2 + (fss->AdjustedX + tx) * Bytespp + 0] = NewB / 2;
				Pixels2[(fss->Adjustedy + ty) * pitch2 + (fss->AdjustedX + tx) * Bytespp + 1] = NewG / 2;
				Pixels2[(fss->Adjustedy + ty) * pitch2 + (fss->AdjustedX + tx) * Bytespp + 2] = NewR / 2;
			}
	}
	FreeImage_Save(FIF_PNG, dib2, "guessedFonts.png", 0);
#endif
}