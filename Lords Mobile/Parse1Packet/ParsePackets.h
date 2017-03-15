#pragma once

void ParseOfflineDump(const char *FileName);
void ProcessPacket1(unsigned char *packet, int size);

//extern void(*PacketParserPointer)(unsigned char *packet, int size);

#define MAX_PACKET_CIRCULAR_BUFFER	1000
//extern unsigned char	*PacketCircularBuffer[MAX_PACKET_CIRCULAR_BUFFER];
//extern int				PacketCircularBufferReadIndex;
//extern int				PacketCircularBufferWriteIndex;
//extern void*			PacketProcessThreadHandle;
//extern int				KeepThreadsRunning;
void	CreateBackgroundPacketProcessThread();
void	QueuePacketToProcess(unsigned char *data, int size);
void	StopThreadedPacketParser();