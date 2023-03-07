#include "StdAfx.h"

static int LineDrawCounter = 0;
void GetLineColor(BYTE& LineBaseR, BYTE& LineBaseG, BYTE& LineBaseB)
{
	// red line ?
	if (LineDrawCounter % 3 == 0)
	{
		LineBaseR = 64;
		LineBaseG = 64;
		LineBaseB = 255;
	}
	// green line
	if (LineDrawCounter % 3 == 1)
	{
		LineBaseR = 64;
		LineBaseG = 255;
		LineBaseB = 64;
	}
	// blue line
	if (LineDrawCounter % 3 == 2)
	{
		LineBaseR = 255;
		LineBaseG = 64;
		LineBaseB = 64;
	}
	LineDrawCounter++;
}

#define FLIP_DRAW_VERTICALLY 0
#define FLIP_DRAW_HORIZONTALLY 0

void DrawBinLineOnPNG(FIBITMAP* in_Img, float& x, float& y, float* line)
{
	if (line == NULL || line[0] <= 0)
	{
		printf("Unexpected no length line\n");
		return;
	}

	size_t stride = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	int32_t Width = FreeImage_GetWidth(in_Img);
	int32_t Height = FreeImage_GetHeight(in_Img);
	size_t linePixels = (size_t)line[0];
	size_t drawLine = (size_t)line[1];
	BYTE LineBaseR = 64, LineBaseG = 64, LineBaseB = 64;
	if (drawLine && linePixels > 4)
	{
		GetLineColor(LineBaseR, LineBaseG, LineBaseB);
	}
	line += 2;
#define LineSize 2
	for (size_t i = 0; i < linePixels; i++)
	{
#if FLIP_DRAW_HORIZONTALLY == 0
		x += line[0];
#else
		x -= line[0];
#endif
#if FLIP_DRAW_VERTICALLY == 0
		y += line[1];
#else
		y -= line[1];
#endif
		size_t canDraw = 1;
		if (x - LineSize < 0)
		{
			canDraw = 0;
		}
		if (x + LineSize >= Width)
		{
			canDraw = 0;
		}
		if (y - LineSize < 0)
		{
			canDraw = 0;
		}
		if (y >= Height + LineSize)
		{
			canDraw = 0;
		}
		if (drawLine && canDraw)
		{
			// start at full brightness and end with 50% brightness
			BYTE pixelR = (BYTE)((int)LineBaseR - ((double)i / (double)linePixels / 2.0 * LineBaseR));
			BYTE pixelG = (BYTE)((int)LineBaseG - ((double)i / (double)linePixels / 2.0 * LineBaseG));
			BYTE pixelB = (BYTE)((int)LineBaseB - ((double)i / (double)linePixels / 2.0 * LineBaseB));
			for (int y2 = -2; y2 <= 2; y2++)
			{
				for (int x2 = -2; x2 <= 2; x2++)
				{
					if ( abs((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 0] - (int)pixelR ) > 1
						&& abs((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 1] - (int)pixelG ) > 1
						&& abs((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 2] - (int)pixelB ) > 1
						)
					{
						BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 0] = MIN(255, ((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 0] + (int)pixelR));
						BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 1] = MIN(255, ((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 1] + (int)pixelG));
						BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 2] = MIN(255, ((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 2] + (int)pixelB));
					}
					else
					{
						BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 0] = pixelR;
						BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 1] = pixelG;
						BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 2] = pixelB;
					}
				}
			}
		}
		line += 2;
	}
}

void DrawCircleAt(FIBITMAP* in_Img, float x, float y, float radius)
{
	size_t stride = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	int32_t Width = FreeImage_GetWidth(in_Img);
	int32_t Height = FreeImage_GetHeight(in_Img);
	// angle speed should depend on radius, but we do not have time for that now
	for (double angle = 0; angle < 360; angle += 0.05)
	{
		int px = (int)(x + (double)radius * cos(angle));
		int py = (int)(y + (double)radius * sin(angle));
		for (int y2 = -2; y2 <= 2; y2++)
		{
			for (int x2 = -2; x2 <= 2; x2++)
			{
				BITS[(py + y2) * stride + (px + x2) * Bytespp + 0] = 255;
				BITS[(py + y2) * stride + (px + x2) * Bytespp + 1] = 255;
				BITS[(py + y2) * stride + (px + x2) * Bytespp + 2] = 255;
			}
		}
	}
}