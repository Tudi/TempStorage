#pragma once

/*********************************************
* Minimalistic SOCKS 5 client. It will initiate a session between our proxy and the tunnel
* After connection was established, all packets are simply bridged from source to the tunnel
*********************************************/

#include <WinSock2.h>
#include <in6addr.h>

//Does not actually support username ased login. It just performs the formal login
bool socksLogin(SOCKET fd);
//tell the tunnel where to connect on the other side
bool socksConnect(SOCKET fd, const in_addr& dest, unsigned short port);
//receive a "packet" worth of data
int recvData(SOCKET fd, void* data, int len, bool disconnectOk = false);
bool sendData(SOCKET fd, void* data, int len);

#pragma pack(push, 1)
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
#pragma pack(pop)