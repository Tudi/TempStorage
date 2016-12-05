#include <cassert>
#include <QDateTime.h>
#include <QFileDialog.h>
#include <QFileInfo.h>
#include "server.h"
#include <iostream>

const QString INVALID_FILE_MESSAGE = "Existing and readable key and certificate files must be specified.";

Server::Server()
{

  // Check for SSL support.  If SSL support is not available, show a
  // message to the user describing what to do to enable SSL support.
  if (QSslSocket::supportsSsl())
  {
  }
  else
  {

	std::cout << "missing SSL" << std::endl;
  }

  connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

Server::~Server()
{
  if (server.isListening())
  {
    server.close();
  }

  foreach (QSslSocket *socket, sockets)
  {
    delete socket;
  }

}

void Server::startStopButtonClicked()
{
  if (server.isListening())
  {
    server.close();
  }
  else
  {
	if (server.listen(QHostAddress::Any, 8081))
    {
    }
	else
	{
		std::cout << "Could not bind" << std::endl;
	}
  }
}

void Server::checkFileStatus()
{
	key = "../../Certificates/server_.key";
	certificate = "../../Certificates/server_.csr";

  QFileInfo keyInfo(key);
  QFileInfo certificateInfo(certificate);
  if (keyInfo.exists() && keyInfo.isReadable() &&
      certificateInfo.exists() && certificateInfo.isReadable())
  {
  }
  else
  {
	  std::cout << "invalid key file" << key.toStdString() << keyInfo.exists() << " " << keyInfo.isReadable() << std::endl;
	  std::cout << "invalid certificate file" << certificate.toStdString() << certificateInfo.exists() << " " << certificateInfo.isReadable() << std::endl;
  }
}

// Accept connection from server and initiate the SSL handshake
void Server::acceptConnection()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(server.nextPendingConnection());
  assert(socket);

  // QSslSocket emits the encrypted() signal after the encrypted connection is established
  connect(socket, SIGNAL(encrypted()), this, SLOT(handshakeComplete()));

  // Report any SSL errors that occur
  connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));

  socket->setPrivateKey(key);
  socket->setLocalCertificate(certificate);

  socket->setPeerVerifyMode(QSslSocket::VerifyNone);
  socket->startServerEncryption();
}

// Receive notification that the SSL handshake has completed successfully
void Server::handshakeComplete()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  connect(socket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

  std::cout << "Accepted connection from " << socket->peerAddress().toString().toStdString() << ":" << socket->peerPort() << std::endl;

  sockets.push_back(socket);
}

void Server::sslErrors(const QList<QSslError> &errors)
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  QString errorStrings;
  foreach (QSslError error, errors)
  {
    errorStrings += error.errorString();
    if (error != errors.last())
    {
      errorStrings += ';';
    }
  }

  std::cout << "SSL error " << errorStrings.toStdString() << std::endl;
}

void Server::receiveMessage()
{
  QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
  assert(socket);

  if (socket->canReadLine())
  {
    QByteArray message = socket->readLine();
    QString sender = QString("%1:%2")
        .arg(socket->peerAddress().toString())
        .arg(socket->peerPort());

	QString Message = message;
	std::cout << "got message " << Message.toStdString() << std::endl;

    sender += " -> ";
    foreach (QSslSocket *s, sockets)
    {
      s->write(sender.toLocal8Bit().constData());
      s->write(message);
    }
  }
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