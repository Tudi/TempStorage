#include "StdAfx.h"

#if 0
FIBITMAP* openImage(const char* fileName)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
	const char* InputFileName = fileName;
	if (InputFileName == NULL)
	{
		return NULL;
	}
	dib = LoadImage_(InputFileName);

	if (dib == NULL)
	{
		printf("Could not open input file %s\n", InputFileName);
		return NULL;
	}

	int bpp = FreeImage_GetBPP(dib);
	if (bpp != Bytespp * 8)
		dib = FreeImage_ConvertTo24Bits(dib);
	bpp = FreeImage_GetBPP(dib);
	if (bpp != Bytespp * 8)
	{
		printf("!!!!Only support 24 bpp input. Upgrade software or convert input from %d to 24\n", bpp);
		return NULL;
	}

	return dib;
}
#endif

int main(int argc, char** argv)
{
	LoadImage_("NONE"); // initialize lib

	RunAllTests(argv, argc);

	FreeImage_DeInitialise();
}