#pragma once

/*********************************************
* socket chain to pass packets from one point to another while injecting a SOCKS 5 login sequence
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
#include "Socket.h"

class ProxyToTunnelSocket : public Socket
{
	enum SocksLoginSteps
	{
		SLS_None,
		SLS_SentIdentReq,
		SLS_SentConnectReq,
		SLS_Done,
		SLS_FAILED,
	};
public:
	ProxyToTunnelSocket() :Socket(0)
	{
		App_s = NULL;
		LoginState = SLS_None;
	}
	//data received from the tunnel, we pass it to the app
	void OnRead()
	{
		if (LoginState != SLS_Done)
		{
			DoSocks5Login();
			return;
		}
		if (App_s != NULL)
			App_s->Send((const unsigned char*)GetReadBuffer().GetBufferStart(), (unsigned int)GetReadBuffer().GetSize());
		GetReadBuffer().Remove(GetReadBuffer().GetSize());
	}
	//forward data comming from tunnel to the proxy socket
	void SetAppSocket(Socket* s)
	{
		App_s = s;
	}
	void OnConnect()
	{
		DoSocks5Login();
	}
	void DoSocks5Login();
	void SetDestination(unsigned int dip, unsigned short dport);
private:
	SocksLoginSteps LoginState;
	Socket *App_s;
	unsigned short DstPort;
	unsigned int DstIP;
};

class AppToProxySocket: public Socket
{
public:
	AppToProxySocket(SOCKET s) : Socket(s) 
	{
		Proxy_s = NULL;
	}
	void OnConnect();
	//when application sent data to proxy socket
	void OnRead()
	{
		//connect might not be instant. We might get data from the app but we not yet finished
		//connecting to the tunnel
		if (Proxy_s != NULL)
		{
			Proxy_s->Send((const unsigned char*)GetReadBuffer().GetBufferStart(), (unsigned int)GetReadBuffer().GetSize());
			GetReadBuffer().Remove(GetReadBuffer().GetSize());
		}
	}
	void OnWrite()
	{
		GetWriteBuffer().Remove(GetWriteBuffer().GetSize());
	}
private:
	Socket *Proxy_s;
};
#endif