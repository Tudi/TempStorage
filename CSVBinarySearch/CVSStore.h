#pragma once

typedef struct
{
	unsigned char *FileContent;
	int FileSize;
	int RowCount;
	unsigned char **RowStarts;
}CSVFileStore;

CSVFileStore *InitCVSSeeker(const char *FileName);
void PrintLinesWithKey(CSVFileStore *CVS, const unsigned char *Key, const unsigned char ColumnDelimiter);