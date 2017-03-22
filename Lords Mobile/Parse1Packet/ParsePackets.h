#pragma once

void ParseOfflineDump(const char *FileName);
void ProcessPacket1(unsigned char *packet, int size);
void ProcessPacketDebug(char *HexStr);

#define MAX_PACKET_CIRCULAR_BUFFER	10000
void	CreateBackgroundPacketProcessThread();
void	QueuePacketToProcess(unsigned char *data, int size);
void	StopThreadedPacketParser();