#include <string.h>
#include "PacketContentGenerator.h"
#include "ParseServerToClient.h"

int OnPacketForClickCastle(unsigned char *packet, unsigned int len)
{
#define CastleClickPacketBytesSize 11
	//if we receive the same packet we sent out, do not parse it again
	static char PrevPacketSent[CastleClickPacketBytesSize];
	if (memcmp(PrevPacketSent, packet, CastleClickPacketBytesSize) == 0)
		return 0;

	int x, y;
	if (GeteneratePosToScan(x, y) != 0)
		return 1; // do not change the packet
	// set len to new size
	packet[0] = 11;
	//opcode
	packet[2] = 11;
	packet[3] = 11;
	//unk
	packet[4] = 0x2D;
	// write coordinates
	unsigned int GUID = GenerateIngameGUID(x, y);
	//failed to generate GUID from x, y
	if (GUID == 0)
		return 0;
	*(unsigned int*)&packet[5] = GUID;

	memcpy(PrevPacketSent, packet, CastleClickPacketBytesSize);

	return 0; // overrided content
}

int OnClientToServerPacket(unsigned char *packet, unsigned int len)
{
	//is this "delete opened gifts" packet
	if (len == 12 && packet[0] == 12 && packet[1] == 0 && packet[2] == 0xff && packet[3] == 0xff)
	{
		int ret = OnPacketForClickCastle(packet, len);
		if (ret == 0)
			return 0;
	}
	//on chat packet that is large enough
	if (len == 11 && packet[0] == 11 && packet[1] == 0 && packet[2] == 0xfE && packet[3] == 0xfE)
	{
		int ret = OnPacketForClickCastle(packet, len);
		if (ret == 0)
			return 0;
	}
	//we did not change the packet
	return 1;
}