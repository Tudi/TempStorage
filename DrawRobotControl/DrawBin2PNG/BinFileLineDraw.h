#pragma once

#define MIN_POINTS_LINE_EXTEND	100

#pragma pack(push, 1)
typedef struct RelativeLinePoint
{
	float dx, dy;
}RelativeLinePoint;
typedef struct RelativePointsLine
{
	static int ensureCanStoreLinePoint(RelativePointsLine** line, int count);
	static int storeNextPoint(RelativePointsLine** line, double dx, double dy);
	static int setPenPosition(RelativePointsLine** line, int penPos);
	static int setStartingPosition(RelativePointsLine** line, double sx, double sy); // only makes sense if you wish to draw the line on screen without other relative lines
	int numberOfPoints;
	int numberOfPointsCanStore;
	int penPosition;
	float startx, starty; // not yet used
	RelativeLinePoint moves[1]; // the size of this array is unknown. Though it says 1 elements, it's actually dynamic
}RelativePointsLine;
#pragma pack(pop)


void DrawBinLineOnPNG(FIBITMAP *dib, float&x, float&y, RelativePointsLine*line);
void DrawCircleAt(FIBITMAP* dib, float x, float y, float radius);
// create a line that will be similar to what it would look like read from the BIN file
// using this as an experiment to check if issues are with opcode rotations
void DrawLineRelativeInMem(float sx, float sy, float ex, float ey, RelativePointsLine** line);