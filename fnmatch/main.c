#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>

int GetFileSize(const char *FileName) // path to file
{
	FILE *p_file = NULL;
	p_file = fopen(FileName, "rb");
	if (p_file == NULL)
		return 0;
	fseek(p_file, 0, SEEK_END);
	int size = ftell(p_file);
	fclose(p_file);
	return size;
}

void ReadOneLine(FILE *f, unsigned char *OutputBuff)
{
	while (!feof(f))
	{
		size_t BytesReadNow = fread(OutputBuff, 1, 1, f);
		if (BytesReadNow == 0)
			break;
		if (OutputBuff[0] == '\n')
			break;
		OutputBuff++;
	}
	OutputBuff[0] = '\0';
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		printf("Usage : fnmatch --pattern <pattern> <InputFileName>\n");
		printf("Example : fnmatch --pattern 'test*' input.txt\n");
		return;
	}
	if (strcmp(argv[1], "--pattern") != 0)
	{
		printf("First param should be --pattern\n");
		return;
	}
	if (strlen(argv[2]) == 0)
	{
		printf("Please provide a valid string to use as search pattern\n");
		return;
	}
	int FileSize = GetFileSize(argv[3]);
	if(FileSize <= 0)
	{
		printf("Could not open file %s for reading\n", argv[3]);
		return;
	}
	unsigned char *LineBuffer = malloc(FileSize);
	if (LineBuffer == NULL)
	{
		printf("Unable to allocate memory\n");
		return -1;
	}
	FILE *InputFile = fopen(argv[3], "rt");
	do {
		ReadOneLine(InputFile, LineBuffer);
		int res = fnmatch(argv[2], LineBuffer, 0);
		if (res == 0)
			printf("%s\n", LineBuffer);
	} while (!feof(InputFile));
	return 0;
}
