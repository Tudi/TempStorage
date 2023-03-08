#pragma once

void WriteBinHeader(uint8_t* f, uint32_t& readPos, RobotDrawSession* robotSession);
void WriteBinFooter(uint8_t* f, uint32_t& readPos, RobotDrawSession* robotSession);

struct RelativePointsLine;
int WriteBinLine(uint8_t* f, uint32_t& readPos, size_t fileSize, RelativePointsLine** line, RobotDrawSession* robotSession);
