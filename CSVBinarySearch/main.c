#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	unsigned char *FileContent;
	int FileSize;
	int RowCount;
	unsigned char **RowStarts;
	unsigned int *KeyLen;
}CSVFileStore;

#define ColumnSeparatorChar ' '

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

#define ALPHABETICAL_SMALLER -1
#define ALPHABETICAL_EQUAL 0
#define ALPHABETICAL_LARGER 1
#define ALPHABETICAL_ERROR -2

int IsStringAlphabeticallySmaller(const unsigned char *SearchingForKey, const int size1, const unsigned char *RowStart, const int size2, const unsigned char ColumnDelimiter)
{
	if (SearchingForKey == NULL || RowStart == NULL)
		return ALPHABETICAL_ERROR;
	if (size1 > size2)
		return ALPHABETICAL_LARGER;
	if(size1 < size2)
		return ALPHABETICAL_SMALLER;
	return memcmp(SearchingForKey, RowStart, size1);
}

typedef struct
{
	unsigned char *RowStartLoc;
	void *NextNode;
}RowStartNode;

CSVFileStore *InitCSVSeeker(const char *FileName)
{
	CSVFileStore *ret = (CSVFileStore*)malloc(sizeof(CSVFileStore));
	//out of memory
	if (ret == NULL)
		return NULL;
	//init the whole structure with default values
	memset(ret, 0, sizeof(CSVFileStore));
	//check how much memory we need to store this
	ret->FileSize = GetFileSize(FileName);
	if (ret->FileSize == 0)
		return ret;
	ret->FileContent = (unsigned char *)malloc(ret->FileSize);
	//already checked that the file is valid
	FILE *p_file = NULL;
	p_file = fopen(FileName, "rb");
	//cache the whole content of the file
	fread(ret->FileContent, 1, ret->FileSize, p_file);
	fclose(p_file);
	//generate line starts
	RowStartNode *RowStartList = NULL;
	RowStartNode *LastNode = NULL;
	unsigned char *RowStartedAt = ret->FileContent;
	int ParserIndex = 0;
	for (; ParserIndex < ret->FileSize; ParserIndex++)
	{
		//did we finish reading a line ?
		if (ret->FileContent[ParserIndex] == '\n')
		{
			ret->FileContent[ParserIndex] = 0; //make it so we can print it out later
			ret->RowCount++;
			RowStartNode *NewNode = (RowStartNode *)malloc(sizeof(RowStartNode));
			memset(NewNode, 0, sizeof(RowStartNode));
			if (RowStartList == NULL)
				RowStartList = NewNode;
			if (LastNode != NULL)
				LastNode->NextNode = NewNode;
			LastNode = NewNode;
			NewNode->RowStartLoc = RowStartedAt;
			RowStartedAt = &ret->FileContent[ParserIndex + 1]; // memorize at what index we found a new row
		}
	}
	//generate an array where we can use Binary search to lookup a row 
	ret->RowStarts = malloc(ret->RowCount * sizeof(unsigned char *));
	ret->KeyLen = malloc(ret->RowCount * sizeof(unsigned int));
	int RowIndex = 0;
	for (; RowIndex < ret->RowCount; RowIndex++)
	{
		ret->RowStarts[RowIndex] = RowStartList->RowStartLoc;
		LastNode = RowStartList;
		RowStartList = RowStartList->NextNode;
		free(LastNode);
		//check key len
		ret->KeyLen[RowIndex] = 0;
		while (ret->RowStarts[RowIndex][ret->KeyLen[RowIndex]] != 0	&& ret->RowStarts[RowIndex][ret->KeyLen[RowIndex]] != ColumnSeparatorChar)
			ret->KeyLen[RowIndex]++;
	}
	return ret;
}

void PrintCSVLine(const unsigned char *LineStart)
{
	printf("%s\n", LineStart);
}

void PrintLinesWithKey(CSVFileStore *CSV, const unsigned char *Key, const unsigned char ColumnDelimiter)
{
	if (CSV == NULL)
		return;
	if (Key == NULL)
		return;
	if (CSV->RowCount <= 0)
		return;
	int CompareRes = ALPHABETICAL_ERROR;
	//perform a binary search to find a line that matches search key
	int SearchLength = CSV->RowCount / 2;
	int CheckPosition = CSV->RowCount / 2;
	int KeyLen = strlen(Key);
	while (SearchLength > 0)
	{
		CompareRes = IsStringAlphabeticallySmaller(CSV->RowStarts[CheckPosition], CSV->KeyLen[CheckPosition], Key, KeyLen, ColumnDelimiter);
		if (CompareRes == ALPHABETICAL_EQUAL)
			break;
		//something went wrong. Time to bail out
		if (CompareRes == ALPHABETICAL_ERROR)
			return;
		SearchLength = (SearchLength + 1) / 2;
		if (CompareRes == ALPHABETICAL_LARGER)
		{
			CheckPosition -= SearchLength;
			if (CheckPosition < 0)
			{
				CheckPosition = 0;
				SearchLength = 0;
			}
		}
		else
		{
			CheckPosition += SearchLength;
			if (CheckPosition >= CSV->RowCount)
			{
				CheckPosition = CSV->RowCount - 1;
				SearchLength = 0;
			}
		}
	}
	if (CompareRes == ALPHABETICAL_EQUAL)
	{
		int PrintPositionStart = CheckPosition;
		//seek to the first row ( in case there are multiple ) that has the matching key
		while (CompareRes == ALPHABETICAL_EQUAL && PrintPositionStart > 1)
		{
			PrintPositionStart--;
			CompareRes = IsStringAlphabeticallySmaller(CSV->RowStarts[PrintPositionStart], CSV->KeyLen[PrintPositionStart], Key, KeyLen, ColumnDelimiter);

		}
		//jump back to first matching row
		if (CompareRes != ALPHABETICAL_EQUAL)
			PrintPositionStart++;
		int PrintPositionEnd = CheckPosition;
		CompareRes = ALPHABETICAL_EQUAL;
		while (CompareRes == ALPHABETICAL_EQUAL && PrintPositionEnd < CSV->RowCount)
		{
			PrintPositionEnd++;
			CompareRes = IsStringAlphabeticallySmaller(CSV->RowStarts[PrintPositionEnd], CSV->KeyLen[PrintPositionEnd], Key, KeyLen, ColumnDelimiter);

		}
		if (CompareRes != ALPHABETICAL_EQUAL)
			PrintPositionEnd--;
		//start printing as long as we can
		while (PrintPositionStart <= PrintPositionEnd)
		{
			PrintCSVLine(CSV->RowStarts[PrintPositionStart]);
			PrintPositionStart++;
		}
	}
}

void main(int argc, char **argv)
{
	if (argc < 4)
	{
		printf("Usage : bsearch --key <columnvalue> <InputFileName>\n");
		printf("Expected colum delimited is '%c' character\n", ColumnSeparatorChar);
		return;
	}
	if (strcmp(argv[1], "--key") != 0)
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
	CSVFileStore *csv = InitCSVSeeker(argv[3]);
	PrintLinesWithKey(csv, argv[2], ColumnSeparatorChar);
	//	getch();
}