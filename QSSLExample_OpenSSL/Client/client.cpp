#include "StdAfx.h"
#include <qt/qobject.h>
#include <qt/qdatetime.h>
#include "client.h"
#include "../Server/NetworkPacket.h"

Client::Client() 
{
  // Check for SSL support.  If SSL support is not available, show a
  // message to the user describing what to do to enable SSL support.
  if (QSslSocket::supportsSsl() == false )
	std::cout << "SSL is NOT supported !" << std::endl;

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
//#define _USE_ENCRYPTION
#ifdef _USE_ENCRYPTION
  connect(&socket, SIGNAL(encrypted()), this, SLOT(connectedToServer()));
#else
  connect(&socket, SIGNAL(connected()), this, SLOT(connectedToServer()));
#endif
  // Report any SSL errors that occur
  connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

  connect(&socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
  connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));

  LastPacketContent = new char[MAX_BUFFER_TO_SEND];
  ProfilePacketData = new char[MAX_BUFFER_TO_SEND];
  memset(ProfilePacketData, 0, MAX_BUFFER_TO_SEND);
}

Client::~Client()
{
  if (socket.isOpen())
    socket.close();

  if (LastPacketContent != NULL)
  {
	  delete LastPacketContent;
	  LastPacketContent = NULL;
  }

  if (IncommingPacket != NULL)
  {
	  delete IncommingPacket;
	  IncommingPacket = NULL;
  }
}

void Client::ToggleConnectDisconnect(const QString &hostName, quint16 port)
{
  if (socket.state() == QAbstractSocket::UnconnectedState)
  {
	  // Initiate an SSL connection to the chat server.
#define _USE_ENCRYPTION
#ifdef _USE_ENCRYPTION
	  socket.connectToHostEncrypted(hostName, port);
#else
	  socket.connectToHost(hostName, port);
#endif
  }
  else
  {
	  socket.close();
  }
}

void Client::InitAndStartProfiling()
{
	// do not really care about how it is initialize. Might as well be 0. 
	for (int i = 0; i < MAX_BUFFER_TO_SEND; i++)
		ProfilePacketData[i] = (char)(i & 0xFF);

	PacketHeader *ProfilingPacket = (PacketHeader*)ProfilePacketData;
	ProfilingPacket->SendSize = INITIAL_PACKET_SIZE;
	ProfilingPacket->SendCount = 0;
	ProfilingPacket->SendStamp = GetTimeTick();
	ProfilingPacket->TypeStamp = GetTimeTick();
	ProfilingPacket->PacketType = 1;
	ProfilingPacket->SendCountType = 0;
	ProfilingPacket->SentTotal = 0;

	//send first of a chain of packets
	std::cout << "Will resend the same packets " << MIN_SEND_COUNT << " times, or at least " << MIN_SEND_TIME / 1000 << " seconds " << std::endl;

	std::cout << "Start sending packets with size " << ProfilingPacket->SendSize << std::endl;
	FragmentedNetworkPacket::SendPacket(&socket, ProfilePacketData, ProfilingPacket->SendSize);
}

void Client::ContinueProfiling()
{
	PacketHeader *ProfilingPacket = (PacketHeader*)ProfilePacketData;

	//check how much time we spent resending this type of packet
	double ProfileTimeThisSize = GetTimeTick() - ProfilingPacket->TypeStamp;

	//start performing the next send
	ProfilingPacket->SendCountType++;

	if (ProfileTimeThisSize < MIN_SEND_TIME || ProfilingPacket->SendCountType < MIN_SEND_COUNT)
	{
//		std::cout << "Resending packet with size " << ProfilingPacket->SendSize << std::endl;
		AsciiLoadingBar::Print(1);
		//resend the same packet until we have conclusive data for statistics
		FragmentedNetworkPacket::SendPacket(&socket, ProfilePacketData, ProfilingPacket->SendSize);
		return;
	}

	ProfilingPacket->SendCount++;

	// calculate statistics for this type of packet
	__int64 TypeByteTraffic = ProfilingPacket->SendSize * ProfilingPacket->SendCountType * 2;
	int BytesPerSecond = TypeByteTraffic / (ProfileTimeThisSize / 1000.0);
	std::cout << "Processing speed " << BytesPerSecond << " Bytes / second (" << (BytesPerSecond/1024/1024) << "MB/s)for this packet size" << std::endl;

	//calculate avg packet statistics
	double ProfileTimeTotal = GetTimeTick() - ProfilingPacket->SendStamp;
	ProfilingPacket->SentTotal += TypeByteTraffic;
	int BytesPerSecondAvg = ProfilingPacket->SentTotal / (ProfileTimeTotal / 1000.0);
	std::cout << "Processing speed " << BytesPerSecondAvg << " Bytes / second (" << (BytesPerSecondAvg / 1024 / 1024) << "MB/s) average" << std::endl;

	// done profiling ?
	if (ProfilingPacket->SendSize * 2 > MAX_BUFFER_TO_SEND)
	{
		std::cout << "\n\nProfiling ended. You can close this window now." << std::endl;
		return;
	}

	// create a new type and start sending that
	ProfilingPacket->TypeStamp = GetTimeTick();
	ProfilingPacket->PacketType++;
	ProfilingPacket->SendCountType = 1;
	ProfilingPacket->SendSize = min(MAX_BUFFER_TO_SEND - 1, ProfilingPacket->SendSize * 2);
	std::cout << "Start sending packets with " << ProfilingPacket->SendSize << " bytes size" << std::endl;
	FragmentedNetworkPacket::SendPacket(&socket, ProfilePacketData, ProfilingPacket->SendSize);
}

void Client::connectedToServer()
{
	std::cout << "Connected to the server" << std::endl;

	//profile our connection speed
	InitAndStartProfiling();
}

// Process SSL errors
void Client::sslErrors(const QList<QSslError> &errors)
{
	for (QList<QSslError>::const_iterator error = errors.begin(); error != errors.end(); error++)
		std::cout << "SSL errors : " << error->errorString().toStdString() << std::endl;

  // don't worry, just be happy
	socket.ignoreSslErrors();
}

void Client::receiveMessage()
{
	//create resource on demand
	if (IncommingPacket == NULL)
		IncommingPacket = new FragmentedNetworkPacket;
	else if (IncommingPacket->IsReadComplete())
		IncommingPacket->ReInit();

	//read data. Might be a fragmented packet
	IncommingPacket->ReadFromSocket(&socket);

	//if we read the whole packet. Send it back to the client
	if (IncommingPacket->IsReadComplete())
	{
#ifdef _PRINT_SOCKET_READ_INFO
		std::cout << "Received whole packet" << std::endl;
#endif
		//should check for data integrity here
		//send the next packet we want to profile
		ContinueProfiling();
	}
}

void Client::connectionClosed()
{
	std::cout << "connection closed" << std::endl;
}

void Client::socketError()
{
	std::cout << socket.errorString().toStdString() << std::endl;

	if (socket.state() != QAbstractSocket::ConnectedState)
		connectionClosed();
	
	socket.close();
}

#include "client_moc.cxx"