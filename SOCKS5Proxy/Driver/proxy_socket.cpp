#ifdef USE_NONBLOCKING_SOCKETS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "proxy_socket.h"
#include "FlowManager.h"
#include "Logger.h"
#include "Socks5Proxy.h"
#include "Utils.h"

void AppToProxySocket::OnConnect()
{
	FlowStatusStore* fss = GetFlowStatus(GetRemoteStruct().sin_port);

	if (fss == NULL)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Failed to obtain flow session for socket");
		return;
	}

	//create a pair socket : app -> proxy -> tunnel
	ProxyToTunnelSocket*t = new ProxyToTunnelSocket();

	//has to set destination before connect. Since this is async, the login might execute before the "set"
	t->SetDestination(htonl(fss->ip_hdr.DstAddr), htons(fss->tcp_hdr.DstPort));

	in_addr TunnelIP;
	TunnelIP.S_un.S_addr = htonl(fss->RedirectToIP);
	bool res = t->Connect(TunnelIP, fss->RedirectToPort);
	if (res == false)
	{
		sLog.Log(LL_Warning, __FILE__, __LINE__, "could not connect to SOCKS 5 tunnel");
		return;
	}

	Proxy_s = t;
	((ProxyToTunnelSocket*)Proxy_s)->SetAppSocket(this);
}

void ProxyToTunnelSocket::SetDestination(unsigned int dip, unsigned short dport)
{
	DstPort = dport;
	DstIP = dip;
}

#pragma pack(push, 1)
struct socks5_ident_req_no_login
{
	unsigned char Version;
	unsigned char NumberOfMethods;
	unsigned char Methods[1];
};

struct socks5_req_IPV4
{
	unsigned char Version;
	unsigned char Cmd;
	unsigned char Reserved;
	unsigned char AddrType;
	in_addr DestAddr;
	unsigned short DestPort;
};

struct socks5_resp_IPV4
{
	unsigned char Version;
	unsigned char Reply;
	unsigned char Reserved;
	unsigned char AddrType;
	in_addr IPv4;
	unsigned short BindPort;
};
#pragma pack(pop)

void ProxyToTunnelSocket::DoSocks5Login()
{
	if (LoginState == SLS_Done)
		return;
	if (LoginState == SLS_None)
	{
		socks5_ident_req_no_login req;
		req.Version = 5;
		req.NumberOfMethods = 1;
		req.Methods[0] = 0x00; // no login method
		Send((unsigned char*)&req, sizeof(socks5_ident_req_no_login));

		LoginState = SLS_SentIdentReq;

		//flush our read buffer. At this point we expect read buffer to be empty !
//		GetReadBuffer().Remove(GetReadBuffer().GetSize());
	}
	else if (LoginState == SLS_SentIdentReq && GetReadBuffer().GetSize() >= sizeof(socks5_ident_resp))
	{
		socks5_ident_resp* resp = (socks5_ident_resp*)GetReadBuffer().GetBufferStart();
		if (resp->Version != 5)
		{
			sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS v5 identification failed");
			LoginState = SLS_FAILED;
			Disconnect();
			return;
		}
		if (resp->Method == 0xFF)
		{
			sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS v5 authentication failed");
			LoginState = SLS_FAILED;
			Disconnect();
			return;
		}

		//remove this structure from our read buffer
		GetReadBuffer().Remove(sizeof(socks5_ident_resp));

		//send request for IPV4 redirection
		socks5_req_IPV4 req;
		req.Version = 5;
		req.Cmd = 1;
		req.Reserved = 0;
		req.AddrType = 1;
		req.DestAddr.S_un.S_addr = htonl(DstIP);
		req.DestPort = htons(DstPort);

		Send((unsigned char*)&req, sizeof(socks5_req_IPV4)); 

		char DstIP[50];
		IPv4Tostr(req.DestAddr.S_un.S_addr, DstIP, sizeof(DstIP));
		sLog.Log(LL_Info, __FILE__, __LINE__, "asking SOCKS to transfer data to %s:%d", DstIP, DstPort);

		LoginState = SLS_SentConnectReq;
	}
	else if (LoginState == SLS_SentConnectReq && GetReadBuffer().GetSize() >= sizeof(socks5_resp_IPV4))
	{
		socks5_resp_IPV4* resp = (socks5_resp_IPV4*)GetReadBuffer().GetBufferStart();
		GetReadBuffer().Remove(sizeof(socks5_resp_IPV4));

		LoginState = SLS_Done;
	}
}
#endif