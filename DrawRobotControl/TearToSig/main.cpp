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
	rows[0] = rows[1] = 0;
	//does this column have at least 2 red pixels ?
	for (; col < Width; col++)
	{
		int RedsFound = 0;
		int IsRed = 0;
		for (size_t y = 0; y < Height; y++)
		{
			if (IsTearRed(&BITS[((int)y) * stride + ((int)col) * Bytespp + 0]) && y < (Height - 1))
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
		if (RedsFound == 2)
		{
//			printf("At col %d found %d reds at %d, %d\n", col, RedsFound, rows[0], rows[1]);
			return 0;
		}
		else
		{
			printf("!!! At col %d only found %d reds at %d, %d. Needed 2\n", col, RedsFound, rows[0], rows[1]);
		}
	}
	return 1;
}

int ScanImageNextRow(FIBITMAP* in_Img, int32_t& row, int32_t* cols)
{
	size_t stride = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	int32_t Width = FreeImage_GetWidth(in_Img);
	int32_t Height = FreeImage_GetHeight(in_Img);
	cols[0] = cols[1] = 0;
	//does this column have at least 2 red pixels ?
	for (; row < Height; row++)
	{
		int RedsFound = 0;
		int IsRed = 0;
		for (size_t x = 0; x < Width; x++)
		{
			if (IsTearRed(&BITS[((int)row) * stride + ((int)x) * Bytespp + 0]) && x < (Width - 1))
			{
				IsRed = 1;
			}
			else if (IsRed)
			{
				IsRed = 0;
				if (RedsFound < 2)
				{
					cols[RedsFound] = (int32_t)x;
				}
				else
				{
					break;
				}
				RedsFound++;
			}
		}
		if (RedsFound == 2)
		{
//			printf("At row %d found %d reds at %d, %d\n", row, RedsFound, cols[0], cols[1]);
			return 0;
		}
		else
		{
			printf("!!! At row %d only found %d reds at %d, %d. Needed 2\n", row, RedsFound, cols[0], cols[1]);
		}
	}
	return 1;
}

static float colOffset = 0;
static float rowOffset = 0;
void AddLineToSIGFile(FILE* f, float sx, float sy, float ex, float ey, float PixelsPerINCH_X, float PixelsPerINCH_Y)
{
	fprintf(f, "PLINESTART\n");
	fprintf(f, "%.02f, %.02f\n", colOffset + sx / PixelsPerINCH_X, rowOffset + sy / PixelsPerINCH_Y);
	fprintf(f, "%.02f, %.02f\n", colOffset + ex / PixelsPerINCH_X, rowOffset + ey / PixelsPerINCH_Y);
	fprintf(f, "PLINEEND\n");
}

void AddFooterToSIGFile(FILE* f)
{
	fprintf(f, "Setting\n");
	fprintf(f, "Setting\n");
	fprintf(f, "0,0,0,11\n");
	fprintf(f, "34082,0,0,0,1\n");
}

float GetLineLenINCH(int sx, int sy, int ex, int ey, float PixelsPerINCH_X, float PixelsPerINCH_Y)
{
	float dx = (float)(sx - ex);
	float dy = (float)(sy - ey);
	dx = ((float)dx / PixelsPerINCH_X);
	dy = ((float)dy / PixelsPerINCH_Y);
	return (float)sqrt(dx * dx + dy * dy);
}

#define ORIGIN_X 281.0f // in pixels. Need to update this number if input image gets flipped !
#define ORIGIN_Y 214.0f 
#define TEARDROP_INCH_WIDTH 9.25f
#define TEARDROP_INCH_HEIGHT 8.66f
#define MIN_LINE_LEN_TO_DRAW_INCH	0.0393701f // around 1 mm

int GenSigFileColWithBreaks(FIBITMAP* dib, size_t break_to_pieces, float PixelsPerINCH_X, float PixelsPerINCH_Y)
{
	FILE* sigfile;
	char fileName[500];
	sprintf_s(fileName, sizeof(fileName), "vert_%d.sig", (int)break_to_pieces);
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
		float lenINCH = GetLineLenINCH(col, rows[0], col, rows[1], PixelsPerINCH_X, PixelsPerINCH_Y);
		float segmentLen = lenINCH / (float)break_to_pieces;
		if (break_to_pieces == 2) // lots of very small lines
		{
			segmentLen = MIN_LINE_LEN_TO_DRAW_INCH;
		}
		size_t segment_count = (size_t)(lenINCH / segmentLen);
		float segment_len_pixel = (float)(rows[1] - rows[0]) / (float)segment_count;
		for (size_t segment = 0; segment < segment_count; segment++)
		{
			float start = rows[0] + segment_len_pixel * segment;
			float end = rows[0] + segment_len_pixel * (segment + 1);
			AddLineToSIGFile(sigfile, (float)col, (float)start, (float)col, (float)end, PixelsPerINCH_X, PixelsPerINCH_Y);
		}
		col++;
	}
	AddFooterToSIGFile(sigfile);
	fclose(sigfile);
	return 0;
}

int GenSigFileRowWithBreaks(FIBITMAP* dib, size_t break_to_pieces, float PixelsPerINCH_X, float PixelsPerINCH_Y)
{
	FILE* sigfile;
	char fileName[500];
	sprintf_s(fileName, sizeof(fileName), "hor_%d.sig", (int)break_to_pieces);
	errno_t openErr = fopen_s(&sigfile, fileName, "wt");
	if (sigfile == NULL)
	{
		printf("Failed to open output file\n");
		return 1;
	}
	int row = 0;
	int cols[2];
	while (ScanImageNextRow(dib, row, cols) == 0)
	{
		float lenINCH = GetLineLenINCH(cols[0], row, cols[1], row, PixelsPerINCH_X, PixelsPerINCH_Y);
		float segmentLen = lenINCH / (float)break_to_pieces;
		if (break_to_pieces == 2) // lots of very small lines
		{
			segmentLen = MIN_LINE_LEN_TO_DRAW_INCH;
		}
		size_t segment_count = (size_t)(lenINCH / segmentLen);
		float segment_len_pixel = (float)(cols[1] - cols[0]) / (float)segment_count;
		for (size_t segment = 0; segment < segment_count; segment++)
		{
			float start = cols[0] + segment_len_pixel * segment;
			float end = cols[0] + segment_len_pixel * (segment + 1);
			AddLineToSIGFile(sigfile, (float)start, (float)row, (float)end, (float)row, PixelsPerINCH_X, PixelsPerINCH_Y);
		}
		row++;
	}
	AddFooterToSIGFile(sigfile);
	fclose(sigfile);
	return 0;
}

int main()
{
	FIBITMAP* dib = LoadImage_("SA_2_Tear.bmp"); // initialize lib
	int32_t Width = FreeImage_GetWidth(dib);
	int32_t Height = FreeImage_GetHeight(dib);

	float PixelsPerINCH_X = Width / TEARDROP_INCH_WIDTH;
	float PixelsPerINCH_Y = Height / TEARDROP_INCH_HEIGHT;

	float origin_x_pct = ORIGIN_X / Width;
	float origin_y_pct = ORIGIN_Y / Height;

	colOffset = -TEARDROP_INCH_WIDTH * origin_x_pct;
	rowOffset = -TEARDROP_INCH_HEIGHT * origin_y_pct;

	// largest line is 235 INCH horizontally and 220 INCH vertically
	for (size_t break_to_pieces = 1; break_to_pieces <= 4; break_to_pieces += 1)
	{
		GenSigFileColWithBreaks(dib, break_to_pieces, PixelsPerINCH_X, PixelsPerINCH_Y);
		GenSigFileRowWithBreaks(dib, break_to_pieces, PixelsPerINCH_X, PixelsPerINCH_Y);
	}
	FreeImage_Unload(dib);

	FreeImage_DeInitialise();
	return 0;
}