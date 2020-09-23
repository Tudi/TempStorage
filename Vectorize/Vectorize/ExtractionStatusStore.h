#pragma once

enum ExtractedMarkValues
{
	EXTRACTED_NONE = 0,
	EXTRACTED_SQAURE = 1,
	EXTRACTED_VERT_LINE = 2,
	EXTRACTED_HOR_LINE = 3,
};

struct FIBITMAP;
void InitStatusStore(FIBITMAP* dib);
void CleanupStatusStore();
BYTE* GetExtractedMap();