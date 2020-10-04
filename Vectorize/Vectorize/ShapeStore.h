#pragma once

enum ShapeTypes
{
	SHT_INVALID = 0,
	SHT_SQUARE = 1,
	SHT_LINE = 2,
};

struct ShapeSquare
{
	int StartX, StartY;
	int Width, Height;
};
struct ShapeLine
{
	int StartX, StartY;
	int EndX, EndY;
	int Width;
	float Angle;
};

struct ShapeGeneric
{
	int		ShapeType;
	int		R, G, B;
	union Shapes
	{
		ShapeSquare sq;
		ShapeLine sl;
	} Shapes;
};

void AddShapeCopyFrom(ShapeGeneric* gs);
void MergeSquaresToLines(FIBITMAP* dib);
int GetShapeCount();
void PaintShapes(FIBITMAP* dib,const char *FileName);
std::list<ShapeGeneric*>* GetExractedShapes();