#include "stdafx.h"

void TestWriterProperCalc()
{
	//	const char* fileName = "1.saf";
	//	const char* fileName = "0018 Names From Excel in Block.saf";
	//	const char* fileName = "0006 Three Vertical Two Inch Lines From -1 to 1 Two Inches Apart.saf";
	const char* fileName = "0008 Vertical Half Inch Line followed by Horizontal Half Inch Line without Transition.saf";
	//	const char* fileName = "0007 Vertical Half Inch Line followed by Horizontal Half Inch Line with Transition.saf";
	//	const char* fileName = "0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.saf";
	//	const char* fileName = "two lines with transition.saf";
	SAFFile SAFReader;
	SAFReader.ReadFile(fileName);
	SAFReader.PrintContent();

	//check to see if we could properly reproduce values that we just read
	SAFFile SAFReader2;
	SAFReader2.ReadFile(fileName);
	SAFReader2.UpdateFileInfo();
	//automated equality check
	SAFReader2.IsEqual(&SAFReader);

#if DO_FILE_INFO_POINT_TEST
	for (int i = 0; i < _countof(SAFReader2.fileInfo.points); i++)
	{
		SAFReader2.fileInfo.points[i].x = 254;
		SAFReader2.fileInfo.points[i].y = 254;
	}
#endif
#if DO_LINE_LEN_SCALE_TEST
	(*(*(*SAFReader2.sections.begin())->lines.begin())->points.begin())->x -= 8 * SAF_INCH_MULTIPLIER;
	(*(*(*SAFReader2.sections.begin())->lines.begin())->points.begin())->y -= 8 * SAF_INCH_MULTIPLIER;
#endif
#if TEST_IF_ALWAYSEMPTY_FIELD_IS_DESCRIPTION
	strcpy_s(SAFReader2.fileInfo.alwaysemptystring, sizeof(SAFReader2.fileInfo.alwaysemptystring), "Dennis");
#endif

	SAFReader2.WriteFile("t.saf");
}

void DrawClockAnyRadius(double R_inches, double lineCount)
{
	char fileName[500];
	sprintf_s(fileName, sizeof(fileName), "clock_%.02f_%d.saf", R_inches, (int)lineCount);

	SAFFile safFile;

	double angleIncrement = 360 / lineCount;
	double originX = 0;
	double originY = 0;
	// angle speed should depend on radius, but we do not have time for that now
	for (double angle = 0; angle <= 360; angle += angleIncrement)
	{
		double radians = 3.14 / 180.0 * angle;
		int ex = (int)(originX + (double)R_inches * SAF_INCH_MULTIPLIER * cos(radians));
		int ey = (int)(originY + (double)R_inches * SAF_INCH_MULTIPLIER * sin(radians));
		//		bfw.AddLine(0, 0, (float)ex, (float)ey);
		safFile.AddNewLine(0, 0);
		safFile.AppendToLine((float)ex, (float)ey);
	}

	safFile.SetDisplayName(fileName);
	safFile.WriteFile(fileName);
}

void TestGenClockSAF()
{
	DrawClockAnyRadius(1, 10);
	DrawClockAnyRadius(2, 20);
	DrawClockAnyRadius(2.5, 60);
	DrawClockAnyRadius(3, 30);
}