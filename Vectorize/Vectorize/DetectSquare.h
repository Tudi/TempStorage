#pragma once

struct ShapeGeneric;
struct FIBITMAP;
int DetectSquareAt(FIBITMAP*dib,int x, int y, ShapeGeneric* ret);
void MarkShapeSquareExtracted(FIBITMAP* dib, ShapeGeneric* ret);
void PaintShapeSquareExtracted(FIBITMAP* dib, ShapeGeneric* ret, int R, int G, int B);
