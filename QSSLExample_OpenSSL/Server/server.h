#ifndef SERVER_H
#define SERVER_H

#include <QList.h>
#include <QSslSocket.h>
#include "sslserver.h"

class FragmentedNetworkPacket;

class Server : public QObject
{
  Q_OBJECT

public:
  explicit Server();
  ~Server();

  // Slots to receive signals from UI
  void ToggleStartStopListening(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
  void LoadCertificates(QString KeyAndCertPath);

protected slots:

  // Slots to receive signals from sockets
  void acceptConnection();
  void handshakeComplete();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void connectionFailure();

private:
  QString				key;
  QString				certificate;
  SslServer				server;
  QList<QSslSocket *>	sockets;

  //good enough for a simple speed tested
  FragmentedNetworkPacket	*IncommingPacket;
};

#endif // SERVER_H
