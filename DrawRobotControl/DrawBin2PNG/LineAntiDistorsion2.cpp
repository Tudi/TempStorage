#include "StdAfx.h"

#define POSITION_ADJUST_FILE_VERSION 3
// the physical tear size is around 10x10 inches
// we need around 600 commands to draw 1 inch => we would need 6000x6000 map to represent every command
// the map in the file would be [-3000,3000]x[-3000,3000]
#define ADJUST_MAP_DEFAULT_WIDTH 1600
#define ADJUST_MAP_DEFAULT_HEIGHT 1600
// we can adjust every 10th command. Inbetween values should be smoothed by line draw
#define DEFAULT_WIDTH_SCALER 0.20f 
#define DEFAULT_HEIGHT_SCALER 0.20f
#define LINE_GAP_CONSIDERED_BUG	100 // measured in movement units. 600 movements = 1 inch

LineAntiDistorsionAdjuster2 sLineAdjuster2;
PositionAdjustInfo2 adjustInfoMissing2;

LineAntiDistorsionAdjuster2::LineAntiDistorsionAdjuster2()
{
	memset(&adjustInfoMissing2, 0, sizeof(adjustInfoMissing2));
	memset(&adjustInfoHeader, 0, sizeof(adjustInfoHeader));
	adjustInfoMap = NULL;
	hasUnsavedAdjustments = 0;
	// load static distorsion map
	LoadAdjusterMap();
}

LineAntiDistorsionAdjuster2::~LineAntiDistorsionAdjuster2()
{
	if (hasUnsavedAdjustments)
	{
		SaveAdjusterMap();
	}
	free(adjustInfoMap);
	adjustInfoMap = NULL;
}

PositionAdjustInfo2* LineAntiDistorsionAdjuster2::GetAdjustInfo(int x, int y)
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

Adjusted2DPos2 LineAntiDistorsionAdjuster2::GetAdjustedPos(double x, double y)
{
	Adjusted2DPos2 ret = { 0 };

	double x2 = ((double)x * adjustInfoHeader.scaleX + adjustInfoHeader.originX);
	double y2 = ((double)y * adjustInfoHeader.scaleY + adjustInfoHeader.originY);

	if ((int)floor(x2) == (int)ceil(x2) && (int)floor(y2) == (int)ceil(y2))
	{
		PositionAdjustInfo2* poi = GetAdjustInfoNoChange((int)x2, (int)y2);
		if (poi)
		{
			if (poi->HasX())
			{
				ret.x = (float)poi->GetNewX();
			}
			else
			{
				return ret;
			}
			if (poi->HasY())
			{
				ret.y = (float)poi->GetNewY();
			}
			else
			{
				return ret;
			}
		}
		ret.HasValues = 1;
		return ret;
	}

	double ax[2][2] = { 0 };
	double ay[2][2] = { 0 };

	int valuesX = 0;
	int valuesY = 0;
	for (int x3 : {(int)floor(x2), (int)ceil(x2)})
	{
		valuesX = 0;
		for (int y3 : {(int)floor(y2), (int)ceil(y2)})
		{
			PositionAdjustInfo2* poi = GetAdjustInfoNoChange(x3, y3);
			if (poi)
			{
				if (poi->HasX())
				{
					ax[valuesX][valuesY] = (float)poi->GetNewX();
				}
				else
				{
					return ret;
				}
				if (poi->HasY())
				{
					ay[valuesX][valuesY] = (float)poi->GetNewY();
				}
				else
				{
					return ret;
				}
			}
			valuesX++;
		}
		valuesY++;
	}

	double xCoef = x2 - floor(x2);
	double yCoef = y2 - floor(y2);
	double coef00 = (1 - xCoef) * (1 - yCoef);
	double coef10 = (1 - xCoef) * yCoef;
	double coef01 = xCoef * (1 - yCoef);
	double coef11 = xCoef * yCoef;
	double x_out = coef00 * ax[0][0] + coef10 * ax[1][0] + coef01 * ax[0][1] + coef11 * ax[1][1];
	double y_out = coef00 * ay[0][0] + coef10 * ay[1][0] + coef01 * ay[0][1] + coef11 * ay[1][1];

	ret.x = x_out;
	ret.y = y_out;
	ret.HasValues = 1;

	return ret;
}

void LineAntiDistorsionAdjuster2::CreateNewMap(PositionAdjustInfoHeader2* header)
{
	PositionAdjustInfoHeader2 tempHeader;
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

	adjustInfoMap = (PositionAdjustInfo2*)malloc(adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo2));
	if (adjustInfoMap != NULL)
	{
		memset(adjustInfoMap, 0, adjustInfoHeader.width * adjustInfoHeader.height * sizeof(PositionAdjustInfo2));
	}
}

void LineAntiDistorsionAdjuster2::SaveAdjusterMap()
{
	hasUnsavedAdjustments = 0;

	// nothing to save. create content first
	if (adjustInfoMap == NULL)
	{
		return;
	}

	adjustInfoHeader.version = POSITION_ADJUST_FILE_VERSION;
	adjustInfoHeader.fourCC = *(int*)CALIBRATION_4CC;
	adjustInfoHeader.headerSize = sizeof(PositionAdjustInfoHeader2);
	adjustInfoHeader.infoSize = sizeof(PositionAdjustInfo2);

	FILE* f;
	errno_t err_open = fopen_s(&f, CALIBRATION_FILE_NAME,"wb");
	if (f == NULL)
	{
		return;
	}

	// write header 
	fwrite(&adjustInfoHeader, sizeof(adjustInfoHeader), 1, f);

	//write the map
	fwrite(adjustInfoMap, sizeof(PositionAdjustInfo2) * adjustInfoHeader.width * adjustInfoHeader.height, 1, f);

	// donw writing the map
	fclose(f);
}

void LineAntiDistorsionAdjuster2::LoadAdjusterMap()
{
	FILE* f;
	errno_t err_open = fopen_s(&f, CALIBRATION_FILE_NAME, "rb");
	if (f == NULL)
	{
		return;
	}

	PositionAdjustInfoHeader2 fHeader;
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
	if (fHeader.headerSize != sizeof(PositionAdjustInfoHeader2))
	{
		printf("Invalid calibration file header size\n");
		fclose(f);
		return;
	}
	if (fHeader.infoSize != sizeof(PositionAdjustInfo2))
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

	fread(adjustInfoMap, sizeof(PositionAdjustInfo2) * adjustInfoHeader.width * adjustInfoHeader.height, 1, f);

	fclose(f);
}

PositionAdjustInfo2* LineAntiDistorsionAdjuster2::GetAdjustInfoNoChange(int x, int y)
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

void LineAntiDistorsionAdjuster2::AdjustPosition(int x, int y, double shouldBeX, double shouldBeY, int isInitial)
{
	PositionAdjustInfo2* ai = GetAdjustInfo(x, y);
	SOFT_ASSERT(ai != NULL, "Calibration map should always be larger than calibration image size");
	if (isInitial)
	{
		if (ai->GetNewX() != shouldBeX || (ai->HasXMeasured()) == 0 ||
			ai->GetNewY() != shouldBeY || (ai->HasYMeasured()) == 0)
		{
			ai->SetNewX(shouldBeX);
			ai->SetNewY(shouldBeY);
			if (shouldBeX != 0)
			{
				ai->flags = (PositionAdjustInfoFlags2)(ai->flags | X_IS_MEASURED);
			}
			if (shouldBeY != 0)
			{
				ai->flags = (PositionAdjustInfoFlags2)(ai->flags | Y_IS_MEASURED);
			}
			SOFT_ASSERT((int)(ai->GetNewX() * 100) == (int)(shouldBeX * 100), "Precision loss error");
			SOFT_ASSERT((int)(ai->GetNewY() * 100) == (int)(shouldBeY * 100), "Precision loss error");
			hasUnsavedAdjustments = 1;
		}
	}
	else
	{
		double newX = ai->GetNewX() + shouldBeX;
		double newY = ai->GetNewY() + shouldBeY;
		ai->SetNewX(newX);
		ai->SetNewY(newY);
		if (shouldBeX != 0)
		{
			ai->flags = (PositionAdjustInfoFlags2)(ai->flags | X_IS_MEASURED);
		}
		if (shouldBeY != 0)
		{
			ai->flags = (PositionAdjustInfoFlags2)(ai->flags | Y_IS_MEASURED);
		}
		SOFT_ASSERT((int)(ai->GetNewX() * 100) == (int)(newX * 100), "Precision loss error");
		SOFT_ASSERT((int)(ai->GetNewY() * 100) == (int)(newY * 100), "Precision loss error");
		hasUnsavedAdjustments = 1;
	}
}

void AppendLineSegment(double sx, double sy, double ex, double ey, RelativePointsLine* out_line, double &leftOverX, double &leftOverY)
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
	SOFT_ASSERT(abs(leftOverX) < 1, "LeftoverX is greater than 1");
	SOFT_ASSERT(abs(leftOverY) < 1, "LeftoverY is greater than 1");
}

double angleBetweenPoints(double x1, double y1, double x2, double y2) 
{
	double deltaX = x2 - x1;
	double deltaY = y2 - y1;
	return atan2(deltaY, deltaX) * (180 / 3.14);
}

int LineAntiDistorsionAdjuster2::DrawLine(double sx, double sy, double ex, double ey, RelativePointsLine* out_line, double &leftOverX, double &leftOverY)
{
	out_line->setStartingPosition(sx, sy);
	out_line->setEndPosition(ex, ey);

	double dx = ex - sx;
	double dy = ey - sy;
	if (dx == dy && dx == 0)
	{
		return 1;
	}

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

#define SUB_LINE_LEN 30.0
	int ret = 0;
	for (double step = 0; step < out_lineDrawSteps; step += SUB_LINE_LEN)
	{
		double sx2 = sx + step * xIncForStep;
		double sy2 = sy + step * yIncForStep;
		double stepsEnd = (step + SUB_LINE_LEN);
		if (stepsEnd > out_lineDrawSteps)
		{
			stepsEnd = out_lineDrawSteps;
			SOFT_ASSERT((int)((sx + stepsEnd * xIncForStep) * 1) == (int)(ex * 1), "Precision loss error");
			SOFT_ASSERT((int)((sy + stepsEnd * yIncForStep) * 1) == (int)(ey * 1), "Precision loss error");
		}
		double ex2 = sx + stepsEnd * xIncForStep;
		double ey2 = sy + stepsEnd * yIncForStep;

		// try sub pixel accuracy adjusting
		Adjusted2DPos2 aiStart = GetAdjustedPos(sx2, sy2);
		Adjusted2DPos2 aiEnd = GetAdjustedPos(ex2, ey2);
		if (aiStart.HasValues && aiEnd.HasValues)
		{
			sx2 = aiStart.x; 
			sy2 = aiStart.y;
			ex2 = aiEnd.x;
			ey2 = aiEnd.y;
		}
		else
		{
			printf("Line segments is not within tear : %lf,%lf - %lf,%lf\n", sx2, sy2, ex2, ey2);
			ret = 1;
		}
//#define DEBUG_LINE_CONTINUITY
#ifdef DEBUG_LINE_CONTINUITY
		{
			static double prevEXsrc = 0, prevEYsrc = 0;
			static double prevEX = 0, prevEY = 0;
			static double anglePrev = 0;
			static int firstInit = 1;
			double sx22 = sx + step * xIncForStep;
			double sy22 = sy + step * yIncForStep;
			double ex22 = sx + stepsEnd * xIncForStep;
			double ey22 = sy + stepsEnd * yIncForStep;
			printf("\t Draw subline from %f,%f - %f,%f\n", 
				(float)sx2, (float)sy2, (float)ex2, (float)ey2);
			if (firstInit)
			{
				firstInit = 0;
			}
			// check if the ending of the previous line is the start of this line
			else
			{
				double dx = sx2 - prevEX;
				double dy = sy2 - prevEY;
				if (dx >= 1 || dy >= 1)
				{
					printf("\t Cought discontinuity : %f - %f\n", dx, dy);
					printf("\t from prev end to cur start %f,%f - %f,%f\n",
						(float)prevEX, (float)prevEY, (float)sx2, (float)sy2);
					Adjusted2DPos2 aiStart2 = GetAdjustedPos((float)sx22, (float)sy22);
					Adjusted2DPos2 aiEnd2 = GetAdjustedPos((float)ex22, (float)ey22);
				}
			}
			// check if the angle of this line is similar to the main line. We do expect deviation, but not a large one
			double angleMain = angleBetweenPoints(sx, sy, ex, ey);
			double angleNow = angleBetweenPoints(sx2, sy2, ex2, ey2);
			if ((step != 0 && abs(angleNow - anglePrev) > 40) 
//				|| (abs(angleMain-angleNow) > 40)
				)
			{
				printf("\t Angle of the line is very divergent compared to previous angle : %f - %f\n", angleNow, anglePrev);
				Adjusted2DPos2 aiStart2 = GetAdjustedPos((float)sx22, (float)sy22);
				Adjusted2DPos2 aiEnd2 = GetAdjustedPos((float)ex22, (float)ey22);
			}

			prevEX = ex2;
			prevEY = ey2;
			prevEXsrc = sx + stepsEnd * xIncForStep;
			prevEYsrc = sy + stepsEnd * yIncForStep;
			anglePrev = angleNow;
		}
#endif
#ifdef DEBUG_LEFTOVER_ROLLOVER
		int pcprev = out_line->GetPointsCount();
		double preLeftOverX = leftOverX;
		double preLeftOverY = leftOverY;
#endif
		AppendLineSegment(sx2, sy2, ex2, ey2, out_line, leftOverX, leftOverY);
#ifdef DEBUG_LEFTOVER_ROLLOVER
		if (pcprev == out_line->GetPointsCount())
		{
			printf("\t no point subline between %.02f,%.02f to %.02f,%.02f. preleftover, nowleftover : %.02f,%.02f to %.02f,%.02f\n", sx2, sy2, ex2, ey2, preLeftOverX, preLeftOverY, leftOverX, leftOverY);
		}
#endif
	}
	return ret;
}

// visualize the callibration map itself. Scale correction values to calibration map size
void LineAntiDistorsionAdjuster2::DebugDumpMapToImage(int col)
{
	{
#define SCALE_DOWN_X 2
		FIBITMAP* dib = CreateNewImage(ADJUST_MAP_DEFAULT_WIDTH + 1000, ADJUST_MAP_DEFAULT_HEIGHT + 1000);
		int x = col;
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, y);
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
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, y);
			float x2 = (x - adjustInfoHeader.originX) / SCALE_DOWN_Y;
			if (ai == NULL || ai->HasY() == 0)
			{
				DrawLineColor(dib, (float)(cx+x2), (float)(0), (float)(cx+x2), (float)(Height), 0, 255, 0);
				continue;
			}
			float y1 = (y - adjustInfoHeader.originY) / SCALE_DOWN_Y;
			float y2 = (float)ai->GetNewY() * adjustInfoHeader.scaleY  / SCALE_DOWN_Y; // newx comes in the range of [-2700,2700]
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

#define ESTIMATOR_COUNT 5
double *getEstimatedValAt(int startPos, int endPos, int *srcVals, double *vals)
{
	char cmd[6400];
//	sprintf_s(cmd, sizeof(cmd), "python curve_fit/fit5.py %d %d %d %d %d %d %f %f %f %f", 
//		startPos, endPos, srcVals[0], srcVals[1], srcVals[2], srcVals[3], vals[0], vals[1], vals[2], vals[3]);
	sprintf_s(cmd, sizeof(cmd), "python curve_fit/fit6.py %d %d %d %d %d %d %d %f %f %f %f %f",
		startPos, endPos, srcVals[0], srcVals[1], srcVals[2], srcVals[3], srcVals[4], vals[0], vals[1], vals[2], vals[3], vals[4]);
	char *cmd_ret = exec(cmd);
	double *ret = (double* )malloc(4000 * sizeof(double));
	memset(ret, 0, 4000 * sizeof(double));
	if (cmd_ret != NULL && ret != NULL)
	{
		char* cmd_ret2 = cmd_ret;
		int i = 0;
		while (1)
		{
			char* nextScan;
			ret[i] = strtod(cmd_ret2, &nextScan);
			i++;
			if (cmd_ret2 == nextScan)
			{
				break;
			}
			cmd_ret2 = nextScan;
		}
		printf("Executed %s\n", cmd);
		printf("Received %s\n", cmd_ret);
		free(cmd_ret);
	}
	return ret;
}

void LineAntiDistorsionAdjuster2::ScanMatFillMissingValues(int sx, int sy, int dirX, int dirY, int checkFlags)
{
	int ex, ey;
	if (dirX < 0)
	{
		ex = 0;
	}
	else if (dirX == 0)
	{
		ex = sx;
	}
	else
	{
		ex = adjustInfoHeader.width;
	}
	if (dirY < 0)
	{
		ey = 0;
	}
	else if (dirY == 0)
	{
		ey = sy;
	}
	else
	{
		ey = adjustInfoHeader.height;
	}

	SOFT_ASSERT((ex - sx) * dirX >= 0, "Function will deadlock");
	SOFT_ASSERT((ey - sy) * dirY >= 0, "Function will deadlock");

	// search for the first available cell
	int sx2 = sx, sy2 = sy;
	int metX = 0, metY = 0;
	while (metX == 0 || metY == 0)
	{
		if (sx2 == ex)
		{
			metX = 1;
		}
		if (sy2 == ey)
		{
			metY = 1;
		}
		PositionAdjustInfo2* ai = GetAdjustInfoNoChange(sx2, sy2);
		if (ai == NULL)
		{
			// move forward in the search
			sx2 += dirX;
			sy2 += dirY;
			continue;
		}
		// found a cell with values we are looking for ?
		if ((ai->flags & checkFlags) == checkFlags)
		{
			break;
		}
		// move forward in the search
		sx2 += dirX;
		sy2 += dirY;
	}
	// fill in estimator values
	int needsX = (checkFlags & (X_IS_MEASURED | X_IS_SET)) != 0;
	int needsY = (checkFlags & (Y_IS_MEASURED | Y_IS_SET)) != 0;
	double estimatorsX_out[ESTIMATOR_COUNT] = { 0 };
	double estimatorsY_out[ESTIMATOR_COUNT] = { 0 };
	int estimatorsX_in[ESTIMATOR_COUNT] = { 0 };
	int estimatorsY_in[ESTIMATOR_COUNT] = { 0 };
	int valuesFound = 0;
	int sx3 = sx2, sy3 = sy2;
	for (int i = 0; i < ESTIMATOR_COUNT; i++)
	{
		PositionAdjustInfo2* ai = GetAdjustInfoNoChange(sx3, sy3);
		if (ai == NULL)
		{
			sx3 += dirX;
			sy3 += dirY;
			continue;
		}
		if ((ai->flags & checkFlags) != checkFlags)
		{
			break;
		}
		valuesFound++;
		estimatorsX_out[i] = ai->GetNewX();
		estimatorsX_in[i] = sx3;
		estimatorsY_out[i] = ai->GetNewY();
		estimatorsY_in[i] = sy3;
		sx3 += dirX;
		sy3 += dirY;
	}
	// can't fill values from this start and this direction
	if (valuesFound != ESTIMATOR_COUNT)
	{
		return;
	}
	// use the estimator to estimate values for empty cells
	double* estimatedX = NULL, * estimatedY = NULL;
	if (needsX)
	{
		estimatedX = getEstimatedValAt(sx, sx2, estimatorsX_in, estimatorsX_out);
		if (estimatedX == NULL)
		{
			printf("Failed to generate estimated values\n");
			return;
		}
	}
	if (needsY)
	{
		estimatedY = getEstimatedValAt(sy, sy2, estimatorsY_in, estimatorsY_out);
		if (estimatedY == NULL)
		{
			printf("Failed to generate estimated values\n");
			return;
		}
	}
	sx2 = sx, sy2 = sy;
	int readIndex = 0;
	metX = 0, metY = 0;
	while (metX == 0 || metY == 0)
	{
		if (sx2 == ex)
		{
			metX = 1;
		}
		if (sy2 == ey)
		{
			metY = 1;
		}
		PositionAdjustInfo2* ai = GetAdjustInfoNoChange(sx2, sy2);
		if (ai == NULL)
		{
			// move forward in the search
			sx2 += dirX;
			sy2 += dirY;
			continue;
		}
		// found a cell with values we are looking for ?
		if ((ai->flags & checkFlags) == checkFlags)
		{
			break;
		}
		// move forward in the search
		sx2 += dirX;
		sy2 += dirY;
		if (needsX)
		{
			ai->SetNewX(estimatedX[readIndex]);
		}
		if (needsY)
		{
			ai->SetNewY(estimatedY[readIndex]);
		}
		readIndex++;
	}
	free(estimatedX);
	free(estimatedY);
}

void LineAntiDistorsionAdjuster2::FillMissingInfo()
{
#define USE_DIAGONAL_VALUE_GUESSING
#ifdef USE_DIAGONAL_VALUE_GUESSING
	// expects map to be a square
	for (int diag = 0; diag < adjustInfoHeader.height; diag++)
	{
		printf("Process diag %d\n", diag);
		// first row/col to lower right corner scan
		ScanMatFillMissingValues(diag, 0, 1, 1, X_IS_MEASURED | Y_IS_MEASURED);
		ScanMatFillMissingValues(0, diag, 1, 1, X_IS_MEASURED | Y_IS_MEASURED);
		// first row / last col to lower left scan
		ScanMatFillMissingValues(diag, 0, -1, 1, X_IS_MEASURED | Y_IS_MEASURED);
		ScanMatFillMissingValues(adjustInfoHeader.width - 1, diag, -1, 1, X_IS_MEASURED | Y_IS_MEASURED);
		// first col / last row to upper right corner scan
		ScanMatFillMissingValues(0, diag, 1, -1, X_IS_MEASURED | Y_IS_MEASURED);
		ScanMatFillMissingValues(diag, adjustInfoHeader.height - 1, 1, -1, X_IS_MEASURED | Y_IS_MEASURED);
		// last col / last row to upper left col scan
		ScanMatFillMissingValues(adjustInfoHeader.width - 1, diag, -1, -1, X_IS_MEASURED | Y_IS_MEASURED);
		ScanMatFillMissingValues(diag, adjustInfoHeader.height - 1, -1, -1, X_IS_MEASURED | Y_IS_MEASURED);
	} 
#endif
	for (int y = 0; y < adjustInfoHeader.height; y++)
	{
		printf("Process row %d\n", y);
		// scan left to right
		ScanMatFillMissingValues(0, y, 1, 0, X_IS_MEASURED);
		// scan right to left
		ScanMatFillMissingValues(adjustInfoHeader.width - 1, y, -1, 0, X_IS_MEASURED);
	}
	for (int x = 0; x < adjustInfoHeader.width; x++)
	{
		printf("Process col %d\n", x);
		// scan top to bottom
		ScanMatFillMissingValues(x, 0, 0, 1, Y_IS_MEASURED);
		// scan bottom to top
		ScanMatFillMissingValues(x, adjustInfoHeader.height - 1, 0, -1, Y_IS_MEASURED);
	}
	// in the 4 corners, we will still have missing values, fill these by replicating first seen value
/*	for (int y = 0; y < adjustInfoHeader.height; y++)
	{
		printf("Process row %d\n", y);
		// scan left to right
		ScanMatFillMissingValues(0, y, 1, 0, X_IS_SET);
		// scan right to left
		ScanMatFillMissingValues(adjustInfoHeader.width - 1, y, -1, 0, X_IS_SET);
	}
	for (int x = 0; x < adjustInfoHeader.width; x++)
	{
		printf("Process col %d\n", x);
		// scan top to bottom
		ScanMatFillMissingValues(x, 0, 0, 1, Y_IS_SET);
		// scan bottom to top
		ScanMatFillMissingValues(x, adjustInfoHeader.height - 1, 0, -1, Y_IS_SET);
	} */
}

void LineAntiDistorsionAdjuster2::FillMissingInfo2()
{
#if 0
	for (int y = 0; y < adjustInfoHeader.height; y++)
	{
		int minX = 10000, maxX = -10000;
		for (int x = 0; x < adjustInfoHeader.width; x++)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, y);
			if (ai == NULL)
			{
				continue;
			}
			if (minX > x && ai->HasXMeasured())
			{
				minX = x;
			}
			if (maxX < x && ai->HasXMeasured())
			{
				maxX = x;
			}
			// if Y above us are missing values, copy them
			{
				PositionAdjustInfo2* aiTop = GetAdjustInfoNoChange(x, y - 1);
				if (aiTop != NULL && ai->HasYMeasured() && aiTop->HasYMeasured() == 0)
				{
					for (int y2 = y - 1; y2 >= 0; y2--)
					{
						PositionAdjustInfo2* aiTop2 = GetAdjustInfoNoChange(x, y2);
						aiTop2->SetNewY(ai->GetNewY());
					}
				}
			}
			// if Y below us are missing values, copy them
			{
				PositionAdjustInfo2* aiBottom = GetAdjustInfoNoChange(x, y + 1);
				if (aiBottom != NULL && ai->HasYMeasured() && aiBottom->HasYMeasured() == 0)
				{
					for (int y2 = y + 1; y2 < adjustInfoHeader.height; y2++)
					{
						PositionAdjustInfo2* aiTop2 = GetAdjustInfoNoChange(x, y2);
						aiTop2->SetNewY(ai->GetNewY());
					}
				}
			}
		}
		// spread X values to the left
		if(minX != 10000)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(minX, y);
			for (int x2 = minX - 1; x2 >= 0; x2--)
			{
				PositionAdjustInfo2* aiLeft = GetAdjustInfoNoChange(x2, y);
				aiLeft->SetNewX(ai->GetNewX());
			}
		}
		// spread X values to the right
		if (maxX != -10000)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(maxX, y);
			for (int x2 = maxX + 1; x2 < adjustInfoHeader.width; x2++)
			{
				PositionAdjustInfo2* aiRight = GetAdjustInfoNoChange(x2, y);
				aiRight->SetNewX(ai->GetNewX());
			}
		}
	}
	for (int x = 0; x < adjustInfoHeader.width; x++)
	{
		int minY = 10000, maxY = -10000;
		for (int y = 0; y < adjustInfoHeader.height; y++)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, y);
			if (ai == NULL)
			{
				continue;
			}
			if (minY > y && ai->HasYMeasured())
			{
				minY = y;
			}
			if (maxY < y && ai->HasYMeasured())
			{
				maxY = y;
			}
			// if X to left are missing values, copy them
			{
				PositionAdjustInfo2* aiLeft = GetAdjustInfoNoChange(x - 1, y);
				if (aiLeft != NULL && ai->HasXMeasured() && aiLeft->HasXMeasured() == 0)
				{
					for (int x2 = x - 1; x2 >= 0; x2--)
					{
						PositionAdjustInfo2* aiLeft2 = GetAdjustInfoNoChange(x2, y);
						aiLeft2->SetNewX(ai->GetNewX());
					}
				}
			}
			// if X right to us are missing values, copy them
			{
				PositionAdjustInfo2* aiRight = GetAdjustInfoNoChange(x + 1, y);
				if (aiRight != NULL && ai->HasXMeasured() && aiRight->HasXMeasured() == 0)
				{
					for (int x2 = x + 1; x2 < adjustInfoHeader.width; x2++)
					{
						PositionAdjustInfo2* aiRight2 = GetAdjustInfoNoChange(x2, y);
						aiRight2->SetNewX(ai->GetNewX());
					}
				}
			}
		}
		// spread Y values to the left
		if (minY != 10000)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, minY);
			for (int y2 = minY - 1; y2 >= 0; y2--)
			{
				PositionAdjustInfo2* aiTop = GetAdjustInfoNoChange(x, y2);
				aiTop->SetNewY(ai->GetNewY());
			}
		}
		// spread X values to the right
		if (maxY != -10000)
		{
			PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, maxY);
			for (int y2 = maxY + 1; y2 < adjustInfoHeader.height; y2++)
			{
				PositionAdjustInfo2* aiBottom = GetAdjustInfoNoChange(x, y2);
				aiBottom->SetNewY(ai->GetNewY());
			}
		}
	}
#endif
}