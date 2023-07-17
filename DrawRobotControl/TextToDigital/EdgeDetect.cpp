#include "StdAfx.h"

void MarkEdges(FIBITMAP* img, BYTE CR, BYTE CG, BYTE CB)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	const int bgColor = 0x00FFFFFF;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (isBGColor(bytes, x, y, stride, width, height) != 0)
			{
				continue;
			}
			for (int y2 = -1; y2 <= 1; y2++)
			{
				for (int x2 = -1; x2 <= 1; x2++)
				{
					if (x2 == 0 && y2 == 0)
					{
						continue;
					}
					if (isBGColor(bytes, x + x2, y + y2, stride, width, height) == 1)
					{
						// mark it
//						bytes[(y + y2) * stride + (x + x2) * BYTESPP + 0] = CB;
//						bytes[(y + y2) * stride + (x + x2) * BYTESPP + 1] = CG;
//						bytes[(y + y2) * stride + (x + x2) * BYTESPP + 2] = CR;
						bytes[y * stride + x * BYTESPP + 0] = CB;
						bytes[y * stride + x * BYTESPP + 1] = CG;
						bytes[y * stride + x * BYTESPP + 2] = CR;
					}
				}
			}
		}
	}
}
