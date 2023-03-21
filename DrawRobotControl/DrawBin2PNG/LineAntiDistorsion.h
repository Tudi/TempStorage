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
	int width, height;
	int originX, originY;
}PositionAdjustInfoHeader;

// all the info required to make a line draw straight
typedef struct PositionAdjustInfo
{
	float relativeCommandMultiplierX; // based on the position of the pen, the same line might require more commands to be drawn
	float relativeCommandMultiplierY; // based on the position of the pen, the same line might require more commands to be drawn
	float adjustX; // add extra commands to compensate for the pen moving in different direction than we intend
	float adjustY; // add extra commands to compensate for the pen moving in different direction than we intend
	char isMeasured; // not every location will be adjusted. Locations between known adjustments are averaged
}PositionAdjustInfo;
#pragma pack(pop)

// class to adjust a straight drawn line into a bent line in order for a robot to draw it straight
class LineAntiDistorsionAdjuster
{
public:
	LineAntiDistorsionAdjuster();
	~LineAntiDistorsionAdjuster();
	void AdjustLine(RelativePointsLine* line);
	void CreateNewMap(PositionAdjustInfoHeader* header);
private:
	void LoadAdjusterMap();
	void SaveAdjusterMap();
	PositionAdjustInfo* GetAdjustInfo(int x, int y);
	PositionAdjustInfoHeader adjustInfoHeader;
	PositionAdjustInfo* adjustInfoMap;
};

// global resource with static initialization
extern LineAntiDistorsionAdjuster sLineAdjuster;