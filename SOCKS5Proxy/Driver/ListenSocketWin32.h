#ifndef LISTEN_SOCKET_WIN32_H
#define LISTEN_SOCKET_WIN32_H

/*********************************************
* AsyncSocket implementation to start listening on a specific IP/Port combination
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
#include "Network.h"
#include "Threading.h"

template<class T>
class ListenSocket : public ThreadBase
{
public:
	ListenSocket(const char * ListenAddress, unsigned int Port)
	{
		m_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		SocketOps::ReuseAddr(m_socket);
		SocketOps::Blocking(m_socket);

		m_address.sin_family = AF_INET;
		m_address.sin_port = ntohs((u_short)Port);
		m_address.sin_addr.s_addr = htonl(INADDR_ANY);
		m_opened = false;

/*		if(strcmp(ListenAddress, "0.0.0.0"))
		{
			getaddrininfo()
			struct hostent * hostname = gethostbyname(ListenAddress);
			if(hostname != 0)
				memcpy(&m_address.sin_addr.s_addr, hostname->h_addr_list[0], hostname->h_length);
		}*/

		// bind.. well attempt to.
		int ret = bind(m_socket, (const sockaddr*)&m_address, sizeof(m_address));
		if(ret != 0)
		{
//			printf("Bind unsuccessful on port %u.", Port);
			return;
		}

		ret = listen(m_socket, 5);
		if(ret != 0) 
		{
//			printf("Unable to listen on port %u.", Port);
			return;
		}

		m_opened = true;
		len = sizeof(sockaddr_in);
		m_cp = sSocketMgr.GetCompletionPort();
	}

	~ListenSocket()
	{
		Close();	
	}

	bool run()
	{
		while(m_opened)
		{
			aSocket = WSAAccept(m_socket, (sockaddr*)&m_tempAddress, (LPINT)&len, NULL, NULL);
			if(aSocket == INVALID_SOCKET)
				continue;		// shouldn't happen, we are blocking.

			socket = new T(aSocket);
			socket->SetCompletionPort(m_cp);
			socket->Accept(&m_tempAddress);
		}
		return false;
	}

	void Close()
	{
		// prevent a race condition here.
		bool mo = m_opened;
		m_opened = false;

		if(mo)
		{
			SocketOps::CloseSocket(m_socket);
			//not dead sure about these parts
			m_socket = 0;
			socket = NULL;
			m_cp = 0;
		}
	}

	bool IsOpen() { return m_opened; }

private:
	SOCKET m_socket;
	struct sockaddr_in m_address;
	struct sockaddr_in m_tempAddress;
	bool m_opened;
	unsigned int len;
	SOCKET aSocket;
	T * socket;
	HANDLE m_cp;
};
#endif
#endif
