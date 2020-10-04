#pragma once

int DetectLineAtVertHor(FIBITMAP* dib, int x, int y, ShapeGeneric* ret);
void MarkShapeLineExtracted(FIBITMAP* dib, ShapeGeneric* ret);
void PaintShapeLineExtracted(FIBITMAP* dib, ShapeGeneric* ret, int R, int G, int B);
int GetLineLength(ShapeGeneric* ret);