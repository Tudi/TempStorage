#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket.h>

class FragmentedNetworkPacket;

class Client : public QObject
{
  Q_OBJECT

public:
  explicit Client();
  ~Client();
 
  void ToggleConnectDisconnect(const QString &hostName, quint16 port);
  int isConnected() { return socket.ConnectedState; }

protected slots:

  // Slots to receive signals from QSslSocket
  void connectedToServer();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void socketError();

  void InitAndStartProfiling();
  void ContinueProfiling();
private:
  QSslSocket socket;

  FragmentedNetworkPacket	*IncommingPacket;

  // good enough for a simple speed tested
  // store incomming data in case we want to compare to the data we sent out
  char *LastPacketContent;
  int	LastPacketSize;

  //lasy way to store data for our profiling
  char *ProfilePacketData; 
};

#endif // CLIENT_H
