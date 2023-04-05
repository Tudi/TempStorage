#include "StdAfx.h"

#define POSITION_ADJUST_FILE_VERSION 3
// the physical tear size is around 10x10 inches
// we need around 600 commands to draw 1 inch => we would need 6000x6000 map to represent every command
// the map in the file would be [-3000,3000]x[-3000,3000]
#define ADJUST_MAP_DEFAULT_WIDTH 1600
#define ADJUST_MAP_DEFAULT_HEIGHT 1600
// we can adjust every 10th command. Inbetween values should be smoothed by line draw
#define DEFAULT_WIDTH_SCALER 0.30f 
#define DEFAULT_HEIGHT_SCALER 0.30f
#define LINE_GAP_CONSIDERED_BUG	100 // measured in movement units. 600 movements = 1 inch

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
	int x2 = (int)((float)x * adjustInfoHeader.scaleX + adjustInfoHeader.originX);
	if (x2 < 0 || x2 >= adjustInfoHeader.width)
	{
		return NULL;
	}
	int y2 = (int)((float)y * adjustInfoHeader.scaleY + adjustInfoHeader.originY);
	if (y2 < 0 || y2 >= adjustInfoHeader.height)
	{
		return NULL;
	}
	return &adjustInfoMap[adjustInfoHeader.width * y2 + x2];
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
		header->width = (int)(ADJUST_MAP_DEFAULT_WIDTH);
		header->height = (int)(ADJUST_MAP_DEFAULT_HEIGHT);
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

	adjustInfoMap = (PositionAdjustInfo*)malloc(adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo));
	if (adjustInfoMap != NULL)
	{
		memset(adjustInfoMap, 0, adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo));
	}
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
	fwrite(adjustInfoMap, sizeof(PositionAdjustInfo) * adjustInfoHeader.width * adjustInfoHeader.height, 1, f);

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

	fread(adjustInfoMap, sizeof(PositionAdjustInfo) * adjustInfoHeader.width * adjustInfoHeader.height, 1, f);

	fclose(f);
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

void LineAntiDistorsionAdjuster::FindClosestKnownRight(int sx, int sy, PositionAdjustInfo** out_right, int& atx_right)
{
//#define PROPAGATE_SEARCH_RADIUS (adjustInfoHeader.width * 10 / 100)
#define PROPAGATE_SEARCH_RADIUS 100
	*out_right = NULL;
	for (int x = sx + 1; x < sx + PROPAGATE_SEARCH_RADIUS; x++)
	{
		PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, sy);
		if (ai && (ai->flags & X_IS_MEASURED))
		{
			*out_right = ai;
			atx_right = x;
			return;
		}
	}
}

void LineAntiDistorsionAdjuster::FindClosestKnownDown(int sx, int sy, PositionAdjustInfo** out_down, int& aty_down)
{
	*out_down = NULL;
	for (int y = sy + 1; y < sy + PROPAGATE_SEARCH_RADIUS; y++)
	{
		PositionAdjustInfo* ai = GetAdjustInfoNoChange(sx, y);
		if (ai && ai->HasYMeasured())
		{
			*out_down = ai;
			aty_down = y;
			return;
		}
	}
}

// There is most probably a faster way to do this, but I do not expect this function to be executed many times
void LineAntiDistorsionAdjuster::BleedAdjustmentsToNeighbours()
{
	int applySmoothing = needsBleedX | needsBleedY;

#define APPLY_SMOOTHING_OF_MEASURED_VALUES_ON_MAP // due to scanner precision, values will oscilate. Blend these oscilations
#ifdef APPLY_SMOOTHING_OF_MEASURED_VALUES_ON_MAP
	if (applySmoothing)
	{
		PositionAdjustInfo* adjustInfoMapOld = (PositionAdjustInfo*)malloc(adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo));
		if (adjustInfoMapOld == NULL)
		{
			return;
		}
		memcpy(adjustInfoMapOld, adjustInfoMap, adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo));

		uint32_t startStamp = GetTickCount();
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			for (int x = 0; x < adjustInfoHeader.width; x++)
			{
				// only process locations that do not have real values
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				// only smooth positions that have a measured value set
				if (ai->HasXMeasured() == 0 && ai->HasYMeasured() == 0)
				{
					continue;
				}
				int sumX = 0;
				int sumY = 0;
				int sumCountX = 0;
				int sumCountY = 0;
#define SMOOTH_RADIUS 3
				for (int y2 = y - SMOOTH_RADIUS; y2 <= y + SMOOTH_RADIUS; y2++)
				{
					for (int x2 = x - SMOOTH_RADIUS; x2 <= x + SMOOTH_RADIUS; x2++)
					{
						PositionAdjustInfo* ai2 = NULL;
						if (x2 < 0 || x2 >= adjustInfoHeader.width)
						{
							continue;
						}
						if (y2 < 0 || y2 >= adjustInfoHeader.height)
						{
							continue;
						}
						ai2 = &adjustInfoMapOld[adjustInfoHeader.width * y2 + x2];
						if (ai2->HasXMeasured())
						{
							sumX += ai2->GetNewX();
							sumCountX += 1;
						}
						if (ai2->HasYMeasured())
						{
							sumY += ai2->GetNewY();
							sumCountY += 1;
						}
					}
				}
				if (sumCountX > 1 && ai->HasXMeasured())
				{
					int newX = (int)(sumX / sumCountX);
					ai->SetNewX(newX);
				}
				if (sumCountY > 1 && ai->HasYMeasured())
				{
					int newY = (int)(sumY / sumCountY);
					ai->SetNewY(newY);
				}
			}
		}
		uint32_t endStamp = GetTickCount();
		printf("Took %f minutes to smooth calibration transitions\n", (endStamp - startStamp) / 1000.0f / 60.0f);
		free(adjustInfoMapOld);
	}
#endif

	if (needsBleedX)
	{
		needsBleedX = 0;
		uint32_t startStamp = GetTickCount();
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			PositionAdjustInfo* prev_ai = NULL, *next_ai = NULL;
			int atx_prev = 0, atx_next = 0;
			// find first value that we will propagate to the left
			for (int firstX = 0; firstX < adjustInfoHeader.width; firstX++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(firstX, y);
				if (ai && ai->HasXMeasured())
				{
					atx_prev = firstX;
					prev_ai = ai;
					break;
				}
			}
			// this row does not contain any calibration info
			if (prev_ai == NULL)
			{
				continue;
			}
			// propagate first value to the start of the row
			for (int x = MAX(0, atx_prev - PROPAGATE_SEARCH_RADIUS); x < atx_prev; x++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				int distanceMapUnits = (int)((x - atx_prev) / adjustInfoHeader.scaleX);
//				int distanceMapUnits = (int)((atx_prev - x) / adjustInfoHeader.scaleX);
				ai->SetNewX(prev_ai->GetNewX() + distanceMapUnits);
			}
			// while we can find a next
			FindClosestKnownRight(atx_prev, y, &next_ai, atx_next);
			while (next_ai != NULL)
			{
				// blend values from start to end
				int correctionStartVal = prev_ai->GetNewX();
				int correctionEndVal = next_ai->GetNewX();
				double valuesToSplit = correctionEndVal - correctionStartVal;
				double stepsThatSplit = atx_next - atx_prev + 1;
				double correctionPerStep = valuesToSplit / stepsThatSplit;
				double correctionToAdd = correctionPerStep;
				//spread out the values between the 2 known points
				for (int x = atx_prev + 1; x < atx_next; x++)
				{
					PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
					ai->SetNewX((int)(correctionStartVal + correctionToAdd));
					correctionToAdd += correctionPerStep;
				}
				// jump to next values
				prev_ai = next_ai;
				atx_prev = atx_next;
				FindClosestKnownRight(atx_prev, y, &next_ai, atx_next);
			}
			// No more next, spread the last known value until the end of the line
			for (int x = atx_prev + 1; x < MIN(atx_prev + PROPAGATE_SEARCH_RADIUS, adjustInfoHeader.width); x++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				int distanceMapUnits = (int)((x - atx_prev) / adjustInfoHeader.scaleX);
				ai->SetNewX(prev_ai->GetNewX() + distanceMapUnits);
			}
		}
		uint32_t endStamp = GetTickCount();
		printf("Took %f minutes to propagate calibration changes on X axis\n", (endStamp - startStamp) / 1000.0f / 60.0f);
	}

	if (needsBleedY)
	{
		needsBleedY = 0;
		uint32_t startStamp = GetTickCount();
		for (int x = 0; x < adjustInfoHeader.width; x++)
		{
			PositionAdjustInfo* prev_ai = NULL, * next_ai = NULL;
			int aty_prev = 0, aty_next = 0;
			// find first value that we will propagate to the left
			for (int firstY = 0; firstY < adjustInfoHeader.height; firstY++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, firstY);
				if (ai && ai->HasYMeasured())
				{
					aty_prev = firstY;
					prev_ai = ai;
					break;
				}
			}
			// this col does not contain any calibration info
			if (prev_ai == NULL)
			{
				continue;
			}
			// propagate first value to the start of the col
			for (int y = MAX(0, aty_prev - PROPAGATE_SEARCH_RADIUS); y < aty_prev; y++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				int distanceMapUnits = (int)((y - aty_prev) / adjustInfoHeader.scaleY);
				ai->SetNewY(prev_ai->GetNewY() + distanceMapUnits);
			}
			// while we can find a next
			FindClosestKnownDown(x, aty_prev, &next_ai, aty_next);
			while (next_ai != NULL)
			{
				// blend values from start to end
				int correctionStartVal = prev_ai->GetNewY();
				int correctionEndVal = next_ai->GetNewY();
				double valuesToSplit = correctionEndVal - correctionStartVal;
				double stepsThatSplit = aty_next - aty_prev + 1;
				double correctionPerStep = valuesToSplit / stepsThatSplit;
				double correctionToAdd = correctionPerStep;
				//spread out the values between the 2 known points
				for (int y = aty_prev + 1; y < aty_next; y++)
				{
					PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
					ai->SetNewY((int)(correctionStartVal + correctionToAdd));
					correctionToAdd += correctionPerStep;
				}
				// jump to next values
				prev_ai = next_ai;
				aty_prev = aty_next;
				FindClosestKnownDown(x, aty_prev, &next_ai, aty_next);
			}
			// No more next, spread the last known value until the end of the line
			for (int y = aty_prev + 1; y < MIN(aty_prev + PROPAGATE_SEARCH_RADIUS, adjustInfoHeader.height); y++)
			{
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				int distanceMapUnits = (int)((y - aty_prev) / adjustInfoHeader.scaleY);
				ai->SetNewY(prev_ai->GetNewY() + distanceMapUnits);
			}
		}
		uint32_t endStamp = GetTickCount();
		printf("Took %f minutes to propagate calibration changes on Y axis\n", (endStamp - startStamp) / 1000.0f / 60.0f);
	}
//#define APPLY_SMOOTHING_ON_MAP
#ifdef APPLY_SMOOTHING_ON_MAP
	if (applySmoothing)
	{
		PositionAdjustInfo* adjustInfoMapOld = (PositionAdjustInfo*)malloc(adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo));
		if (adjustInfoMapOld == NULL)
		{
			return;
		}
		memcpy(adjustInfoMapOld, adjustInfoMap, adjustInfoHeader.width* adjustInfoHeader.height * sizeof(PositionAdjustInfo));

		uint32_t startStamp = GetTickCount();
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			for (int x = 0; x < adjustInfoHeader.width; x++)
			{
				// only process locations that do not have real values
				PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
				int sumX = 0;
				int sumY = 0;
				int sumCountX = 0;
				int sumCountY = 0;
#define SMOOTH_RADIUS 3
				for (int y2 = y - SMOOTH_RADIUS; y2 <= y + SMOOTH_RADIUS; y2++)
				{
					for (int x2 = x - SMOOTH_RADIUS; x2 <= x + SMOOTH_RADIUS; x2++)
					{
						PositionAdjustInfo* ai2 = NULL;
						if (x2 < 0 || x2 >= adjustInfoHeader.width)
						{
							continue;
						}
						if (y2 < 0 || y2 >= adjustInfoHeader.height)
						{
							continue;
						}
						ai2 = &adjustInfoMapOld[adjustInfoHeader.width * y2 + x2];
						if (ai2->HasX())
						{
							sumX += ai2->GetNewX();
							sumCountX += 1;
						}
						if (ai2->HasY())
						{
							sumY += ai2->GetNewY();
							sumCountY += 1;
						}
					}
				}
				if (sumCountX > 1)
				{
					int newX = (int)(sumX / sumCountX);
					ai->SetNewX(newX);
				}
				if (sumCountY > 1)
				{
					int newY = (int)(sumY / sumCountY);
					ai->SetNewY(newY);
				}
			}
		}
		uint32_t endStamp = GetTickCount();
		printf("Took %f minutes to smooth calibration transitions\n", (endStamp - startStamp) / 1000.0f / 60.0f);
		free(adjustInfoMapOld);
	}
#endif
}

void LineAntiDistorsionAdjuster::AdjustPositionX(int x, int y, int shouldBeX)
{
	needsBleedX = 1;
	PositionAdjustInfo* ai = GetAdjustInfo(x, y);
	SOFT_ASSERT(ai != NULL, "Calibration map should always be larger than calibration image size");
	if (ai->GetNewX() != shouldBeX || (ai->flags & X_IS_MEASURED) == 0)
	{
		ai->SetNewX(shouldBeX);
		SOFT_ASSERT((int)(ai->GetNewX() * 1000) == (int)(shouldBeX * 1000), "Precision loss error");
		ai->flags = (PositionAdjustInfoFlags)(ai->flags | X_IS_MEASURED);
		hasUnsavedAdjustments = 1;
	}
	// in case the resolution of the input image is lower then of our internal storage, there will be gaps between the lines
	// around 24 values per 1 mm
#define SPREAD_SAME_MEASURE_VALUE_RADIUS 6
	for (int y2 = y - SPREAD_SAME_MEASURE_VALUE_RADIUS; y2 <= y + SPREAD_SAME_MEASURE_VALUE_RADIUS; y2++)
	{
		PositionAdjustInfo* ai3 = GetAdjustInfo(x, y2);
		if (ai3 != NULL && ai3->HasXMeasured() == 0)
		{
			ai3->SetNewX(shouldBeX);
			ai3->flags = (PositionAdjustInfoFlags)(ai3->flags | X_IS_MEASURED);
		}
	}
}

void LineAntiDistorsionAdjuster::AdjustPositionY(int x, int y, int shouldBeY)
{
	needsBleedY = 1;
	PositionAdjustInfo* ai = GetAdjustInfo(x, y);
	SOFT_ASSERT(ai != NULL, "Calibration map should always be larger than calibration image size");
	if (ai->GetNewY() != shouldBeY || ai->HasYMeasured() == 0)
	{
		ai->SetNewY(shouldBeY);
		SOFT_ASSERT((int)(ai->GetNewY() * 1000) == (int)(shouldBeY * 1000), "Precision loss error");
		ai->flags = (PositionAdjustInfoFlags)(ai->flags | Y_IS_MEASURED);
		hasUnsavedAdjustments = 1;
	}
	// in case the resolution of the input image is lower then of our internal storage, there will be gaps between the lines
	// around 24 values per 1 mm
	for (int x2 = x - SPREAD_SAME_MEASURE_VALUE_RADIUS; x2 <= x + SPREAD_SAME_MEASURE_VALUE_RADIUS; x2++)
	{
		PositionAdjustInfo* ai3 = GetAdjustInfo(x2, y);
		if (ai3 != NULL && ai3->HasYMeasured() == 0)
		{
			ai3->SetNewY(shouldBeY);
			ai3->flags = (PositionAdjustInfoFlags)(ai3->flags | Y_IS_MEASURED);
		}
	}
}

// try to draw a line normally. If there is correction info at a specific location, use that instead
#if 0
void LineAntiDistorsionAdjuster::DrawLineEveryPoint(float sx, float sy, float ex, float ey, RelativePointsLine* out_line)
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

	double xIncForStep = dx / out_lineDrawSteps;
	double yIncForStep = dy / out_lineDrawSteps;

	double leftoverXDiff = 0;
	double leftoverYDiff = 0;
	RelativePointsLine tempLine;
#ifdef _DEBUG
	int xsign = 0;
	int acx, apx, acy, apy;
	int ysign = 0;
#endif
	for (double step = 1; step <= out_lineDrawSteps; step += 1)
	{
		double prevXPos = sx + (step-1) * xIncForStep;
		double prevYPos = sy + (step-1) * yIncForStep;
		double curXPos = sx + step * xIncForStep;
		double curYPos = sy + step * yIncForStep;

		double xDiff = curXPos - prevXPos;
		double yDiff = curYPos - prevYPos;
		PositionAdjustInfo* aiPrev = GetAdjustInfo((int)prevXPos, (int)prevYPos);
		PositionAdjustInfo* aiCur = GetAdjustInfo((int)curXPos, (int)curYPos);
		acx = apx = 0;
		if (aiCur && aiPrev)
		{
			if (aiCur->HasX() && aiPrev->HasX())
			{
				xDiff = aiCur->GetNewX() - aiPrev->GetNewX();
				acx = aiCur->GetNewX();
				apx = aiPrev->GetNewX();
			}
			if (aiCur->HasY() && aiPrev->HasY())
			{
				yDiff = aiCur->GetNewY() - aiPrev->GetNewY();
				acy = aiCur->GetNewY();
				apy = aiPrev->GetNewY();
			}
		}

		if (xDiff == 0 && yDiff == 0)
		{
			continue;
		}

		xDiff += leftoverXDiff;
		yDiff += leftoverYDiff;
		if (abs(xDiff) < 1.0 && abs(yDiff) < 1.0)
		{
			leftoverXDiff = xDiff - (int)xDiff;
			leftoverYDiff = yDiff - (int)yDiff;
			continue;
		}

		if (abs(xDiff) > LINE_GAP_CONSIDERED_BUG || abs(yDiff) > LINE_GAP_CONSIDERED_BUG)
		{
			printf("Step %f ) Line gap %f %f at %f,%f is too large to continue drawing\n", step, abs(xDiff), abs(yDiff), curXPos, curYPos);
			//			continue;
		}

		leftoverXDiff = xDiff - (int)xDiff;
		leftoverYDiff = yDiff - (int)yDiff;

#ifdef _DEBUG
		if (xsign == 0)
		{
			xsign = getSign(xDiff);
		}
		else if(xDiff != 0)
		{
			if (xsign != getSign(xDiff))
				printf("%f/%f)Line changed x orientation prev %f,%f cur %f,%f, acx, apx %d, %d\n", step, out_lineDrawSteps, prevXPos, prevYPos, curXPos, curYPos, acx, apx);
		}
		if (ysign == 0)
		{
			ysign = getSign(yDiff);
		}
		else if(yDiff != 0)
		{
			if (ysign != getSign(yDiff)) 
				printf("%f/%f)Line changed y orientation prev %f,%f cur %f,%f, acx, apx %d, %d\n", step, out_lineDrawSteps, prevXPos, prevYPos, curXPos, curYPos, acx, apx);
		}
#endif

		tempLine.storeNextPoint((int)xDiff, (int)yDiff);
	}

// around 24 actions per mm
//#define PEN_MOVEMENT_SPEED_SMOOTHING 20
#ifdef PEN_MOVEMENT_SPEED_SMOOTHING
	for (int i = 0; i < tempLine.GetPointsCount(); i+= PEN_MOVEMENT_SPEED_SMOOTHING)
	{
		float xDiffs = 0;
		float yDiffs = 0;
		for (int j = i; j < MIN(i + PEN_MOVEMENT_SPEED_SMOOTHING, tempLine.GetPointsCount()); j++)
		{
			xDiffs += tempLine.GetDX(j);
			yDiffs += tempLine.GetDY(j);
		}
		// first write xDiffs so that the motors do not need to turn on and off too often
		int xSign = getSign(xDiffs);
		int ySign = getSign(yDiffs);
		for (int j = 0; j < abs(xDiffs); j++)
		{
			out_line->storeNextPoint(xSign, 0);
		}
		for (int j = 0; j < abs(yDiffs); j++)
		{
			out_line->storeNextPoint(0, ySign);
		}
	}
#else
	for (int i = 0; i < tempLine.GetPointsCount(); i++)
	{
		out_line->storeNextPoint(tempLine.GetDX(i), tempLine.GetDY(i));
	}
#endif

	out_line->setEndPosition(ex, ey);
}
#endif

void AppendLineSegment(float sx, float sy, float ex, float ey, RelativePointsLine* out_line)
{
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

	int curx = (int)sx;
	int cury = (int)sy;

	double xIncForStep = dx / lineDrawSteps;
	double yIncForStep = dy / lineDrawSteps;
	int writtenDiffX = 0;
	int writtenDiffY = 0;
	for (double step = 1; step <= lineDrawSteps; step += 1)
	{
		double curXPos = step * xIncForStep;
		double curYPos = step * yIncForStep;
		int xdiff = (int)curXPos - writtenDiffX;
		int ydiff = (int)curYPos - writtenDiffY;

		if (xdiff < -1)
		{
			xdiff = -1;
		}
		else if (xdiff > 1)
		{
			xdiff = 1;
		}
		if (ydiff < -1)
		{
			ydiff = -1;
		}
		else if (ydiff > 1)
		{
			ydiff = 1;
		}

		if (xdiff != 0)
		{
			writtenDiffX += xdiff;
			out_line->storeNextPoint(xdiff, 0);
			curx += xdiff;
		}
		if (ydiff != 0)
		{
			writtenDiffY += ydiff;
			out_line->storeNextPoint(0, ydiff);
			cury += ydiff;
		}
	}

	// fix rounding errors
	if (dx < 0)
	{
		while (writtenDiffX > (int)dx)
		{
			writtenDiffX--;
			out_line->storeNextPoint(-1, 0);
		}
	}
	if (dx > 0)
	{
		while (writtenDiffX < (int)dx)
		{
			writtenDiffX++;
			out_line->storeNextPoint(1, 0);
		}
	}
	if (dy < 0)
	{
		while (writtenDiffY > (int)dy)
		{
			writtenDiffY--;
			out_line->storeNextPoint(0, -1);
		}
	}
	if (dy > 0)
	{
		while (writtenDiffY < (int)dy)
		{
			writtenDiffY++;
			out_line->storeNextPoint(0, 1);
		}
	}
}

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

	double xIncForStep = dx / out_lineDrawSteps;
	double yIncForStep = dy / out_lineDrawSteps;

#define SUB_LINE_LEN 50
	for (double step = 0; step < out_lineDrawSteps; step += SUB_LINE_LEN)
	{
		double sx2 = sx + step * xIncForStep;
		double sy2 = sy + step * yIncForStep;
		double stepsEnd = (step + SUB_LINE_LEN);
		if (stepsEnd > out_lineDrawSteps)
		{
			stepsEnd = out_lineDrawSteps;
		}
		double ex2 = sx + stepsEnd * xIncForStep;
		double ey2 = sy + stepsEnd * yIncForStep;

		PositionAdjustInfo* aiStart = GetAdjustInfo((int)sx2, (int)sy2);
		PositionAdjustInfo* aiEnd = GetAdjustInfo((int)ex2, (int)ey2);
		if (aiStart && aiEnd)
		{
			if (aiStart->HasX() && aiEnd->HasX())
			{
				sx2 = aiStart->GetNewX();
				ex2 = aiEnd->GetNewX();
			}
			if (aiStart->HasY() && aiEnd->HasY())
			{
				sy2 = aiStart->GetNewY();
				ey2 = aiEnd->GetNewY();
			}
		}

		AppendLineSegment((float)sx2, (float)sy2, (float)ex2, (float)ey2, out_line);
	}

	out_line->setEndPosition(ex, ey);
}

// visualize the callibration map itself. Scale correction values to calibration map size
void LineAntiDistorsionAdjuster::DebugDumpMapToImage(int col)
{
	{
#define SCALE_DOWN_X 2
		FIBITMAP* dib = CreateNewImage(ADJUST_MAP_DEFAULT_WIDTH + 1000, ADJUST_MAP_DEFAULT_HEIGHT + 1000);
		int x = col;
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
			if (ai == NULL || ai->HasX() == 0)
			{
				DrawLineColor(dib, (float)(1000), (float)(500 + y), (float)(1000 + ADJUST_MAP_DEFAULT_WIDTH / SCALE_DOWN_X), (float)(500 + y), 0, 255, 0);
				continue;
			}
			int x2 = (int)(adjustInfoHeader.originX + ai->GetNewX() * adjustInfoHeader.scaleX); // newx comes in the range of [-2700,2700]
			DrawLineColor(dib, (float)(1000 + x / SCALE_DOWN_X), (float)(500 + y),
				(float)(1000 + x2 / SCALE_DOWN_X), (float)(500 + y), 255, 255, 255);
		}
		//draw a line at the center
		DrawLineColor(dib, 1000 + 0, 500 + ADJUST_MAP_DEFAULT_HEIGHT / 2, 1000 + ADJUST_MAP_DEFAULT_WIDTH / 4, 500 + ADJUST_MAP_DEFAULT_HEIGHT / 2, 255, 0, 0);
		DrawLineColor(dib, 1000 + ADJUST_MAP_DEFAULT_WIDTH / SCALE_DOWN_X / 2, 500 + 0, 1000 + ADJUST_MAP_DEFAULT_WIDTH / SCALE_DOWN_X / 2, 500 + ADJUST_MAP_DEFAULT_HEIGHT, 255, 0, 0);
		char fileName[500];
		sprintf_s(fileName, sizeof(fileName), "map_col_%d.png", col);
		SaveImagePNG(dib, fileName);
		FreeImage_Unload(dib);
	}
	{
#define SCALE_DOWN_Y 3.0f
		int Width = 10 + (int)(adjustInfoHeader.width / SCALE_DOWN_Y);
		int Height = 10 + (int)(adjustInfoHeader.height / SCALE_DOWN_Y);
		FIBITMAP* dib = CreateNewImage(Width + 10, Height + 10);
		int y = col;
		float cx = 5.0f + adjustInfoHeader.originX / SCALE_DOWN_Y;
		float cy = 5.0f + adjustInfoHeader.originY / SCALE_DOWN_Y;
		for (int x = 0; x < adjustInfoHeader.width; x++)
		{
			PositionAdjustInfo* ai = GetAdjustInfoNoChange(x, y);
			float x2 = (x - adjustInfoHeader.originX) / SCALE_DOWN_Y;
			if (ai == NULL || ai->HasY() == 0)
			{
				DrawLineColor(dib, (float)(cx+x2), (float)(0), (float)(cx+x2), (float)(Height), 0, 255, 0);
				continue;
			}
			float y1 = (y - adjustInfoHeader.originY) / SCALE_DOWN_Y;
			float y2 = ai->GetNewY() * adjustInfoHeader.scaleY  / SCALE_DOWN_Y; // newx comes in the range of [-2700,2700]
			DrawLineColor(dib, (float)(cx+x2), (float)(cy + y1),
				(float)(cx + x2), (float)(cy + y2), 255, 255, 255);
		}
		//draw a line at the center
		DrawLineColor(dib, cx, 0, cx, (float)Height, 255, 0, 0);
		DrawLineColor(dib, 0, cy, (float)Width, cy, 255, 0, 0);
		char fileName[500];
		sprintf_s(fileName, sizeof(fileName), "map_row_%d.png", col);
		SaveImagePNG(dib, fileName);
		FreeImage_Unload(dib);
	}
}