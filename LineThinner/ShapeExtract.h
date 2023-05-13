#pragma once

#include "FreeImage.h"

typedef struct ShapeLoc
{
	int x, y;
}ShapeLoc;

typedef struct ShapeStore
{
	int startX, startY;					// position where we started scanning the whole shape
	int minX, maxX, minY, maxY;			// bound boxing the shape. Not expecting the shape to have box size though
	int centerx, centery;				// based on bounding box, the center is ...
	int centerx2, centery2;				// based on pixels present, the center is ...
	int expectedx, expectedy;
	int lineId;
	int checkedNeighbours;
	int lineIndex; // temp store
	//	int shapeRelativX, shapeRelativY; // relativ to center, where is this shape located ?
	int HasValues() { return (minX != maxX); }
	int shapePointCount;
	ShapeLoc* points;
	void Constructor() { memset(this, 0, sizeof(this)); }
	void Destructor() { free(points); }
}ShapeStore;

typedef enum ShapeExtractFlags
{
	SEF_NONE,
	SEF_ExtractPoints = 1,
	SEF_ExtractMiniImg = 2,
	SEF_ReplaceWithLineId = 4,
	SEF_ReplaceWithColor = 8,
}ShapeExtractFlags;

ShapeStore ExtractShapeAtLoc(FIBITMAP* Img, int x, int y, int jumpGap = 3, int searchRadiusX = -1, int searchRadiusY = -1, ShapeExtractFlags sef = SEF_NONE);
void SetShapeColor(FIBITMAP* Img, ShapeStore* ss, BYTE R, BYTE G, BYTE B);