#include "StdAfx.h"

ShapeGeneric *InputImage = NULL;
BYTE* ImageConverted = NULL;

void InitStatusStore(FIBITMAP* dib)
{
	if (InputImage != NULL)
		free(InputImage);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int MaxShapeCounts = Width * Height;
	InputImage = (ShapeGeneric*)malloc(MaxShapeCounts * sizeof(ShapeGeneric));

	if (ImageConverted)
		free(ImageConverted);
	ImageConverted = (BYTE*)malloc(MaxShapeCounts * sizeof(BYTE));
	memset(ImageConverted, 0, MaxShapeCounts * sizeof(BYTE));
}

BYTE* GetExtractedMap()
{
	return ImageConverted;
}

void CleanupStatusStore()
{
	if (InputImage != NULL)
		free(InputImage);
	InputImage = NULL;
	if (ImageConverted)
		free(ImageConverted);
	ImageConverted = NULL;
}