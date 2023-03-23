#include "StdAfx.h"

#define POSITION_ADJUST_FILE_VERSION 1
#define ADJUST_MAP_DEFAULT_WIDTH 6000
#define ADJUST_MAP_DEFAULT_HEIGHT 6000
#define DEFAULT_WIDTH_SCALER 0.1f
#define DEFAULT_HEIGHT_SCALER 0.1f
#define LINE_GAP_CONSIDERED_BUG	20 // measured in movement units

LineAntiDistorsionAdjuster sLineAdjuster;
PositionAdjustInfo adjustInfoMissing;

LineAntiDistorsionAdjuster::LineAntiDistorsionAdjuster()
{
	memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
	memset(&adjustInfoHeader, 0, sizeof(adjustInfoHeader));
	adjustInfoMap = NULL;
	hasUnsavedAdjustments = 0;
	needsBleedX = needsBleedY = 0;
	// load static distorsion map
	LoadAdjusterMap();
}

LineAntiDistorsionAdjuster::~LineAntiDistorsionAdjuster()
{
	if (hasUnsavedAdjustments)
	{
		SaveAdjusterMap();
	}
	free(adjustInfoMap);
	adjustInfoMap = NULL;
}

PositionAdjustInfo* LineAntiDistorsionAdjuster::GetAdjustInfo(int x, int y)
{
	if (adjustInfoMap == NULL)
	{
		CreateNewMap(NULL);
	}
	if (adjustInfoMap == NULL)
	{
		return NULL;
	}
	x = (int)(x * adjustInfoHeader.scaleX);
	x += adjustInfoHeader.originX;
	if (x < 0 || x >= adjustInfoHeader.width)
	{
		return NULL;
	}
	y = (int)(y * adjustInfoHeader.scaleY);
	y += adjustInfoHeader.originY;
	if (y < 0 || y >= adjustInfoHeader.height)
	{
		return NULL;
	}
	return &adjustInfoMap[adjustInfoHeader.width * y + x];
}

void LineAntiDistorsionAdjuster::CreateNewMap(PositionAdjustInfoHeader* header)
{
	PositionAdjustInfoHeader tempHeader;
	// sanity checks
	if (header == NULL)
	{
		memset(&tempHeader, 0, sizeof(tempHeader));
		header = &tempHeader;
		header->scaleX = DEFAULT_WIDTH_SCALER;
		header->scaleY = DEFAULT_HEIGHT_SCALER;
		header->width = (int)(ADJUST_MAP_DEFAULT_WIDTH * header->scaleX);
		header->height = (int)(ADJUST_MAP_DEFAULT_HEIGHT * header->scaleY);
		header->originX = header->width / 2;
		header->originY = header->height / 2;
	}
	if (header->height == 0 || header->width == 0)
	{
		return;
	}

	SOFT_ASSERT(header->originX >= 0 && header->originX < header->width, "Origin X should reside inside the input map");
	SOFT_ASSERT(header->originY >= 0 && header->originY < header->height, "Origin Y should reside inside the input map");

	//get rid of old info
	free(adjustInfoMap);
	adjustInfoMap = NULL;

	adjustInfoHeader = *header;

	adjustInfoMap = (PositionAdjustInfo*)calloc(1, adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo));
}

void LineAntiDistorsionAdjuster::SaveAdjusterMap()
{
	hasUnsavedAdjustments = 0;
	if (needsBleedX || needsBleedY)
	{
		BleedAdjustmentsToNeighbours();
	}

	// nothing to save. create content first
	if (adjustInfoMap == NULL)
	{
		return;
	}

	adjustInfoHeader.version = POSITION_ADJUST_FILE_VERSION;
	adjustInfoHeader.fourCC = *(int*)CALIBRATION_4CC;
	adjustInfoHeader.headerSize = sizeof(PositionAdjustInfoHeader);
	adjustInfoHeader.infoSize = sizeof(PositionAdjustInfo);

	FILE* f;
	errno_t err_open = fopen_s(&f, CALIBRATION_FILE_NAME,"wb");
	if (f == NULL)
	{
		return;
	}

	// write header 
	fwrite(&adjustInfoHeader, sizeof(adjustInfoHeader), 1, f);

	//write the map
	fwrite(adjustInfoMap, sizeof(PositionAdjustInfo), adjustInfoHeader.width * adjustInfoHeader.height, f);

	// donw writing the map
	fclose(f);
}

void LineAntiDistorsionAdjuster::LoadAdjusterMap()
{
	FILE* f;
	errno_t err_open = fopen_s(&f, CALIBRATION_FILE_NAME, "rb");
	if (f == NULL)
	{
		return;
	}

	PositionAdjustInfoHeader fHeader;
	fread(&fHeader, sizeof(fHeader), 1, f);
	if (fHeader.fourCC != *(int*)CALIBRATION_4CC)
	{
		printf("Invalid calibration file\n");
		fclose(f);
		return;
	}
	if (fHeader.version != POSITION_ADJUST_FILE_VERSION)
	{
		printf("Invalid calibration file version\n");
		fclose(f);
		return;
	}
	if (fHeader.headerSize != sizeof(PositionAdjustInfoHeader))
	{
		printf("Invalid calibration file header size\n");
		fclose(f);
		return;
	}
	if (fHeader.infoSize != sizeof(PositionAdjustInfo))
	{
		printf("Invalid calibration file content size\n");
		fclose(f);
		return;
	}

	CreateNewMap(&fHeader);

	if (adjustInfoMap == NULL)
	{
		printf("Failed to load calibration file content\n");
		fclose(f);
		return;
	}

	fread(adjustInfoMap, sizeof(PositionAdjustInfo), adjustInfoHeader.width * adjustInfoHeader.height, f);

	fclose(f);
}

void LineAntiDistorsionAdjuster::FindClosestKnown(int x, int y, int flag, PositionAdjustInfo** out_ai, int &atx, int &aty)
{
	*out_ai = NULL;
	int maxRadius = MAX(adjustInfoHeader.width, adjustInfoHeader.height) / 2;
	for (int radius = 1; radius < maxRadius; radius++)
	{
		PositionAdjustInfo* ai;
		int atx2, aty2;
		for (int y2 = y - radius; y2 <= y + radius; y2++)
		{
			atx2 = x - radius;
			aty2 = y2;
			ai = GetAdjustInfoNoChange(atx2, aty2);
			if (ai && (ai->flags & flag))
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
			atx2 = x + radius;
			aty2 = y2;
			ai = GetAdjustInfoNoChange(x + radius, y2);
			if (ai && (ai->flags & flag))
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
		}
		for (int x2 = x - radius; x2 <= x + radius; x2++)
		{
			atx2 = x2;
			aty2 = y - radius;
			ai = GetAdjustInfoNoChange(x2, y - radius);
			if (ai && (ai->flags & flag))
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
			atx2 = x2;
			aty2 = y + radius;
			ai = GetAdjustInfoNoChange(x2, y + radius);
			if (ai && (ai->flags & flag))
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
		}
	}
}

PositionAdjustInfo* LineAntiDistorsionAdjuster::GetAdjustInfoNoChange(int x, int y)
{
	if (x < 0 || x >= adjustInfoHeader.width)
	{
		return NULL;
	}
	if (y < 0 || y >= adjustInfoHeader.height)
	{
		return NULL;
	}
	return &adjustInfoMap[adjustInfoHeader.width * y + x];

}

// There is most probably a faster way to do this, but I do not expect this function to be executed many times
void LineAntiDistorsionAdjuster::BleedAdjustmentsToNeighbours()
{
	if (needsBleedX)
	{
		needsBleedX = 0;
		uint32_t startStamp = GetTickCount();
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			PositionAdjustInfo* aig;
			int atx, aty;
			for (int x = 0; x < adjustInfoHeader.width; x++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				// this position relocation is estimated
				// based on nearby neighbours, try to make a guess where it should be
				if ((ai->flags & X_IS_MEASURED) == 0)
				{
					FindClosestKnown(x, y, X_IS_MEASURED, &aig, atx, aty);
					if (aig != NULL)
					{
						ai->shouldBeX = aig->shouldBeX + (x - atx) / adjustInfoHeader.scaleX;
					}
				}
			}
		}
		uint32_t endStamp = GetTickCount();
		printf("Took %f minutes to propagate calibration changes on X axis\n", (endStamp - startStamp) / 1000.0f / 60.0f);
	}
	if (needsBleedY)
	{
		needsBleedY = 0;
		uint32_t startStamp = GetTickCount();
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			PositionAdjustInfo* aig;
			int atx, aty;
			for (int x = 0; x < adjustInfoHeader.width; x++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				// this position relocation is estimated
				// based on nearby neighbours, try to make a guess where it should be
				if ((ai->flags & Y_IS_MEASURED) == 0)
				{
					FindClosestKnown(x, y, Y_IS_MEASURED, &aig, atx, aty);
					if (aig != NULL)
					{
						ai->shouldBeY = aig->shouldBeY - aty + y;
					}
				}
			}
		}
		uint32_t endStamp = GetTickCount();
		printf("Took %f minutes to propagate calibration changes on Y axis\n", (endStamp - startStamp) / 1000.0f / 60.0f);
	}
}

void LineAntiDistorsionAdjuster::AdjustPositionX(int x, int y, int shouldBeX)
{
	needsBleedX = 1;
	PositionAdjustInfo* ai = GetAdjustInfo(x, y);
	if (ai->shouldBeX != shouldBeX || (ai->flags & X_IS_MEASURED) == 0)
	{
		ai->shouldBeX = (float)shouldBeX;
		ai->flags = (PositionAdjustInfoFlags)(ai->flags | X_IS_MEASURED);
		hasUnsavedAdjustments = 1;
	}
}
/*
void LineAntiDistorsionAdjuster::MarkAdjustmentsOutdated()
{
	for (size_t y = 0; y < adjustInfoHeader.height; y++)
	{
		for (size_t x = 0; x < adjustInfoHeader.width; x++)
		{
			PositionAdjustInfo* ai = GetAdjustInfo(x, y);
			ai->isMeasured = 0;
		}
	}
}*/

void LineAntiDistorsionAdjuster::DrawLine(float sx, float sy, float ex, float ey, RelativePointsLine* out_line)
{
	double dx = ex - sx;
	double dy = ey - sy;
	if (dx == dy && dx == 0)
	{
		return;
	}

	out_line->setStartingPosition(sx, sy);

	// just to increase the draw accuracy. More points, more smoothness
	double out_lineDrawSteps;
	if (abs(dy) > abs(dx))
	{
		out_lineDrawSteps = abs(dy);
	}
	else
	{
		out_lineDrawSteps = abs(dx);
	}

	int curx = (int)sx;
	int cury = (int)sy;

	double xIncForStep = dx / out_lineDrawSteps;
	double yIncForStep = dy / out_lineDrawSteps;
	double prevAddedX = 0x7FFFFFFF;
	double prevAddedY = 0x7FFFFFFF;

	PositionAdjustInfo* ai = sLineAdjuster.GetAdjustInfo((int)sx, (int)sy);
	if (ai != NULL && ai->shouldBeX != 0 && ai->shouldBeY != 0)
	{
		prevAddedX = ai->shouldBeX;
		prevAddedY = ai->shouldBeY;
	}

	for (double step = 1; step <= out_lineDrawSteps; step += 1)
	{
		double curXPos = step * xIncForStep;
		double curYPos = step * yIncForStep;
		double curXPos2 = 0, curYPos2 = 0;

		PositionAdjustInfo* ai = sLineAdjuster.GetAdjustInfo((int)curXPos, (int)curYPos);
		if (ai != NULL && ai->shouldBeX != 0 && ai->shouldBeY != 0)
		{
			curXPos2 = ai->shouldBeX;
			curYPos2 = ai->shouldBeY;
		}
		else
		{
			printf("missing info in callibration map at %f %f. Should have not happened\n", curXPos, curYPos);
			continue;
		}

		int xdiff = (int)(curXPos2 - prevAddedX);
		int ydiff = (int)(curYPos2 - prevAddedY);

		if (xdiff == 0 && ydiff == 0)
		{
			continue;
		}

		prevAddedX = curXPos2;
		prevAddedY = curYPos2;
		if (abs(xdiff) > LINE_GAP_CONSIDERED_BUG || abs(ydiff) > LINE_GAP_CONSIDERED_BUG)
		{
			printf("Line gap at %f,%f is too large to continue drawing\n", curXPos, curYPos);
			continue;
		}

		out_line->storeNextPoint(xdiff, ydiff);
	}

	out_line->setEndPosition(sx + dx, sy + dy);
}
