#include "tests.h"


void RunAllTests(char** argv, int argc)
{
//	Test_DrawReadLine();
//	Test_DrawClock();
	Test_SigToBin(argv, argc);
	Test_LoadBinFile(argv, argc);
	// basically me testing stuff
	if (argc == 1)
	{
		Test_DrawUnitsOfMeasurement();
	}
}