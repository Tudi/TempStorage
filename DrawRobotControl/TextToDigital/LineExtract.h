#pragma once

typedef struct LinePoint
{
	short x, y;
}LinePoint;

typedef struct LinePoints
{
	int pointCount;
	LinePoint points[10000];
}LinePoints;

typedef struct LineStore
{
	int sx, sy, ex, ey;
	double len;
}LineStore;

// from a starting position, try to draw lines to all possible directions
// check what is the longest line that could be drawn to what position
// Todo : instead of try it all, follow possible options
void ExtractLineAt(struct FIBITMAP* img, int x, int y, LineStore *res);

void ExtractAllLines(struct FIBITMAP* img);

void EliminateSmallLines();

void DrawLines(struct FIBITMAP* img);

void ProcessInputUsingLines(struct FIBITMAP* img);