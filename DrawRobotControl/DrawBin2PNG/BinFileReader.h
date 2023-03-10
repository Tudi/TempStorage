#pragma once

void RobotCommand_Constructor(RobotCommand *comm, BYTE fromByte);
void RobotSession_Constructor(RobotDrawSession *robotSession);
uint8_t* OpenBinFile(const char* name, uint32_t& readPos, size_t& fileSize);
void ReadBinHeader(uint8_t* f, uint32_t& readPos, RobotDrawSession *robotSession);
void ReadBinFooter(uint8_t* f, uint32_t& readPos, RobotDrawSession *robotSession);

struct RelativePointsLine;
int ReadBinLine(uint8_t* f, uint32_t& readPos, size_t fileSize, RelativePointsLine** line, RobotDrawSession *robotSession);
const char* GetDirectionString(PenRobotMovementCodesPrimary movementCode);
const char* GetDirectionStringRelative(PenRobotMovementCodesPrimary movementCode, PenRobotMovementCodesRelative relative);