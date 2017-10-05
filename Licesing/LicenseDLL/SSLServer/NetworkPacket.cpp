#include "NetworkPacket.h"
#include <iostream>
#include <cassert>

FragmentedNetworkPacket::FragmentedNetworkPacket()
{
	Data = NULL;
	ExpectedBytes = 0;
	HandledBytes = 0;
	DataAllocSize = 0;
	ExternalBufferUsed = false;
}

void FragmentedNetworkPacket::ReInit(int PreAllocSize)
{
	ExpectedBytes = 0;
	HandledBytes = 0;
	if (ExternalBufferUsed == true)
		Data = NULL;
	else if (DataAllocSize < PreAllocSize)
		Destroy();

	if (PreAllocSize && Data == NULL)
	{
		DataAllocSize = PreAllocSize;
		Data = new char[PreAllocSize];
	}
}

void FragmentedNetworkPacket::Destroy()
{
	if (Data != NULL && ExternalBufferUsed == false)
		delete[] Data;
	Data = NULL;
}

FragmentedNetworkPacket::~FragmentedNetworkPacket()
{
	Destroy();
}

void FragmentedNetworkPacket::ReadFromSocket(QSslSocket *socket, char *OutputBuffer)
{
	if (ExpectedBytes == 0)
	{
		socket->peek((char*)&ExpectedBytes, sizeof(ExpectedBytes));
		assert(ExpectedBytes < 2 * MAX_PACKET_SIZE && ExpectedBytes > 0);
		HandledBytes = 0;
#ifdef _PRINT_SOCKET_READ_INFO
		std::cout << "New incomming packet with size " << ExpectedBytes << std::endl;
#endif
		if (DataAllocSize<ExpectedBytes)
			Destroy();
		if (OutputBuffer != NULL)
		{
			Data = OutputBuffer;
			ExternalBufferUsed = true;
		}
		else if (Data == NULL)
		{
			DataAllocSize = ExpectedBytes + 100;
			Data = new char[DataAllocSize];
		}
	}
	assert(ExpectedBytes < DataAllocSize);
	if (ExpectedBytes > HandledBytes)
	{
		int CurReadSize = socket->read(&Data[HandledBytes], ExpectedBytes - HandledBytes);
#ifdef _PRINT_SOCKET_READ_INFO
		std::cout << "read " << CurReadSize << " requested " << (ExpectedBytes - HandledBytes) << " has bytes " << socket->bytesAvailable() << " need now " << (ExpectedBytes - CurReadSize) << " need total " << ExpectedBytes << std::endl;
#endif
		HandledBytes += CurReadSize;
	}
}

void FragmentedNetworkPacket::SendPacket(QSslSocket *socket, char *Data, int Len)
{
	//sanity check
	if (Data == NULL || Len == 0)
		return;
	//stream the data in chunks
	int WrittenCount = 0;
	int Remaining = Len;
	while (Remaining > 0)
	{
		int WriteOneGo = socket->write(&Data[WrittenCount], Remaining);
#ifdef _PRINT_SOCKET_READ_INFO
		std::cout << "socket wrote " << WriteOneGo << std::endl;
#endif
		Remaining -= WriteOneGo;
		WrittenCount += WriteOneGo;
		if (Remaining > 0)
			socket->waitForBytesWritten(1000);
	}
}
