#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "proxy.h"
#include "Utils.h"
#include "Socks5Proxy.h"
#include "RedirectHistory.h"
#include "ConfigHandler.h"

#pragma comment(lib, "Ws2_32.lib")

/*
 * Proxy server thread.
 */
DWORD proxy(LPVOID arg)
{
    PPROXY_CONFIG config = (PPROXY_CONFIG)arg;
    UINT16 proxy_port = config->proxy_port;
    int on = 1;
    WSADATA wsa_data;
    WORD wsa_version = MAKEWORD(2, 2);
    struct sockaddr_in addr;
    SOCKET s;
    HANDLE thread;

    free(config);

    if (WSAStartup(wsa_version, &wsa_data) != 0)
    {
        error("failed to start WSA (%d)", GetLastError());
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        error("failed to create socket (%d)", WSAGetLastError());
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == SOCKET_ERROR)
    {
        error("failed to re-use address (%d)", GetLastError());
    }

#ifdef _DEBUG
    printf("Our proxy is listening on port : %d\n", proxy_port);
#endif

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(proxy_port);
    if (bind(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        error("failed to bind socket (%d)", WSAGetLastError());
    }

    if (listen(s, 16) == SOCKET_ERROR)
    {
        error("failed to listen socket (%d)", WSAGetLastError());
    }

    while (TRUE)
    {
        // Wait for a new connection.
        PPROXY_CONNECTION_CONFIG config;
        int size = sizeof(addr);
        SOCKET t = accept(s, (SOCKADDR*)&addr, &size);
        if (t == INVALID_SOCKET)
        {
            warning("failed to accept socket (%d)", WSAGetLastError());
            continue;
        }

        // Spawn proxy connection handler thread.
        config = (PPROXY_CONNECTION_CONFIG)malloc(sizeof(PROXY_CONNECTION_CONFIG));
        if (config == NULL)
        {
            error("failed to allocate memory");
        }
        config->s = t;
//        config->dest = addr.sin_addr;
        memcpy(&config->src_addr, &addr, sizeof(addr));
        thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)proxy_connection_handler,(LPVOID)config, 0, NULL);
        if (thread == NULL)
        {
            warning("failed to create thread (%d)", GetLastError());
            closesocket(t);
            free(config);
            continue;
        }
        CloseHandle(thread);
    }
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
        warning("failed to create socket (%d)", WSAGetLastError());
        closesocket(s);
        return 0;
    }

    //create a connection to our SOCKS 5 tunnel
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(GetSOCKSTunnelPort());
        addr.sin_addr.S_un.S_addr = htonl(GetSOCKSTunnelIP());
        if (connect(t, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            warning("failed to connect socket (%d)", WSAGetLastError());
            closesocket(s);
            closesocket(t);
            return 0;
        }

        if (!socksLogin(t))
        {
            printf("SOCKS 5 Login failed\n");
            closesocket(s);
            closesocket(t);
            return 0;
        }

        unsigned short OriginalDSTPort = htons(GetRedirectedPacketOriginalPort(SrcPort));
        in_addr OriginalIP;
        OriginalIP.S_un.S_addr = htonl(GetRedirectedPacketOriginalIP(SrcPort));
        if (!socksConnect(t, OriginalIP, OriginalDSTPort))
        {
            printf("SOCKS 5 connect failed\n");
            closesocket(s);
            closesocket(t);
            return 0;
        }
    }

    config1 = (PPROXY_TRANSFER_CONFIG)malloc(sizeof(PROXY_TRANSFER_CONFIG));
    config2 = (PPROXY_TRANSFER_CONFIG)malloc(sizeof(PROXY_TRANSFER_CONFIG));
    if (config1 == NULL || config2 == NULL)
    {
        error("failed to allocate memory");
    }
    config1->inbound = FALSE;
    config2->inbound = TRUE;
    config2->t = config1->s = s;
    config2->s = config1->t = t;
    thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)proxy_transfer_handler, (LPVOID)config1, 0, NULL);
    if (thread == NULL)
    {
        warning("failed to create thread (%d)", GetLastError());
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
    char buf[8192];
    int len, len2, i;

    free(config);

    while (TRUE)
    {
        // Read data from s.
        len = recv(s, buf, sizeof(buf), 0);
        if (len == SOCKET_ERROR)
        {
            warning("failed to recv from socket (%d)", WSAGetLastError());
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

#ifdef PRINT_PACKET_CONTENT
        // Dump stream information to the screen.
        HANDLE console;
        if (lock == NULL)
            lock = CreateMutex(NULL, FALSE, NULL);
        if (lock == NULL)
        {
            fprintf(stderr, "error: failed to create mutex (%d)\n", GetLastError());
            exit(EXIT_FAILURE);
        }
        console = GetStdHandle(STD_OUTPUT_HANDLE);
        WaitForSingleObject(lock, INFINITE);
        printf("[%.4d] ", len);
        SetConsoleTextAttribute(console, (inbound ? FOREGROUND_RED : FOREGROUND_GREEN));
        for (i = 0; i < len && i < MAX_LINE; i++)
        {
            putchar((isprint(buf[i]) ? buf[i] : '.'));
        }
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("%s\n", (len > MAX_LINE ? "..." : ""));
        ReleaseMutex(lock);
#endif
        // Send data to t.
        for (i = 0; i < len; )
        {
            len2 = send(t, buf + i, len - i, 0);
            if (len2 == SOCKET_ERROR)
            {
                warning("failed to send to socket (%d)", WSAGetLastError());
                shutdown(s, SD_BOTH);
                shutdown(t, SD_BOTH);
                return 0;
            }
            i += len2;
        }
    }

    return 0;
}
