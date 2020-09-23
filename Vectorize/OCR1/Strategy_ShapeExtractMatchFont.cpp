#include "StdAfx.h"
#include "ShapeExtract.h"
#include "FontLib.h"
#include "FileWriter.h"

#define FONT_PIXEL_RATIO				10
#define ALLOWED_FONT_HEIGHT_DEVIATION	1

void ExtractShapesThanMatchFontsToShapes(FIBITMAP* dib, int BinarizationStrength)
{
	printf("Used binarization strength : 150\n");
	printf("Extracting characters with size : 8-30 px\n");
	//	OCR_ResizeFontsStaticSize(42, 42);

//	OCR_LoadFontsFromDir("FontsRescaled");
	OCR_BinarizeFonts( 200 );
	OCR_GenBlurredFonts(4);
#if defined( _DEBUG ) && 0
	OCR_SaveFontVisualDebug();
#endif // DEBUG

	//backup original image
	BYTE* IMG_Dup = ImgDup(dib);

	//binarize colors
	if (BinarizationStrength == -1)
		BinarizationStrength = 150;
	BinarizeImage(dib, BinarizationStrength); // limit 155 - 180 . 110 for orange font
#ifdef _DEBUG
	SaveImagePNG(dib, "Binarized.png");
#endif

	//extract shapes
	int IDMarker = 2;
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);
	int pitch = FreeImage_GetPitch(dib);

//	ErodeOutside(Pixels, Width, Height, pitch, 128);
#ifdef _DEBUG
//	SaveImagePNG(dib, "Eroded.png");
#endif

	std::list<FontExtracted*> FontShapes;
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			//check if we can detect a shape at this location
			FontExtracted* es = GetShapeBoundaries(dib, x, y, IDMarker);
			if (es == NULL)
				continue;

			IDMarker = (IDMarker * 0x007007007) & 0x00FFFFFF;

			//we expect fonts to have a pixel to size ratio of at least x%
			//chances are this is some sort of "line" / "shape"
			float ShapePixelCount = (float)(es->Width * es->Height);
			float PixelRatio = es->PixelCount * 100 / ShapePixelCount;
			if (PixelRatio < FONT_PIXEL_RATIO || es->Height < 8)
			{
				free(es);
				continue;
			}

			FontShapes.push_back(es);
		}
#ifdef TRY_TO_GUESS_FONT_SIZE
	//try to guess present font heights
	//numbers should have similar heights
	//labels will probably start with [x]. The brackets should have same size
	int HeightSum = 0;
	int HeightSumCount = 0;
	int MinHeight = 100;
	int MaxHeight = 0;
	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		int SimilarHeightedShapesCount = 0;
		int RefHeight = (*itr)->Height;
		int RefWidth = (*itr)->Width;
		if (RefHeight < 10)
			continue;
		if (RefHeight > 100)
			continue;
		for (auto itr2 = FontShapes.begin(); itr2 != FontShapes.end(); itr2++)
		{
			int CurHeight = (*itr2)->Height;
			int CurWidth = (*itr2)->Width;
			if (CurHeight - ALLOWED_FONT_HEIGHT_DEVIATION <= RefHeight && CurHeight + ALLOWED_FONT_HEIGHT_DEVIATION >= RefHeight
				&& CurWidth - ALLOWED_FONT_HEIGHT_DEVIATION <= RefWidth && CurWidth + ALLOWED_FONT_HEIGHT_DEVIATION >= RefWidth)
				SimilarHeightedShapesCount++;
		}
		if (SimilarHeightedShapesCount >= 2)
		{
			HeightSum += RefHeight;
			HeightSumCount++;
			if (MinHeight > RefHeight)
				MinHeight = RefHeight;
			if (MaxHeight < RefHeight)
				MaxHeight = RefHeight;
			//			printf("Possible font size : %dx%d. Found %d\n", RefHeight, RefWidth, SimilarHeightedShapesCount);
		}
		//		else
		//			printf("Non font siutable size : %dx%d\n", RefHeight, RefWidth);
	}
	int AvgSize = 0;
	if (HeightSumCount > 0)
		AvgSize = HeightSum / HeightSumCount;
	printf("FontGuessing : Avg shape size %d, min %d, max %d\n", AvgSize, MinHeight, MaxHeight);
	int AvgFontHeightOCR = OCR_GetNumbersAvgFontHeight();
	printf("FontLib : Avg number height %d\n", AvgFontHeightOCR);
	float RescaleTo = (float)AvgFontHeightOCR / (float)MinHeight;
	printf("Will try to upscale input image by %f\n", RescaleTo);
	int NewHeight = Height * RescaleTo;
	int NewWidth = Width * RescaleTo;
	FIBITMAP* NewImg = RescaleImg(dib, NewWidth, NewHeight);
#ifdef _DEBUG
	SaveImagePNG(NewImg, "Rescaled.png");
#endif
	BlurrImageToGrayScale(NewImg, 4);
#ifdef _DEBUG
	SaveImagePNG(NewImg, "RescaledBlurred.png");
#endif
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

//	RestoreContentFromDup(dib, IMG_Dup);

	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		FontExtracted* fe = *itr;
		if (fe->Pixels == NULL)
			CopySourceToShape(dib, fe);
		FontSimilarityScore* fss = GetBestMatchedFont(dib, fe);
		if (fss == NULL)
			continue;
		if (fss->fi == NULL)
		{
			free(fss);
			continue;
		}
#ifdef _DEBUG
		printf("At pos %dx%d found string %s\n", fe->x, fe->y, fss->fi->AssignedString);
#endif
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

	//try to merge consecutive shape texts that are from left to right
	int Progress = 0;
	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		FontExtracted* fe = *itr;
		fe->LastX = fe->x + fe->Width;
		fe->LastY = fe->y;
		fe->MergedNext = NULL;
		fe->MergedPrev = NULL;
	}
	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		FontExtracted* fe = *itr;
		if (fe->AssignedString == NULL)
			continue;
		//if we already used this shape
		if (fe->MergedNext != NULL || fe->MergedPrev != NULL)
			continue;
		//get all shapes on this row
		std::list<FontExtracted*> RowShapes;
		RowShapes.clear();
		for (auto itr2 = FontShapes.begin(); itr2 != FontShapes.end(); itr2++)
		{
			FontExtracted* fe2 = *itr2;
			if (fe2->AssignedString == NULL)
				continue;
			if (fe2->MergedNext != NULL || fe->MergedPrev != NULL)
				continue;
			if (abs(fe2->y - fe->y) > 10)
				continue;
			RowShapes.push_back(fe2);
		}

		//order shapes by x
		RowShapes.sort([](const FontExtracted* a, const FontExtracted* b) { return a->x < b->x; });

		//merge from left to right characters into strings
		FontExtracted* cur = NULL;
		for (auto itr2 = RowShapes.begin(); itr2 != RowShapes.end(); itr2++)
		{
			FontExtracted* next = *itr2;
			if (cur == NULL)
			{
				cur = next;
//				printf("%dx%d %s", next->x, next->y, next->AssignedString);
				continue;
			}
			if (!(cur->LastX - 5 > next->x || cur->LastX + 30 < next->x))
			{
				cur->MergedNext = next;
				next->MergedPrev = cur; //merged to self :P
				cur->LastX = next->LastX;
//				printf("%s", next->AssignedString);
			}
			else
			{
//				printf("\n"); //end if a string;
//				printf("%dx%d %s", next->x, next->y, next->AssignedString);
			}
			cur = next;
		}
//		printf("\n"); //end if a string;
	}
#ifdef _DEBUG
	for (auto itr = FontShapes.begin(); itr != FontShapes.end(); itr++)
	{
		FontExtracted* fe = *itr;
		if (fe->AssignedString == NULL)
			continue;
		if (fe->MergedPrev != NULL)
			continue;
		//if we already used this shape
		if (fe->MergedNext == NULL && fe->MergedPrev == NULL)
		{
			printf("At %dx%d, char : %s\n", fe->x, fe->y, fe->AssignedString);
			continue;
		}
		//print sequence of chars
		printf("At %dx%d, text : %s", fe->x, fe->y, fe->AssignedString);
		fe = fe->MergedNext;
		while (fe != NULL)
		{
			printf("%s", fe->AssignedString);
			fe = fe->MergedNext;
		}
		printf("\n");
	}
#endif
#endif

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

#ifdef _DEBUG
	SaveImagePNG(dib, "ShapesMarked.png");
#endif
	WriteCharsToFile(&FontShapes,"CharactersExtracted.txt");
	WriteTextToFileMerged(&FontShapes,"TextExtracted.txt");
}