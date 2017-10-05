#ifndef CLIENT_H
#define CLIENT_H

#if defined(QT4_BASED_BUILD)
	#include "ssl/QSslSocket.h"
#else
	#include "QtNetwork/QSslSocket.h"
#endif

#define ERROR_SSL_NOT_SUPPORTED (-2)
#define ERROR_SOCKET_STATE		(-3)
#define ERROR_SOCKET_WAIT_PEER	(-4)

#define SOCKET_SEARCH_TIMEOUT_MS	2000

class FragmentedNetworkPacket;

class Client : public QObject
{
	Q_OBJECT
public:
	explicit Client(QObject *parent);
	~Client();
 
  int	Connect(const QString &hostName, quint16 port);
  int	GetRemoteUUID(char *UUID, int bufsize);
  void	ResetMessageState() { MessageState = MESSAGE_NOT_SENT; }

protected slots:

  // Slots to receive signals from QSslSocket
  void connectedToServer();
  void sslErrors(const QList<QSslError> &errors);
  void receiveMessage();
  void connectionClosed();
  void socketError();

  void SendRequestQueryUUID();
private:
	enum StoredMessageStates {
		MESSAGE_NOT_SENT = 1,
		MESSAGE_SENDING_QUERY,
		MESSAGE_READING_REPLY,
		MESSAGE_RECEIVED,
	};
  QSslSocket				socket;
  int						UseSSLEncrypt;
  FragmentedNetworkPacket	*IncommingPacket;
  void						*MessageStore;
  int						MessageStoreSize;
  int						MessageState;
};

#endif // CLIENT_H
