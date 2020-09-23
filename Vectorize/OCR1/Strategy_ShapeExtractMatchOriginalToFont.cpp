#include "StdAfx.h"
#include "ShapeExtract.h"
#include "FontLib.h"

#define FONT_PIXEL_RATIO				10
#define ALLOWED_FONT_HEIGHT_DEVIATION	1

void ExtractShapesThanMatchFontsToOriginal(FIBITMAP* dib)
{
	BYTE* IMG_Dup = ImgDup(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);

//	OCR_GenBlurredFonts(4);
	//binarize colors
//	BinarizeImage(dib, 180);
#ifdef _DEBUG
//	SaveImagePNG(dib, "Binarized.png");
#endif

	//convert image to YUV
/*	{
		int Width = FreeImage_GetWidth(dib);
		int Height = FreeImage_GetHeight(dib);
		for (int y = 0; y < Height; y++)
			for (int x = 0; x < Width; x++)
			{
				float b = Pixels[y * pitch + x * Bytespp + 0];
				float g = Pixels[y * pitch + x * Bytespp + 1];
				float r = Pixels[y * pitch + x * Bytespp + 2];
				float yc = 0.299 * r + 0.587 * g + 0.114 * b;
				if (yc > 255)
					yc = 255;
				Pixels[y * pitch + x * Bytespp + 0] = yc;
				Pixels[y * pitch + x * Bytespp + 1] = yc;
				Pixels[y * pitch + x * Bytespp + 2] = yc;
			}
		FreeImage_Save(FIF_PNG, dib, "YUVImg.png", 0);
		//restore original image
//		memcpy(Pixels, IMG_Dup, Height * pitch);
	}/**/
/*	{
		int Width = FreeImage_GetWidth(dib);
		int Height = FreeImage_GetHeight(dib);
		for (int y = 0; y < Height; y++)
			for (int x = 0; x < Width; x++)
			{
				int b = Pixels[y * pitch + x * Bytespp + 0];
				int g = Pixels[y * pitch + x * Bytespp + 1];
				int r = Pixels[y * pitch + x * Bytespp + 2];
				int yc = (r + g + b) / 3;
				if (yc > 255)
					yc = 255;
				Pixels[y * pitch + x * Bytespp + 0] = yc;
				Pixels[y * pitch + x * Bytespp + 1] = yc;
				Pixels[y * pitch + x * Bytespp + 2] = yc;
			}
		FreeImage_Save(FIF_PNG, dib, "GrayScale.png", 0);
		//restore original image
//		memcpy(Pixels, IMG_Dup, Height * pitch);
	}/**/
	{
		int Width = FreeImage_GetWidth(dib);
		int Height = FreeImage_GetHeight(dib);
		for (int y = 0; y < Height; y++)
			for (int x = 0; x < Width; x++)
			{
				int b = Pixels[y * pitch + x * Bytespp + 0];
				int g = Pixels[y * pitch + x * Bytespp + 1];
				int r = Pixels[y * pitch + x * Bytespp + 2];
				int yc = MAX(MAX(r,g),b);
				if (yc > 255)
					yc = 255;
				Pixels[y * pitch + x * Bytespp + 0] = yc;
				Pixels[y * pitch + x * Bytespp + 1] = yc;
				Pixels[y * pitch + x * Bytespp + 2] = yc;
			}
		FreeImage_Save(FIF_PNG, dib, "GrayScaleMax.png", 0);
		//restore original image
//		memcpy(Pixels, IMG_Dup, Height * pitch);
	}/**/
	 
	 //mark extracted shapes with this color
//	RemoveNoColor(dib, 1, 1, 1);

	//extract shapes
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int* ExtractMap = (int*)malloc(Width * Height * sizeof(int));
	memset(ExtractMap, 0, Width * Height * sizeof(int));
	std::list<FontExtracted*> FontShapes;
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			//check if we can detect a shape at this location
			FontExtracted* es = GetGradientShapeBoundaries(dib, x, y, ExtractMap, 25, 25, 25);
			if (es == NULL)
				continue;

			//we expect fonts to have a pixel to size ratio of at least x%
			//chances are this is some sort of "line" / "shape"
			float ShapePixelCount = (float)(es->Width * es->Height);
			float PixelRatio = es->PixelCount * 100 / ShapePixelCount;
			if (PixelRatio < FONT_PIXEL_RATIO)
			{
				free(es);
				continue;
			}

			FontShapes.push_back(es);
		}

#define MARK_SHAPES_ON_INPUTIMAGE
#ifdef MARK_SHAPES_ON_INPUTIMAGE
	//mark shapes on original image
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
			*(int*)&Pixels[y * pitch + x * Bytespp] = ExtractMap[y * Width + x];
	FreeImage_Save(FIF_PNG, dib, "MarkedShapes.png", 0);
	//restore original image
	memcpy(Pixels, IMG_Dup, Height * pitch);
#endif

#define TryToMatchShapesToFonts
#ifdef TryToMatchShapesToFonts

#define DUMP_GUESSED_CHARS_TO_NEW_IMG
#ifdef DUMP_GUESSED_CHARS_TO_NEW_IMG
	FIBITMAP* dib2 = FreeImage_Allocate(Width, Height, 24);
	int pitch2 = FreeImage_GetPitch(dib2);
	BYTE* Pixels2 = FreeImage_GetBits(dib2);
	memset(Pixels2, 255, Height * pitch2);
#endif

	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		FontExtracted* fe = *itr;
		if (fe->Pixels == NULL)
			CopySourceToShape(dib, fe);
		FontSimilarityScore* fss = GetBestMatchedFont(dib,fe);
		if (fss == NULL)
			continue;
		if (fss->fi == NULL)
		{
			free(fss);
			continue;
		}
		//printf("At pos %dx%d found string %s\n", fe->x, fe->y, fss->fi->AssignedString);
		fe->AssignedString = _strdup(fss->fi->AssignedString);
#ifdef DUMP_GUESSED_CHARS_TO_NEW_IMG
		BYTE* ScaledFont = RescaleImg(fss->fi->Pixels, fss->fi->Width, fss->fi->Height, fss->fi->pitch, fe->Width, fe->Height);
		for (int ty = 0; ty < fe->Height; ty++)
			for (int tx = 0; tx < fe->Width; tx++)
			{
				Pixels2[(fe->y + ty) * pitch2 + (fe->x + tx) * Bytespp + 0] = ScaledFont[ty * fe->Width * Bytespp + tx * Bytespp + 0];
				Pixels2[(fe->y + ty) * pitch2 + (fe->x + tx) * Bytespp + 1] = ScaledFont[ty * fe->Width * Bytespp + tx * Bytespp + 1];
				Pixels2[(fe->y + ty) * pitch2 + (fe->x + tx) * Bytespp + 2] = ScaledFont[ty * fe->Width * Bytespp + tx * Bytespp + 2];
			}
#endif
		free(fss);
	}
#ifdef DUMP_GUESSED_CHARS_TO_NEW_IMG
	FreeImage_Save(FIF_PNG, dib2, "guessedFonts.png", 0);
#endif
#endif
/*
#ifdef _DEBUG
	//save all unrecognized shapes to library
	int FileNameIndex = 1;
	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		char FileName[_MAX_PATH];
		sprintf_s(FileName, sizeof(FileName), "./UnrecognizedFonts/%d.png", FileNameIndex);
		SaveShapeToPNGFile(dib, *itr, FileName);
		FileNameIndex++;
	}
#endif
*/
#ifdef _DEBUG
	SaveImagePNG(dib, "ShapesMarked.png");
#endif
	//select shapes with similar sizes and with box like size ratio

}