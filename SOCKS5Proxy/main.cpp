#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "windivert.h"

#include "proxy.h"
#include "Utils.h"
#include "RedirectHistory.h"
#include "ConfigHandler.h"

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
    UINT16 port, proxy_port;
    int r;
    char filter[256];
    INT16 priority = 123;       // Arbitrary.
    PPROXY_CONFIG config;
    unsigned char packet[MAXBUF];
    UINT packet_len;
    WINDIVERT_ADDRESS addr;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_TCPHDR tcp_header;

    port = 80;
    if (port < 0 || port > 0xFFFF)
    {
        fprintf(stderr, "error: invalid port number (%d)\n", port);
        exit(EXIT_FAILURE);
    }
    proxy_port = GetOurProxyPort();
    lock = CreateMutex(NULL, FALSE, NULL);
    if (lock == NULL)
    {
        fprintf(stderr, "error: failed to create mutex (%d)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    // Divert all traffic to/from `port', `proxy_port' and `alt_port'.
    r = snprintf(filter, sizeof(filter),
        "tcp and "
        "(tcp.DstPort == %d or tcp.DstPort == %d or "
        "tcp.SrcPort == %d or tcp.SrcPort == %d )",
        port, proxy_port, port, proxy_port);
 //   r = snprintf(filter, sizeof(filter),
 //       "outbound and tcp.DstPort != %d and tcp.DstPort != %d and tcp.DstPort = 80",
 //       GetOurProxyPort(), GetSOCKSTunnelPort());
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
//            if (SkipRedirectOnOutboundDestPort(tcp_header->DstPort) == 0)
            {
                AddRedirection(tcp_header, ip_header);
                if (tcp_header->DstPort == htons(port))
                {
                    // Reflect: PORT ---> PROXY
#ifdef _DEBUG
                    unsigned short OriginalSrcPort = htons(tcp_header->SrcPort);
                    unsigned short OriginalDestPort = htons(tcp_header->DstPort);
#endif
                    UINT32 dst_addr = ip_header->DstAddr;
                    tcp_header->DstPort = htons(GetOurProxyPort());
                    ip_header->DstAddr = ip_header->SrcAddr; // redirect back to the network adapter it came from ?
//                    ip_header->DstAddr = htonl(GetOurProxyIP()); // redirect back to the network adapter it came from ?
                    ip_header->SrcAddr = dst_addr;
                    addr.Outbound = FALSE;
#ifdef _DEBUG
                    printf("redirecting from %d to proxy port : %d\n", port, proxy_port);
                    printf("\t Destination port %d, src port %d\n", OriginalDestPort, OriginalSrcPort);
#endif
                }
                else if (tcp_header->SrcPort == htons(proxy_port))
                {
                    // Reflect: PROXY ---> PORT
#ifdef _DEBUG
                    unsigned short OriginalSrcPort = htons(tcp_header->SrcPort);
                    unsigned short OriginalDestPort = htons(tcp_header->DstPort);
#endif
                    unsigned short OriginalSRCPort = GetRedirectedPacketOriginalPort(tcp_header->DstPort);
                    UINT32 dst_addr = ip_header->DstAddr;
                    tcp_header->SrcPort = OriginalSRCPort;
                    ip_header->DstAddr = ip_header->SrcAddr;
                    ip_header->SrcAddr = dst_addr;
                    addr.Outbound = FALSE;
#ifdef _DEBUG
                    printf("redirecting from proxy %d to %d\n", proxy_port, port);
                    printf("\t Destination port(original src) %d, src port(original dest) %d\n", OriginalDestPort, OriginalSRCPort);
#endif
                }
                /**/
            }
/*            else if (tcp_header->DstPort == htons(alt_port))
            {
                // Redirect: ALT ---> PORT
                tcp_header->DstPort = htons(port);
            }*/
        }
/*        else
        {
            if (tcp_header->SrcPort == htons(port))
            {
                // Redirect: PORT ---> ALT
                tcp_header->SrcPort = htons(alt_port);
            }
        }*/

        WinDivertHelperCalcChecksums(packet, packet_len, &addr, 0);
        if (!WinDivertSend(handle, packet, packet_len, NULL, &addr))
        {
            warning("failed to send packet (%d)", GetLastError());
            continue;
        }
    }

    return 0;
}
