#pragma once

#define CALIBRATION_FILE_NAME "SA2.cal"
#define CALIBRATION_4CC	"cal "
#define POSITION_TO_SHORT_SCALER (65000.0/10.0) // map 10 inches into 65 values

class RelativePointsLine;

#pragma pack(push, 1)
typedef struct PositionAdjustInfoHeader
{
	int version; // make sure it's always first so we can take a peak
	int fourCC;
	int headerSize, infoSize; // sanity checkes in case you forgot to increase version number
	int width, height; // given in commands. Tear size was 10x10. We can squeeze less than 600 commands in an inch. Would need 6000x6000 map (that is too large !)
	int originX, originY;
	float scaleX, scaleY; // MapFile might get large unless scaled down
}PositionAdjustInfoHeader;

enum PositionAdjustInfoFlags : char
{
	X_IS_MEASURED = 1 << 0,
	Y_IS_MEASURED = 1 << 1,
	X_IS_SET = 1 << 2,
	Y_IS_SET = 1 << 3,
};

typedef struct Adjusted2DPos
{
	float x, y;
	char HasValues;
}Adjusted2DPos;

// all the info required to make a line draw straight
typedef struct PositionAdjustInfo
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
	void SetNewX(int newX) 
	{ 
		flags = (PositionAdjustInfoFlags)(flags | X_IS_SET);
		shouldBeX = (short)(newX); 
	}
	int GetNewX() 
	{ 
		return shouldBeX; 
	}

	int HasY() 
	{ 
		return (flags & Y_IS_SET); 
	}
	void SetNewY(int newY) 
	{ 
		flags = (PositionAdjustInfoFlags)(flags | Y_IS_SET);
		shouldBeY = (short)(newY); 
	}
	int HasYMeasured()
	{
		return (flags & Y_IS_MEASURED);
	}
	int GetNewY()
	{ 
		return shouldBeY; 
	}
	PositionAdjustInfoFlags flags; // not every location will be adjusted. Locations between known adjustments are averaged
private:
	// using shorts instead floats to avoid very large map file size. If speed is more important than size, use floats
	short shouldBeX; // line needs to be moved to this new specific location in order to look straight
	short shouldBeY; // line needs to be moved to this new specific location in order to look straight
}PositionAdjustInfo;
#pragma pack(pop)

// class to adjust a straight drawn line into a bent line in order for a robot to draw it straight
class LineAntiDistorsionAdjuster
{
public:
	LineAntiDistorsionAdjuster();
	~LineAntiDistorsionAdjuster();
	/// <summary>
	/// Center (0,0) is in the middle
	/// Units of measurement are expected to be in robot commands = inches * PIXELS_IN_INCH
	/// Anti distortion map was generated expecting an inch to have 600 commands
	/// </summary>
	/// <param name="header"></param>
	void DrawLine(float sx, float sy, float ex, float ey, RelativePointsLine* out_line);
	/// position is in robot commands = inches * PIXELS_IN_INCH, 0,0 is at the center
	void AdjustPositionX(int x, int y, int shouldBeX);
	void AdjustPositionY(int x, int y, int shouldBeY);
	// position is robot commands = inches * PIXELS_IN_INCH, 0,0 is at the center
	PositionAdjustInfo* GetAdjustInfo(int x, int y);
	void DebugDumpMapToImage(int col);
	// to support sub pixel accuracy. Required to avoid accumulating positioning error on lots of small segments
	Adjusted2DPos GetAdjustedPos(float x, float y);
private:
	void CreateNewMap(PositionAdjustInfoHeader* header);
	void LoadAdjusterMap();
	void SaveAdjusterMap();
	void BleedAdjustmentsToNeighbours();
	// position is non adjusted memory map position
	PositionAdjustInfo* GetAdjustInfoNoChange(int x, int y);
	void FindClosestKnown(int x, int y, int flag, PositionAdjustInfo** out_ai, int& atx, int& aty);
	void FindClosestKnownRight(int sx, int sy, PositionAdjustInfo** out_right, int& atx_right);
	void FindClosestKnownDown(int sx, int sy, PositionAdjustInfo** out_down, int& aty_right);
	PositionAdjustInfoHeader adjustInfoHeader;
	PositionAdjustInfo* adjustInfoMap;
	int hasUnsavedAdjustments;
	int needsBleedX, needsBleedY;
};

// global resource with static initialization
extern LineAntiDistorsionAdjuster sLineAdjuster;