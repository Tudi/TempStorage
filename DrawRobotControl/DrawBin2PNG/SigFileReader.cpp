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
		if (prevX != INVALID_VALUE && prevY != INVALID_VALUE)
		{
			bfw.AddLine(prevX * PIXELS_IN_INCH, prevY * PIXELS_IN_INCH, curX * PIXELS_IN_INCH, curY * PIXELS_IN_INCH);
		}
		prevX = curX;
		prevY = curY;
	};
	fclose(f);
	bfw.CloseFile();
}