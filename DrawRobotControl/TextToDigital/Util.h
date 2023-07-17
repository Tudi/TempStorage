#pragma once

#define BYTESPP 3
#define BACKGROUND_COLOR	0x00FFFFFF
#define FONT_COLOR			0x00000000
#define FONT_EDGE_COLOR			0x000000FF

#define ColorMatch(a,b) (((a)&0x00FFFFFF)==((b)&0x00FFFFFF))

#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

typedef unsigned char BYTE;

int isBGColor(BYTE* bytes, int x, int y, int stride, int width, int height);
void RemoveBlackColor(struct FIBITMAP* img);
void BinarizeImage(struct FIBITMAP* Img);
void RemoveSmallDots(struct FIBITMAP* img);
double GetLineLen(double sx, double sy, double ex, double ey);
double abs(double x);