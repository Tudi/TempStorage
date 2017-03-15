#pragma once

void ParseOfflineDump(const char *FileName);
void ProcessPacket1(unsigned char *packet, int size);

//extern void(*PacketParserPointer)(unsigned char *packet, int size);