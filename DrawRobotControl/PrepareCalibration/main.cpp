#include <stdio.h>
#include <stdlib.h>
#include "ImageHandler.h"

#define BYTESPP 3
#define ColorMatch(a,b) (((a)&0x00FFFFFF)==((b)&0x00FFFFFF))

#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

void GetLineCenterAndWidth(BYTE* Ori, BYTE *Dst, int x, int y, int width, int height, int stride, int &out_x, int &out_y, int &out_width)
{
	int lineColor = *(int*)&Ori[y * stride + x * 3];
	lineColor = lineColor & 0x00FFFFFF;
	// find the minx,maxx
	int minX = x;
	BYTE *row = &Ori[y * stride];
	for (; minX > 0; minX--)
	{
		if (!ColorMatch(*(int*)&row[minX * 3], lineColor))
		{
			minX++;
			break;
		}
	}

	int maxX = x;
	row = &Ori[y * stride];
	for (; maxX < width; maxX++)
	{
		if (!ColorMatch(*(int*)&row[maxX * 3], lineColor))
		{
			maxX--;
			break;
		}
	}

	int minY = y;
	for (; minY > 0; minY--)
	{
		if (!ColorMatch(*(int*)&Ori[minY * stride + x * 3], lineColor))
		{
			minY++;
			break;
		}
	}

	int maxY = y;
	for (; maxY < height; maxY++)
	{
		if (!ColorMatch(*(int*)&Ori[maxY * stride + x * 3], lineColor))
		{
			maxY--;
			break;
		}
	}

	int lineWidth = maxX - minX + 1;
	int lineHeight = maxY - minY + 1;
	out_width = MIN(lineWidth, lineHeight);

	int minX2 = MAX(x - out_width, minX);
	int maxX2 = MIN(x + out_width, maxX);
	int minY2 = MAX(y - out_width, minY);
	int maxY2 = MIN(y + out_width, maxY);

	int lineCenterX = (minX2 + maxX2) / 2;
	int lineCenterY = (minY2 + maxY2) / 2;

	out_x = lineCenterX;
	out_y = lineCenterY;
}

void MarkLineCenter(BYTE* Ori, BYTE* Dst, int x, int y, int width, int height, int stride, int markColor)
{
	int lineColor = *(int*)&Ori[y * stride + x * 3];
	lineColor = lineColor & 0x00FFFFFF;

	// find the aprox center
	int lineWidth, x2, y2;
	GetLineCenterAndWidth(Ori, Dst, x, y, width, height, stride, x2, y2, lineWidth );

	// probably just noise
	if (lineWidth == 1)
	{
//		return;
	}

	// find the weighted center
	int minX = MAX(x2 - lineWidth, 0);
	int maxX = MIN(x2 + lineWidth, width);
	int minY = MAX(y2 - lineWidth, 0);
	int maxY = MIN(y2 + lineWidth, height);
	__int64 sumX = 0, sumY = 0, count = 0;
	for (int y2 = minY; y2 < maxY; y2++)
	{
		for (int x2 = minX; x2 < maxX; x2++)
		{
			if (ColorMatch(*(int*)&Ori[y2*stride + x2 * 3], lineColor))
			{
				sumX += x2;
				sumY += y2;
				count++;
			}
		}
	}

	// mark the center
	int newB = markColor & 0xFF;
	int newG = (markColor >> 8) & 0xFF;
	int newR = (markColor >> 16) & 0xFF;

	int weigtedX = (int)(sumX / count);
	int weigtedY = (int)(sumY / count);

	Dst[weigtedY * stride + weigtedX * 3 + 0] = newB;
	Dst[weigtedY * stride + weigtedX * 3 + 1] = newG;
	Dst[weigtedY * stride + weigtedX * 3 + 2] = newR;
}

//void ThinLine(FIBITMAP* img, BYTE lineR, BYTE lineG, BYTE lineB, BYTE newR, BYTE newG, BYTE newB)
void ThinLine(FIBITMAP* img, BYTE newR, BYTE newG, BYTE newB)
{
	int width = FreeImage_GetWidth(img);
	int height = FreeImage_GetHeight(img);
	int stride = FreeImage_GetPitch(img);
	BYTE* bytes = FreeImage_GetBits(img);
	// duplicate original image
	BYTE* bytesOri = (BYTE*)malloc(height * stride + 32);
	if (bytesOri == NULL)
	{
		return;
	}
	memcpy(bytesOri, bytes, height * stride);

//	int lineColor = (int)lineR | ((int)lineG << 8) | ((int)lineB << 16);
	int bgColor = 0x00FFFFFF;
	int newColor = (int)newR | ((int)newG << 8) | ((int)newB << 16);
	for (size_t y = 0; y < height; y++)
	{
		BYTE* row = &bytesOri[y * stride];
		for (size_t x = 0; x < width; x++)
		{
//			if (ColorMatch(*(int*)&row[x*3], lineColor))
			if (!ColorMatch(*(int*)&row[x * 3], bgColor))
			{
				MarkLineCenter(bytesOri, bytes, (int)x, (int)y, width, height, stride, newColor);
			}
		}
	}

	free(bytesOri);
}

inline int isBGColor(BYTE* bytes, int x, int y, int stride, int width, int height)
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

#define ORIGIN_R 237
#define ORIGIN_G 28
#define ORIGIN_B 36
int FindMarkedLocation(FIBITMAP* Img, int& manualMarkedOriginX, int& manualMarkedOriginY)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	int bpp = FreeImage_GetBPP(Img) / 8;

	manualMarkedOriginX = Width / 2;
	manualMarkedOriginY = Height / 2;

#define ACCEPTED_DEVIATION 20
	for (size_t y = 0; y < Height; y++)
	{
		for (size_t x = 0; x < Width; x++)
		{
			BYTE* bNow = &Bytes[y * Stride + x * bpp];
			if (abs((int)bNow[0] - (int)ORIGIN_B) < ACCEPTED_DEVIATION &&
				abs((int)bNow[1] - (int)ORIGIN_G) < ACCEPTED_DEVIATION &&
				abs((int)bNow[2] - (int)ORIGIN_R) < ACCEPTED_DEVIATION)
			{
				manualMarkedOriginX = (int)x;
				manualMarkedOriginY = (int)y;
				return 0;
			}
		}
	}

	return 1;
}

void MarkCenterOfImage(FIBITMAP* Img, int manualMarkedOriginX, int manualMarkedOriginY, int radius = 1)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	for (int y = manualMarkedOriginY - radius; y <= manualMarkedOriginY + radius; y++)
	{
		for (int x = manualMarkedOriginX - radius; x <= manualMarkedOriginX + radius; x++)
		{
			if (x < 0 || y < 0 || x >= Width || y >= Height)
			{
				continue;
			}

			BYTE* bNow = &Bytes[y * Stride + x * BYTESPP];
			bNow[0] = ORIGIN_B;
			bNow[1] = ORIGIN_G;
			bNow[2] = ORIGIN_R;
		}
	}
}

void BlendImages(FIBITMAP* Img1, int manualMarkedOriginX1, int manualMarkedOriginY1,
	FIBITMAP* Img2, int manualMarkedOriginX2, int manualMarkedOriginY2)
{
	BYTE* Bytes1 = FreeImage_GetBits(Img1);
	int Stride1 = FreeImage_GetPitch(Img1);
	int Width1 = FreeImage_GetWidth(Img1);
	int Height1 = FreeImage_GetHeight(Img1);
	BYTE* Bytes2 = FreeImage_GetBits(Img2);
	int Stride2 = FreeImage_GetPitch(Img2);
	int Width2 = FreeImage_GetWidth(Img2);
	int Height2 = FreeImage_GetHeight(Img2);
	int copyToDX = manualMarkedOriginX1 - manualMarkedOriginX2;
	int copyToDY = manualMarkedOriginY1 - manualMarkedOriginY2;
	for (int sy = 0; sy < Height2; sy++)
	{
		for (int sx = 0; sx < Width2; sx++)
		{
			int dx = sx + copyToDX;
			int dy = sy + copyToDY;
			if (dx < 0 || dx >= Width1 || dy < 0 || dy >= Height1)
			{
				continue;
			}
			BYTE* bDst = &Bytes1[dy * Stride2 + dx * BYTESPP];
			BYTE* bSrc = &Bytes2[sy * Stride2 + sx * BYTESPP];

			// pure white color is considered transparent color
			if (bSrc[0] == 255)
			{
				continue;
			}

			// overwrite destination values
			bDst[0] = bSrc[0];
			bDst[1] = bSrc[1];
			bDst[2] = bSrc[2];
		}
	}
}

void BinarizeImage(FIBITMAP* Img)
{
	BYTE* Bytes = FreeImage_GetBits(Img);
	int Stride = FreeImage_GetPitch(Img);
	int Width = FreeImage_GetWidth(Img);
	int Height = FreeImage_GetHeight(Img);
	for (size_t y = 0; y < Height; y++)
	{
		for (size_t x = 0; x < Width; x++)
		{
			BYTE* bNow = &Bytes[y * Stride + x * BYTESPP];
			int Intensity = (int)bNow[0] + (int)bNow[1] + (int)bNow[2];
			if (Intensity >= 255 * 3 / 2)
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

int main()
{
	const char* fileName1 = "ver_4.png"; // should be the vertical lines image
	const char* fileName2 = "hor_4.png"; // should be the horizontal lines image
	FIBITMAP *img1 = LoadImage_(fileName1);
	FIBITMAP* img2 = LoadImage_(fileName2);
	if (img1 == NULL || img2 == NULL)
	{
		printf("Failed to load input image\n");
		return -2;
	}

	if (FreeImage_GetBPP(img1) != 24)
	{
		printf("Convert image 1 to 24 bpp \n");
		img1 = FreeImage_ConvertTo24Bits(img1);
	}
	if (FreeImage_GetBPP(img2) != 24)
	{
		printf("Convert image 2 to 24 bpp \n");
		img2 = FreeImage_ConvertTo24Bits(img2);
	}

	printf("Find center of the images \n");
	int cx1, cy1, cx2, cy2;
	if ((FindMarkedLocation(img1, cx1, cy1) + FindMarkedLocation(img2, cx2, cy2)) != 0)
	{
		printf("Failed to find manually marked center of the image. Will use geometric center, but that is not safe ! \n");
	}

	printf("Resize images to double size \n");
#define RESCALE_INPUT_BY 2
	img1 = FreeImage_Rescale(img1, FreeImage_GetWidth(img1) * RESCALE_INPUT_BY, FreeImage_GetHeight(img1) * RESCALE_INPUT_BY);
	img2 = FreeImage_Rescale(img2, FreeImage_GetWidth(img2) * RESCALE_INPUT_BY, FreeImage_GetHeight(img2) * RESCALE_INPUT_BY);
	cx1 *= 2; cy1 *= 2; cx2 *= 2; cy2 *= 2;

	printf("Binarize images \n");
	BinarizeImage(img1);
	BinarizeImage(img2);

	printf("Make lines as thin as possible \n");
	ThinLine(img1, 252, 110, 107);
	ThinLine(img2, 7, 251, 1);
	RemoveBlackColor(img1);
	RemoveBlackColor(img2);

	printf("Remove small aberations \n");
	RemoveSmallDots(img1);
	RemoveSmallDots(img2);

	printf("Blend the 2 images\n");
	BlendImages(img1, cx1, cy1, img2, cx2, cy2);

	printf("Mark the center of the image\n");
	MarkCenterOfImage(img1, cx1, cy1, 2);

	printf("Save the calibration image\n");
	SaveImage(img1, "calibration.bmp");

	FreeImage_Unload(img1);
	FreeImage_Unload(img2);

	// Deinitialize FreeImage library
	FreeImage_DeInitialise();

	return 0;
}