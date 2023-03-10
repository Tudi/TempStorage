#include "StdAfx.h"

// create a line, draw it in a bin file
// save the bin file
// load the bin file
// read the line
// compare written line to read line
void TestDrawReadLineSpecific(float sx, float sy, float ex, float ey)
{
	RobotDrawSession robotSession;
	robotSession.curx = 100;
	robotSession.cury = 100;

	RelativePointsLine* lineWritten = NULL;
	RelativePointsLine* lineRead = NULL;
	RelativePointsLine::setStartingPosition(&lineWritten, robotSession.curx, robotSession.cury);
	DrawLineRelativeInMem(sx, sy, ex, ey, &lineWritten);

	{
		FILE* f = NULL;
		errno_t openerr = fopen_s(&f, "temp.bin", "wb");
		if (f == NULL)
		{
			return;
		}

		WriteBinHeader(f, &robotSession);
		WriteBinLine(f, lineWritten, &robotSession);
		WriteBinFooter(f, &robotSession);

		fclose(f);
	}

	// reset robot after write
	robotSession.curx = 100;
	robotSession.cury = 100;

	// now read it
	{
		uint32_t readPos = 0;
		size_t fileSize = 0;
		uint8_t* f = OpenBinFile("temp.bin", readPos, fileSize);
		ReadBinHeader(f, readPos, &robotSession);
		RelativePointsLine::setStartingPosition(&lineRead, robotSession.curx, robotSession.cury);
		int ret = ReadBinLine(f, readPos, fileSize, &lineRead, &robotSession);
		ReadBinFooter(f, readPos, &robotSession);

		// no longer need the file
		FreeAndNULL(f);
	}

	// compare the 2 lines
	SOFT_ASSERT(lineWritten->numberOfPoints == lineRead->numberOfPoints, "written and read line move count not the same");
	for (size_t i = 0; i < lineWritten->numberOfPoints; i++)
	{
		SOFT_ASSERT((int)lineWritten->moves[i].dx == (int)lineRead->moves[i].dx, "Line move mismatch");
		SOFT_ASSERT((int)lineWritten->moves[i].dy == (int)lineRead->moves[i].dy, "Line move mismatch");
	}

	// cleanup
	FreeAndNULL(lineWritten); 
	FreeAndNULL(lineRead);
}

void Test_DrawReadLine()
{
	// horizontal - right
	TestDrawReadLineSpecific(100, 100, 200, 100);
	// vertical - up
	TestDrawReadLineSpecific(100, 100, 100, 200);
	//diagonal
	TestDrawReadLineSpecific(100, 100, 200, 200);
	// left
	TestDrawReadLineSpecific(100, 100, -100, 100);
	// down
	TestDrawReadLineSpecific(100, 100, 100, -100);
}
