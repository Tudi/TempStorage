#include "StdAfx.h"

void WriteShapesToFile(const char* FileName, int Width, int Height)
{
	std::list<ShapeGeneric*>* shapes = GetExractedShapes();
	FILE* f;
	errno_t er = fopen_s(&f, FileName, "wt");
	if (f == NULL)
	{
		printf("Could not open output file for writing\n");
		return;
	}
	//write version
	fprintf(f, "ver:%s\n", SHP_FILE_VERSION_STRING);
	fprintf(f, "BACKCOLOR:%d,%d,%d\n",255,255,255);
	fprintf(f, "ImageSize:%d,%d\n", Width, Height);
	fprintf(f, "ShapeCount:%d\n",(int)shapes->size());
	for (auto itr = shapes->begin(); itr != shapes->end(); itr++)
	{
		ShapeGeneric* sg = (*itr);
		if (sg->ShapeType == SHT_SQUARE)
		{
			fprintf(f, "% 10s,% 5d,% 5d,% 5d,% 6d,% 6d,% 6d,% 6d\n", "SQUARE", sg->R, sg->G, sg->B, sg->Shapes.sq.StartX, sg->Shapes.sq.StartY, sg->Shapes.sq.Width, sg->Shapes.sq.Height);
		}
		else if (sg->ShapeType == SHT_LINE)
		{
			fprintf(f, "% 10s,% 5d,% 5d,% 5d,% 6d,% 6d,% 6d,% 6d,% 3d\n", "LINE", sg->R, sg->G, sg->B, sg->Shapes.sl.StartX, sg->Shapes.sl.StartY, sg->Shapes.sl.EndX, sg->Shapes.sl.EndY, sg->Shapes.sl.Width);
		}
	}
	fclose(f);
}