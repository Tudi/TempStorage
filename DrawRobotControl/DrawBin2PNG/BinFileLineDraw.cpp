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
#define ADDED_DRAW_LINE_WIDTH 1

void DrawBinLineOnPNG(FIBITMAP* in_Img, double& x, double& y, RelativePointsLine* line)
{
	if (line == NULL || line->GetPointsCount() <= 0)
	{
		printf("Unexpected no length line\n");
		return;
	}

	size_t stride = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	int32_t Width = FreeImage_GetWidth(in_Img);
	int32_t Height = FreeImage_GetHeight(in_Img);
	size_t linePixels = (size_t)line->GetPointsCount();
	size_t drawLine = (size_t)line->getPenPosition();
	BYTE LineBaseR = 64, LineBaseG = 64, LineBaseB = 64;
	double startx = x, starty = y;
	if (drawLine
		&& linePixels > 4 // probably just position adjustment line
		)
	{
		GetLineColor(LineBaseR, LineBaseG, LineBaseB);
	}
	for (size_t i = 0; i < linePixels; i++)
	{
#if FLIP_DRAW_HORIZONTALLY == 0
		x += line->GetDX(i);
#else
		x -= line->moves[i].dx;
#endif
#if FLIP_DRAW_VERTICALLY == 0
		y += line->GetDY(i);
#else
		y -= line->GetDY(i);
#endif
		size_t canDraw = 1;
		if (x - ADDED_DRAW_LINE_WIDTH < 0)
		{
			canDraw = 0;
		}
		if (x + ADDED_DRAW_LINE_WIDTH >= Width)
		{
			canDraw = 0;
		}
		if (y - ADDED_DRAW_LINE_WIDTH < 0)
		{
			canDraw = 0;
		}
		if (y + ADDED_DRAW_LINE_WIDTH >= Height)
		{
			canDraw = 0;
		}
		if (drawLine && canDraw)
		{
			// start at full brightness and end with 50% brightness
			BYTE pixelR = (BYTE)((int)LineBaseR - ((double)i / (double)linePixels / 2.0 * LineBaseR));
			BYTE pixelG = (BYTE)((int)LineBaseG - ((double)i / (double)linePixels / 2.0 * LineBaseG));
			BYTE pixelB = (BYTE)((int)LineBaseB - ((double)i / (double)linePixels / 2.0 * LineBaseB));
			for (int y2 = -ADDED_DRAW_LINE_WIDTH; y2 <= ADDED_DRAW_LINE_WIDTH; y2++)
			{
				for (int x2 = -ADDED_DRAW_LINE_WIDTH; x2 <= ADDED_DRAW_LINE_WIDTH; x2++)
				{
					if (abs((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 0] - (int)pixelR) > 1
						&& abs((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 1] - (int)pixelG) > 1
						&& abs((int)BITS[((int)y + y2) * stride + ((int)x + x2) * Bytespp + 2] - (int)pixelB) > 1
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
	printf("Finished line draw from %.02f,%.02f to %.02f,%.02f\n", startx, starty, x, y);
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

// relative negative y means to draw it downwards on paper
// in memory, this would be a positive y
// Movement units are measured in robot commands ! That means it's rounded to 1 or 0
void DrawLineRelativeInMem(double sx, double sy, double ex, double ey, RelativePointsLine* out_line, double& leftOverX, double& leftOverY)
{
	out_line->setStartingPosition(sx, sy);
	out_line->setEndPosition(ex, ey);

	double dx = ex - sx;
	double dy = ey - sy;
	if (dx == dy && dx == 0)
	{
		return;
	}

	// just to increase the draw accuracy. More points, more smoothness
	double lineDrawSteps;
	if (abs(dy) > abs(dx))
	{
		lineDrawSteps = abs(dy);
	}
	else
	{
		lineDrawSteps = abs(dx);
	}

	double xIncForStep = dx / lineDrawSteps;
	double yIncForStep = dy / lineDrawSteps;
	double writtenX = -leftOverX;
	double writtenY = -leftOverY;
	double step = 0;
	do
	{
		step += 1;
		if (step > lineDrawSteps)
		{
			step = lineDrawSteps;
		}
		double curXPos = step * xIncForStep;
		double curYPos = step * yIncForStep;
		double xdiff = curXPos - writtenX;
		double ydiff = curYPos - writtenY;

		if (xdiff <= -1.0)
		{
			writtenX += -1.0;
			out_line->storeNextPoint(-1.0, 0);
		}
		else if (xdiff >= 1.0)
		{
			writtenX += 1.0;
			out_line->storeNextPoint(1.0, 0);
		}
		if (ydiff <= -1.0)
		{
			writtenY += -1.0;
			out_line->storeNextPoint(0, -1.0);
		}
		else if (ydiff >= 1.0)
		{
			writtenY += 1.0;
			out_line->storeNextPoint(0, 1.0);
		}
	} while (step != lineDrawSteps);

	double curXPos = lineDrawSteps * xIncForStep;
	double curYPos = lineDrawSteps * yIncForStep;
	leftOverX = curXPos - writtenX;
	leftOverY = curYPos - writtenY;
}

int RelativePointsLine::ensureCanStoreLinePoint(int count = 1)
{
	// stop doing stupid things
	if (count <= 0)
	{
		return 1;
	}
	if (numberOfPoints + count >= numberOfPointsCanStore)
	{
		int newCount = numberOfPointsCanStore + count + MIN_POINTS_LINE_EXTEND;
		RelativeLinePoint* newStore = (RelativeLinePoint*)realloc(moves, newCount * sizeof(RelativeLinePoint));
		if (newStore == NULL)
		{
			return 1;
		}
		numberOfPointsCanStore = newCount;
		moves = newStore;
	}
	return 0;
}

int RelativePointsLine::storeNextPoint(double dx, double dy)
{
	int err = ensureCanStoreLinePoint();
	if (err != 0)
	{
		return err;
	}
	SOFT_ASSERT(numberOfPoints < numberOfPointsCanStore, "Out of bounds index");
	moves[numberOfPoints].dx = (float)dx;
	moves[numberOfPoints].dy = (float)dy;
	numberOfPoints++;
	return 0;
}

float RelativePointsLine::GetDX(size_t at)
{
	SOFT_ASSERT(at < numberOfPointsCanStore, "Out of bounds index");
	return moves[at].dx;
}
float RelativePointsLine::GetDY(size_t at)
{
	SOFT_ASSERT(at < numberOfPointsCanStore, "Out of bounds index");
	return moves[at].dy;
}
void RelativePointsLine::SetDX(size_t at, float dx)
{
	SOFT_ASSERT(at < numberOfPointsCanStore, "Out of bounds index");
	moves[at].dx = dx;
}

void RelativePointsLine::SetDY(size_t at, float dy)
{
	SOFT_ASSERT(at < numberOfPointsCanStore, "Out of bounds index");
	moves[at].dy = dy;
}