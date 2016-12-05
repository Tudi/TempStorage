#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket.h>

class Client : public QObject
{
  Q_OBJECT

public:
  explicit Client();
  ~Client();
 
  void connectDisconnectButtonPressed();
  void sendButtonPressed();

protected slots:

  // Slots to receive signals from QSslSocket
  void connectedToServer();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void socketError();

private:
  QSslSocket socket;
};

#endif // CLIENT_H
