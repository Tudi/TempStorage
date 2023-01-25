#pragma once

struct FIBITMAP;

FIBITMAP* LoadImage_(const char *FileName);
bool SaveImagePNG(FIBITMAP *, const char* FileName);
void SaveImagePNG(BYTE *, int Width, int Height, int pitch, const char* FileName);