#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "windivert.h"
#include "PacketContentGenerator.h"
#include "StreamInfo.h"
#include "ParseClientToServer.h"
#include "ParseServerToClient.h"

#define MAXBUF  0xFFFF

//https://reqrypt.org/windivert-doc-1.4.html
//https://reqrypt.org/windivert-doc-1.4.html#filter_language
//https://github.com/basil00/Divert/blob/v1.1.8/examples/netfilter/netfilter.c
int __cdecl main(int argc, char **argv)
{
	HANDLE handle;
	INT16 priority = 0;
	unsigned char packet[MAXBUF];
	UINT packet_len;
	WINDIVERT_ADDRESS addr;
	PWINDIVERT_IPHDR ip_header;
	PWINDIVERT_TCPHDR tcp_header;
	const char *err_str;
//	char Filter[255] = "outbound and tcp.PayloadLength > 0";
//	char Filter[255] = "inbound and tcp.PayloadLength > 0";
	char Filter[255] = "tcp.PayloadLength > 0";
	unsigned int ServerIP[4] = { 192, 243, 0, 0 };

	// Divert traffic matching the filter:
	handle = WinDivertOpen(Filter, WINDIVERT_LAYER_NETWORK, priority,	WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_DROP);
	if (handle == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_INVALID_PARAMETER && !WinDivertHelperCheckFilter(Filter, WINDIVERT_LAYER_NETWORK, &err_str, NULL))
		{
			fprintf(stderr, "error: invalid filter \"%s\"\n", err_str);
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "error: failed to open the WinDivert device (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	// Max-out the packet queue:
	if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_LEN, 8192))
	{
		fprintf(stderr, "error: failed to set packet queue length (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}
	if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_TIME, 2048))
	{
		fprintf(stderr, "error: failed to set packet queue time (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	InitContentGenerator();
	InitShowPacketInfo(1);

	FILE *f;
	errno_t opener = fopen_s(&f, "ServerIP.txt", "rt");
	if (f)
	{
		fscanf_s(f, "%d.%d.%d.%d", &ServerIP[0], &ServerIP[1], &ServerIP[2], &ServerIP[3]);
		fclose(f);
	}

	// Main loop:
	while (TRUE)
	{
		// Read a matching packet.
		if (!WinDivertRecv(handle, packet, sizeof(packet), &addr, &packet_len))
		{
			fprintf(stderr, "warning: failed to read packet (%d)\n", GetLastError());
			continue;
		}

		//in case we wish to see the content on console or maybe file
		ShowPacketInfo(addr,packet,packet_len);

		//check what we received
		PWINDIVERT_IPV6HDR ipv6_header;
		PWINDIVERT_ICMPHDR icmp_header;
		PWINDIVERT_ICMPV6HDR icmpv6_header;
		PWINDIVERT_UDPHDR udp_header;
		WinDivertHelperParsePacket(packet, packet_len, &ip_header,	&ipv6_header, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL, NULL);
		if (ip_header == NULL && ipv6_header == NULL)
		{
			fprintf(stderr, "warning: junk packet\n");
		}

		//we only care about our packets
//		if (ip_header == NULL || tcp_header == NULL)
//			continue;

		//filter out IP that we do not wish to see
		//we only care about outgoing packets
		if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND)
		{
			if ((ServerIP[0] == 0 || ((unsigned char*)&ip_header->DstAddr)[0] == ServerIP[0])
				&& (ServerIP[1] == 0 || ((unsigned char*)&ip_header->DstAddr)[1] == ServerIP[1])
				&& (ServerIP[2] == 0 || ((unsigned char*)&ip_header->DstAddr)[2] == ServerIP[2])
				&& (ServerIP[3] == 0 || ((unsigned char*)&ip_header->DstAddr)[3] == ServerIP[3]))
			{
				unsigned int PayloadContentChanged = OnClientToServerPacket(packet, packet_len);
				if (PayloadContentChanged == 1)
				{
					WinDivertHelperCalcChecksums(packet, packet_len, &addr, 0);
				}
			}
		}
		else
		{
			if ((ServerIP[0] == 0 || ((unsigned char*)&ip_header->SrcAddr)[0] == ServerIP[0])
				&& (ServerIP[1] == 0 || ((unsigned char*)&ip_header->SrcAddr)[1] == ServerIP[1])
				&& (ServerIP[2] == 0 || ((unsigned char*)&ip_header->SrcAddr)[2] == ServerIP[2])
				&& (ServerIP[3] == 0 || ((unsigned char*)&ip_header->SrcAddr)[3] == ServerIP[3]))
			{
				OnServerToClientPacket(packet, packet_len);
			}
		}

		//send the packet
		if (!WinDivertSend(handle, packet, sizeof(packet), &addr, &packet_len))
		{
			fprintf(stderr, "warning: failed to put packet back to the send stream (%d)\n", GetLastError());
			continue;
		}
	}

	//cleanup
	WinDivertClose(handle);
	handle = NULL;
}
