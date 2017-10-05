#include "../StdAfx.h"
//#include "QDateTime.h"
//#include "QFileDialog.h"
#include "QtCore/QFileInfo.h"
//#include "qapplication.h"
#include "GenericServer.h"
#include <iostream>
#include "NetworkPacket.h"
#include <cassert>
#include <time.h>
#include "../GetBiosUUID.h"
#include "../SharedMem/SharedMem.h"
#include "../VMTools.h"

Server::Server()
{
	if (QSslSocket::supportsSsl() == false)
		DEBUG_SSL_PRINT(std::cout << "Missing SSL. Please install it." << std::endl;);

	//only accept new connections if we are uniquely run on this computer
	if (IsServerUnique() == 1 && Detect_VM() == 0)
	{
		connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
	}
	else
	{
		DEBUG_SSL_PRINT(std::cout << "You can only run 1 instance of SSL server on a physical comnputer. Ignoring connections from this instance" << std::endl;);
	}

  UseEncryption = 1;
  ListenPort = 8081;

  IncommingPacket = NULL;
}

Server::~Server()
{
  if (server.isListening())
    server.close();

  foreach (QSslSocket *socket, sockets)
    delete socket;
  sockets.clear();

  if (IncommingPacket != NULL)
  {
	  delete IncommingPacket;
	  IncommingPacket = NULL;
  }
}

int Server::ToggleStartStopListening(const QHostAddress &address, quint16 port)
{
	if (server.isListening())
	{
		DEBUG_SSL_PRINT(std::cout << "Stopped listening" << std::endl;);
		server.close();
	}
  else
  {
	  int Port = port;
	  if (Port == 0)
		  Port = ListenPort;
	  if (server.listen(address, Port) == false)
	  {
		  DEBUG_SSL_PRINT(std::cout << "Could not bind to port " << Port << std::endl;)
		  return 1;
	  }
	  else
	  {
		  DEBUG_SSL_PRINT(std::cout << "Started listening on port " << Port << std::endl;)
	  }
  }
	return 0;
}

int Server::LoadCertificates( QString KeyAndCertPath )
{
	key = KeyAndCertPath + "server_.key";
	certificate = KeyAndCertPath + "server_.csr";

  QFileInfo keyInfo(key);
  QFileInfo certificateInfo(certificate);

  if (keyInfo.exists() == false)
  {
	  DEBUG_SSL_PRINT(std::cout << "Key file does not exist" << key.toStdString() << keyInfo.exists() << " " << keyInfo.isReadable() << std::endl;);
	  return 1;
  }
  if (keyInfo.isReadable() == false)
  {
	  DEBUG_SSL_PRINT(std::cout << "Key file is not readable" << key.toStdString() << keyInfo.exists() << " " << keyInfo.isReadable() << std::endl;);
	  return 1;
  }
  if ( certificateInfo.exists() == false )
  {
	  DEBUG_SSL_PRINT(std::cout << "certificate file does not exist" << certificate.toStdString() << certificateInfo.exists() << " " << certificateInfo.isReadable() << std::endl;);
	  return 1;
  }
  if ( certificateInfo.isReadable() == false )
  {
	  DEBUG_SSL_PRINT(std::cout << "certificate file is not readable" << certificate.toStdString() << certificateInfo.exists() << " " << certificateInfo.isReadable() << std::endl;);
	  return 1;
  }

  return 0;
}

// Accept connection from server and initiate the SSL handshake
void Server::acceptConnection()
{
/*	if (sockets.empty() == false)
	{
		DEBUG_SSL_PRINT(std::cout << "Server is made for 1 connection only. Need to update to handle multiple connections" << std::endl;);
		return;
	}*/

	QTcpSocket *socketTCP = server.nextPendingConnection();
	assert(socketTCP);
	QSslSocket *socket = dynamic_cast<QSslSocket *>(socketTCP);	// if this fails, than we failed to intercept "new connection"
	assert(socket);

	// Report any SSL errors that occur
	connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
	sockets.push_back(socket);

//	DEBUG_SSL_PRINT(std::wcout << "Accepted connection from " << socket->peerAddress().toString().toStdWString() << L":" << socket->peerPort() << L" .SSL: " << socket->isEncrypted() << std::endl);
	DEBUG_SSL_PRINT(std::cout << "Accepted connection from XXXX::" << socket->peerPort() << " .SSL: " << socket->isEncrypted() << std::endl;);

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
  if (UseEncryption)
  {
	  DEBUG_SSL_PRINT(std::cout << "Using encrypted communication" << std::endl;);
	  socket->setPrivateKey(key);
	  socket->setLocalCertificate(certificate);
	  socket->setPeerVerifyMode(QSslSocket::VerifyNone);
	  socket->startServerEncryption();
  }

}

void Server::sslErrors(const QList<QSslError> &errors)
{
	// this is not an error. It's a warning ! => SelfSignedCertificate = 9
	DEBUG_SSL_PRINT(std::cout << "Received SSL errors" << std::endl;);
	QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
	assert(socket);
	foreach(QSslError error, errors)
		//		DEBUG_SSL_PRINT( std::cout << "SSL error : " << error.errorString().toStdString() << std::endl );
		DEBUG_SSL_PRINT(std::cout << "SSL error : " << (int)error.error() << std::endl;);
}

void Server::connectionClosed()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  DEBUG_SSL_PRINT(std::cout << "connection closed " << socket->errorString().toStdString() << std::endl;);
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}

void Server::connectionFailure()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  QAbstractSocket::SocketError er = socket->error();
//  DEBUG_SSL_PRINT( std::wcout << "connection error " << (int)er << L" " << socket->errorString().toStdWString() << std::endl );
  if (er != QAbstractSocket::SocketError::RemoteHostClosedError)
	  DEBUG_SSL_PRINT(std::cout << "connection error " << (int)er << std::endl;)
  else
	DEBUG_SSL_PRINT(std::cout << "connection closed by remote " << std::endl;)
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}

void Server::receiveMessage()
{
	QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
	assert(socket);

	DEBUG_SSL_PRINT(std::cout << "Got a message" << std::endl;);
	//create new packet reader. Should be socket specific !!!!!
	if (IncommingPacket == NULL)
		IncommingPacket = new FragmentedNetworkPacket;
	else if (IncommingPacket->IsReadComplete())
		IncommingPacket->ReInit(MAX_BUFFER_TO_SEND * 2);

	//read data. Might be a fragmented packet
	IncommingPacket->ReadFromSocket(socket);

	//if we read the whole packet. Send it back to the client
	if (IncommingPacket->IsReadComplete())
	{
		DEBUG_SSL_PRINT(std::cout << "Got full message" << std::endl;);
		PacketHeader *IncommingPacketHeader = (PacketHeader *)IncommingPacket->GetData();
		if (IncommingPacketHeader->PacketType == PACKET_TYPE_UUID_QUERY)
		{
			PH_QueryUUID();
		}
	}
}

int	Server::IsServerUnique()
{
	//check if we are running alone or there are other instances of us running on this system
	DWORD TheProcess;
	int ErrCode = SharedMemGetValue("licensedll", "TheOne", (char*)&TheProcess, sizeof(TheProcess));
	if (ErrCode == 2)	//this means nobody else yet set this value. We will use it as a semaphore
	{
		TheProcess = GetCurrentProcessId();
		SharedMemSetValue("licensedll", "TheOne", (char*)&TheProcess, sizeof(TheProcess));
	}
	else if (TheProcess != GetCurrentProcessId())
	{
		DEBUG_SSL_PRINT(std::cout << "SSL server is used in another instance also" << std::endl;);
		return 0;
	}
	return 1;
}
#include "GenericServer_moc.hxx"