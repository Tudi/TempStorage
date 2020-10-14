#pragma once

#define MAXBUF          0xFFFF
#define PROXY_PORT      5557 // this is my local proxy that should convert any connection to socks5 connection
#define ALT_PORT        5558 // this is your SOCKS 5 local tunnel entrance
#define MAX_LINE        65

/*
 * Proxy server configuration.
 */
typedef struct
{
    UINT16 proxy_port;
    UINT16 alt_port;
} PROXY_CONFIG, * PPROXY_CONFIG;

typedef struct
{
    SOCKET s;
    UINT16 alt_port;
    struct in_addr dest;
} PROXY_CONNECTION_CONFIG, * PPROXY_CONNECTION_CONFIG;

typedef struct
{
    BOOL inbound;
    SOCKET s;
    SOCKET t;
} PROXY_TRANSFER_CONFIG, * PPROXY_TRANSFER_CONFIG;

DWORD proxy(LPVOID arg);
static DWORD proxy_connection_handler(LPVOID arg);
static DWORD proxy_transfer_handler(LPVOID arg);