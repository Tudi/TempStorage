#pragma once
#if defined(QT4_BASED_BUILD)
#include "ssl/QSslSocket.h"
#else
#include "QtNetwork/QSslSocket.h"
#endif

enum FingerprintServicePacketTypes
{
	PACKET_TYPE_UUID_QUERY = 1,
	PACKET_TYPE_UUID_REPLY,
	PACKET_TYPE_MAX			//last value to test invalid values
};

#pragma pack(push, 1)
struct PacketHeader
{
	int		PacketSize; // number of bytes the "reader" should allocate to receive this packet. This must be first value !!!
	int		PacketType; // If we would be using mutliple packet types, we could use this as an ID
	int		PacketStamp;// sender timestamp if we want to calculate latency or generate logs
};
//VM qill query a physical computer UUID. We need to make sure that this VM is not present in some other network.
struct UUIDQueryPacket : PacketHeader
{
	char	UUID[16];
	int		RequestCounter;
};
struct UUIDQueryReplyPacket : PacketHeader
{
	char UUID[16];
};
#define MAX_PACKET_SIZE	( sizeof(UUIDQueryPacket) )
#pragma pack(pop)

class FragmentedNetworkPacket
{
public:
	FragmentedNetworkPacket();
	~FragmentedNetworkPacket();
	void			ReInit(int PreAllocSize=0);
	void			ReadFromSocket(QSslSocket *socket, char *OutputBuffer=NULL);
	bool			IsReadComplete() { return ExpectedBytes == HandledBytes; }
	bool			HasData() { return HandledBytes != 0; }
	char			*GetData() { return Data; }
	__int64			GetDataLen() { return HandledBytes; }
	static void		SendPacket(QSslSocket *socket, char *Data, int Len);
private:
	void			Destroy();
	char			*Data;
	int				DataAllocSize;
	bool			ExternalBufferUsed;
	signed int		ExpectedBytes;
	signed int		HandledBytes;
};