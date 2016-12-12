#include "StdAfx.h"
#include <cassert>
#include <QDateTime.h>
#include <QFileDialog.h>
#include <QFileInfo.h>
#include <qapplication.h>
#include "server.h"
#include <iostream>
#include "NetworkPacket.h"

Server::Server()
{
  if (QSslSocket::supportsSsl()==false)
	std::cout << "Missing SSL. Please install it." << std::endl;

  connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

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

void Server::ToggleStartStopListening(const QHostAddress &address, quint16 port)
{
  if (server.isListening())
    server.close();
  else if (server.listen(address, port)==false)
	std::cout << "Could not bind" << std::endl;
}

void Server::LoadCertificates( QString KeyAndCertPath )
{
	key = KeyAndCertPath + "server_.key";
	certificate = KeyAndCertPath + "server_.csr";

  QFileInfo keyInfo(key);
  QFileInfo certificateInfo(certificate);

  if (keyInfo.exists() == false)
  {
	  std::cout << "Key file does not exist" << key.toStdString() << keyInfo.exists() << " " << keyInfo.isReadable() << std::endl;
	  return;
  }
  if (keyInfo.isReadable() == false)
  {
	  std::cout << "Key file is not readable" << key.toStdString() << keyInfo.exists() << " " << keyInfo.isReadable() << std::endl;
	  return;
  }
  if ( certificateInfo.exists() == false )
  {
	  std::cout << "certificate file does not exist" << certificate.toStdString() << certificateInfo.exists() << " " << certificateInfo.isReadable() << std::endl;
	  return;
  }
  if ( certificateInfo.isReadable() == false )
  {
	  std::cout << "certificate file is not readable" << certificate.toStdString() << certificateInfo.exists() << " " << certificateInfo.isReadable() << std::endl;
	  return;
  }
}

// Accept connection from server and initiate the SSL handshake
void Server::acceptConnection()
{
	if (sockets.empty() == false)
		std::cout << "Server is mad efor 1 connection also. Need to update to handle multiple connections" << std::endl;

  QSslSocket *socket = dynamic_cast<QSslSocket *>(server.nextPendingConnection());
  assert(socket);


  // Report any SSL errors that occur
  connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));

  
  // QSslSocket emits the encrypted() signal after the encrypted connection is established
#define _USE_ENCRYPTION
#ifdef _USE_ENCRYPTION
  connect(socket, SIGNAL(encrypted()), this, SLOT(handshakeComplete()));
  socket->setPrivateKey(key);
  socket->setLocalCertificate(certificate);

  socket->setPeerVerifyMode(QSslSocket::VerifyNone);
  socket->startServerEncryption();
#else
  connect(socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
  sockets.push_back(socket);
  std::cout << "Accepted connection from " << socket->peerAddress().toString().toStdString() << ":" << socket->peerPort() << " .Encrypted : " << socket->isEncrypted() << std::endl;
#endif
}

// Receive notification that the SSL handshake has completed successfully
void Server::handshakeComplete()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  connect(socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

  std::cout << "Accepted connection from " << socket->peerAddress().toString().toStdString() << ":" << socket->peerPort() << " .Encrypted : " << socket->isEncrypted() << std::endl;

  sockets.push_back(socket);
}

void Server::sslErrors(const QList<QSslError> &errors)
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);
  foreach (QSslError error, errors)
	  std::cout << "SSL error : " << error.errorString().toStdString() << std::endl; 
}

void Server::receiveMessage()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  //create new packet reader. Should be socket specific !

  //create resource on demand
  if (IncommingPacket == NULL)
	  IncommingPacket = new FragmentedNetworkPacket;
  else if (IncommingPacket->IsReadComplete())
	  IncommingPacket->ReInit();

  //read data. Might be a fragmented packet
  IncommingPacket->ReadFromSocket(socket);

  //if we read the whole packet. Send it back to the client
  if (IncommingPacket->IsReadComplete())
	  FragmentedNetworkPacket::SendPacket( socket, IncommingPacket->GetData(), IncommingPacket->GetDataLen());
}

void Server::connectionClosed()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  std::cout << "connection closed " << socket->errorString().toStdString() << std::endl;
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}

void Server::connectionFailure()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  std::cout << "connection error " << socket->errorString().toStdString() << std::endl;
  sockets.removeOne(socket);
  socket->disconnect();
  socket->deleteLater();
}

#include "server_moc.hxx"