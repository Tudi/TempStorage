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

double interpolate(double q11, double q12, double q21, double q22, double x, double y) 
{
	double r1 = (q21 - q11) * x + q11;
	double r2 = (q22 - q12) * x + q12;
	return (r2 - r1) * y + r1;
}

// sub pixel accuracy to not accumulate rounding errors when drawing a long line
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
	else if ((int)floor(x2) == (int)ceil(x2))
	{
		double valy[2];
		int indy = 0;
		for (int y3 : {(int)floor(y2), (int)ceil(y2)})
		{
			PositionAdjustInfo2* poi = GetAdjustInfoNoChange((int)x2, y3);
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
					valy[indy++] = (float)poi->GetNewY();
				}
				else
				{
					return ret;
				}
			}
		}
		double coef = y2 - floor(y2);
		ret.y = valy[0] * coef + valy[1] * (1 - coef);
		ret.HasValues = 1;
		return ret;
	}
	else if ((int)floor(y2) == (int)ceil(y2))
	{
		double valy[2];
		int indy = 0;
		for (int x3 : {(int)floor(x2), (int)ceil(x2)})
		{
			PositionAdjustInfo2* poi = GetAdjustInfoNoChange(x3, (int)y2);
			if (poi)
			{
				if (poi->HasX())
				{
					valy[indy++] = (float)poi->GetNewX();
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
		}
		double coef = x2 - floor(x2);
		ret.x = valy[0] * coef + valy[1] * (1 - coef);
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
			PositionAdjustInfo2 *poi = GetAdjustInfoNoChange(x3, y3);
			if (poi)
			{
				if (poi->HasX())
				{
					ax[valuesY][valuesX] = (float)poi->GetNewX();
				}
				else
				{
					return ret;
				}
				if (poi->HasY())
				{
					ay[valuesY][valuesX] = (float)poi->GetNewY();
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

	ret.x = (float)interpolate(ax[0][0], ax[0][1], ax[1][0], ax[1][1], x2 - floor(x2), y2 - floor(y2));
	ret.y = (float)interpolate(ay[0][0], ay[0][1], ay[1][0], ay[1][1], x2 - floor(x2), y2 - floor(y2));
	ret.HasValues = 1;

	SOFT_ASSERT(ret.x > -3000, "Return value is unexpectedly small");
	SOFT_ASSERT(ret.x < 3000, "Return value is unexpectedly large");
	SOFT_ASSERT(ret.y > -3000, "Return value is unexpectedly small");
	SOFT_ASSERT(ret.y < 3000, "Return value is unexpectedly large");

//	SOFT_ASSERT(abs(ret.x - x) < 200, "Return value is unexpectedly large");
//	SOFT_ASSERT(abs(ret.y - y) < 200, "Return value is unexpectedly large");

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

void LineAntiDistorsionAdjuster2::FindClosestKnownRight(int sx, int sy, PositionAdjustInfo2** out_right, int& atx_right)
{
//#define PROPAGATE_SEARCH_RADIUS (adjustInfoHeader.width * 10 / 100)
#define PROPAGATE_SEARCH_RADIUS 100
	*out_right = NULL;
	for (int x = sx + 1; x < sx + PROPAGATE_SEARCH_RADIUS; x++)
	{
		PositionAdjustInfo2* ai = GetAdjustInfoNoChange(x, sy);
		if (ai && (ai->flags & X_IS_MEASURED))
		{
			*out_right = ai;
			atx_right = x;
			return;
		}
	}
}

void LineAntiDistorsionAdjuster2::FindClosestKnownDown(int sx, int sy, PositionAdjustInfo2** out_down, int& aty_down)
{
	*out_down = NULL;
	for (int y = sy + 1; y < sy + PROPAGATE_SEARCH_RADIUS; y++)
	{
		PositionAdjustInfo2* ai = GetAdjustInfoNoChange(sx, y);
		if (ai && ai->HasYMeasured())
		{
			*out_down = ai;
			aty_down = y;
			return;
		}
	}
}

void LineAntiDistorsionAdjuster2::AdjustPosition(int x, int y, double shouldBeX, double shouldBeY)
{
	PositionAdjustInfo2* ai = GetAdjustInfo(x, y);
	SOFT_ASSERT(ai != NULL, "Calibration map should always be larger than calibration image size");
	if (ai->GetNewX() != shouldBeX || (ai->HasXMeasured()) == 0 ||
		ai->GetNewY() != shouldBeY || (ai->HasYMeasured()) == 0)
	{
		ai->SetNewX(shouldBeX);
		ai->SetNewY(shouldBeY);
		SOFT_ASSERT((int)(ai->GetNewX() * 100) == (int)(shouldBeX * 100), "Precision loss error");
		SOFT_ASSERT((int)(ai->GetNewY() * 100) == (int)(shouldBeY * 100), "Precision loss error");
		hasUnsavedAdjustments = 1;
	}
}

void AppendLineSegment(float sx, float sy, float ex, float ey, RelativePointsLine* out_line, float &leftOverX, float& leftOverY)
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

	dx += leftOverX;
	dy += leftOverY;

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

	leftOverX = (float)(dx - writtenDiffX);
	leftOverY = (float)(dy - writtenDiffY);
}

void LineAntiDistorsionAdjuster2::DrawLine(float sx, float sy, float ex, float ey, RelativePointsLine* out_line)
{
	double dx = ex - sx;
	double dy = ey - sy;
	float leftOverX = 0;
	float leftOverY = 0;
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

		// try sub pixel accuracy adjusting
		Adjusted2DPos2 aiStart = GetAdjustedPos((float)sx2, (float)sy2);
		Adjusted2DPos2 aiEnd = GetAdjustedPos((float)ex2, (float)ey2);
		if (aiStart.HasValues && aiEnd.HasValues)
		{
			sx2 = aiStart.x;
			ex2 = aiEnd.x;
			sy2 = aiStart.y;
			ey2 = aiEnd.y;
		}
		else
		{
			PositionAdjustInfo2* aiStart = GetAdjustInfo((int)sx2, (int)sy2);
			PositionAdjustInfo2* aiEnd = GetAdjustInfo((int)ex2, (int)ey2);
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
			else
			{
				printf("Failed to find adjustment info. Sad case");
			}
		}

		AppendLineSegment((float)sx2, (float)sy2, (float)ex2, (float)ey2, out_line, leftOverX, leftOverY);
	}

	out_line->setEndPosition(ex - leftOverX, ey - leftOverY);
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