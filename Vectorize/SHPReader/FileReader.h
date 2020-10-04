#pragma once

struct ShapeGeneric;

struct SHPFileDetails
{
	char* Version;
	int Width, Height;
	int Back_R, Back_G, Back_B;
	std::list<ShapeGeneric*>* shapes;
};

SHPFileDetails* ReadShapesFromFile(const char* FileName);