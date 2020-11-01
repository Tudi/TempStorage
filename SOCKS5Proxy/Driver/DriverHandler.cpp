#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "WinDivert/windivert.h"
#include "proxy.h"
#include "ConfigHandler.h"
#include "Utils.h"
#include "RulesManager.h"
#include "FlowManager.h"
#include "Logger.h"

HANDLE DriverHandle = NULL;
void SetupDriver()
{
    HANDLE thread;
    int r;
    char filter[256];
    INT16 priority = 123;       // Arbitrary.

    r = snprintf(filter, sizeof(filter),
//        "outbound and ip and (tcp.DstPort = 80 or tcp.DstPort = 3306 or tcp.DstPort = 81 or tcp.DstPort = 443 or tcp.SrcPort = 5557)");/**/
//        "outbound and (tcp.DstPort = 80 or tcp.SrcPort = 5557)");//good
//        "outbound and (tcp.DstPort = 80 or tcp.SrcPort = 5557 or tcp.DstPort = 5557)");//good
//          "outbound and (tcp.DstPort = 80 or tcp.SrcPort = 5557 or tcp.DstPort = 5557 or tcp.SrcPort = 5556)");//bad
//          "outbound and (tcp.DstPort = 80 or tcp.SrcPort = 5557 or tcp.DstPort = 5557 or tcp.DstPort = 5556)");//good but slow ? worked once ? maybe browser cache ?
//    "outbound and (tcp.DstPort = 80 or tcp.SrcPort = 5557 or tcp.DstPort = 5557 or tcp.SrcPort = 5556 or tcp.DstPort = 5556)");//bad
//    "outbound and tcp and ip and (tcp.DstPort != 5557 and tcp.SrcPort != 5556 and tcp.DstPort != 5556)");//very slow due to program name search
    "outbound and tcp and ip");//very slow due to program name search
//    "outbound and tcp and ip and tcp.DstPort != 443");//very slow due to program name search
//        "outbound and ip");/**/
    if (r < 0 || r >= sizeof(filter))
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "Failed to create filter string");
        SetProgramTerminated();
        return;
    }
    DriverHandle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, priority, 0);
    if (DriverHandle == INVALID_HANDLE_VALUE)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "Failed to open the WinDivert device (%d)", GetLastError());
        SetProgramTerminated();
        return;
    }

    // Spawn proxy thread,
    thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)proxy, (LPVOID)NULL, 0, NULL);
    if (thread == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "Failed to create thread (%d)", GetLastError());
        SetProgramTerminated();
        return;
    }
    CloseHandle(thread);
}

DWORD RedirectLoopThread(LPVOID arg)
{
    unsigned char packet[MAXBUF];
    UINT packet_len;
    WINDIVERT_ADDRESS addr;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_IPV6HDR ip_header_v6;
    PWINDIVERT_TCPHDR tcp_header;
    PWINDIVERT_UDPHDR udp_header;
    PWINDIVERT_ICMPHDR icmp_header;
    PWINDIVERT_ICMPV6HDR icmp_header_v6;
    UINT16 proxy_port = GetOurProxyPort();
    //seems like recv will not time out and this thread will be locking application exit
 //   AutoMonitorThreadExit AM; //don't delete me

    // Main loop:
    while (IsWaitingForUserExitProgram())
    {
        if (!WinDivertRecv(DriverHandle, packet, sizeof(packet), &packet_len, &addr))
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to read packet (%d)", GetLastError());
            continue;
        }

        if (IsRedirectionEnabled() == 1)
        {
            WinDivertHelperParsePacket(packet, packet_len, &ip_header, &ip_header_v6, NULL,
                &icmp_header, &icmp_header_v6, &tcp_header, &udp_header, NULL, NULL, NULL, NULL);

            if (IsTrafficFlowSelectedForRedirection(tcp_header, ip_header) == TFRS_Redirected)
            {
                if (tcp_header->SrcPort == htons(proxy_port))
                {
#define SHOW_REDIRECTS
                    // Reflect: PROXY ---> PORT
#if defined( _DEBUG ) || defined(SHOW_REDIRECTS)
                    unsigned short OriginalSrcPort = htons(tcp_header->SrcPort);
                    unsigned short OriginalDestPort = htons(tcp_header->DstPort);
#endif
                    FlowStatusStore* fss = GetFlowStatus(tcp_header->DstPort);
                    unsigned int dst_addr = ip_header->DstAddr;
                    tcp_header->SrcPort = fss->tcp_hdr.DstPort;
                    ip_header->DstAddr = ip_header->SrcAddr;
                    ip_header->SrcAddr = dst_addr;
                    addr.Outbound = FALSE;
#if defined( _DEBUG ) || defined(SHOW_REDIRECTS)
                    if (sLog.LogTraffic())
                    {
                        sLog.Log(LL_Traffic, __FILE__, __LINE__, "%X redirecting from proxy %d to %d, original dest %d\n", fss, OriginalSrcPort, OriginalDestPort, htons(fss->tcp_hdr.DstPort));
                    }
#endif
                }
                else
                    //                if (tcp_header->DstPort == htons(port))
                {
                    // Reflect: PORT ---> PROXY
#if defined( _DEBUG ) || defined(SHOW_REDIRECTS)
                    unsigned short OriginalSrcPort = htons(tcp_header->SrcPort);
                    unsigned short OriginalDestPort = htons(tcp_header->DstPort);
#endif
                    unsigned int dst_addr = ip_header->DstAddr;
                    tcp_header->DstPort = htons(GetOurProxyPort());
                    ip_header->DstAddr = ip_header->SrcAddr; // redirect back to the network adapter it came from ?
                    ip_header->SrcAddr = dst_addr;
                    addr.Outbound = FALSE;
#if defined( _DEBUG ) || defined(SHOW_REDIRECTS)
                    if (sLog.LogTraffic())
                    {
                        FlowStatusStore* fs = GetFlowStatus(htons(OriginalSrcPort));
                        sLog.Log(LL_Traffic, __FILE__, __LINE__, "%X redirecting from %d to proxy port %d. Original dest %d\n", fs, OriginalSrcPort, proxy_port, OriginalDestPort);
                    }
#endif
                }
                WinDivertHelperCalcChecksums(packet, packet_len, &addr, 0);
            }
#if defined( _DEBUG ) || defined(SHOW_REDIRECTS)
            else
            {
                if (sLog.LogTraffic())
                {
                    int OriginalSrcPort = htons(tcp_header->SrcPort);
                    int OriginalDestPort = htons(tcp_header->DstPort);
                    FlowStatusStore* fs = GetFlowStatus(tcp_header, ip_header, 1);
                    sLog.Log(LL_Traffic, __FILE__, __LINE__, "%X passthrough from %d to %d\n", fs, OriginalSrcPort, OriginalDestPort);
                }
            }
#endif
        }
#if defined( _DEBUG ) || defined(SHOW_REDIRECTS)
        else
        {
            if (sLog.LogTraffic())
            {
                WinDivertHelperParsePacket(packet, packet_len, &ip_header, &ip_header_v6, NULL,
                    &icmp_header, &icmp_header_v6, &tcp_header, &udp_header, NULL, NULL, NULL, NULL);
                int OriginalSrcPort = htons(tcp_header->SrcPort);
                int OriginalDestPort = htons(tcp_header->DstPort);
                sLog.Log(LL_Traffic, __FILE__, __LINE__, "passthrough from %d to %d\n", OriginalSrcPort, proxy_port, OriginalDestPort);
            }
        }
#endif
        if (!WinDivertSend(DriverHandle, packet, packet_len, NULL, &addr))
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to send packet (%d)", GetLastError());
            continue;
        }
    }
    return 0;
}

void StartRedirectLoop()
{
    HANDLE thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)RedirectLoopThread, (LPVOID)NULL, 0, NULL);
    if (thread == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create redirectloop thread (%d)", GetLastError());
        SetProgramTerminated();
        return;
    }
    CloseHandle(thread);
}