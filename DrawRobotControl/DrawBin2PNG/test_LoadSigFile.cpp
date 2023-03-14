#include "StdAfx.h"

void Test_SigToBin(char** argv, int argc)
{
	if (argc < 3)
	{
		printf("expecting array of input SIG file names\n");
		return;
	}
	if (strcmp(argv[1], "-s2b") != 0)
	{
		return;
	}
	for (size_t i = 2; i < argc; i++)
	{
		ReadSigFile(argv[i]);
	}
}