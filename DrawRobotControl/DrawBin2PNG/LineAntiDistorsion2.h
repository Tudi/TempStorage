#pragma once

#define CALIBRATION_FILE_NAME "SA2.cal"
#define CALIBRATION_4CC	"cal "
#define POSITION_TO_SHORT_SCALER (65000.0f/3000.0f) // 3000 values to 65000

class RelativePointsLine;

#pragma pack(push, 1)
typedef struct PositionAdjustInfoHeader2
{
	int version; // make sure it's always first so we can take a peak
	int fourCC;
	int headerSize, infoSize; // sanity checkes in case you forgot to increase version number
	int width, height; // given in commands. Tear size was 10x10. We can squeeze less than 600 commands in an inch. Would need 6000x6000 map (that is too large !)
	int originX, originY;
	float scaleX, scaleY; // MapFile might get large unless scaled down
}PositionAdjustInfoHeader2;

enum PositionAdjustInfoFlags2 : char
{
	X_IS_MEASURED = 1 << 0,
	Y_IS_MEASURED = 1 << 1,
	X_IS_SET = 1 << 2,
	Y_IS_SET = 1 << 3,
	X_IS_UPDATED = 1 << 4,
	Y_IS_UPDATED = 1 << 5,
};

typedef struct Adjusted2DPos2
{
	double x, y;
	char HasValues;
}Adjusted2DPos2;

// all the info required to make a line draw straight
typedef struct PositionAdjustInfo2
{
public:
	int HasX() 
	{ 
		return (flags & X_IS_SET); 
	}
	int HasXMeasured()
	{
		return (flags & X_IS_MEASURED);
	}
	void SetNewX(double newX) 
	{ 
		flags = (PositionAdjustInfoFlags2)(flags | X_IS_SET);
		shouldBeX = newX;
	}
	double GetNewX() 
	{ 
		return shouldBeX;
	}

	int HasY() 
	{ 
		return (flags & Y_IS_SET); 
	}
	void SetNewY(double newY) 
	{ 
		flags = (PositionAdjustInfoFlags2)(flags | Y_IS_SET);
		shouldBeY = newY;
	}
	int HasYMeasured()
	{
		return (flags & Y_IS_MEASURED);
	}
	double GetNewY()
	{ 
		return shouldBeY;
	}
	PositionAdjustInfoFlags2 flags; // not every location will be adjusted. Locations between known adjustments are averaged
private:
	// using shorts instead floats to avoid very large map file size. If speed is more important than size, use floats
	double shouldBeX; // line needs to be moved to this new specific location in order to look straight
	double shouldBeY; // line needs to be moved to this new specific location in order to look straight
}PositionAdjustInfo2;
#pragma pack(pop)

// class to adjust a straight drawn line into a bent line in order for a robot to draw it straight
class LineAntiDistorsionAdjuster2
{
public:
	LineAntiDistorsionAdjuster2();
	~LineAntiDistorsionAdjuster2();
	/// <summary>
	/// Center (0,0) is in the middle
	/// Units of measurement are expected to be in robot commands = inches * PIXELS_IN_INCH
	/// Anti distortion map was generated expecting an inch to have 600 commands
	/// </summary>
	/// <param name="header"></param>
	int DrawLine(double sx, double sy, double ex, double ey, RelativePointsLine* out_line, double& leftOverX, double& leftOverY);
	/// position is in robot commands = inches * PIXELS_IN_INCH, 0,0 is at the center
	void AdjustPosition(int x, int y, double shouldBeX, double shouldBeY, int isInitial);
	// position is robot commands = inches * PIXELS_IN_INCH, 0,0 is at the center
	PositionAdjustInfo2* GetAdjustInfo(int x, int y);
	// to support sub pixel accuracy. Required to avoid accumulating positioning error on lots of small segments
	Adjusted2DPos2 GetAdjustedPos(double x, double y);
	void CreateNewMap(PositionAdjustInfoHeader2* header);

	void DebugDumpMapRowColToImage(int col);
	void DebugDumpMapToImage(int flags, const char *fileName, int inputLayout = 0);
	void DebugDumpMapToTear();
	void FillMissingInfo();
	void FillMissingInfo2();
private:
	void LoadAdjusterMap();
	void SaveAdjusterMap();
	void ScanMatFillMissingValues(int sx, int sy, int dirX, int dirY, int checkFlags);
	// position is non adjusted memory map position
	PositionAdjustInfo2* GetAdjustInfoNoChange(int x, int y);
	PositionAdjustInfoHeader2 adjustInfoHeader;
	PositionAdjustInfo2* adjustInfoMap;
	int hasUnsavedAdjustments;
};

// global resource with static initialization
extern LineAntiDistorsionAdjuster2 sLineAdjuster2;