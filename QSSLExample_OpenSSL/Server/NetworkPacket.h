#pragma once
#include <QSslSocket.h>

#pragma pack(push, 1)
struct PacketHeader
{
	int		SendSize;	// leave it on the first location. Always
	double	SendStamp;	// we want to check the roundtrip time of a packet
	int		SendCount;	// just because we can. Count the roundtrips of this packet
	int		PacketType; // If we would be using mutliple packet types, we could use this as an ID
	int		SendCountType;	// Count the roundtrips of this packet type
	double	TypeStamp;	// this specific size was first sent at ..
	__int64 SentTotal;	// number of bytes we sent so far
	char	Data[8];	// bloating it for the sake of size rounding of initial packet
};
#pragma pack(pop, 1)

class FragmentedNetworkPacket
{
public:
	FragmentedNetworkPacket();
	~FragmentedNetworkPacket();
	void		ReInit(int PreAllocSize=0);
	void		ReadFromSocket(QSslSocket *socket, char *OutputBuffer=NULL);
	bool		IsReadComplete() { return ExpectedBytes == HandledBytes; }
	char		*GetData() { return Data; }
	__int64		GetDataLen() { return HandledBytes; }
	static void		SendPacket(QSslSocket *socket, char *Data, int Len);
private:
	void		Destroy();
	char		*Data;
	bool		ExternalBufferUsed;
	__int64		ExpectedBytes;
	__int64		HandledBytes;
};