#include "StdAfx.h"

// we expect the background to be white
int isBGColor(BYTE* bytes, int x, int y, int stride, int width, int height)
{
	if (x < 0 || x >= width)
	{
		return 2;
	}
	if (y < 0 || y >= height)
	{
		return 2;
	}
	if (ColorMatch(*(int*)&bytes[y * stride + x * 3], 0x00FFFFFF))
	{
		return 1;
	}
	return 0;
}

void setToBGColor(BYTE* bytes, int x, int y, int stride, int width, int height)
{
	bytes[y * stride + x * 3 + 0] = 255;
	bytes[y * stride + x * 3 + 1] = 255;
	bytes[y * stride + x * 3 + 2] = 255;
}

void RemoveBlackColor(FIBITMAP* img)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (ColorMatch(*(int*)&bytes[y * stride + x * 3], 0))
			{
				setToBGColor(bytes, x, y, stride, width, height);
			}
		}
	}
}

void BinarizeImage(FIBITMAP* Img)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
#define LUMINOSITY_PCT_BECOME_WHITE 0.7
	for (size_t y = 0; y < Height; y++)
	{
		for (size_t x = 0; x < Width; x++)
		{
			BYTE* bNow = &Bytes[y * Stride + x * BYTESPP];
			int Intensity = (int)bNow[0] + (int)bNow[1] + (int)bNow[2];
			if (Intensity >= (int)(255 * 3 * LUMINOSITY_PCT_BECOME_WHITE))
			{
				bNow[0] = 255;
				bNow[1] = 255;
				bNow[2] = 255;
			}
			else
			{
				bNow[0] = 0;
				bNow[1] = 0;
				bNow[2] = 0;
			}
		}
	}
}

int GetNonMatchingColorCountNearby(BYTE* bytes, int x, int y, int stride, int width, int height, int color)
{
	// check if surrounding pixels are BG pixels
	int bgColorCount = 0;
	for (int y2 = y - 1; y2 <= y + 1; y2++)
	{
		for (int x2 = x - 1; x2 <= x + 1; x2++)
		{
			if (y2 == y && x == x2)
			{
				continue;
			}
			if (y2 < 0 || y2 >= height || x2 < 0 || x2 >= width)
			{
				continue;
			}
			if (ColorMatch(*(int*)&bytes[y2 * stride + x2 * 3], color) == 0)
			{
				bgColorCount++;
			}
		}
	}
	return bgColorCount;
}

void RemoveSmallDots(FIBITMAP* img)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	int bgColor = (int)255 | ((int)255 << 8) | ((int)255 << 16);

	// duplicate original image
	BYTE* bytesOri = (BYTE*)malloc(height * stride + 32);
	if (bytesOri == NULL)
	{
		return;
	}
	memcpy(bytesOri, bytes, height * stride);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (isBGColor(bytesOri, x, y, stride, width, height) == 0)
			{
				int myColor = *(int*)&bytesOri[y * stride + x * 3] & 0x00FFFFFF;
				int bgColorCount = GetNonMatchingColorCountNearby(bytesOri, x, y, stride, width, height, myColor);

				// maybe it's a 2 pixel small dot
				if (bgColorCount == 7 && ColorMatch(*(int*)&bytesOri[y * stride + (x + 1) * 3], myColor))
				{
					int bgColorCount2 = GetNonMatchingColorCountNearby(bytesOri, x + 1, y, stride, width, height, myColor);
					if (bgColorCount2 == 7)
					{
						setToBGColor(bytes, x, y, stride, width, height);
						setToBGColor(bytes, x + 1, y, stride, width, height);
					}
				}
				if (bgColorCount == 7 && ColorMatch(*(int*)&bytesOri[(y + 1) * stride + x * 3], myColor))
				{
					int bgColorCount2 = GetNonMatchingColorCountNearby(bytesOri, x, y + 1, stride, width, height, myColor);
					if (bgColorCount2 == 7)
					{
						setToBGColor(bytes, x, y, stride, width, height);
						setToBGColor(bytes, x, y + 1, stride, width, height);
					}
				}

				if (bgColorCount == 8)
					//				if (bgColorCount >= 7)
				{
					setToBGColor(bytes, x, y, stride, width, height);
				}
			}
		}
	}

	free(bytesOri);
}

double GetLineLen(double sx, double sy, double ex, double ey)
{
	double dx = sx - ex;
	double dy = sy - ey;
	return sqrt(dx * dx + dy * dy);
}

double abs(double x)
{
	if(x<0)
	{
		return -x;
	}
	return x;
}