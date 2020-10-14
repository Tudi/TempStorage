#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "windivert.h"

#include "proxy.h"
#include "Utils.h"

/*
 * Lock to sync output.
 */
HANDLE lock;

/*
 * Entry.
 */
int __cdecl main(int argc, char** argv)
{
    HANDLE handle, thread;
    UINT16 port, proxy_port, alt_port;
    int r;
    char filter[256];
    INT16 priority = 123;       // Arbitrary.
    PPROXY_CONFIG config;
    unsigned char packet[MAXBUF];
    UINT packet_len;
    WINDIVERT_ADDRESS addr;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_TCPHDR tcp_header;
 //   DWORD len;

    // Init.
/*    if (argc != 2)
    {
        fprintf(stderr, "usage: %s dest-port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    port = (UINT16)atoi(argv[1]);*/
    port = 80;
    if (port < 0 || port > 0xFFFF)
    {
        fprintf(stderr, "error: invalid port number (%d)\n", port);
        exit(EXIT_FAILURE);
    }
    proxy_port = (port == PROXY_PORT ? PROXY_PORT + 1 : PROXY_PORT);
    alt_port = (port == ALT_PORT ? ALT_PORT + 1 : ALT_PORT);
    lock = CreateMutex(NULL, FALSE, NULL);
    if (lock == NULL)
    {
        fprintf(stderr, "error: failed to create mutex (%d)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    // Divert all traffic to/from `port', `proxy_port' and `alt_port'.
    r = snprintf(filter, sizeof(filter),
        "tcp and "
        "(tcp.DstPort == %d or tcp.DstPort == %d or tcp.DstPort == %d or "
        "tcp.SrcPort == %d or tcp.SrcPort == %d or tcp.SrcPort == %d)",
        port, proxy_port, alt_port, port, proxy_port, alt_port);
    if (r < 0 || r >= sizeof(filter))
    {
        error("failed to create filter string");
    }
    handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, priority, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
        error("failed to open the WinDivert device (%d)", GetLastError());
    }

    // Spawn proxy thread,
    config = (PPROXY_CONFIG)malloc(sizeof(PROXY_CONFIG));
    if (config == NULL)
    {
        error("failed to allocate memory");
    }
    config->proxy_port = proxy_port;
    config->alt_port = alt_port;
    thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)proxy,(LPVOID)config, 0, NULL);
    if (thread == NULL)
    {
        error("failed to create thread (%d)", GetLastError());
    }
    CloseHandle(thread);

    // Main loop:
    while (TRUE)
    {
        if (!WinDivertRecv(handle, packet, sizeof(packet), &packet_len, &addr))
        {
            warning("failed to read packet (%d)", GetLastError());
            continue;
        }

        WinDivertHelperParsePacket(packet, packet_len, &ip_header, NULL, NULL,
            NULL, NULL, &tcp_header, NULL, NULL, NULL, NULL, NULL);
        if (ip_header == NULL || tcp_header == NULL)
        {
            warning("failed to parse packet (%d)", GetLastError());
            continue;
        }

        if (addr.Outbound)
        {
            if (tcp_header->DstPort == htons(port))
            {
                // Reflect: PORT ---> PROXY
                UINT32 dst_addr = ip_header->DstAddr;
                tcp_header->DstPort = htons(proxy_port);
                ip_header->DstAddr = ip_header->SrcAddr;
                ip_header->SrcAddr = dst_addr;
                addr.Outbound = FALSE;
                printf("redirecting from %d to proxy port : %d\n", port, proxy_port);
            }
            else if (tcp_header->SrcPort == htons(proxy_port))
            {
                // Reflect: PROXY ---> PORT
                UINT32 dst_addr = ip_header->DstAddr;
                tcp_header->SrcPort = htons(port);
                ip_header->DstAddr = ip_header->SrcAddr;
                ip_header->SrcAddr = dst_addr;
                addr.Outbound = FALSE;
                printf("redirecting from proxy %d to %d\n", proxy_port, port);
            }
            else if (tcp_header->DstPort == htons(alt_port))
            {
                // Redirect: ALT ---> PORT
                tcp_header->DstPort = htons(port);
            }
        }
        else
        {
            if (tcp_header->SrcPort == htons(port))
            {
                // Redirect: PORT ---> ALT
                tcp_header->SrcPort = htons(alt_port);
            }
        }

        WinDivertHelperCalcChecksums(packet, packet_len, &addr, 0);
        if (!WinDivertSend(handle, packet, packet_len, NULL, &addr))
        {
            warning("failed to send packet (%d)", GetLastError());
            continue;
        }
    }

    return 0;
}
