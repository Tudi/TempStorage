#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <in6addr.h>
#include <ip2string.h>
#pragma comment(lib,"ws2_32.lib")

#include <pshpack1.h>
struct socks5_ident_req
{
    unsigned char Version;
    unsigned char NumberOfMethods;
    unsigned char Methods[256];
};

struct socks5_ident_resp
{
    unsigned char Version;
    unsigned char Method;
};

struct socks5_req
{
    unsigned char Version;
    unsigned char Cmd;
    unsigned char Reserved;
    unsigned char AddrType;
    union {
        in_addr IPv4;
        in6_addr IPv6;
        struct {
            unsigned char DomainLen;
            char Domain[256];
        };
    } DestAddr;
    unsigned short DestPort;
};

struct socks5_resp
{
    unsigned char Version;
    unsigned char Reply;
    unsigned char Reserved;
    unsigned char AddrType;
    union {
        in_addr IPv4;
        in6_addr IPv6;
        struct {
            unsigned char DomainLen;
            char Domain[256];
        };
    } BindAddr;
    unsigned short BindPort;
};
#include <poppack.h>

bool sendData(SOCKET fd, void* data, int len)
{
    char* ptr = (char*)data;

    while (len > 0)
    {
        int sent = send(fd, ptr, len, 0);
        if (sent <= 0)
        {
            printf("send() error: %d\n", WSAGetLastError());
            return false;
        }
        ptr += sent;
        len -= sent;
    }

    return true;
}

int recvData(SOCKET fd, void* data, int len, bool disconnectOk = false)
{
    char* ptr = (char*)data;
    int total = 0;

    while (len > 0)
    {
        int recvd = recv(fd, ptr, len, 0);
        if (recvd < 0)
        {
            printf("recv() error: %d\n", WSAGetLastError());
            return -1;
        }
        if (recvd == 0)
        {
            if (disconnectOk)
                break;
            printf("disconnected\n");
            return -1;
        }
        ptr += recvd;
        len -= recvd;
        total -= recvd;
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
        printf("Could not send data to SOCKS 5 proxy\n");
        return false;
    }

    if (recvData(fd, &resp, sizeof(resp)) == -1)
    {
        printf("Socks proxy did not reply for our method request\n");
        return false;
    }

    if (resp.Version != 5)
    {
        printf("SOCKS v5 identification failed\n");
        return false;
    }

    if (resp.Method == 0xFF)
    {
        printf("SOCKS v5 authentication failed\n");
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
        printf("socksRequest:could not send data\n");
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
        printf("SOCKS 5 requesting unknown address type\n");
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
        printf("SOCKS 5 bound to unknown address type\n");
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
        printf("SOCKS v5 connect request failed\n");
        return false;
    }

    if (resp.Reply != 0x00)
    {
        printf("SOCKS v5 connect failed, error: 0x%02X\n", resp.Reply);
        return false;
    }

    return true;
}

int main666()
{
    WSADATA wsaData;
    int rv = WSAStartup(MAKEWORD(2, 0), &wsaData);
    if (rv != 0)
    {
        printf("WSAStartup() error: %d", rv);
        return 1;
    }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
    {
        printf("socket() error: %d", WSAGetLastError());
        return 1;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
//    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(9150);

    if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
    {
        printf("connect() error: %d", WSAGetLastError());
        return 1;
    }

    if (!socksLogin(fd))
        return 1;

//    in_addr inaddr = RtlIpv4StringToAddress("xx.xx.xx.xx");
//    if (!socksConnect(fd, inet_addr("xx.xx.xx.xx"), 80))
//        return 1;

    printf("Success!");

    // now send/receive desired data as needed using existing fd ...

    return 0;
}