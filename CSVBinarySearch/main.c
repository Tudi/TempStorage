#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "CVSStore.h"

void main(int argc, char **argv)
{
	if (argc < 4)
	{
		printf("Usage : bsearch --key <columnvalue> <InputFileName>\n");
		return;
	}
	if (strcmp(argv[1], "--key") != 0 )
	{
		printf("First param should be --key\n");
		return;
	}
	if (strlen(argv[2]) == 0)
	{
		printf("Please provide a valid string to use as search key\n");
		return;
	}
	FILE *file;
	if (file = fopen(argv[3], "r")) 
		fclose(file);
	else
	{
		printf("Could not open file %s for reading\n", argv[3]);
		return;
	}
	printf("Expected colum delimited is ' ' ( space ) character\n");
	CSVFileStore *csv = InitCVSSeeker(argv[3]);
	PrintLinesWithKey(csv, argv[2], ' ');
//	getch();
}