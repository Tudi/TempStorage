#pragma once

#define MAXBUF          0xFFFF
#define MAX_LINE        65

/*
 * Proxy server configuration.
 */
typedef struct
{
    UINT16 proxy_port;
} PROXY_CONFIG, * PPROXY_CONFIG;

typedef struct
{
    SOCKET s;
    struct sockaddr_in src_addr;
//    struct in_addr dest;
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