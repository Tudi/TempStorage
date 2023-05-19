#pragma once

#include <string>

class BinFileWriter
{
public:
	BinFileWriter(const char *fileName);
	void AddLine(double sx, double sy, double ex, double ey);
	void AddLineAntiDistorted(double sx, double sy, double ex, double ey);
	void CloseFile();
	void SetDrawSpeedPCT(double pct) { robotSession.moveSpeedCoeff = 100 - pct / 100.0; }
private:
	void OpenBinFile();
	bool bHeaderWritten;
	bool bFooterWritten;
	std::string sFileName;
	RobotDrawSession robotSession;
	FILE* fBinFile;
};

// every BIN file starts with this 
void WriteBinHeader(FILE* f, RobotDrawSession* robotSession);
// every BIN file ends with this
void WriteBinFooter(FILE* f, RobotDrawSession* robotSession);
// pause the reading of the file
// wait for the operator to reposition the pen
// wait for the operator to resume reading the file
void WriteBinTransition(FILE* f, RobotDrawSession* robotSession, int writePaperSwap);

class RelativePointsLine;
int WriteBinLine(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession);
void MovePenToLineStart_DrawLineInFile(FILE* f, RobotDrawSession* robotSession, float sx, float sy, float ex, float ey);