#include "StdAfx.h"
#include "FontLib.h"
#include "ShapeExtract.h"

void WriteCharsToFile(std::list<FontExtracted*>* FontShapes, const char* FileName)
{
	FILE* f;
	errno_t er = fopen_s(&f, FileName, "wt");
	if (f == NULL)
	{
		printf("Could not open output file for writing\n");
		return;
	}
	//write version
	fprintf(f, "ver:%s\n", SHP_FILE_VERSION_STRING);
	fprintf(f, "FileType:CharactersOnly\n");

	for (auto itr = FontShapes->begin(); itr != FontShapes->end(); itr++)
	{
		FontExtracted* fe = *itr;
		if (fe->AssignedString == NULL)
			continue;
		fprintf(f, "CHAR:%d,%d,%d,%d,%s\n", fe->x, fe->y, fe->Width, fe->Height, fe->AssignedString);
	}
}

void WriteTextToFileMerged(std::list<FontExtracted*>* FontShapes, const char* FileName)
{
	FILE* f;
	errno_t er = fopen_s(&f, FileName, "wt");
	if (f == NULL)
	{
		printf("Could not open output file for writing\n");
		return;
	}
	//write version
	fprintf(f, "ver:%s\n", SHP_FILE_VERSION_STRING);
	fprintf(f, "FileType:CharactersMerged\n");

	for (auto itr = FontShapes->begin(); itr != FontShapes->end(); itr++)
	{
		FontExtracted* fe = *itr;
		if (fe->AssignedString == NULL)
			continue;
		if (fe->MergedPrev != NULL)
			continue;
		//if we already used this shape
		if (fe->MergedNext == NULL && fe->MergedPrev == NULL)
		{
			fprintf(f, "CHAR:%d,%d,%d,%d,%s\n", fe->x, fe->y, fe->Width, fe->Height, fe->AssignedString);
			continue;
		}
		//print sequence of chars
		fprintf(f, "TEXT:%d,%d,%d,%d,%s", fe->x, fe->y, fe->Width, fe->Height, fe->AssignedString);
		fe = fe->MergedNext;
		while (fe != NULL)
		{
			fprintf(f, "%s", fe->AssignedString);
			fe = fe->MergedNext;
		}
		fprintf(f, "\n");
	}
}