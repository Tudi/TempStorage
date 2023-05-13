#pragma once

#include <stdio.h>
#include "FreeImage.h"

struct FIBITMAP;

FIBITMAP* LoadImage_(const char *FileName);
FIBITMAP* CreateNewImage(int width, int height);
bool SaveImagePNG(FIBITMAP *, const char* FileName);
void SaveImagePNG(BYTE *, int Width, int Height, int pitch, const char* FileName);