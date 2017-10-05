#include "../StdAfx.h"
#include <qtcore/qobject.h>
#include <qtcore/qdatetime.h>
#include <Qtcore/qcoreapplication.h>
#include "SSLClient.h"
#include "../SSLServer/NetworkPacket.h"
#include <time.h>
#include <iostream>
#include "../GetBiosUUID.h"
#include "../SharedMem/SharedMem.h"

Client::Client(QObject *parent) :
	QObject(parent),
	socket(parent)
{
  // Check for SSL support.  If SSL support is not available, show a
  // message to the user describing what to do to enable SSL support.
	if (QSslSocket::supportsSsl() == false)
	{
		DEBUG_SSL_PRINT(std::cout << "SSL is NOT supported !" << std::endl;);
		return;
	}

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
	connect(&socket, SIGNAL(encrypted()), this, SLOT(connectedToServer()));
	connect(&socket, SIGNAL(connected()), this, SLOT(connectedToServer()));
	// Report any SSL errors that occur
	connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

	connect(&socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));

	UseSSLEncrypt = 1;
	IncommingPacket = NULL;

	MessageStore = NULL;
	MessageStoreSize = 0;
	MessageState = MESSAGE_NOT_SENT;
}

Client::~Client()
{
	if (socket.isOpen())
		socket.close();

	if (IncommingPacket != NULL)
	{
		delete IncommingPacket;
		IncommingPacket = NULL;
	}
}

int Client::Connect(const QString &hostName, quint16 port)
{
	if (QSslSocket::supportsSsl() == false)
	{
		DEBUG_SSL_PRINT(std::cout << "Socket does not support SSL" << std::endl;);
		return ERROR_SSL_NOT_SUPPORTED;
	}

  if (socket.state() == QAbstractSocket::UnconnectedState)
  {
	  // Initiate an SSL connection to the chat server.
	  if (UseSSLEncrypt)
	  {
		  DEBUG_SSL_PRINT(std::cout << "Connecting to the encrypted server" << std::endl;);
		  socket.connectToHostEncrypted(hostName, port);
	  }
	  else
	  {
		  DEBUG_SSL_PRINT(std::cout << "Connecting to the server" << std::endl;);
		  socket.connectToHost(hostName, port);
	  }
  }
/*  else
  {
	  DEBUG_SSL_PRINT(std::cout << "Close connection to server" << std::endl;);
	  socket.close();
  } */
  return 0;
}

void Client::connectedToServer()
{
	DEBUG_SSL_PRINT(std::cout << "Connected to the server" << std::endl;);
}

// Process SSL errors
void Client::sslErrors(const QList<QSslError> &errors)
{
	// this is not an error. It's a warning ! => SelfSignedCertificate = 9
	DEBUG_SSL_PRINT(std::cout << "SSL errors received" << std::endl;);
	foreach(QSslError error, errors)
	//		DEBUG_SSL_PRINT( std::cout << "SSL errors : " << error->errorString().toStdString() << std::endl );
		DEBUG_SSL_PRINT(std::cout << "SSL error : " << (int)error.error() << std::endl;);

  // don't worry, just be happy
	socket.ignoreSslErrors();
}

void Client::receiveMessage()
{
	DEBUG_SSL_PRINT(std::cout << "Received message from server" << std::endl;);
	MessageState = MESSAGE_READING_REPLY;

	//create resource on demand
	if (IncommingPacket == NULL)
		IncommingPacket = new FragmentedNetworkPacket;
	else if (IncommingPacket->IsReadComplete())
		IncommingPacket->ReInit(MAX_BUFFER_TO_SEND * 2);

	//read data. Might be a fragmented packet
	IncommingPacket->ReadFromSocket(&socket);

	//if we read the whole packet. Send it back to the client
	if (IncommingPacket->IsReadComplete())
	{
		PacketHeader *IncommingPacketHeader = (PacketHeader *)IncommingPacket->GetData();
		DEBUG_SSL_PRINT(std::cout << "Received packet type " << IncommingPacketHeader->PacketType << std::endl;);
		if (IncommingPacketHeader->PacketType == PACKET_TYPE_UUID_REPLY)
		{
			UUIDQueryReplyPacket *ReplyPacket = (UUIDQueryReplyPacket *)IncommingPacket->GetData();
			if (MessageStore != NULL)
			{
				MessageStoreSize = min(sizeof(ReplyPacket->UUID), MessageStoreSize);
				DEBUG_SSL_PRINT(std::cout << "will store byte count : " << MessageStoreSize << std::endl;);
				DEBUG_SSL_PRINT(for (int i = 0; i < 16; i++)printf("%02X", ReplyPacket->UUID[i]); printf("\n"););
				memcpy(MessageStore, ReplyPacket->UUID, MessageStoreSize);
				MessageState = MESSAGE_RECEIVED;
			}
		}

		DEBUG_SSL_PRINT(std::cout << "Received whole packet" << std::endl;);
	}
}

void Client::connectionClosed()
{
	DEBUG_SSL_PRINT(std::cout << "connection closed" << std::endl;);
}

void Client::socketError()
{
	DEBUG_SSL_PRINT(std::cout << "Socket error : " << socket.error() << std::endl;);
//	DEBUG_SSL_PRINT(std::cout << socket.errorString().toStdString() << std::endl);

	if (socket.state() != QAbstractSocket::ConnectedState)
		connectionClosed();
	
	socket.close();
}

void Client::SendRequestQueryUUID()
{
	int RequestCounter = 0;
	int ret = SharedMemGetValue("VMSession", "RequestId", &RequestCounter, sizeof(RequestCounter));

	//increase request counter
	RequestCounter++;
	SharedMemSetValue("VMSession", "RequestId", &RequestCounter, sizeof(RequestCounter));

	UUIDQueryPacket UUIDQueryPacket;
	UUIDQueryPacket.PacketSize = sizeof(UUIDQueryPacket);
	UUIDQueryPacket.PacketStamp = time(NULL);
	UUIDQueryPacket.PacketType = PACKET_TYPE_UUID_QUERY;
	UUIDQueryPacket.RequestCounter = RequestCounter; //we will only get a reply if server request counter matches ours. This is to avoid connecting multiple VMs to the same server
	GetBiosUUID((unsigned char*)UUIDQueryPacket.UUID, sizeof(UUIDQueryPacket.UUID));

	DEBUG_SSL_PRINT(std::cout << "Start sending packet with size " << UUIDQueryPacket.PacketSize << std::endl;);
	FragmentedNetworkPacket::SendPacket(&socket, (char*)&UUIDQueryPacket, UUIDQueryPacket.PacketSize);
}

int	Client::GetRemoteUUID(char *UUID,int bufsize)
{
	if (QSslSocket::supportsSsl() == false)
	{
		DEBUG_SSL_PRINT(std::cout << "Socket does not support SSL" << std::endl;);
		return ERROR_SSL_NOT_SUPPORTED;
	}
	//can we get the data ?
	QAbstractSocket::SocketState sState = socket.state();
	if (sState != QAbstractSocket::ConnectedState)
	{
		DEBUG_SSL_PRINT(std::cout << "Socket state is not good : " << (int)sState << std::endl;);
#ifdef _DEBUG
		if (sState == QAbstractSocket::ConnectingState || sState == QAbstractSocket::UnconnectedState)
			Sleep(2000);
#endif
		return ERROR_SOCKET_STATE;
	}

	//always init
	if (MessageState == MESSAGE_NOT_SENT)
	{
		//prepare the store
		MessageState = MESSAGE_SENDING_QUERY;
		MessageStore = UUID;
		MessageStoreSize = bufsize;

		//query the host for the UUID
		DEBUG_SSL_PRINT(std::cout << "sending UUID query " << std::endl;);
		SendRequestQueryUUID();

		return ERROR_SOCKET_WAIT_PEER;
	}

	//finally got a message ?
	if (MessageState == MESSAGE_RECEIVED)
		return 0;

	return ERROR_SOCKET_WAIT_PEER;
}

#include "SSLClient_moc.hxx"