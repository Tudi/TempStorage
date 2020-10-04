#pragma once
#include <list>

struct FontExtracted;


void WriteCharsToFile(std::list<FontExtracted*>* FontShapes, const char* FileName); 
void WriteTextToFileMerged(std::list<FontExtracted*>* FontShapes, const char* FileName);