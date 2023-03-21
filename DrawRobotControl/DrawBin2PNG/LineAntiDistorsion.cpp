#include "StdAfx.h"

#define POSITION_ADJUST_FILE_VERSION 1

LineAntiDistorsionAdjuster sLineAdjuster;
PositionAdjustInfo adjustInfoMissing;

LineAntiDistorsionAdjuster::LineAntiDistorsionAdjuster()
{
	memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
	memset(&adjustInfoHeader, 0, sizeof(adjustInfoHeader));
	adjustInfoMap = NULL;
	// load static distorsion map
}

LineAntiDistorsionAdjuster::~LineAntiDistorsionAdjuster()
{
	free(adjustInfoMap);
	adjustInfoMap = NULL;
}

PositionAdjustInfo* LineAntiDistorsionAdjuster::GetAdjustInfo(int x, int y)
{
	if (adjustInfoMap == NULL)
	{
		return &adjustInfoMissing;
	}
	if (x < 0 || x >= adjustInfoHeader.width)
	{
		return &adjustInfoMissing;
	}
	if (y < 0 || y >= adjustInfoHeader.height)
	{
		return &adjustInfoMissing;
	}
	return &adjustInfoMap[adjustInfoHeader.width * y + x];
}

// walk the line, at every position, adjust the movement based on the adjuster map
// do a second walk, coalesce half movement into rounded up / down movement
void LineAntiDistorsionAdjuster::AdjustLine(RelativePointsLine* line)
{
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
}

void LineAntiDistorsionAdjuster::CreateNewMap(PositionAdjustInfoHeader* header)
{
	// sanity checks
	if (header == NULL)
	{
		return;
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
	adjustInfoHeader.version = POSITION_ADJUST_FILE_VERSION;
	adjustInfoHeader.fourCC = *(int*)CALIBRATION_4CC;
	adjustInfoHeader.headerSize = sizeof(PositionAdjustInfoHeader);
	adjustInfoHeader.infoSize = sizeof(PositionAdjustInfo);

	// nothing to save. create content first
	if (adjustInfoMap == NULL)
	{
		return;
	}

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