#include <stdio.h>
#include "PECategorization.h"

int main(int argc, char* argv[])
{
	// sanity check
	if (argc < 2)
	{
		printf("Usage : %s <file 1> ... <file N>", argv[0]);
		printf("Description : Categorize a PE x32 file if sections are compressed or not\n");
		return 1;
	}

	printf("Filename Category\n");
	printf("-------- --------\n");

	// process files 1 by 1
	for (size_t i = 1; i < argc; i++)
	{
		CategorizeFile(argv[i]);
	}

	// all good if we got here
	return 0;
}