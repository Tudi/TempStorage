#include "StdAfx.h"

static int ImageLibInitialized = 0;

FIBITMAP* GenericLoader(const char* lpszPathName, int flag)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if (fif == FIF_UNKNOWN) {
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		// ok, let's load the file
		FIBITMAP* dib = FreeImage_Load(fif, lpszPathName, flag);
		// unless a bad file format, we are done !
		return dib;
	}
	return NULL;
}

bool GenericWriter(FIBITMAP* dib, const char* lpszPathName, int flag)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	BOOL bSuccess = FALSE;

	if (dib) {
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
		if (fif != FIF_UNKNOWN) {
			// check that the plugin has sufficient writing and export capabilities ...
			WORD bpp = FreeImage_GetBPP(dib);
			if (FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp)) {
				// ok, we can save the file
				bSuccess = FreeImage_Save(fif, dib, lpszPathName, flag);
				// unless an abnormal bug, we are done !
			}
		}
	}
	return (bSuccess == TRUE) ? true : false;
}

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char* message)
{
	printf("\n*** ");
	if (fif != FIF_UNKNOWN) {
		printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
	}
	printf(message);
	printf(" ***\n");
}

void InitLib()
{
	if (ImageLibInitialized != 0)
		return;
	ImageLibInitialized = 1;

	FreeImage_Initialise();
	FreeImage_SetOutputMessage(FreeImageErrorHandler);
}

FIBITMAP* LoadImage_(const char* FileName)
{
	InitLib();
	return GenericLoader(FileName, 0);
}

bool SaveImagePNG(FIBITMAP *dib, const char* FileName)
{
	return GenericWriter(dib, FileName, PNG_DEFAULT);
}

void SaveImagePNG(BYTE* Pixels, int Width, int Height, int pitch, const char* FileName)
{
	FIBITMAP* dib2 = FreeImage_Allocate(Width, Height, 24);
	int pitch2 = FreeImage_GetPitch(dib2);
	BYTE* Pixels2 = FreeImage_GetBits(dib2);
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			Pixels2[y * pitch2 + x * 3 + 0] = Pixels[y * pitch + x * 3 + 0];
			Pixels2[y * pitch2 + x * 3 + 1] = Pixels[y * pitch + x * 3 + 1];
			Pixels2[y * pitch2 + x * 3 + 2] = Pixels[y * pitch + x * 3 + 2];
		}
	FreeImage_Save(FIF_PNG, dib2, FileName, 0);
}