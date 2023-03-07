#include "StdAfx.h"

void DrawBinLineOnPNG(FIBITMAP* in_Img, int32_t& x, int32_t& y, int32_t* line)
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
	size_t linePixels = line[0];
	size_t drawLine = line[1];
	line += 2;
	for (size_t i = 0; i < linePixels; i++)
	{
		x += line[0];
		y += line[1];
		if (x < 0)
		{
			x = 0;
		}
		if (x >= Width)
		{
			x = Width - 1;
		}
		if (y < 0)
		{
			y = 0;
		}
		if (y >= Height)
		{
			y = Height - 1;
		}
		if (drawLine)
		{
			for (int y2 = -2; y2 <= 2; y2++)
			{
				for (int x2 = -2; x2 <= 2; x2++)
				{
					BITS[(y + y2) * stride + (x + x2) * Bytespp + 0] = 255;
					BITS[(y + y2) * stride + (x + x2) * Bytespp + 1] = 255;
					BITS[(y + y2) * stride + (x + x2) * Bytespp + 2] = 255;
				}
			}
		}
		line += 2;
	}
}