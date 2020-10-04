#include "StdAfx.h"

void TestCanChangeImg(FIBITMAP* dib)
{
	int bpp = FreeImage_GetBPP(dib);
	int pitch = FreeImage_GetPitch(dib);
	// try to mark the image
	BYTE* BITS = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	for (int i = 0; i < MIN(Width, Height) / 2; i++)
	{
		/*		RGBQUAD c;
				c.rgbBlue = 0;
				c.rgbRed = 0;
				c.rgbGreen = 0;
				FreeImage_SetPixelColor(dib, i, i, &c);*/
		BITS[i * pitch + i * Bytespp + 0] = 0;
		BITS[i * pitch + i * Bytespp + 1] = 0;
		BITS[i * pitch + i * Bytespp + 2] = i;
	}

	// save the file as PNG
	bool bSuccess = SaveImagePNG(dib, "t.png");
}

void TrySimplifyImage(FIBITMAP* dib)
{
	SnapColorToDominantSimilar(dib);
#ifdef _DEBUG
	PaintShapes(dib, "SnapToSimilar.png");
#endif
	RemoveGradient(dib);
#ifdef _DEBUG
	PaintShapes(dib, "Nograd.png");
#endif
	ReduceColorDepth(dib, 3);
#ifdef _DEBUG
	PaintShapes(dib, "FewColors.png");
#endif
	Errode(dib, 1);
#ifdef _DEBUG
	PaintShapes(dib, "Erode.png");
#endif
	//SnapSmallPixelsToLines(dib);
	RemoveNoColor(dib, 0, 0, 0);
	//	RemoveExtraPixels(dib);
	MarkColorAsExtracted(dib, 255, 255, 255);
	//	MarkColorAsExtracted(dib, 250, 250, 250); // this is just so i can focus on what is important
#ifdef _DEBUG
	PaintShapes(dib, "t1.png");
#endif
}
/*
void EstimateAvgLineWidth(FIBITMAP* dib)
{
	//binarize image
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* NewPixels = (BYTE*)malloc(Height * pitch);
	memcpy(NewPixels, Pixels, Height * pitch);
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			if (Pixels[y * pitch + x * 3 + 0] > 255 / 2
				|| Pixels[y * pitch + x * 3 + 1] > 255 / 2
				|| Pixels[y * pitch + x * 3 + 2] > 255 / 2)
			{
				Pixels[y * pitch + x * 3 + 0] = 255
			}
		}
	free(NewPixels);
}*/

void ParseImgSquareFirst(FIBITMAP* dib)
{
	int ShapesExtracted = 0;
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			ShapeGeneric gs;
			DetectSquareAt(dib, x, y, &gs);
			if (gs.ShapeType == SHT_INVALID)
				continue;
			if (gs.ShapeType == SHT_SQUARE)
				MarkShapeSquareExtracted(dib, &gs);
			AddShapeCopyFrom(&gs);
			if (gs.ShapeType == SHT_SQUARE)
				x += gs.Shapes.sq.Width - 1;
			ShapesExtracted++;
		}
#ifdef _DEBUG
	PaintShapes(dib, "t2.png");
#endif

	printf("Initial shape count : %d\n", ShapesExtracted);
	MergeSquaresToLines(dib);
#ifdef _DEBUG
	PaintShapes(dib, "t3.png");
#endif

	printf("Merge1 shape count : %d\n", GetShapeCount());
}

void ParseImgLinesFirst(FIBITMAP* dib)
{
	//detect longest lines. Needs to have a minimum to avoid detecting small junk
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
/*	{
		ShapeGeneric tgs;
		//			ShapeGeneric *gs = (ShapeGeneric*)malloc(sizeof(ShapeGeneric));
		ShapeGeneric* gs = &tgs;
		memset(gs, 0, sizeof(ShapeGeneric));
		DetectLineAtVertHor(dib, 1083, 836-617, gs);
	}*/
//	ShapeGeneric** ExtractedShapesMap = (ShapeGeneric**)malloc(Width * Height * sizeof(ShapeGeneric*));
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			ShapeGeneric tgs;
//			ShapeGeneric *gs = (ShapeGeneric*)malloc(sizeof(ShapeGeneric));
			ShapeGeneric* gs = &tgs;
			memset(gs, 0, sizeof(ShapeGeneric));
			DetectLineAtVertHor(dib, x, y, gs);
			if (gs->ShapeType == SHT_INVALID)
				continue;
			if (gs->ShapeType == SHT_LINE)
			{
				MarkShapeLineExtracted(dib, gs);
			}
			AddShapeCopyFrom(gs);
			if (gs->ShapeType == SHT_LINE && gs->Shapes.sl.Angle == 0)
				x += (gs->Shapes.sl.EndX - gs->Shapes.sl.StartX);
		}
	printf("Extracted shape count : %d\n", GetShapeCount());
#ifdef _DEBUG
	PaintShapes(dib, "t3.png");
#endif
	//detect long parallel lines
	const int MinLineLength = 50;
	std::list<ShapeGeneric*>* shapes = GetExractedShapes();
	for (auto itr = shapes->begin(); itr != shapes->end(); itr++)
	{
		//we only merge lines
		if ((*itr)->ShapeType != SHT_LINE)
			continue;
		//if too short, skip it
		if (GetLineLength((*itr)) < MinLineLength)
			continue;
		ShapeLine* l1 = &(*itr)->Shapes.sl;
		auto itr2 = itr;
		for (itr2++; itr2 != shapes->end(); itr2++)
		{
			if ((*itr)->ShapeType != SHT_LINE)
				continue;
			//if too short, skip it
			if (GetLineLength((*itr)) < MinLineLength)
				continue;
			//is it a vertical parallel line
			ShapeLine* l2 = &(*itr2)->Shapes.sl;
			//the 2 lines start/end about at the same location
			if (l2->StartX - 5 < l1->StartX && l2->StartX + 5 > l1->StartX
				&& l2->StartY - 5 < l1->StartY && l2->StartY + 5 > l1->StartY
				&& l2->EndX - 5 < l1->EndX && l2->EndX + 5 > l1->EndX
				&& l2->EndY - 5 < l1->EndY && l2->EndY + 5 > l1->EndY)
				(*itr2)->ShapeType = SHT_INVALID;
		}
	}
}

int main(int argc, char **argv)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
	const char* InputFileName = "../FilesReceived/R367968.jpg";
	if (argc > 1)
		InputFileName = argv[1];
	dib = LoadImage(InputFileName);

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
	//TestCanChangeImg(dib); return 0;

	InitStatusStore(dib);

	TrySimplifyImage(dib);

//	ParseImgSquareFirst(dib);
	ParseImgLinesFirst(dib);

	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	printf("Image pixel count : %d\n", Width * Height);
	printf("Image byte count : %d\n", Width * Height * 3);

	// free the dib
	FreeImage_Unload(dib);

	FreeImage_DeInitialise();

	char OutFileName[_MAX_PATH];
	sprintf_s(OutFileName, sizeof(OutFileName), "%s.shp", InputFileName);
	WriteShapesToFile(OutFileName, Width, Height);

	return 0;
}