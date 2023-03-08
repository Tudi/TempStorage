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

void DrawBinLineOnPNG(FIBITMAP* in_Img, float& x, float& y, RelativePointsLine* line)
{
	if (line == NULL || line->numberOfPoints <= 0)
	{
		printf("Unexpected no length line\n");
		return;
	}

	size_t stride = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	int32_t Width = FreeImage_GetWidth(in_Img);
	int32_t Height = FreeImage_GetHeight(in_Img);
	size_t linePixels = (size_t)line->numberOfPoints;
	size_t drawLine = (size_t)line->penPosition;
	BYTE LineBaseR = 64, LineBaseG = 64, LineBaseB = 64;
	if (drawLine 
		&& linePixels > 4 // probably just position adjustment line
		)
	{
		GetLineColor(LineBaseR, LineBaseG, LineBaseB);
	}
#define LineSize 2
	for (size_t i = 0; i < linePixels; i++)
	{
#if FLIP_DRAW_HORIZONTALLY == 0
		x += line->moves[i].dx;
#else
		x -= line->moves[i].dx;
#endif
#if FLIP_DRAW_VERTICALLY == 0
		y += line->moves[i].dy;
#else
		y -= line->moves[i].dy;
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

void DrawLineRelativeInMem(float sx, float sy, float ex, float ey, RelativePointsLine** line)
{
	double dx = sx - ex;
	double dy = sy - ey;
	if (dx == dy && dx == 0)
	{
		return;
	}

	RelativePointsLine::setPenPosition(line, 1);
	RelativePointsLine::setStartingPosition(line, sx, sy);

	// just to increase the draw accuracy. More points, more smoothness
	if (dy > dx)
	{
		double lineDrawSteps = dy;
		double xIncForY = dx / lineDrawSteps;
		for (double step = 0; step <= lineDrawSteps; step += 1)
		{
			double x_rel = step * xIncForY;
			double y_rel = step;
			RelativePointsLine::storeNextPoint(line, x_rel, y_rel);
		}
	}
	else 
	{
		double lineDrawSteps = dx;
		double yIncForx = dy / lineDrawSteps;
		for (double step = 0; step <= lineDrawSteps; step += 1)
		{
			double x_rel = step;
			double y_rel = step * yIncForx;
			RelativePointsLine::storeNextPoint(line, x_rel, y_rel);
		}
	}
}

int RelativePointsLine::ensureCanStoreLinePoint(RelativePointsLine** line, int count = 1)
{
	// stop doing stupid things
	if (count <= 0)
	{
		return 1;
	}
	if (*line == NULL)
	{
		*line = (RelativePointsLine*)malloc(sizeof(RelativePointsLine) + (count + MIN_POINTS_LINE_EXTEND) * sizeof(RelativeLinePoint));
		if (*line == NULL)
		{
			// should report mem allocation error
			return 1; 
		}
		(*line)->numberOfPointsCanStore = count + MIN_POINTS_LINE_EXTEND;
		(*line)->numberOfPoints = 0;
		(*line)->startx = 0;
		(*line)->starty = 0;
	}
	else if((*line)->numberOfPoints + count >= (*line)->numberOfPointsCanStore)
	{
		(*line)->numberOfPointsCanStore += count + MIN_POINTS_LINE_EXTEND;
		RelativePointsLine* newStore = (RelativePointsLine*)realloc((*line), sizeof(RelativePointsLine) + (*line)->numberOfPointsCanStore * sizeof(RelativeLinePoint));
		if (newStore == NULL)
		{
			return 1;
		}
		*line = newStore;
	}
	return 0;
}

int RelativePointsLine::storeNextPoint(RelativePointsLine** line, double dx, double dy)
{
	if (int err = ensureCanStoreLinePoint(line) != 0)
	{
		return err;
	}
	(*line)->moves[(*line)->numberOfPoints].dx = (float)dx;
	(*line)->moves[(*line)->numberOfPoints].dy = (float)dy;
	(*line)->numberOfPoints++;
	return 0;
}

int RelativePointsLine::setPenPosition(RelativePointsLine** line, int penPos)
{
	if (int err = ensureCanStoreLinePoint(line) != 0)
	{
		return err;
	}
	(*line)->penPosition = penPos;
	return 0;
}

int RelativePointsLine::setStartingPosition(RelativePointsLine** line, double sx, double sy)
{
	if (int err = ensureCanStoreLinePoint(line) != 0)
	{
		return err;
	}
	(*line)->startx = (float)sx;
	(*line)->starty = (float)sy;
	return 0;
}