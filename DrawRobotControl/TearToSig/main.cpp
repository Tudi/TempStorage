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

int IsTearRed(BYTE* pixel)
{
	if (pixel[0] < 50 && pixel[1] < 50 && pixel[2]>200)
	{
		return 1;
	}
	return 0;
}

int ScanImageNextCol(FIBITMAP* in_Img, int32_t& col, int32_t* rows)
{
	size_t stride = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	int32_t Width = FreeImage_GetWidth(in_Img);
	int32_t Height = FreeImage_GetHeight(in_Img);
	//does this column have at least 2 red pixels ?
	for (; col < Width; col++)
	{
		int RedsFound = 0;
		int IsRed = 0;
		for (size_t y = 0; y < Height; y++)
		{
			if (IsTearRed(&BITS[((int)y) * stride + ((int)col) * Bytespp + 0]))
			{
				IsRed = 1;
			}
			else if (IsRed)
			{
				IsRed = 0;
				if (RedsFound < 2)
				{
					rows[RedsFound] = (int32_t)y;
				}
				else
				{
					break;
				}
				RedsFound++;
			}
		}
		printf("At col %d found %d reds at %d, %d\n", col, RedsFound, rows[0], rows[1]);
		if (RedsFound == 2)
		{
			return 0;
		}
	}
	return 1;
}

void AddLineToSIGFile(FILE* f, float sx, float sy, float ex, float ey, float PixelsPerMM_X, float PixelsPerMM_Y)
{
	fprintf(f, "PLINESTART\n");
	fprintf(f, "%.02f, %.02f\n", sx * PixelsPerMM_X, sy * PixelsPerMM_Y);
	fprintf(f, "%.02f, %.02f\n", ex * PixelsPerMM_X, ey * PixelsPerMM_Y);
	fprintf(f, "PLINEEND\n");
}

void AddFooterToSIGFile(FILE* f)
{
	fprintf(f, "Setting\n");
	fprintf(f, "Setting\n");
	fprintf(f, "0,0,0,11\n");
	fprintf(f, "34082,0,0,0,1\n");
}

float GetLineLenMM(int sx, int sy, int ex, int ey, float PixelsPerMM_X, float PixelsPerMM_Y)
{
	int dx = sx - ex;
	int dy = sy - ey;
	dx *= PixelsPerMM_X;
	dy *= PixelsPerMM_Y;
	return sqrt(dx * dx + dy * dy);
}

#define MIN_LINE_LEN_TO_DRAW_MM	1

int main()
{
	FIBITMAP* dib = LoadImage_("SA_2_Tear.bmp"); // initialize lib
	int32_t Width = FreeImage_GetWidth(dib);
	int32_t Height = FreeImage_GetHeight(dib);

	float PixelsPerMM_X = Width / 235;
	float PixelsPerMM_Y = Height / 235;

	// largest line is 235 mm horizontally and 220 mm vertically
	for (size_t break_to_pieces = 1; break_to_pieces <= 4; break_to_pieces += 1)
	{
		FILE* sigfile;
		char fileName[500];
		sprintf_s(fileName, sizeof(fileName), "vert_%d.sig", break_to_pieces);
		errno_t openErr = fopen_s(&sigfile, fileName, "wt");
		if (sigfile == NULL)
		{
			printf("Failed to open output file\n");
			return 1;
		}
		int col = 0;
		int rows[2];
		while (ScanImageNextCol(dib, col, rows) == 0)
		{
			float lenMM = GetLineLenMM(col, rows[0], col, rows[1], PixelsPerMM_X, PixelsPerMM_Y);
			if (break_to_pieces == 1)
			{
				AddLineToSIGFile(sigfile, col, rows[0], col, rows[1], PixelsPerMM_X, PixelsPerMM_Y);
			}
			else if (break_to_pieces == 2)
			{
				for (size_t segment = 1; segment < rows[1] - rows[0]; segment++)
				{
					AddLineToSIGFile(sigfile, col, rows[0] + segment, col, rows[0] + segment + 1, PixelsPerMM_X, PixelsPerMM_Y);
				}
			}
			else if (lenMM / break_to_pieces > MIN_LINE_LEN_TO_DRAW_MM)
			{
				float segmentLen = lenMM / break_to_pieces;
				for (float segment = 0; segment < lenMM; segment += segmentLen)
				{
					AddLineToSIGFile(sigfile, col * PixelsPerMM_X, rows[0] * PixelsPerMM_Y + segment, col * PixelsPerMM_X, rows[0] + segment + segmentLen, 1, 1);
				}
			}
			col++;
		}
		AddFooterToSIGFile(sigfile);
		fclose(sigfile);
	}
	FreeImage_Unload(dib);

	FreeImage_DeInitialise();
	return 0;
}