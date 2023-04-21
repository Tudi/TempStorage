#include "stdafx.h"

int main(int argc, char **argv)
{
	SAFFile SAFReader;

	if (argc > 1)
	{
		printf("Reading file content : %s\n", argv[1]);
		SAFReader.ReadFile(argv[1]);
	}
	else
	{
//		SAFReader.ReadFile("1.saf");
		//SAFReader.ReadFile("0006 Three Vertical Two Inch Lines From -1 to 1 Two Inches Apart.saf");
		//SAFReader.ReadFile("0008 Vertical Half Inch Line followed by Horizontal Half Inch Line without Transition.saf");
		//SAFReader.ReadFile("0007 Vertical Half Inch Line followed by Horizontal Half Inch Line with Transition.saf");
		//SAFReader.ReadFile("0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.saf");
		SAFReader.ReadFile("0018 Names From Excel in Block.saf");
//		SAFReader.ReadFile("two lines with transition.saf");
	}

	SAFReader.PrintContent();
}