#pragma once

typedef unsigned char BYTE;

typedef struct CircleStore
{
	short cx, cy, r; // r=1 means single pixel. Can't cover others. r=2 is actually 1 pixel neighbour coverage
	BYTE isCovering; // when radius equals
}CircleStore;

template<int rad>
int CircleIsCoveredHandMade(struct FIBITMAP* img, int cx, int cy, int circle_points[rad * 2 + 1][rad * 2 + 1]);

int CircleIsCovered(struct FIBITMAP* img, int cx, int cy, int r);

void ProcessInputUsingWeights(struct FIBITMAP* img);