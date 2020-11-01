#pragma once

/*********************************************
* Routed packets will first end up in our proxy. Our proxy will create a SOCKS 5 session for the 
* routed packets. After the session, all packets are simply bridged from the source to the SOCKS 5 tunnel
*********************************************/

#define MAXBUF          0xFFFF
#define MAX_LINE        65

typedef struct
{
    SOCKET s;
    struct sockaddr_in src_addr;
} PROXY_CONNECTION_CONFIG, * PPROXY_CONNECTION_CONFIG;

typedef struct
{
    BOOL inbound;
    SOCKET s;
    SOCKET t;
} PROXY_TRANSFER_CONFIG, * PPROXY_TRANSFER_CONFIG;

//Will listen to aour proxy port. All incomming packets will get routed to the SOCKS 5 tunnel
DWORD proxy(LPVOID arg);
//kill the proxy thread
void ShutdownProxyThread();