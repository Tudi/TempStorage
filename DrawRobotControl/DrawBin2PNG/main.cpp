#include "StdAfx.h"

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

#define ImageSizeMultiplier 2
int main()
{
	LoadImage_("NONE"); // initialize lib

	FIBITMAP* dib = CreateNewImage(2048 * ImageSizeMultiplier,2048 * ImageSizeMultiplier);
	float x = 2048 * ImageSizeMultiplier / 2;
	float y = 2048 * ImageSizeMultiplier / 2;

	DrawCircleAt(dib, x, y, 300);

	uint32_t readPos = 0;
	size_t fileSize = 0;
	RobotCommand prevCommand;
	RobotCommand_Constructor(&prevCommand);
	PenRobotMovementCodesPrimary prevDirection = Move1_Down; // should be derived from header

//	uint8_t* f = OpenBinFile("../BinFiles/0002 Three Half Inch Vertical Lines Half Inch Apart.bin", readPos, fileSize);
//	uint8_t* f = OpenBinFile("../BinFiles/0004 Three Half Inch Horizontal Lines Half Inch Apart.bin", readPos, fileSize);
	uint8_t* f = OpenBinFile("../BinFiles/0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.bin", readPos, fileSize);
//	uint8_t* f = OpenBinFile("../ConstructBinFiles/BinFiles/CheckArmAngle_11_0_20_40_60_80.bin", readPos, fileSize);
	ReadBinHeader(f, readPos, &prevCommand);

	for (size_t i = 0; i < 6; i++)
	{
		float* line = NULL;
		int ret = ReadBinLine(f, readPos, fileSize, &line, &prevCommand, &prevDirection);
		if (ret != 0)
		{
			free(line);
			break;
		}

		// draw line on PNG
		DrawBinLineOnPNG(dib, x, y, line);

		free(line);
	}
	ReadBinFooter(f, readPos, &prevCommand);

	free(f);

	SaveImagePNG(dib, "bin2PNG.png");
	FreeImage_Unload(dib);

	FreeImage_DeInitialise();
}