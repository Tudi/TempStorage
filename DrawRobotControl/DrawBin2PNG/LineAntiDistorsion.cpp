#include "StdAfx.h"

#define POSITION_ADJUST_FILE_VERSION 1

LineAntiDistorsionAdjuster sLineAdjuster;
PositionAdjustInfo adjustInfoMissing;

LineAntiDistorsionAdjuster::LineAntiDistorsionAdjuster()
{
	memset(&adjustInfoMissing, 0, sizeof(adjustInfoMissing));
	memset(&adjustinfoHeader, 0, sizeof(adjustinfoHeader));
	adjustInfoMap = NULL;
	// load static distorsion map
}

PositionAdjustInfo* LineAntiDistorsionAdjuster::GetAdjustInfo(int x, int y)
{
	if (adjustInfoMap == NULL)
	{
		return &adjustInfoMissing;
	}
	if (x < 0 || x >= adjustinfoHeader.width)
	{
		return &adjustInfoMissing;
	}
	if (y < 0 || y >= adjustinfoHeader.height)
	{
		return &adjustInfoMissing;
	}
	return &adjustInfoMap[adjustinfoHeader.width * y + x];
}

// walk the line, at every position, adjust the movement based on the adjuster map
// do a second walk, coalesce half movement into rounded up / down movement
void LineAntiDistorsionAdjuster::AdjustLine(RelativePointsLine** line)
{
	if (*line == NULL)
	{
		SOFT_ASSERT(false, "Unexpected NULL param");
		return;
	}

	// adjust line movement based on the position of the pen( arm length changes)
	float curX = (*line)->GetStartX();
	float curY = (*line)->GetStartY();
	for (int i = 0; i < (*line)->GetPointsCount(); i++)
	{
		float nextX = curX + (*line)->GetDX(i);
		float nextY = curY + (*line)->GetDY(i);
		PositionAdjustInfo* adjustInfo = GetAdjustInfo((int)curX, (int)curY);

		(*line)->SetDX(i, (*line)->GetDX(i) * adjustInfo->relativeCommandMultiplierX + adjustInfo->adjustX);
		(*line)->SetDY(i, (*line)->GetDY(i) * adjustInfo->relativeCommandMultiplierY + adjustInfo->adjustY);

		curX = nextX;
		curY = nextY;
	}

	PenRobotMovementCodesPrimary lineMovementX = Move1_Left;
	PenRobotMovementCodesPrimary lineMovementY = Move1_Up;

	// walk the line again. Since we can't issue half commands, we need to group these up
	curX = (*line)->GetStartX();
	curY = (*line)->GetStartY();
	float unusedDX = 0;
	float unusedDY = 0;
	for (int i = 0; i < (*line)->GetPointsCount(); i++)
	{
		unusedDX += ((*line)->GetDX(i) - (int)(*line)->GetDX(i));
		unusedDY += ((*line)->GetDY(i) - (int)(*line)->GetDY(i));

		(*line)->SetDX(i, (float)(int)(*line)->GetDX(i));
		(*line)->SetDY(i, (float)(int)(*line)->GetDY(i));

		// we gathered enough "sub" movements that it's a full move
		if (abs(unusedDX) >= 1)
		{
			if (lineMovementX == Move1_Left)
			{
				// dx is 
				if (unusedDX < 0)
				{

				}
			}
		}
		float nextX = curX + (*line)->GetDX(i);
		float nextY = curY + (*line)->GetDY(i);
	}
}