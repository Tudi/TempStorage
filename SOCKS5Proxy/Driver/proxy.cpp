#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "proxy.h"
#include "Utils.h"
#include "Socks5Proxy.h"
#include "FlowManager.h"
#include "ConfigHandler.h"
#include "Logger.h"

#pragma comment(lib, "Ws2_32.lib")

static DWORD proxy_connection_handler(LPVOID arg);
static DWORD proxy_transfer_handler(LPVOID arg);

#ifndef USE_NONBLOCKING_SOCKETS
/*
 * Proxy server thread.
 */
SOCKET ProxyListenSocket = NULL;
DWORD proxy(LPVOID arg)
{
    UINT16 proxy_port = GetOurProxyPort();
    int on = 1;
    WSADATA wsa_data;
    WORD wsa_version = MAKEWORD(2, 2);
    struct sockaddr_in addr;
    HANDLE thread;
    AutoMonitorThreadExit AM; //don't delete me

    if (WSAStartup(wsa_version, &wsa_data) != 0)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to start WSA (%d)", GetLastError());
        SetProgramTerminated();
        return -1;
    }

    ProxyListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ProxyListenSocket == INVALID_SOCKET)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create socket (%d)", WSAGetLastError());
        SetProgramTerminated();
        return -1;
    }

    if (setsockopt(ProxyListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == SOCKET_ERROR)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to re-use address (%d)", GetLastError());
        SetProgramTerminated();
        return -1;
    }

    sLog.Log(LL_Info, __FILE__, __LINE__, "Proxy listening on port : %d", proxy_port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(proxy_port);
    if (bind(ProxyListenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to bind socket (%d)", WSAGetLastError());
        SetProgramTerminated();
        return -1;
    }

    if (listen(ProxyListenSocket, 16) == SOCKET_ERROR)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to listen socket (%d)", WSAGetLastError());
        SetProgramTerminated();
        return -1;
    }

    while (IsWaitingForUserExitProgram())
    {
        // Wait for a new connection.
        PPROXY_CONNECTION_CONFIG config;
        int size = sizeof(addr);
        SOCKET t = accept(ProxyListenSocket, (SOCKADDR*)&addr, &size);
        if (t == INVALID_SOCKET)
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "Proxy:failed to accept socket (%d)", WSAGetLastError());
            continue;
        }

        // Spawn proxy connection handler thread.
        config = (PPROXY_CONNECTION_CONFIG)malloc(sizeof(PROXY_CONNECTION_CONFIG));
        if (config == NULL)
        {
            sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to allocate memory");
            SetProgramTerminated();
            return -1;
        }
        config->s = t;
        memcpy(&config->src_addr, &addr, sizeof(addr));
        thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)proxy_connection_handler,(LPVOID)config, 0, NULL);
        if (thread == NULL)
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to create bridge thread (%d)", GetLastError());
            closesocket(t);
            free(config);
            continue;
        }
        CloseHandle(thread);
    }
    return 0;
}

void ShutdownProxyThread()
{
    shutdown(ProxyListenSocket, SD_BOTH);
    closesocket(ProxyListenSocket);
}
/*
 * Proxy connection handler thread.
 */
static DWORD proxy_connection_handler(LPVOID arg)
{
    PPROXY_TRANSFER_CONFIG config1, config2;
    HANDLE thread;
    PPROXY_CONNECTION_CONFIG config = (PPROXY_CONNECTION_CONFIG)arg;
    SOCKET s = config->s, t;
    unsigned short SrcPort = config->src_addr.sin_port;
    free(config);

    t = socket(AF_INET, SOCK_STREAM, 0);
    if (t == INVALID_SOCKET)
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to create bridge socket (%d)", WSAGetLastError());
        closesocket(s);
        return 0;
    }

    //do not spam connections just beacuse we can not set up the tunnel fast enough
    FlowStatusStore* fss = GetFlowStatus(SrcPort);
    if( fss == NULL )
    {
        closesocket(s);
        return 0;
    }

    //create a connection to our SOCKS 5 tunnel
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(fss->RedirectToPort);
        addr.sin_addr.S_un.S_addr = htonl(fss->RedirectToIP);
        if (connect(t, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            char DstIP[50];
            IPv4Tostr(htonl(fss->RedirectToIP), DstIP, sizeof(DstIP));
            sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to connect bridge socket to tunnel %s:%d . Error (%d)", DstIP, fss->RedirectToPort, WSAGetLastError());
            closesocket(s);
            closesocket(t);
            return 0;
        }

        if (!socksLogin(t))
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS 5 Login failed");
            closesocket(s);
            closesocket(t);
            return 0;
        }

        unsigned short OriginalDSTPort = htons(fss->tcp_hdr.DstPort);
        in_addr OriginalIP;
        OriginalIP.S_un.S_addr = fss->ip_hdr.DstAddr;
        if (!socksConnect(t, OriginalIP, OriginalDSTPort))
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "SOCKS 5 connect failed");
            closesocket(s);
            closesocket(t);
            return 0;
        }
    }

    config1 = (PPROXY_TRANSFER_CONFIG)malloc(sizeof(PROXY_TRANSFER_CONFIG));
    config2 = (PPROXY_TRANSFER_CONFIG)malloc(sizeof(PROXY_TRANSFER_CONFIG));
    if (config1 == NULL || config2 == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to allocate memory");
        SetProgramTerminated();
        return -1;
    }
    config1->inbound = FALSE;
    config2->inbound = TRUE;
    config2->t = config1->s = s;
    config2->s = config1->t = t;
    thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)proxy_transfer_handler, (LPVOID)config1, 0, NULL);
    if (thread == NULL)
    {
        sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to create bridge thread (%d)", GetLastError());
        closesocket(s);
        closesocket(t);
        free(config1);
        free(config2);
        return 0;
    }
    proxy_transfer_handler((LPVOID)config2);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    closesocket(s);
    closesocket(t);

//    fss->RedirectStatus = TFRS_ConnectionClosed;
    if (sLog.LogConnections())
    {
        char SrcIP[50], DstIP[50];
        IPv4Tostr(fss->ip_hdr.SrcAddr, SrcIP, sizeof(SrcIP));
        IPv4Tostr(fss->ip_hdr.DstAddr, DstIP, sizeof(DstIP));
        sLog.Log(LL_Connections, __FILE__, __LINE__, "%X Closed connection from %s:%d to %s:%d", fss, SrcIP, htons(fss->tcp_hdr.SrcPort), DstIP, htons(fss->tcp_hdr.DstPort));
    }

    return 0;
}

/*
 * Handle the transfer of data from one socket to another.
 */
static DWORD proxy_transfer_handler(LPVOID arg)
{
    PPROXY_TRANSFER_CONFIG config = (PPROXY_TRANSFER_CONFIG)arg;
    BOOL inbound = config->inbound;
    SOCKET s = config->s, t = config->t;
    char buf[32000];
    int len, len2, i;
    AutoMonitorThreadExit AM; //don't delete me

    free(config);

    while (IsWaitingForUserExitProgram())
    {
        // Read data from s.
        len = recv(s, buf, sizeof(buf), 0);
        if (len == SOCKET_ERROR)
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to recv from socket (%d)", WSAGetLastError());
            shutdown(s, SD_BOTH);
            shutdown(t, SD_BOTH);
            return 0;
        }
        if (len == 0)
        {
            shutdown(s, SD_RECEIVE);
            shutdown(t, SD_SEND);
            return 0;
        }

        // Send data to t.
        for (i = 0; i < len; )
        {
            len2 = send(t, buf + i, len - i, 0);
            if (len2 == SOCKET_ERROR)
            {
                sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to send to socket (%d)", WSAGetLastError());
                shutdown(s, SD_BOTH);
                shutdown(t, SD_BOTH);
                return 0;
            }
            i += len2;
        }
    }

    return 0;
}

#endif