#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <in6addr.h>
#include <ip2string.h>
#include <pshpack1.h>
#include <poppack.h>
#include "Logger.h"
#include "Socks5Proxy.h"

#pragma comment(lib,"ws2_32.lib")

bool sendData(SOCKET fd, void* data, int len)
{
    char* ptr = (char*)data;

    while (len > 0)
    {
        int sent = send(fd, ptr, len, 0);
        if (sent <= 0)
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "send() error: %d", WSAGetLastError());
            return false;
        }
        ptr += sent;
        len -= sent;
    }

    return true;
}

int recvData(SOCKET fd, void* data, int len, bool disconnectOk)
{
    char* ptr = (char*)data;
    int total = 0;

    while (len > 0)
    {
        int recvd = recv(fd, ptr, len, 0);
        if (recvd < 0)
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "recv() error: %d", WSAGetLastError());
            return -1;
        }
        if (recvd == 0)
        {
            if (disconnectOk)
                break;
            sLog.Log(LL_Warning, __FILE__, __LINE__, "disconnected");
            return -1;
        }
        ptr += recvd;
        len -= recvd;
        total += recvd;
    }

    return total;
}

bool socksLogin(SOCKET fd)
{
    socks5_ident_req req;
    socks5_ident_resp resp;

    req.Version = 5;
    req.NumberOfMethods = 1;
    req.Methods[0] = 0x00; // no login method
    // add other methods as needed...

    if (!sendData(fd, &req, 2 + req.NumberOfMethods))
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "Could not send data to SOCKS 5 proxy");
        return false;
    }

    if (recvData(fd, &resp, sizeof(resp)) == -1)
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "Socks proxy did not reply for our method request");
        return false;
    }

    if (resp.Version != 5)
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS v5 identification failed");
        return false;
    }

    if (resp.Method == 0xFF)
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS v5 authentication failed");
        return false;
    }

    /*
    if (resp.Method != 0x00)
    {
        // authenticate as needed...
    }
    */

    return true;
}

bool socksRequest(SOCKET fd, socks5_req *req, socks5_resp *resp)
{
    memset(resp, 0, sizeof(socks5_resp));

    if (!sendData(fd, req, 4))
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "socksRequest:could not send data");
        return false;
    }

    switch (req->AddrType)
    {
    case 1:
    {
        if (!sendData(fd, &(req->DestAddr.IPv4), sizeof(in_addr)))
            return false;

        break;
    }
    case 3:
    {
        if (!sendData(fd, &(req->DestAddr.DomainLen), 1))
            return false;

        if (!sendData(fd, req->DestAddr.Domain, req->DestAddr.DomainLen))
            return false;

        break;
    }
    case 4:
    {
        if (!sendData(fd, &(req->DestAddr.IPv6), sizeof(in6_addr)))
            return false;

        break;
    }

    default:
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS 5 requesting unknown address type");
        return false;
    }
    }

    unsigned short port = htons(req->DestPort);
    if (!sendData(fd, &port, 2))
        return false;

    if (recvData(fd, resp, 4) == -1)
        return false;

    switch (resp->AddrType)
    {
    case 1:
    {
        if (recvData(fd, &(resp->BindAddr.IPv4), sizeof(in_addr)) == -1)
            return false;

        break;
    }
    case 3:
    {
        if (recvData(fd, &(resp->BindAddr.DomainLen), 1) == -1)
            return false;

        if (recvData(fd, resp->BindAddr.Domain, resp->BindAddr.DomainLen) == -1)
            return false;

        break;
    }
    case 4:
    {
        if (recvData(fd, &(resp->BindAddr.IPv6), sizeof(in6_addr)) == -1)
            return false;

        break;
    }

    default:
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS 5 bound to unknown address type");
        return false;
    }
    }

    if (recvData(fd, &port, 2, 0) == -1)
        return false;

    resp->BindPort = ntohs(port);

    return true;
}

bool socksConnect(SOCKET fd, const in_addr& dest, unsigned short port)
{
    socks5_req req;
    socks5_resp resp;

    req.Version = 5;
    req.Cmd = 1;
    req.Reserved = 0;
    req.AddrType = 1;
    req.DestAddr.IPv4 = dest;
    req.DestPort = port;

    if (!socksRequest(fd, &req, &resp))
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS v5 connect request failed");
        return false;
    }

    if (resp.Reply != 0x00)
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS v5 connect failed, error: 0x%02X", resp.Reply);
        return false;
    }

    return true;
}
