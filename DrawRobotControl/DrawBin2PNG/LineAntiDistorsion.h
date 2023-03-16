#pragma once

class RelativePointsLine;

#pragma pack(push, 1)
typedef struct PositionAdjustInfoHeader
{
	int version;
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
	char isEstimated; // not every location will be adjusted. Locations between known adjustments are averaged
}PositionAdjustInfo;
#pragma pack(pop)

// class to adjust a straight drawn line into a bent line in order for a robot to draw it straight
class LineAntiDistorsionAdjuster
{
public:
	LineAntiDistorsionAdjuster();
	void AdjustLine(RelativePointsLine** line);
private:
	void LoadAdjusterMap();
	void SaveAdjusterMap();
	PositionAdjustInfo* GetAdjustInfo(int x, int y);
	PositionAdjustInfoHeader adjustinfoHeader;
	PositionAdjustInfo* adjustInfoMap;
};

// global resource with static initialization
extern LineAntiDistorsionAdjuster sLineAdjuster;