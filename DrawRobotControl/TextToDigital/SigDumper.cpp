#include "StdAfx.h"

void dumpLinesToSig(const char* SigName, struct LineStore* LineStore, int lineCount)
{
	FILE* f;
	errno_t openres = fopen_s(&f, SigName, "wt");
	if (f == NULL)
	{
		printf("Failed to open file %s\n", SigName);
		return;
	}

	for (int i = 0; i < lineCount; i++)
	{
		if (LineStore[i].len == 0)
		{
			continue;
		}
		fprintf(f,"PLINESTART\n");
		fprintf(f, "%d,%d\n", LineStore[i].sx, LineStore[i].sy);
		fprintf(f, "%d,%d\n", LineStore[i].ex, LineStore[i].ey);
		fprintf(f, "PLINEEND\n");
	}

	fprintf(f,"Setting\nSetting\n0, 0, 0, 11\n34082, 0, 0, 0, 1\n\n\n");

	fclose(f);
}