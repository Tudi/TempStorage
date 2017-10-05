//#include "QSslSocket.h"
#include <QtNetwork/qsslsocket.h>
#include "sslserver.h"

// Intercept new socket connection and enable SSL
void SslServer::incomingConnection(qintptr socketDescriptor)
{
  QSslSocket *serverSocket = new QSslSocket();
  if (serverSocket->setSocketDescriptor(socketDescriptor))
  {
    addPendingConnection (serverSocket);
  }
  else
  {
    delete serverSocket;
  }
}

#include "sslserver_moc.hxx"