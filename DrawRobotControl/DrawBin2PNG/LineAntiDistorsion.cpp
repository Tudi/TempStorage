#include "StdAfx.h"

#define POSITION_ADJUST_FILE_VERSION 1
#define ADJUST_MAP_DEFAULT_WIDTH 4096
#define ADJUST_MAP_DEFAULT_HEIGHT 4096

LineAntiDistorsionAdjuster sLineAdjuster;
PositionAdjustInfo adjustInfoMissing;

LineAntiDistorsionAdjuster::LineAntiDistorsionAdjuster()
{
	memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
	memset(&adjustInfoHeader, 0, sizeof(adjustInfoHeader));
	adjustInfoMap = NULL;
	hasUnsavedAdjustments = 0;
	needsBleed = 0;
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
		memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
		return &adjustInfoMissing;
	}
	if (x < 0 || x >= adjustInfoHeader.width)
	{
		printf("Requesting out of bounds adjust info\n");
		memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
		return &adjustInfoMissing;
	}
	if (y < 0 || y >= adjustInfoHeader.height)
	{
		printf("Requesting out of bounds adjust info\n");
		memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
		return &adjustInfoMissing;
	}
	return &adjustInfoMap[adjustInfoHeader.width * y + x];
}

// walk the line, at every position, adjust the movement based on the adjuster map
// do a second walk, coalesce half movement into rounded up / down movement
void LineAntiDistorsionAdjuster::AdjustLine(RelativePointsLine* line)
{
#if 0
	if (line == NULL)
	{
		SOFT_ASSERT(false, "Unexpected NULL param");
		return;
	}

	// adjust line movement based on the position of the pen( arm length changes)
	float curX = line->GetStartX();
	float curY = line->GetStartY();
	for (int i = 0; i < line->GetPointsCount(); i++)
	{
		float nextX = curX + line->GetDX(i);
		float nextY = curY + line->GetDY(i);
		PositionAdjustInfo* adjustInfo = GetAdjustInfo((int)curX, (int)curY);

		line->SetDX(i, line->GetDX(i) * adjustInfo->relativeCommandMultiplierX + adjustInfo->adjustX);
		line->SetDY(i, line->GetDY(i) * adjustInfo->relativeCommandMultiplierY + adjustInfo->adjustY);

		curX = nextX;
		curY = nextY;
	}

	PenRobotMovementCodesPrimary lineMovementX = Move1_Left;
	PenRobotMovementCodesPrimary lineMovementY = Move1_Up;

	// walk the line again. Since we can't issue half commands, we need to group these up
	curX = line->GetStartX();
	curY = line->GetStartY();
	float unusedDX = 0;
	float unusedDY = 0;
	for (int i = 0; i < line->GetPointsCount(); i++)
	{
		unusedDX += (line->GetDX(i) - (int)line->GetDX(i));
		unusedDY += (line->GetDY(i) - (int)line->GetDY(i));

		line->SetDX(i, (float)(int)line->GetDX(i));
		line->SetDY(i, (float)(int)line->GetDY(i));

		// we gathered enough "sub" movements that it's a full move
		if( unusedDX >= 1)
		{
			line->SetDX(i, (float)(line->GetDX(i) + 1));
			unusedDX -= 1;
		}
		else if (unusedDX <= -1)
		{
			line->SetDX(i, (float)(line->GetDX(i) - 1));
			unusedDX += 1;
		}

		if (unusedDY >= 1)
		{
			line->SetDY(i, (float)(line->GetDY(i) + 1));
			unusedDY -= 1;
		}
		else if (unusedDY <= -1)
		{
			line->SetDY(i, (float)(line->GetDY(i) - 1));
			unusedDY += 1;
		}

		float nextX = curX + line->GetDX(i);
		float nextY = curY + line->GetDY(i);
	}
#endif
}

void LineAntiDistorsionAdjuster::CreateNewMap(PositionAdjustInfoHeader* header)
{
	PositionAdjustInfoHeader tempHeader;
	// sanity checks
	if (header == NULL)
	{
		header = &tempHeader;
		header->width = ADJUST_MAP_DEFAULT_WIDTH;
		header->height = ADJUST_MAP_DEFAULT_HEIGHT;
		header->originX = header->width / 2;
		header->originY = header->height / 2;
		header->scaleX = 1;
		header->scaleY = 1;
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
	if (needsBleed)
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
	for (int radius = 1; radius < adjustInfoHeader.width; radius++)
	{
		PositionAdjustInfo* ai;
		int atx2, aty2;
		for (int y2 = y - radius; y2 <= y + radius; y2++)
		{
			atx2 = x - radius;
			aty2 = y2;
			ai = GetAdjustInfo(atx2, aty2);
			if (ai->flags & flag)
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
			atx2 = x + radius;
			aty2 = y2;
			ai = GetAdjustInfo(x + radius, y2);
			if (ai->flags & flag)
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
			ai = GetAdjustInfo(x2, y - radius);
			if (ai->flags & flag)
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
			atx2 = x2;
			aty2 = y + radius;
			ai = GetAdjustInfo(x2, y + radius);
			if (ai->flags & flag)
			{
				*out_ai = ai;
				atx = atx2;
				aty = aty2;
				return;
			}
		}
	}
}

void LineAntiDistorsionAdjuster::BleedAdjustmentsToNeighbours()
{
	needsBleed = 0;
	for (int y = 0; y < adjustInfoHeader.height; y++)
	{
		PositionAdjustInfo* aig;
		int atx, aty;
		for (int x = 0; x < adjustInfoHeader.width; x++)
		{
			PositionAdjustInfo* ai = GetAdjustInfo(x, y);
			// this position relocation is estimated
			// based on nearby neighbours, try to make a guess where it should be
			if ((ai->flags & X_IS_MEASURED) == 0)
			{
				FindClosestKnown(x, y, X_IS_MEASURED, &aig, atx, aty);
				if (aig != NULL)
				{
					ai->shouldBeX = aig->shouldBeX + x - atx;
				}
			}
			if ((ai->flags & Y_IS_MEASURED) == 0)
			{
				FindClosestKnown(x, y, Y_IS_MEASURED, &aig, atx, aty);
				if (aig != NULL)
				{
					ai->shouldBeY = aig->shouldBeY + y - aty;
				}
			}
		}
	}
}

void LineAntiDistorsionAdjuster::AdjustPositionX(int x, int y, int shouldBeX)
{
	needsBleed = 1;
	PositionAdjustInfo* ai = GetAdjustInfo(x, y);
	ai->shouldBeX = shouldBeX;
	ai->flags = (PositionAdjustInfoFlags)(ai->flags | X_IS_MEASURED);
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