#include "stdafx.h"

void LogMessage(const char* file, int line, const char* msg)
{
	char remsg[5000];
	sprintf_s(remsg, sizeof(remsg), "%s:%d:%s\n", __FILE__, __LINE__, msg);
	printf(remsg);
}

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		printf("Reading file content to verify header generation: %s\n", argv[1]);
		SAFFile SAFReader;
		SAFReader.ReadFile(argv[1]);
		SAFReader.PrintContent();

		SAFFile SAFReader2;
		SAFReader2.ReadFile(argv[1]);
		SAFReader2.UpdateFileInfo();
		SAFReader2.IsEqual(&SAFReader);

	}
	else if (argc == 3 && strcmp(argv[1], "-s2i") == 0)
	{
		SAFFile SAFReader;
		SAFReader.ReadFile(argv[2]);
		SAFReader.PrintContent();
	}
	else if (argc == 3 && strcmp(argv[1], "-s2s") == 0)
	{
		ReadSigFile(argv[2]);
	}
	else
	{
		printf("Execute debug code\n");
//		TestGenClockSAF(); return 0;
		TestWriterProperCalc();
	}
	return 0;
}