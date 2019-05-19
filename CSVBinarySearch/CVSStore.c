#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CVSStore.h"

#define HasSizeBytes 4

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

int IsStringAlphabeticallySmaller(const unsigned char *SearchingForKey, const unsigned char *RowStart, const unsigned char ColumnDelimiter)
{
	if (SearchingForKey == NULL || RowStart == NULL)
		return ALPHABETICAL_ERROR;
	//searching for empty string. Column key length is 0. Not sure how to compare these
	if(SearchingForKey[0] == 0 || RowStart[0] == 0)
		return ALPHABETICAL_ERROR;
	while (SearchingForKey[0] == RowStart[0] && SearchingForKey[0] != '\0' && RowStart[0] != ColumnDelimiter)
	{
		SearchingForKey++;
		RowStart++;
	}
	if(SearchingForKey[0] == '\0' && RowStart[0] == ColumnDelimiter)
		return ALPHABETICAL_EQUAL;
	int ComparisonResult = (int)SearchingForKey[0] - (int)RowStart[0];
	if (ComparisonResult == 0)
		return ALPHABETICAL_EQUAL;
	else if (ComparisonResult < 0)
		return ALPHABETICAL_LARGER;
	return ALPHABETICAL_SMALLER;
}

typedef struct
{
	unsigned char *RowStartLoc;
	void *NextNode;
}RowStartNode;

CSVFileStore *InitCVSSeeker(const char *FileName)
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
	for (int ParserIndex = 0; ParserIndex < ret->FileSize; ParserIndex++)
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
	ret->RowStarts = malloc(ret->RowCount * sizeof( unsigned char *));
	for (int RowIndex = 0; RowIndex < ret->RowCount; RowIndex++)
	{
		ret->RowStarts[RowIndex] = RowStartList->RowStartLoc;
		LastNode = RowStartList;
		RowStartList = RowStartList->NextNode;
		free(LastNode);
	}
	return ret;
}

void PrintCVSLine(const unsigned char *LineStart)
{
	printf("%s\n", LineStart);
}

void PrintLinesWithKey(CSVFileStore *CVS, const unsigned char *Key, const unsigned char ColumnDelimiter)
{
	if (CVS == NULL)
		return;
	if (Key == NULL)
		return;
	if (CVS->RowCount <= 0)
		return;
	int CompareRes = ALPHABETICAL_ERROR;
	//perform a binary search to find a line that matches search key
	int SearchLength = CVS->RowCount / 2;
	int CheckPosition = CVS->RowCount / 2;
	while (SearchLength > 0)
	{
		CompareRes = IsStringAlphabeticallySmaller(Key, CVS->RowStarts[CheckPosition], ColumnDelimiter);
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
			if (CheckPosition >= CVS->RowCount)
			{
				CheckPosition = CVS->RowCount-1;
				SearchLength = 0;
			}
		}
	}
	if (CompareRes == ALPHABETICAL_EQUAL)
	{
		//seek to the first row ( in case there are multiple ) that has the matching key
		while (CompareRes == ALPHABETICAL_EQUAL && CheckPosition > 1)
		{
			CheckPosition--;
			CompareRes = IsStringAlphabeticallySmaller(Key, CVS->RowStarts[CheckPosition], ColumnDelimiter);
		}
		//jump back to first matching row
		if (CompareRes != ALPHABETICAL_EQUAL)
		{
			CheckPosition++;
			CompareRes = ALPHABETICAL_EQUAL;
		}
		//start printing as long as we can
		while (CompareRes == ALPHABETICAL_EQUAL && CheckPosition < CVS->RowCount)
		{
			PrintCVSLine(CVS->RowStarts[CheckPosition]);
			CheckPosition++;
			if (CheckPosition < CVS->RowCount)
				CompareRes = IsStringAlphabeticallySmaller(Key, CVS->RowStarts[CheckPosition], ColumnDelimiter);
			else
				break;
		}
	}
}