#ifndef SERVER_H
#define SERVER_H

#include <QList.h>
#include <QSslSocket.h>
#include <QMainWindow.h>
#include "sslserver.h"

namespace Ui {
  class Server;
}

class Server : public QObject
{
  Q_OBJECT

public:
  explicit Server();
  ~Server();

  // Slots to receive signals from UI
  void startStopButtonClicked();

  void checkFileStatus();

protected slots:

  // Slots to receive signals from sockets
  void acceptConnection();
  void handshakeComplete();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void connectionFailure();

private:
 
private:
  QString key;
  QString certificate;
  SslServer server;
  QList<QSslSocket *> sockets;
};

#endif // SERVER_H
