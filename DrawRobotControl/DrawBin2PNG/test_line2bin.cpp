#include "StdAfx.h"

// create a line, draw it in a bin file
// save the bin file
// load the bin file
// read the line
// compare written line to read line
void TestDrawReadLineSpecific(float sx, float sy, float ex, float ey)
{
	RobotDrawSession robotSession;
	robotSession.curx = 1000;
	robotSession.cury = 1000;

	RelativePointsLine lineWritten;
	RelativePointsLine lineRead;
	lineWritten.setStartingPosition(robotSession.curx, robotSession.cury);
	DrawLineRelativeInMem(sx, sy, ex, ey, &lineWritten);

	{
		FILE* f = NULL;
		errno_t openerr = fopen_s(&f, "temp.bin", "wb");
		if (f == NULL)
		{
			return;
		}

		WriteBinHeader(f, &robotSession);
		WriteBinLine(f, &lineWritten, &robotSession);
		WriteBinFooter(f, &robotSession);

		fclose(f);
	}

	// reset robot after write
	robotSession.curx = 1000;
	robotSession.cury = 1000;

	// now read it
	{
		uint32_t readPos = 0;
		size_t fileSize = 0;
		uint8_t* f = OpenBinFile("temp.bin", readPos, fileSize);
		ReadBinHeader(f, readPos, &robotSession);
		lineRead.setStartingPosition(robotSession.curx, robotSession.cury);
		int ret = ReadBinLine(f, readPos, fileSize, &lineRead, &robotSession);
		ReadBinFooter(f, readPos, &robotSession);

		// no longer need the file
		FreeAndNULL(f);
	}

	// compare the 2 lines
	SOFT_ASSERT(lineWritten.GetPointsCount() == lineRead.GetPointsCount(), "written and read line move count not the same");
	for (size_t i = 0; i < lineWritten.GetPointsCount(); i++)
	{
		SOFT_ASSERT((int)lineWritten.GetDX(i) == (int)lineRead.GetDX(i), "Line move mismatch");
		SOFT_ASSERT((int)lineWritten.GetDY(i) == (int)lineRead.GetDY(i), "Line move mismatch");
	}
}

void Test_DrawReadLine()
{
	// horizontal - to right
	TestDrawReadLineSpecific(100, 100, 200, 100);
	// vertical - from up to down
	TestDrawReadLineSpecific(100, 100, 100, -100);
	// from right to left
	TestDrawReadLineSpecific(100, 100, -100, 100);
	// from down to up
	TestDrawReadLineSpecific(100, 100, 100, 200);
	//diagonal - to right, up
	TestDrawReadLineSpecific(100, 100, 200, 200);
	//diagonal - to right, down
	TestDrawReadLineSpecific(100, 100, 200, -200);
	//diagonal - to left, up
	TestDrawReadLineSpecific(100, 100, -200, 200);
	//diagonal - to left, down
	TestDrawReadLineSpecific(100, 100, -200, -200);
}
