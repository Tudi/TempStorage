#pragma once

// every BIN file starts with this 
void WriteBinHeader(FILE* f, RobotDrawSession* robotSession);
// every BIN file ends with this
void WriteBinFooter(FILE* f, RobotDrawSession* robotSession);
// pause the reading of the file
// wait for the operator to reposition the pen
// wait for the operator to resume reading the file
void WriteBinTransition(FILE* f, RobotDrawSession* robotSession, int writePaperSwap);

struct RelativePointsLine;
int WriteBinLine(FILE* f, RelativePointsLine* line, RobotDrawSession* robotSession);
