#include <qt/qobject.h>
#include <qt/qdatetime.h>
#include "client.h"
#include <iostream>

Client::Client() 
{
  // Check for SSL support.  If SSL support is not available, show a
  // message to the user describing what to do to enable SSL support.
  if (QSslSocket::supportsSsl())
  {
	  std::cout << "have SSL" << std::endl;
  }
  else
  {
	std::cout << "missing SSL" << std::endl;
  }

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
  connect(&socket, SIGNAL(encrypted()), this, SLOT(connectedToServer()));

  // Report any SSL errors that occur
  connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

  connect(&socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
  connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
}

Client::~Client()
{
  if (socket.isOpen())
    socket.close();
}

void Client::connectDisconnectButtonPressed()
{
//  ui->connectDisconnectButton->setEnabled(false);

  if (socket.state() == QAbstractSocket::UnconnectedState)
  {
    // Initiate an SSL connection to the chat server.
	socket.connectToHostEncrypted("127.0.0.1", 8081);
  }
  else
  {
    socket.close();
  }
}

void Client::sendButtonPressed()
{
  socket.write("test\n",7);
}

void Client::connectedToServer()
{
	std::cout << "connected to the server" << std::endl;
}

// Process SSL errors
void Client::sslErrors(const QList<QSslError> &errors)
{

  for (QList<QSslError>::const_iterator error = errors.begin(); error != errors.end(); error++)
  {
    
	std::cout << "SSL errors : " << error->errorString().toStdString() << std::endl;
  }

	socket.ignoreSslErrors();
}

void Client::receiveMessage()
{
	std::cout << "got a message" << std::endl;
  if (socket.canReadLine())
	  std::cout << socket.readLine().constData() << std::endl;
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