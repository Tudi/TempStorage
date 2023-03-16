#pragma once

#define MIN_POINTS_LINE_EXTEND		100
#define PIXELS_IN_INCH				600.0f
#define MIN_LINE_SEGMENT_LENGTH		1

#pragma pack(push, 1)
typedef struct RelativeLinePoint
{
	float dx, dy;
}RelativeLinePoint;

class RelativePointsLine
{
public:
	RelativePointsLine() {
		moves = NULL;  
		numberOfPoints = 0; 
		numberOfPointsCanStore = 0; 
		penPosition = 3;  // invalid value
		startx = endx = starty = endy = 0; 
		isDistorsionAdjusted = 0;
	}
	~RelativePointsLine() { 
		free(moves); 
		moves = NULL; 
		numberOfPoints = 0; 
		numberOfPointsCanStore = 0; 
	}
	int storeNextPoint(double dx, double dy);
	void setPenPosition(int penPos) { penPosition = penPos; }
	int getPenPosition() { return penPosition; }
	void setStartingPosition(double sx, double sy) { startx = (float)sx; starty = (float)sy;}
	void setEndPosition(double ex, double ey) { endx = (float)ex; endy = (float)ey; }
	int GetPointsCount() { return numberOfPoints; }
	float GetDX(size_t at) { return moves[at].dx; }
	float GetDY(size_t at) { return moves[at].dy; }
	void SetDX(size_t at, float dx) { moves[at].dx = dx; }
	void SetDY(size_t at, float dy) { moves[at].dy = dy; }
	float GetStartX() { return startx; }
	float GetStartY() { return starty; }
	float GetEndX() { return endx; }
	float GetEndY() { return endy; }
	void SetIsDistorsionAdjusted(int newVal) { isDistorsionAdjusted = newVal; }
private:
	int ensureCanStoreLinePoint(int count);
	int numberOfPoints;
	int numberOfPointsCanStore;
	int penPosition;
	int isDistorsionAdjusted;
	float startx, starty; // not yet used
	float endx, endy; // not yet used
	RelativeLinePoint *moves; // the size of this array is unknown. Though it says 1 elements, it's actually dynamic
};
#pragma pack(pop)

class RelativePointsLine;
void DrawBinLineOnPNG(FIBITMAP *dib, float&x, float&y, RelativePointsLine*line);
void DrawCircleAt(FIBITMAP* dib, float x, float y, float radius);
// create a line that will be similar to what it would look like read from the BIN file
// using this as an experiment to check if issues are with opcode rotations
void DrawLineRelativeInMem(float sx, float sy, float ex, float ey, RelativePointsLine* line);