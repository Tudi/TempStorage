#ifndef SSLSERVER_H
#define SSLSERVER_H

#if defined(QT4_BASED_BUILD)
#include "ssl/QSslError.h"
#include "socket/QTcpServer.h"
#else
#include "QtNetwork/QSslError.h"
#include "QtNetwork/QTcpServer.h"
#endif

// QTcpServer extension to intercept new socket connection and enable SSL
// To be used by iTCH::Server for accepting encrypted connections
class SslServer : public QTcpServer
{
  Q_OBJECT

public:
#if defined(QT4_BASED_BUILD)
	virtual void incomingConnection(int socketDescriptor);
#else
	virtual void incomingConnection(qintptr socketDescriptor);
#endif
};

#endif // SSLSERVER_H

