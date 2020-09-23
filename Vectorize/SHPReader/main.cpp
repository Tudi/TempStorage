#include "StdAfx.h"
#include "FileReader.h"

int main(int argc, char** argv)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
	const char* InputFileName = "311-010000007001-B.PNG.shp";
	if (argc > 1)
		InputFileName = argv[1];

	//read the shapes from the input file
	SHPFileDetails* shpd = ReadShapesFromFile(InputFileName);
	if (shpd == NULL)
	{
		printf("Could not read input file\n");
		return 1;
	}

	//create an image
	FreeImage_Initialise();
	dib = FreeImage_Allocate(shpd->Width, shpd->Height, 24);

	//paint background
	int pitch = FreeImage_GetPitch(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);
	for (int y = 0; y < shpd->Height; y++)
		for (int x = 0; x < shpd->Width; x++)
		{
			Pixels[y * pitch + x * 3 + 0] = shpd->Back_B;
			Pixels[y * pitch + x * 3 + 1] = shpd->Back_G;
			Pixels[y * pitch + x * 3 + 2] = shpd->Back_R;
		}

	//paint the shapes
	for (auto itr = shpd->shapes->begin(); itr != shpd->shapes->end(); itr++)
	{
		ShapeGeneric* sg = (*itr);
		if (sg->ShapeType == SHT_SQUARE)
			PaintShapeSquareExtracted(dib, sg, sg->R, sg->G, sg->B);
		else if (sg->ShapeType == SHT_LINE)
			PaintShapeLineExtracted(dib, sg, sg->R, sg->G, sg->B);
	}

	char NewFileName[_MAX_PATH];
	sprintf_s(NewFileName, sizeof(NewFileName),"%s.png", InputFileName);
	FreeImage_Save(FIF_PNG, dib, NewFileName, 0);
	return 0;
}