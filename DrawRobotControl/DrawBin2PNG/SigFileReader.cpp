#include "StdAfx.h"

#define ALLOC_CHUNK_SIZE 64

int readLine(FILE* f, char** outBuff, int* out_bytes)
{
	int allocated = ALLOC_CHUNK_SIZE;
	*outBuff = (char*)malloc(allocated);
	if (*outBuff == NULL)
	{
		return 1;
	}
	*out_bytes = 0;
	while (!feof(f))
	{
		char nextByte = 0;
		fread(&nextByte, 1, 1, f);
		if (nextByte == '\r')
		{
			continue;
		}
		if (*out_bytes + 1 >= allocated)
		{
			allocated += ALLOC_CHUNK_SIZE;
			char* newAlloc = (char*)realloc(*outBuff, allocated);
			if (newAlloc == NULL)
			{
				return 1;
			}
			*outBuff = newAlloc;
		}
		(*outBuff)[*out_bytes] = nextByte;
		*out_bytes += 1;
		if (nextByte == '\n')
		{
			(*outBuff)[*out_bytes - 1] = 0;
			break;
		}
	}
	if (*out_bytes > 0)
	{
		(*outBuff)[*out_bytes - 1] = 0;
	}
	return 0;
}

#define INVALID_VALUE (float)9E4

void ReadSigFile(const char* fileName)
{
	printf("Trying to convert %s to BIN file\n", fileName);

	FILE* f = NULL;
	errno_t openerr = fopen_s(&f, fileName, "rt");
	if (f == NULL)
	{
		SOFT_ASSERT(false, "Failed to open input SIG file");
		return;
	}

	char *sBinFileName = _strdup(fileName);
	if (sBinFileName == NULL)
	{
		SOFT_ASSERT(false, "Failed to copy input SIG file name");
		return;
	}
	size_t uiNameLen = strlen(sBinFileName);
	sBinFileName[uiNameLen - 3] = 'b';
	sBinFileName[uiNameLen - 2] = 'i';
	sBinFileName[uiNameLen - 1] = 'n';
	BinFileWriter bfw(sBinFileName);
	free(sBinFileName);

	//read until the end of file
	float prevX = INVALID_VALUE, prevY = INVALID_VALUE;
	while (!feof(f))
	{
		//read next line
		char* line;
		int lineLen;
		readLine(f, &line, &lineLen);
		float curX = INVALID_VALUE, curY = INVALID_VALUE;
		if (strcmp(line, "PLINESTART") == 0)
		{
			prevX = INVALID_VALUE;
			prevY = INVALID_VALUE;
		}
		else if (strcmp(line, "PLINEEND") == 0)
		{
			prevX = INVALID_VALUE;
			prevY = INVALID_VALUE;
		}
		else if (strcmp(line, "Setting") == 0)
		{
			free(line);
			break;
		}
		else
		{
			sscanf_s(line, "%f,%f", &curX, &curY);
		}
		free(line);
		if (prevX != INVALID_VALUE && prevY != INVALID_VALUE && curX != INVALID_VALUE && curY != INVALID_VALUE)
		{
//#define apply_specific_correction curX = curX - (3.6f + 1.1334f) / 2; curY = curY - (9.81374f + 9.79107f) / 2; curX *= 1.3f;curY *= 1.3f;
//#define apply_specific_correction curX = curX - (3.6f + 1.1334f) / 2; curY = curY - (9.81374f + 9.79107f) / 2; // for Dennis name
//#define apply_specific_correction curX = curX - (5.89f + 1.14f) / 2; curY = curY - (9.86f + 8.227f) / 2; // for paragraph
//#define apply_specific_correction curX = curX - (5.89f + 1.14f) / 2; curY = curY - (9.86f + 8.227f) / 2; curX *= 0.7f;curY *= 0.7f; // for paragraph
//#define apply_specific_correction curX = curX - (3.841f + 0.24894f) / 2; // for BigWheel.sig
//#define apply_specific_correction curX = curX - (3.841f + 0.24894f) / 2; curX *= 0.7f;curY *= 0.7f; // for BigWheel.sig
#define apply_specific_correction ;
			apply_specific_correction
//			bfw.AddLine(prevX * PIXELS_IN_INCH, prevY * PIXELS_IN_INCH, curX * PIXELS_IN_INCH, curY * PIXELS_IN_INCH);
			bfw.AddLineAntiDistorted(prevX * PIXELS_IN_INCH, prevY * PIXELS_IN_INCH, curX * PIXELS_IN_INCH, curY * PIXELS_IN_INCH);
		}
		else if (curX != INVALID_VALUE && curY != INVALID_VALUE)
		{
			apply_specific_correction
		}
		prevX = curX;
		prevY = curY;
	};
	fclose(f);
	bfw.CloseFile();
}