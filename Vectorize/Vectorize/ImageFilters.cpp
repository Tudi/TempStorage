#include "StdAfx.h"
#include <map>

void ReduceColorDepth(FIBITMAP* dib, int ColorsPerChannel)
{
	int ColorStep = 255 / ColorsPerChannel;
	int Rounding = ColorStep / 2;
	BYTE *Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	for (int y = 0; y < Height; y++)
	{
		BYTE* SP = &Pixels[y * pitch];
		for (int x = 0; x < Width * Bytespp; x++)
		{
			int NewVal = ((int)SP[x] + Rounding) / ColorStep * ColorStep;
			if (NewVal > 255)
				SP[x] = 255;
			else
				SP[x] = NewVal;
		}
	}
}

int RemoveNoColor(FIBITMAP* dib, BYTE R, BYTE G, BYTE B)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	int PixelsMoved = 0;
	for (int y = 0; y < Height; y++)
	{
		BYTE* SP = &Pixels[y * pitch];
		for (int x = 0; x < Width * Bytespp; x+= Bytespp)
			if (SP[x + 0] == B && SP[x + 1] == G && SP[x + 2] == R)
			{
				SP[x + 0] = B + 1;
				SP[x + 1] = G + 1;
				SP[x + 2] = R + 1;
				PixelsMoved++;
			}
	}
	return PixelsMoved;
}

void MarkColorAsExtracted(FIBITMAP* dib, BYTE R, BYTE G, BYTE B)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* ExtrMap = GetExtractedMap();
	for (int y = 0; y < Height; y++)
	{
		BYTE* SP = &Pixels[y * pitch];
		for (int x = 0; x < Width * Bytespp; x += Bytespp)
			if (SP[x + 0] == B && SP[x + 1] == G && SP[x + 2] == R)
				ExtrMap[y * Width + x / Bytespp] = 1;
	}
}

void SnapSmallPixelsToLines(FIBITMAP* dib)
{
	//need to fix grabbing just 1 byte out of RGB
/*
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	int Bytespp = FreeImage_GetBPP(dib) / 8;
	for (int y = 1; y < Height-1; y++)
	{
		for (int x = 1; x < Width-1; x++)
		{
			if ((Pixels[(y + 0) * pitch + (x + 0) * 3] & 0x00FFFFFF) == 0x00FFFFFF)
				continue;
			//get embracing pixels
			if ((Pixels[(y - 1) * pitch + (x - 1) * 3] & 0x00FFFFFF) != 0x00FFFFFF && (Pixels[(y - 1) * pitch + (x - 1) * 3] & 0x00FFFFFF) == (Pixels[(y + 1) * pitch + (x + 1) * 3] & 0x00FFFFFF))
			{
				if ((Pixels[(y - 1) * pitch + (x - 1) * 3] & 0x00FFFFFF) != (Pixels[(y + 0) * pitch + (x + 0) * 3] & 0x00FFFFFF))
				{
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 0] = Pixels[(y - 1) * pitch + (x - 1) * 3 + 0];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 1] = Pixels[(y - 1) * pitch + (x - 1) * 3 + 1];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 2] = Pixels[(y - 1) * pitch + (x - 1) * 3 + 2];
					continue;
				}
			}
			if ((Pixels[(y - 1) * pitch + (x - 0) * 3] & 0x00FFFFFF) != 0x00FFFFFF && (Pixels[(y - 1) * pitch + (x - 0) * 3] & 0x00FFFFFF) == (Pixels[(y + 1) * pitch + (x + 0) * 3] & 0x00FFFFFF))
			{
				if ((Pixels[(y - 1) * pitch + (x - 0) * 3] & 0x00FFFFFF) != (Pixels[(y + 0) * pitch + (x + 0) * 3] & 0x00FFFFFF))
				{
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 0] = Pixels[(y - 1) * pitch + (x - 0) * 3 + 0];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 1] = Pixels[(y - 1) * pitch + (x - 0) * 3 + 1];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 2] = Pixels[(y - 1) * pitch + (x - 0) * 3 + 2];
					continue;
				}
			}
			if ((Pixels[(y - 1) * pitch + (x + 1) * 3] & 0x00FFFFFF) != 0x00FFFFFF && (Pixels[(y - 1) * pitch + (x + 1) * 3] & 0x00FFFFFF) == (Pixels[(y + 1) * pitch + (x - 1) * 3] & 0x00FFFFFF))
			{
				if ((Pixels[(y - 1) * pitch + (x + 1) * 3] & 0x00FFFFFF) != (Pixels[(y + 0) * pitch + (x + 0) * 3] & 0x00FFFFFF))
				{
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 0] = Pixels[(y - 1) * pitch + (x + 1) * 3 + 0];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 1] = Pixels[(y - 1) * pitch + (x + 1) * 3 + 1];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 2] = Pixels[(y - 1) * pitch + (x + 1) * 3 + 2];
					continue;
				}
			}
			if ((Pixels[(y - 0) * pitch + (x - 1) * 3] & 0x00FFFFFF) != 0x00FFFFFF && (Pixels[(y - 0) * pitch + (x - 1) * 3] & 0x00FFFFFF) == (Pixels[(y + 0) * pitch + (x + 1) * 3] & 0x00FFFFFF))
			{
				if ((Pixels[(y - 0) * pitch + (x - 1) * 3] & 0x00FFFFFF) != (Pixels[(y + 0) * pitch + (x + 0) * 3] & 0x00FFFFFF))
				{
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 0] = Pixels[(y - 1) * pitch + (x - 1) * 3 + 0];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 1] = Pixels[(y - 1) * pitch + (x - 1) * 3 + 1];
					Pixels[(y + 0) * pitch + (x + 0) * 3 + 2] = Pixels[(y - 1) * pitch + (x - 1) * 3 + 2];
					continue;
				}
			}
		}
	}*/
}

void RemoveGradient(FIBITMAP* dib)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	for (int y = 1; y < Height - 1; y++)
	{
		for (int x = 1; x < Width - 1; x++)
		{
			if (((*(int*)&Pixels[(y + 0) * pitch + (x + 0) * 3]) & 0x00FFFFFF) == 0x00FFFFFF)
				continue;
			RGBTRIPLE *p2 = (RGBTRIPLE*)&Pixels[(y + 0) * pitch + (x + 0) * 3];
			for (int j = -1; j <= 0; j++)
				for (int i = -1; i <= 0; i++)
				{
					if (i == 0 && j == 0)
						continue;
					RGBTRIPLE* p1 = (RGBTRIPLE*)&Pixels[(y + j) * pitch + (x + i) * 3];
					RGBTRIPLE* p3 = (RGBTRIPLE*)&Pixels[(y + j * (-1)) * pitch + (x + i * (-1)) * 3];
					if (!(p1->rgbtBlue < p2->rgbtBlue && p2->rgbtBlue < p3->rgbtBlue)
						&& !(p1->rgbtBlue > p2->rgbtBlue && p2->rgbtBlue > p3->rgbtBlue))
						continue;
					if (!(p1->rgbtGreen < p2->rgbtGreen && p2->rgbtGreen < p3->rgbtGreen)
						&& !(p1->rgbtGreen > p2->rgbtGreen && p2->rgbtGreen > p3->rgbtGreen))
						continue;
					if (!(p1->rgbtRed < p2->rgbtRed && p2->rgbtRed < p3->rgbtRed)
						&& !(p1->rgbtRed > p2->rgbtRed && p2->rgbtRed > p3->rgbtRed))
						continue;
					p2->rgbtBlue = MIN(p1->rgbtBlue, p3->rgbtBlue);
					p2->rgbtGreen = MIN(p1->rgbtGreen, p3->rgbtGreen);
					p2->rgbtRed = MIN(p1->rgbtRed, p3->rgbtRed);
				}
		}
	}
}

void RemoveExtraPixels(FIBITMAP* dib)
{
	//has at least 4 neighbours of same color, but different than us
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* NewPixels = (BYTE*)malloc(Height * pitch);
	memcpy(NewPixels, Pixels, Height * pitch);
	for (int y = 1; y < Height - 1; y++)
	{
		for (int x = 1; x < Width - 1; x++)
		{
			int MyColor = ((*(int*)&Pixels[(y + 0) * pitch + (x + 0) * 3]) & 0x00FFFFFF);
			if (MyColor == 0x00FFFFFF)
				continue;
			int NeightbourColors[9];
			int NeightbourColorCounts[9];
			memset(NeightbourColors, -1, sizeof(NeightbourColors));
			memset(NeightbourColorCounts, 0, sizeof(NeightbourColorCounts));
			for (int j = -1; j <= 1; j++)
				for (int i = -1; i <= 1; i++)
				{
					if (i == 0 && j == 0)
						continue;
					int ThisColor = ((*(int*)&Pixels[(y + j) * pitch + (x + i) * 3]) & 0x00FFFFFF);
					int FoundIt = 0;
					for(int t=0;t<9;t++)
						if (NeightbourColors[t] == ThisColor)
						{
							NeightbourColorCounts[t]++;
							FoundIt = 1;
							break;
						}
					if(FoundIt == 0)
						for (int t = 0; t < 9; t++)
							if (NeightbourColors[t] == -1)
							{
								NeightbourColors[t] = ThisColor;
								NeightbourColorCounts[t] = 1;
								break;
							}
				}
			for(int t=0;t<9;t++)
				if (NeightbourColorCounts[t] >= 4)
				{
					if (NeightbourColors[t] != MyColor)
					{
						NewPixels[(y + 0) * pitch + (x + 0) * 3 + 0] = (NeightbourColors[t] >> 0 ) & 0xFF;
						NewPixels[(y + 0) * pitch + (x + 0) * 3 + 1] = (NeightbourColors[t] >> 8 ) & 0xFF;
						NewPixels[(y + 0) * pitch + (x + 0) * 3 + 2] = (NeightbourColors[t] >> 16) & 0xFF;
					}
				}
		}
	}
	memcpy(Pixels, NewPixels, Height * pitch);
	free(NewPixels);
}

void Errode(FIBITMAP* dib, int RequiredSameNeighbours)
{
	//has at least 4 neighbours of same color, but different than us
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* NewPixels = (BYTE*)malloc(Height * pitch);
	memcpy(NewPixels, Pixels, Height * pitch);
	for (int y = 1; y < Height - 1; y++)
	{
		for (int x = 1; x < Width - 1; x++)
		{
			int MyColor = ((*(int*)&Pixels[(y + 0) * pitch + (x + 0) * 3]) & 0x00FFFFFF);
			if (MyColor == 0x00FFFFFF)
				continue;
			int NeighbourCount = 0;
			for (int j = -1; j <= 1; j++)
				for (int i = -1; i <= 1; i++)
				{
					if (i == 0 && j == 0)
						continue;
					int ThisColor = ((*(int*)&Pixels[(y + j) * pitch + (x + i) * 3]) & 0x00FFFFFF);
					if (ThisColor == MyColor)
						NeighbourCount++;
				}
			if (NeighbourCount < RequiredSameNeighbours)
			{
				NewPixels[(y + 0) * pitch + (x + 0) * 3 + 0] = 255;
				NewPixels[(y + 0) * pitch + (x + 0) * 3 + 1] = 255;
				NewPixels[(y + 0) * pitch + (x + 0) * 3 + 2] = 255;
			}
		}
	}
	memcpy(Pixels, NewPixels, Height * pitch);
	free(NewPixels);
}

void SnapColorToDominantSimilar(FIBITMAP* dib)
{
	//has at least 4 neighbours of same color, but different than us
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* NewPixels = (BYTE*)malloc(Height * pitch);
	memcpy(NewPixels, Pixels, Height * pitch);
#define SNAP_KERNEL_SIZE		5
#define SNAP_SIMILARITY_LIMIT	30
	for (int y = SNAP_KERNEL_SIZE; y < Height - SNAP_KERNEL_SIZE; y++)
	{
		for (int x = SNAP_KERNEL_SIZE; x < Width - SNAP_KERNEL_SIZE; x++)
		{
			int MyColor = ((*(int*)&Pixels[(y + 0) * pitch + (x + 0) * 3]) & 0x00FFFFFF);
			if (MyColor == 0x00FFFFFF)
				continue;
			int B = Pixels[(y + 0) * pitch + (x + 0) * 3 + 0];
			int G = Pixels[(y + 0) * pitch + (x + 0) * 3 + 1];
			int R = Pixels[(y + 0) * pitch + (x + 0) * 3 + 2];
			int RGBSUM = (R + G + B + 1);
			float Rcoef = (float)R / (float)RGBSUM;
			float Gcoef = (float)G / (float)RGBSUM;
			float Bcoef = (float)B / (float)RGBSUM;
			int NeightbourColors[SNAP_KERNEL_SIZE * SNAP_KERNEL_SIZE];
			int NeightbourColorCounts[SNAP_KERNEL_SIZE * SNAP_KERNEL_SIZE];
			memset(NeightbourColors, -1, sizeof(NeightbourColors));
			memset(NeightbourColorCounts, 0, sizeof(NeightbourColorCounts));
			for (int j = -SNAP_KERNEL_SIZE; j <= SNAP_KERNEL_SIZE; j++)
				for (int i = -SNAP_KERNEL_SIZE; i <= SNAP_KERNEL_SIZE; i++)
				{
					if (i == 0 && j == 0)
						continue;
					//is it a similar color we could snap to ?
					int B1 = Pixels[(y + j) * pitch + (x + i) * 3 + 0];
					int G1 = Pixels[(y + j) * pitch + (x + i) * 3 + 1];
					int R1 = Pixels[(y + j) * pitch + (x + i) * 3 + 2];

					if (abs(R - R1) > SNAP_SIMILARITY_LIMIT || abs(G - G1) > SNAP_SIMILARITY_LIMIT || abs(B - B1) > SNAP_SIMILARITY_LIMIT)
						continue;

					int ThisColor = ((*(int*)&Pixels[(y + j) * pitch + (x + i) * 3]) & 0x00FFFFFF);
					int FoundIt = 0;
					for (int t = 0; t < SNAP_KERNEL_SIZE * SNAP_KERNEL_SIZE; t++)
						if (NeightbourColors[t] == ThisColor)
						{
							NeightbourColorCounts[t]++;
							FoundIt = 1;
							break;
						}
					if (FoundIt == 0)
						for (int t = 0; t < SNAP_KERNEL_SIZE * SNAP_KERNEL_SIZE; t++)
							if (NeightbourColors[t] == -1)
							{
								NeightbourColors[t] = ThisColor;
								NeightbourColorCounts[t] = 1;
								break;
							}
				}
			int MostDominantCount = -1;
			int MostDominantColor = -1;
			for (int t = 0; t < SNAP_KERNEL_SIZE * SNAP_KERNEL_SIZE; t++)
				if (NeightbourColorCounts[t] >= MostDominantCount)
				{
					MostDominantCount = NeightbourColorCounts[t];
					MostDominantColor = NeightbourColors[t];
				}
			if(MostDominantCount > SNAP_KERNEL_SIZE * SNAP_KERNEL_SIZE / 4)
			{
				NewPixels[(y + 0) * pitch + (x + 0) * 3 + 0] = (MostDominantColor >> 0) & 0xFF;
				NewPixels[(y + 0) * pitch + (x + 0) * 3 + 1] = (MostDominantColor >> 8) & 0xFF;
				NewPixels[(y + 0) * pitch + (x + 0) * 3 + 2] = (MostDominantColor >> 16) & 0xFF;
			}
		}
	}
	memcpy(Pixels, NewPixels, Height * pitch);
	free(NewPixels);
}

void BinarizeImage(FIBITMAP* dib, int ConvertToBlackBelow)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	for (int y = 0; y < Height; y++)
	{
		BYTE* SP = &Pixels[y * pitch];
		for (int x = 0; x < Width * Bytespp; x += Bytespp)
			if (SP[x + 0] < ConvertToBlackBelow || SP[x + 1] < ConvertToBlackBelow && SP[x + 2] < ConvertToBlackBelow)
			{
				SP[x + 0] = 1;
				SP[x + 1] = 1;
				SP[x + 2] = 1;
			}
			else
			{
				SP[x + 0] = 255;
				SP[x + 1] = 255;
				SP[x + 2] = 255;
			}
	}

}

void BlurrImageToGrayScale(BYTE* Pixels, int Width, int Height, int pitch, int KernelSize)
{
	BYTE* PixelsCopy = (BYTE*)malloc(Height * pitch);
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			int SumOfColors = 0;
			for (int y1 = y - KernelSize; y1 <= y + KernelSize; y1++)
				for (int x1 = x - KernelSize; x1 <= x + KernelSize; x1++)
				{
					if (y1 < 0 || y1 >= Height)
					{
						SumOfColors += 255;
						continue;
					}
					if (x1 < 0 || x1 >= Width)
					{
						SumOfColors += 255;
						continue;
					}
					int CurSum = Pixels[y1 * pitch + x1 * Bytespp + 0];
					CurSum += Pixels[y1 * pitch + x1 * Bytespp + 1];
					CurSum += Pixels[y1 * pitch + x1 * Bytespp + 2];
					SumOfColors += CurSum / 3;
				}
			int NewShade = SumOfColors / ((2 * KernelSize + 1) * (2 * KernelSize + 1));
			PixelsCopy[y * pitch + x * Bytespp + 0] = NewShade;
			PixelsCopy[y * pitch + x * Bytespp + 1] = NewShade;
			PixelsCopy[y * pitch + x * Bytespp + 2] = NewShade;
		}
	memcpy(Pixels, PixelsCopy, Height * pitch);
	free(PixelsCopy);
}

void BlurrImageToGrayScaleIfBlack(BYTE* Pixels, int Width, int Height, int pitch, int KernelSize)
{
	BYTE* PixelsCopy = (BYTE*)malloc(Height * pitch);
	memcpy(PixelsCopy, Pixels, Height * pitch);
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			if (Pixels[y * pitch + x * Bytespp + 0] > 128)
				continue;
			int SumOfColors = 0;
			for (int y1 = y - KernelSize; y1 <= y + KernelSize; y1++)
				for (int x1 = x - KernelSize; x1 <= x + KernelSize; x1++)
				{
					if (y1 < 0 || y1 >= Height)
					{
						SumOfColors += 255;
						continue;
					}
					if (x1 < 0 || x1 >= Width)
					{
						SumOfColors += 255;
						continue;
					}
					int CurSum = Pixels[y1 * pitch + x1 * Bytespp + 0];
					CurSum += Pixels[y1 * pitch + x1 * Bytespp + 1];
					CurSum += Pixels[y1 * pitch + x1 * Bytespp + 2];
					SumOfColors += CurSum / 3;
				}
			int NewShade = SumOfColors / ((2 * KernelSize + 1) * (2 * KernelSize + 1));
			PixelsCopy[y * pitch + x * Bytespp + 0] = NewShade;
			PixelsCopy[y * pitch + x * Bytespp + 1] = NewShade;
			PixelsCopy[y * pitch + x * Bytespp + 2] = NewShade;
		}
	memcpy(Pixels, PixelsCopy, Height * pitch);
	free(PixelsCopy);
}

void BlurrImageToGrayScale(FIBITMAP* dib, int KernelSize)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BlurrImageToGrayScale(Pixels, Width, Height, pitch, KernelSize);
}

BYTE* ImgDup(FIBITMAP* dib)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	BYTE* PixelsCopy = (BYTE*)malloc(Height * pitch);
	memcpy(PixelsCopy, Pixels, Height * pitch);
	return PixelsCopy;
}

void RestoreContentFromDup(FIBITMAP* dib, BYTE* dup)
{
	BYTE* Pixels = FreeImage_GetBits(dib);
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	memcpy(Pixels, dup, Height * pitch);
}

FIBITMAP* RescaleImg(FIBITMAP* dib, int NewWidth, int NewHeight)
{
	return FreeImage_Rescale(dib, NewWidth, NewHeight);
}

BYTE* RescaleImg(BYTE* Pixels, int Width, int Height, int pitch, int NewWidth, int NewHeight)
{
	int NewPitch = NewWidth * Bytespp;
	BYTE* ret = (BYTE*)malloc(NewHeight * NewPitch + 16);
	float HeightRatio = (float)Height / (float)NewHeight;
	float WidthRatio = (float)Width / (float)NewWidth;
	int InputPixels = Width * Height;
	int OutputPixels = NewWidth * NewHeight;
	for (int y = 0; y < NewHeight; y++)
		for (int x = 0; x < NewWidth; x++)
		{
			int srcy = (int)((float)y * HeightRatio);
			int srcx = (int)((float)x * WidthRatio);
			if (srcy >= Height)
				srcy = Height;
			if (srcx >= Width)
				srcx = Width;
//				int SrcPixel = *(int*)&Pixels[srcy * pitch + srcx * Bytespp];
//				*(int*)&ret[y * NewWidth + x * Bytespp] = SrcPixel;
			ret[y * NewPitch + x * Bytespp + 0] = Pixels[srcy * pitch + srcx * Bytespp + 0];
			ret[y * NewPitch + x * Bytespp + 1] = Pixels[srcy * pitch + srcx * Bytespp + 1];
			ret[y * NewPitch + x * Bytespp + 2] = Pixels[srcy * pitch + srcx * Bytespp + 2];
		}
	return ret;
}

__int64 Img_SAD(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int pitch2)
{
	__int64 ret = 0;
	for (int y = 0; y < Height1; y++)
		for (int x = 0; x < Width1; x++)
		{
			int Bdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 0] - (int)Pixels2[y * pitch2 + x * Bytespp + 0]);
			int Gdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 1] - (int)Pixels2[y * pitch2 + x * Bytespp + 1]);
			int Rdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 2] - (int)Pixels2[y * pitch2 + x * Bytespp + 2]);
			ret += Bdiff;
			ret += Gdiff;
			ret += Rdiff;
		}
	return ret;
}

__int64 Img_SAD_SQ(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int pitch2)
{
	__int64 ret = 0;
	for (int y = 0; y < Height1; y++)
		for (int x = 0; x < Width1; x++)
//			if(Pixels1[y * pitch1 + x * Bytespp + 0] != 255)
			{
				int Bdiff = ((int)Pixels1[y * pitch1 + x * Bytespp + 0] - (int)Pixels2[y * pitch2 + x * Bytespp + 0]);
				int Gdiff = ((int)Pixels1[y * pitch1 + x * Bytespp + 1] - (int)Pixels2[y * pitch2 + x * Bytespp + 1]);
				int Rdiff = ((int)Pixels1[y * pitch1 + x * Bytespp + 2] - (int)Pixels2[y * pitch2 + x * Bytespp + 2]);
				ret += Bdiff* Bdiff;
				ret += Gdiff* Gdiff;
				ret += Rdiff* Rdiff;
			}
	return ret;
}

__int64 Img_SAD(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int atx, int aty, int pitch2)
{
//#define _DEBUG_SAD
#ifdef _DEBUG_SAD
	FIBITMAP* dib3 = FreeImage_Allocate(Width1, Height1, 24);
	int pitch3 = FreeImage_GetPitch(dib3);
	BYTE* Pixels3 = FreeImage_GetBits(dib3);
	memset(Pixels3, 255, Height1 * pitch3);
#endif
	__int64 ret = 0;
	for (int y = 0; y < Height1; y++)
		for (int x = 0; x < Width1; x++)
		{
			int Bdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 0] - (int)Pixels2[(aty + y) * pitch2 + (atx + x) * Bytespp + 0]);
			int Gdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 1] - (int)Pixels2[(aty + y) * pitch2 + (atx + x) * Bytespp + 1]);
			int Rdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 2] - (int)Pixels2[(aty + y) * pitch2 + (atx + x) * Bytespp + 2]);
			ret += Bdiff;
			ret += Gdiff;
			ret += Rdiff;
#ifdef _DEBUG_SAD
			Pixels3[y * pitch3 + x * Bytespp + 0] = Bdiff;
			Pixels3[y * pitch3 + x * Bytespp + 1] = Gdiff;
			Pixels3[y * pitch3 + x * Bytespp + 2] = Rdiff;
#endif
		}
#ifdef _DEBUG_SAD
	FreeImage_Save(FIF_PNG, dib3, "SAD.png", 0);
#endif

	return ret;
}

__int64 Img_SAD_FontPresent(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int atx, int aty, int pitch2)
{
	//#define _DEBUG_SAD
#ifdef _DEBUG_SAD
	FIBITMAP* dib3 = FreeImage_Allocate(Width1, Height1, 24);
	int pitch3 = FreeImage_GetPitch(dib3);
	BYTE* Pixels3 = FreeImage_GetBits(dib3);
	memset(Pixels3, 255, Height1 * pitch3);
#endif
	__int64 ret = 0;
	for (int y = 0; y < Height1; y++)
		for (int x = 0; x < Width1; x++)
			if(Pixels1[y * pitch1 + x * Bytespp + 0] != 255)
			{
				int Bdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 0] - (int)Pixels2[(aty + y) * pitch2 + (atx + x) * Bytespp + 0]);
				int Gdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 1] - (int)Pixels2[(aty + y) * pitch2 + (atx + x) * Bytespp + 1]);
				int Rdiff = abs((int)Pixels1[y * pitch1 + x * Bytespp + 2] - (int)Pixels2[(aty + y) * pitch2 + (atx + x) * Bytespp + 2]);
				ret += Bdiff;
				ret += Gdiff;
				ret += Rdiff;
#ifdef _DEBUG_SAD
				Pixels3[y * pitch3 + x * Bytespp + 0] = Bdiff;
				Pixels3[y * pitch3 + x * Bytespp + 1] = Gdiff;
				Pixels3[y * pitch3 + x * Bytespp + 2] = Rdiff;
#endif
			}
			else
			{
				ret += 255;
				ret += 255;
				ret += 255;
			}
#ifdef _DEBUG_SAD
	FreeImage_Save(FIF_PNG, dib3, "SAD.png", 0);
#endif

	return ret;
}

void GetPixelRescale(BYTE* Pixels, int Width, int Height, int pitch, float x, float y, float xscale, float yscale, BYTE *B, BYTE *G, BYTE *R)
{
	//source is larger than destination
	//create an averege of 1 destination pixel from multiple source pixels
	if (xscale > 1 && yscale > 1)
	{
		int yscalei = (int)(yscale + 1);
		int xscalei = (int)(xscale + 1);
		int SumR = 0;
		int SumG = 0;
		int SumB = 0;
		int PixelCount = 0;
		for (int ty = (int)y - yscalei / 2; ty <= (int)y + yscalei / 2; ty++)
			for (int tx = (int)x - xscalei / 2; tx <= (int)x + xscalei / 2; tx++)
			{
				if (ty < 0 || ty >= Height)
					continue;
				if (tx < 0 || tx >= Width)
					continue;
				SumB += Pixels[ty * pitch + tx * Bytespp + 0];
				SumG += Pixels[ty * pitch + tx * Bytespp + 1];
				SumR += Pixels[ty * pitch + tx * Bytespp + 2];
				PixelCount++;
			}
		if (PixelCount > 0)
		{
			*B = SumB / PixelCount;
			*G = SumG / PixelCount;
			*R = SumR / PixelCount;
		}
		else
		{
			*B = Pixels[(int)y * pitch + (int)x * Bytespp + 0];
			*G = Pixels[(int)y * pitch + (int)x * Bytespp + 1];
			*R = Pixels[(int)y * pitch + (int)x * Bytespp + 2];
		}
	}
	//source pixel is a sub pixel, destination pixel will have a gradient as it moves from 1 pixel to next
	else if (xscale < 1 && yscale < 1 && (int)y + 1 < Height && (int)x + 1 < Width)
	{
		float ycoefnext = y - (int)y;
		float ycoefprev = 1 - ycoefnext;
		float xcoefnext = x - (int)x;
		float xcoefprev = 1 - xcoefnext;
		int iy = (int)y;
		int ix = (int)x;
		float SumB = Pixels[iy * pitch + ix * Bytespp + 0] * (ycoefprev + xcoefprev)
			+ Pixels[iy * pitch + (ix + 1) * Bytespp + 0] * (ycoefprev + xcoefnext)
			+ Pixels[(iy + 1) * pitch + (ix + 0) * Bytespp + 0] * (ycoefnext + xcoefprev)
			+ Pixels[(iy + 1) * pitch + (ix + 1) * Bytespp + 0] * (ycoefnext + xcoefnext);
		SumB /= 4;
		float SumG = Pixels[iy * pitch + ix * Bytespp + 1] * (ycoefprev + xcoefprev)
			+ Pixels[iy * pitch + (ix + 1) * Bytespp + 1] * (ycoefprev + xcoefnext)
			+ Pixels[(iy + 1) * pitch + (ix + 0) * Bytespp + 1] * (ycoefnext + xcoefprev)
			+ Pixels[(iy + 1) * pitch + (ix + 1) * Bytespp + 1] * (ycoefnext + xcoefnext);
		SumG /= 4;
		float SumR = Pixels[iy * pitch + ix * Bytespp + 2] * (ycoefprev + xcoefprev)
			+ Pixels[iy * pitch + (ix + 1) * Bytespp + 2] * (ycoefprev + xcoefnext)
			+ Pixels[(iy + 1) * pitch + (ix + 0) * Bytespp + 2] * (ycoefnext + xcoefprev)
			+ Pixels[(iy + 1) * pitch + (ix + 1) * Bytespp + 2] * (ycoefnext + xcoefnext);
		SumR /= 4;
		*B = (BYTE)SumB;
		*G = (BYTE)SumG;
		*R = (BYTE)SumR;
	}
	else
	{
		*B = Pixels[(int)y * pitch + (int)x * Bytespp + 0];
		*G = Pixels[(int)y * pitch + (int)x * Bytespp + 1];
		*R = Pixels[(int)y * pitch + (int)x * Bytespp + 2];
	}
}

BYTE* RescaleImgSubPixel(BYTE* Pixels, int Width, int Height, int pitch, int NewWidth, int NewHeight)
{
	int NewPitch = NewWidth * Bytespp;
	BYTE* ret = (BYTE*)malloc(NewHeight * NewPitch + 16);
	float HeightRatio = (float)Height / (float)NewHeight;
	float WidthRatio = (float)Width / (float)NewWidth;
	int InputPixels = Width * Height;
	int OutputPixels = NewWidth * NewHeight;
	for (int y = 0; y < NewHeight; y++)
		for (int x = 0; x < NewWidth; x++)
		{
//			int srcy = (int)((float)y * HeightRatio);
//			int srcx = (int)((float)x * WidthRatio);
			float srcy = ((float)y * HeightRatio);
			float srcx = ((float)x * WidthRatio);
			if (srcy >= (float)Height)
				srcy = (float)Height;
			if (srcx >= (float)Width)
				srcx = (float)Width;
			GetPixelRescale(Pixels, Width, Height, pitch, srcx, srcy, WidthRatio, HeightRatio, &ret[y * NewPitch + x * Bytespp + 0], &ret[y * NewPitch + x * Bytespp + 1], &ret[y * NewPitch + x * Bytespp + 2]);
		}
	return ret;
}

void ErodeInside(BYTE* Pixels, int Width, int Height, int pitch, int ErrodeIfSmaller)
{
	BYTE* TPixels = (BYTE*)malloc(Height * pitch);
	memcpy(TPixels, Pixels, Height * pitch);

	for (int y = 1; y < Height - 1; y++)
		for (int x = 1; x < Width - 1; x++)
		{
			//can't errode a pixel that is already in "erroded" state
			if (Pixels[(y + 0) * pitch + (x + 0) * Bytespp + 0] >= ErrodeIfSmaller
				|| Pixels[(y + 0) * pitch + (x + 0) * Bytespp + 1] >= ErrodeIfSmaller
				|| Pixels[(y + 0) * pitch + (x + 0) * Bytespp + 2] >= ErrodeIfSmaller)
				continue;

			//if pixel is surounded by pixels that are ok to be erroded, than we consider it ok to be removed
			int IsSurrounded = 0;
			for (int y1 = -1; y1 <= 1; y1++)
				for (int x1 = -1; x1 <= 1; x1++)
					if (Pixels[(y + y1) * pitch + (x + x1) * Bytespp + 0] < ErrodeIfSmaller
						|| Pixels[(y + y1) * pitch + (x + x1) * Bytespp + 1] < ErrodeIfSmaller
						|| Pixels[(y + y1) * pitch + (x + x1) * Bytespp + 2] < ErrodeIfSmaller)
						IsSurrounded++;

			if (IsSurrounded >= 9)
			{
				TPixels[(y + 0) * pitch + (x + 0) * Bytespp + 0] = 255;
				TPixels[(y + 0) * pitch + (x + 0) * Bytespp + 1] = 255;
				TPixels[(y + 0) * pitch + (x + 0) * Bytespp + 2] = 255;
			}
		}

	memcpy(Pixels, TPixels, Height * pitch);
	free(TPixels);
}

int CheckKernel(BYTE* Pixels, int pitch, BYTE *Kernel)
{
	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
			if (Pixels[y * pitch + x * Bytespp] != Kernel[y*3+x])
				return 0;
	return 1;
}

void ErodeOutside(BYTE* Pixels, int Width, int Height, int pitch, int ErrodeIfSmaller)
{
	BYTE* TPixels = (BYTE*)malloc(Height * pitch);
	memcpy(TPixels, Pixels, Height * pitch);
	for (int y = 1; y < Height - 1; y++)
		for (int x = 1; x < Width - 1; x++)
		{
			//can't errode a pixel that is already in "erroded" state
			if (Pixels[(y + 0) * pitch + (x + 0) * Bytespp + 0] >= ErrodeIfSmaller
				|| Pixels[(y + 0) * pitch + (x + 0) * Bytespp + 1] >= ErrodeIfSmaller
				|| Pixels[(y + 0) * pitch + (x + 0) * Bytespp + 2] >= ErrodeIfSmaller)
				continue;

			//kernels
			// B B B  B B W  W W W  W B B  W B B  B B B  B W W  W W W  W W B  B B B  B B B  B W W
			// B X B  B X W  B X B  W X B  W X B  B X W  B X W  W X B  W X B  W X B  B X W  B X W
			// W W W  B B W  B B B  W B B  W W B  W W W  B B W  B B B  B B B  W W B  B W W  B B B
			BYTE Kernel1[3][3] = { {1,1,1},{1,1,1},{255,255,255} };
			BYTE Kernel2[3][3] = { {1,1,255},{1,1,255},{1,1,255} };
			BYTE Kernel3[3][3] = { {255,255,255},{1,1,1},{1,1,1} };
			BYTE Kernel4[3][3] = { {255,1,1},{255,1,1},{255,1,1} };
			BYTE Kernel5[3][3] = { {255,1,1},{255,1,1},{255,255,1} };
			BYTE Kernel6[3][3] = { {1,1,1},{1,1,255},{255,255,255} };
			BYTE Kernel7[3][3] = { {1,255,255},{1,1,255},{1,1,255} };
			BYTE Kernel8[3][3] = { {255,255,255},{255,1,1},{1,1,1} };
			BYTE Kernel9[3][3] = { {255,255,1},{255,1,1},{1,1,1} };
			BYTE Kernel10[3][3] = { {1,1,1},{255,1,1},{255,255,1} };
			BYTE Kernel11[3][3] = { {1,1,1},{1,1,255},{1,255,255} };
			BYTE Kernel12[3][3] = { {1,255,255},{1,1,255},{1,1,1} };
			int ty = y - 1;
			int tx = x - 1;
			if( CheckKernel(&Pixels[y*pitch+x*Bytespp], pitch, &Kernel1[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel2[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel3[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel4[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel5[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel6[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel7[0][0])
				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel8[0][0])
//				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel9[0][0])
//				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel10[0][0])
//				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel11[0][0])
//				|| CheckKernel(&Pixels[ty * pitch + tx * Bytespp], pitch, &Kernel12[0][0])
				)
			{
				TPixels[(y + 0) * pitch + (x + 0) * Bytespp + 0] = 255;
				TPixels[(y + 0) * pitch + (x + 0) * Bytespp + 1] = 255;
				TPixels[(y + 0) * pitch + (x + 0) * Bytespp + 2] = 255;
			}
		}
	memcpy(Pixels, TPixels, Height * pitch);
	free(TPixels);
}