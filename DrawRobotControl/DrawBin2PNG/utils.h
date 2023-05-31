#pragma once

typedef struct picLoc
{
	int x, y;
}picLoc;

typedef struct ShapeStore
{
	int startX, startY;					// position where we started scanning the whole shape
	int minX, maxX, minY, maxY;			// bound boxing the shape. Not expecting the shape to have box size though
	int centerx, centery;				// based on bounding box, the center is ...
	int centerx2, centery2;				// based on pixels present, the center is ...
	int expectedx, expectedy;
	int lineId;
	int pixelsFound;
	int checkedNeighbours;
	int lineIndex; // temp store
	picLoc* locs; // this is shared memory between multiple threads. Do not deallocate
	int locCount; 
	//	int shapeRelativX, shapeRelativY; // relativ to center, where is this shape located ?
	int HasValues() { return (minX != maxX); }
}ShapeStore;

#define ColorMatch(a,b) (((a)&0x00FFFFFF)==((b)&0x00FFFFFF))

// extract a shape from an image
// location of the shape will be marked on the image to not extract it a second time
// if the shape is a line, you can specify a "gap" in the line, so it will be considered a continuous line
ShapeStore ExtractShapeAtLoc(BYTE* Bytes, int Stride, int Width, int Height, int x, int y, int jumpGap, int searchRadiusX, int searchRadiusY);
ShapeStore ExtractShapeAtLoc2(FIBITMAP* Img, int x, int y, int jumpGap, int searchRadiusX, int searchRadiusY);
void SetShapeColor(ShapeStore *ss, FIBITMAP* Img, BYTE R, BYTE G, BYTE B);
int IsCallibrationLinePixel(FIBITMAP* Img, int x, int y);
void DrawLineColor(FIBITMAP* Img, float sx, float sy, float ex, float ey, BYTE R, BYTE G, BYTE B);
void DrawLineColorFade(FIBITMAP* Img, float sx, float sy, float ex, float ey, BYTE R, BYTE G, BYTE B);
void SetPixelColor(FIBITMAP* Img, float sx, float sy, int size, BYTE R, BYTE G, BYTE B);
int getSign(double a);
char* exec(const char* cmd);