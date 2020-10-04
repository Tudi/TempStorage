#include "StdAfx.h"

#include "FileReader.h"

void RemoveSpaces(char* Line)
{
	int ReadIndex = 0;
	int WriteIndex = 0;
	while (Line[ReadIndex] != 0)
	{
		if (Line[ReadIndex] != ' ')
		{
			Line[WriteIndex] = Line[ReadIndex];
			WriteIndex++;
		}
		ReadIndex++;
	}
	Line[WriteIndex] = 0;
}

ShapeGeneric *ParseLineAsSquare(char *Line)
{
	ShapeGeneric* sg = (ShapeGeneric*)malloc(sizeof(ShapeGeneric));
	memset(sg, 0, sizeof(ShapeGeneric));
	RemoveSpaces(Line);
	sscanf_s(&Line[7], "%d,%d,%d,%d,%d,%d,%d\n", &sg->R, &sg->G, &sg->B, &sg->Shapes.sq.StartX, &sg->Shapes.sq.StartY, &sg->Shapes.sq.Width, &sg->Shapes.sq.Height);
	sg->ShapeType = SHT_SQUARE;
	return sg;
}

ShapeGeneric* ParseLineAsLine(char* Line)
{
	ShapeGeneric* sg = (ShapeGeneric*)malloc(sizeof(ShapeGeneric));
	memset(sg, 0, sizeof(ShapeGeneric));
	RemoveSpaces(Line);
	sscanf_s(&Line[5], "%d,%d,%d,%d,%d,%d,%d,%d\n", &sg->R, &sg->G, &sg->B, &sg->Shapes.sl.StartX, &sg->Shapes.sl.StartY, &sg->Shapes.sl.EndX, &sg->Shapes.sl.EndY, &sg->Shapes.sl.Width);
	sg->ShapeType = SHT_LINE;
	return sg;
}

SHPFileDetails *ReadShapesFromFile(const char* FileName)
{
	FILE* f;
	errno_t er = fopen_s(&f, FileName, "rt");
	if (f == NULL)
	{
		printf("Could not open output file for reading\n");
		return NULL;
	}
	SHPFileDetails* ret = (SHPFileDetails*)malloc(sizeof(SHPFileDetails));
	memset(ret, 0, sizeof(SHPFileDetails));

	ret->shapes = new std::list<ShapeGeneric*>();

	//write version
	char LineBuffer[5000];
	fscanf_s(f, "ver:%s\n", LineBuffer, (int)sizeof(LineBuffer));
	ret->Version = _strdup(LineBuffer);
	fscanf_s(f, "BACKCOLOR:%d,%d,%d\n", &ret->Back_R, &ret->Back_G, &ret->Back_B);
	fscanf_s(f, "ImageSize:%d,%d\n", &ret->Width, &ret->Height);
	int ShapeCount;
	fscanf_s(f, "ShapeCount:%d\n",&ShapeCount);
	while (!feof(f))
	{
		ShapeGeneric* sg = NULL;
		fgets(LineBuffer, sizeof(LineBuffer), f);
		RemoveSpaces(LineBuffer);
		if (strstr(LineBuffer, "QUARE") == LineBuffer + 1)
			sg = ParseLineAsSquare(LineBuffer);
		if (strstr(LineBuffer, "INE") == LineBuffer + 1)
			sg = ParseLineAsLine(LineBuffer);
		if (sg)
			ret->shapes->push_back(sg);
	}
	fclose(f);
	return ret;
}