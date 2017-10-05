#ifndef SERVER_H
#define SERVER_H

#include "QtCore/QList.h"
#include "sslserver.h"
#if defined(QT4_BASED_BUILD)
#include "ssl/QSslSocket.h"
#else
#include "QtNetwork/QSslSocket.h"
#endif

class FragmentedNetworkPacket;

// Simple ECHO server implementation. It will send back packets that he receives
class Server : public QObject
{
  Q_OBJECT

public:
  explicit Server();
  ~Server();

//  void SetUseEncryption(int pUseEncryption) { UseEncryption = pUseEncryption; }
  void SetListenPort(int pListenPort) { ListenPort = pListenPort; }
  // Start or stop TCP server listening
  int ToggleStartStopListening(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
  // load certificate files that we will use for our SSL communication
  int LoadCertificates(QString KeyAndCertPath);

protected slots:

  // Slots to receive signals from sockets
  void acceptConnection();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void connectionFailure();

private:
	int						IsServerUnique();
	int						IsVMUnique();
	void					PH_QueryUUID();
  QString					key;
  QString					certificate;
  SslServer					server;
  QList<QSslSocket *>		sockets;

  //good enough for a simple speed tested
  bool						UseEncryption;
  int						ListenPort;

  FragmentedNetworkPacket	*IncommingPacket;
  std::map<int, int>		UUIDCounterStatus;
};

#endif // SERVER_H
