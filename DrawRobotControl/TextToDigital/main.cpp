#include "StdAfx.h"

int main()
{
	const char* fileName = "Nemo.jpg"; // should be the vertical lines image
	FIBITMAP *img = LoadImage_(fileName);
	if (img == NULL)
	{
		printf("Failed to load input image\n");
		return -2;
	}

	if (FreeImage_GetBPP(img) != 24)
	{
		printf("Convert image 1 to 24 bpp \n");
		img = FreeImage_ConvertTo24Bits(img);
	}

	printf("Binarize image \n");
	BinarizeImage(img);

//	ProcessInputUsingSimpleWeights(img);
//	ProcessInputUsingWeights(img);
	ProcessInputUsingLines(img);

	FreeImage_Unload(img);

	// Deinitialize FreeImage library
	FreeImage_DeInitialise();

	return 0;
}