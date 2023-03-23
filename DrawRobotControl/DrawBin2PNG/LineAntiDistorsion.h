#pragma once

#define CALIBRATION_FILE_NAME "SA2.cal"
#define CALIBRATION_4CC	"cal "

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
};

// all the info required to make a line draw straight
typedef struct PositionAdjustInfo
{
	float shouldBeX; // line needs to be moved to this new specific location in order to look straight
	float shouldBeY; // line needs to be moved to this new specific location in order to look straight
	PositionAdjustInfoFlags flags; // not every location will be adjusted. Locations between known adjustments are averaged
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
	/// Units of measurement are expected to be inches
	/// Anti distortion map was generated expecting an inch to have 600 commands
	/// </summary>
	/// <param name="header"></param>
	void DrawLine(float sx, float sy, float ex, float ey, RelativePointsLine* out_line);
//	void AdjustLine(RelativePointsLine* line);
	void CreateNewMap(PositionAdjustInfoHeader* header);
	void AdjustPositionX(int x, int y, int shouldBeX);
//	void MarkAdjustmentsOutdated();
private:
	void LoadAdjusterMap();
	void SaveAdjusterMap();
	void BleedAdjustmentsToNeighbours();
	PositionAdjustInfo* GetAdjustInfo(int x, int y);
	PositionAdjustInfo* GetAdjustInfoNoChange(int x, int y);
	void FindClosestKnown(int x, int y, int flag, PositionAdjustInfo** out_ai, int& atx, int& aty);
	PositionAdjustInfoHeader adjustInfoHeader;
	PositionAdjustInfo* adjustInfoMap;
	int hasUnsavedAdjustments;
	int needsBleedX, needsBleedY;
};

// global resource with static initialization
extern LineAntiDistorsionAdjuster sLineAdjuster;