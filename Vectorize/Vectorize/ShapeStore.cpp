#include "StdAfx.h"
#include <list>

std::list<ShapeGeneric*> ExtractedShapes;

std::list<ShapeGeneric*>* GetExractedShapes()
{
	return &ExtractedShapes;
}

int GetShapeCount()
{
	return (int)ExtractedShapes.size();
}

void AddShapeCopyFrom(ShapeGeneric* gs)
{
	ShapeGeneric *ss = new ShapeGeneric();
	memcpy(ss, gs, sizeof(ShapeGeneric));
	ExtractedShapes.push_back(ss);
}

void PaintShapes(FIBITMAP* dib, const char* FileName)
{
	int InitialColor = 0x00000001;
	for (auto itr = ExtractedShapes.begin(); itr != ExtractedShapes.end(); itr++)
	{
		if((*itr)->ShapeType == SHT_SQUARE)
			PaintShapeSquareExtracted(dib, (*itr), ((InitialColor >> 0) & 255), ((InitialColor >> 8) & 255), ((InitialColor >> 16) & 255));
		else if ((*itr)->ShapeType == SHT_LINE)
			PaintShapeLineExtracted(dib, (*itr), ((InitialColor >> 0) & 255), ((InitialColor >> 8) & 255), ((InitialColor >> 16) & 255));
		InitialColor *= 0x00070707;
//		InitialColor++;
	}
	SaveImagePNG(dib, FileName);
}

int CanMergeLeftAndRight(ShapeGeneric* prev, ShapeGeneric* cur)
{
	//only support these types atm
	if (prev->ShapeType != SHT_SQUARE && prev->ShapeType != SHT_LINE)
		return 0;
	if (cur->ShapeType != SHT_SQUARE)
		return 0;

	//color must match for merge
	if (prev->B != cur->B || prev->G != cur->G || prev->R != cur->R)
		return 0;

	//start on the same line
	int PrevShapeY = 0;
	if (prev->ShapeType == SHT_SQUARE)
		PrevShapeY = prev->Shapes.sq.StartY;
	else if (prev->ShapeType == SHT_LINE)
		PrevShapeY = prev->Shapes.sl.StartY;
	int CurShapeY = 0;
	if (cur->ShapeType == SHT_SQUARE)
		CurShapeY = cur->Shapes.sq.StartY;
	else if (cur->ShapeType == SHT_LINE)
		CurShapeY = cur->Shapes.sl.StartY;
	if (PrevShapeY != CurShapeY)
		return 0;

	//for a vertical line merge we need to match height
	int PrevHeight = 0;
	if (prev->ShapeType == SHT_SQUARE)
		PrevHeight = prev->Shapes.sq.Height;
	else if (prev->ShapeType == SHT_LINE)
		PrevHeight = prev->Shapes.sl.Width;
	int CurHeight = 0;
	if (cur->ShapeType == SHT_SQUARE)
		CurHeight = cur->Shapes.sq.Height;
	else if (cur->ShapeType == SHT_LINE)
		CurHeight = cur->Shapes.sl.Width;
	if (PrevHeight != CurHeight)
		return 0;

	//make sure next shape starts after current shape
	int PrevShapeEndX = 0;
	if (prev->ShapeType == SHT_SQUARE)
		PrevShapeEndX = prev->Shapes.sq.StartX + prev->Shapes.sq.Width;
	else if (prev->ShapeType == SHT_LINE)
		PrevShapeEndX = prev->Shapes.sl.EndX;
	int CurShapeStartX = 0;
	if (cur->ShapeType == SHT_SQUARE)
		CurShapeStartX = cur->Shapes.sq.StartX;
	else if (cur->ShapeType == SHT_LINE)
		CurShapeStartX = cur->Shapes.sl.StartX;
	if (PrevShapeEndX != CurShapeStartX)
		return 0;

	return 1;
}

int CanMergeUpAndBelow(ShapeGeneric* prev, ShapeGeneric* cur)
{
	//only support these types atm
	if (prev->ShapeType != SHT_SQUARE && prev->ShapeType != SHT_LINE)
		return 0;
	if (cur->ShapeType != SHT_SQUARE)
		return 0;

	//color must match for merge
	if (prev->B != cur->B || prev->G != cur->G || prev->R != cur->R)
		return 0;

	//start on the same column
	int PrevShapeX = 0;
	if (prev->ShapeType == SHT_SQUARE)
		PrevShapeX = prev->Shapes.sq.StartX;
	else if (prev->ShapeType == SHT_LINE)
		PrevShapeX = prev->Shapes.sl.StartX;
	int CurShapeX = 0;
	if (cur->ShapeType == SHT_SQUARE)
		CurShapeX = cur->Shapes.sq.StartX;
	else if (cur->ShapeType == SHT_LINE)
		CurShapeX = cur->Shapes.sl.StartX;
	if (PrevShapeX != CurShapeX)
		return 0;

	//for a horizontal line merge we need to match height
	int PrevWidth = 0;
	if (prev->ShapeType == SHT_SQUARE)
		PrevWidth = prev->Shapes.sq.Width;
	else if (prev->ShapeType == SHT_LINE)
		PrevWidth = prev->Shapes.sl.Width;
	int CurWidth = 0;
	if (cur->ShapeType == SHT_SQUARE)
		CurWidth = cur->Shapes.sq.Width;
	else if (cur->ShapeType == SHT_LINE)
		CurWidth = cur->Shapes.sl.Width;
	if (PrevWidth != CurWidth)
		return 0;

	//make sure next shape starts after current shape
	int PrevShapeEndX = 0;
	if (prev->ShapeType == SHT_SQUARE)
		PrevShapeEndX = prev->Shapes.sq.StartX + prev->Shapes.sq.Width;
	else if (prev->ShapeType == SHT_LINE)
		PrevShapeEndX = prev->Shapes.sl.EndX;
	int CurShapeStartX = 0;
	if (cur->ShapeType == SHT_SQUARE)
		CurShapeStartX = cur->Shapes.sq.StartX;
	else if (cur->ShapeType == SHT_LINE)
		CurShapeStartX = cur->Shapes.sl.StartX;
	if (PrevShapeEndX != CurShapeStartX)
		return 0;

	return 1;
}

void MergeSquaresToLines(FIBITMAP* dib)
{
	//merge vertical shapes to lines
	std::list<ShapeGeneric*> ExtractedShapes2;
	ShapeGeneric* prev = NULL;
	ShapeGeneric* cur = NULL;
	int MergeCount = 0;
	for (auto itr = ExtractedShapes.begin(); itr != ExtractedShapes.end(); itr++)
	{
		cur = (*itr);
		//check if previous shape matches for a merge
		if (prev == NULL)
		{
			prev = cur;
			continue;
		}

		if(CanMergeLeftAndRight(prev,cur)==0)
		{
			ExtractedShapes2.push_back(prev);
			prev = cur;
			continue;
		}

		//convert previous shape to line
		if (prev->ShapeType != SHT_LINE)
		{
			prev->ShapeType = SHT_LINE;
			ShapeGeneric temp;
			memcpy(&temp, prev, sizeof(ShapeGeneric));
			prev->Shapes.sl.Angle = 0;
			prev->Shapes.sl.StartX = temp.Shapes.sq.StartX;
			prev->Shapes.sl.StartY = temp.Shapes.sq.StartY;
			prev->Shapes.sl.EndX = temp.Shapes.sq.StartX + temp.Shapes.sq.Width;
			prev->Shapes.sl.EndY = temp.Shapes.sq.StartY + temp.Shapes.sq.Height;
			prev->Shapes.sl.Width = temp.Shapes.sq.Width;
		}

		//merge the prev line with this square
		prev->Shapes.sl.EndX = cur->Shapes.sq.StartX + cur->Shapes.sq.Width;
		free(cur);
		MergeCount++;
	}

	if (MergeCount > 0)
	{
		ExtractedShapes.clear();
		ExtractedShapes = ExtractedShapes2;
	}
}