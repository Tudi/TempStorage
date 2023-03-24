#pragma once

typedef struct ShapeStore
{
	int startX, starty;					// position where we started scanning the whole shape
	int minX, maxX, minY, maxY;			// bound boxing the shape. Not expecting the shape to have box size though
	int centerx, centery;				// based on bounding box, the center is ...
	int centerx2, centery2;				// based on pixels present, the center is ...
	int expectedx, expectedy;
	int lineId;
	int checkedNeighbours;
	int lineIndex; // temp store
	//	int shapeRelativX, shapeRelativY; // relativ to center, where is this shape located ?
	int HasValues() { return (minX != maxX); }
}ShapeStore;

// extract a shape from an image
// location of the shape will be marked on the image to not extract it a second time
// if the shape is a line, you can specify a "gap" in the line, so it will be considered a continuous line
ShapeStore ExtractShapeAtLoc(FIBITMAP* Img, int x, int y, int jumpGap, int searchRadiusX, int searchRadiusY);
int IsCallibrationLinePixel(FIBITMAP* Img, int x, int y);
void DrawLineColor(FIBITMAP* Img, float sx, float sy, float ex, float ey, BYTE R, BYTE G, BYTE B);