#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "windivert.h"

#define MAXBUF  0xFFFF

//https://reqrypt.org/windivert-doc-1.4.html
//https://reqrypt.org/windivert-doc-1.4.html#filter_language
//https://github.com/basil00/Divert/blob/v1.1.8/examples/netfilter/netfilter.c
int __cdecl main(int argc, char **argv)
{
	HANDLE handle, console;
	UINT i;
	INT16 priority = 0;
	unsigned char packet[MAXBUF];
	UINT packet_len;
	WINDIVERT_ADDRESS addr;
	PWINDIVERT_IPHDR ip_header;
	PWINDIVERT_TCPHDR tcp_header;
	const char *err_str;
	LARGE_INTEGER base, freq;
	double time_passed;
//	char Filter[255] = "outbound and tcp.PayloadLength > 0";
	char Filter[255] = "inbound and tcp.PayloadLength > 0";

	// Get console for pretty colors.
	console = GetStdHandle(STD_OUTPUT_HANDLE);

	// Divert traffic matching the filter:
	handle = WinDivertOpen(Filter, WINDIVERT_LAYER_NETWORK, priority,	WINDIVERT_FLAG_SNIFF);
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

	// Set up timing:
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&base);

	// Main loop:
	while (TRUE)
	{
		// Read a matching packet.
		if (!WinDivertRecv(handle, packet, sizeof(packet), &addr, &packet_len))
		{
			fprintf(stderr, "warning: failed to read packet (%d)\n", GetLastError());
			continue;
		}

		// Print info about the matching packet.
		PWINDIVERT_IPV6HDR ipv6_header;
		PWINDIVERT_ICMPHDR icmp_header;
		PWINDIVERT_ICMPV6HDR icmpv6_header;
		PWINDIVERT_UDPHDR udp_header;
		WinDivertHelperParsePacket(packet, packet_len, &ip_header,	&ipv6_header, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL, NULL);
		if (ip_header == NULL && ipv6_header == NULL)
		{
			fprintf(stderr, "warning: junk packet\n");
		}
/*
		//we only care about our packets
		if (ip_header == NULL || tcp_header == NULL)
			continue;

		//we only care about outgoing packets
		if (addr.Direction != WINDIVERT_DIRECTION_OUTBOUND)
			continue;
			*/
		// Dump packet info: 
		putchar('\n');
		SetConsoleTextAttribute(console, FOREGROUND_RED);
		time_passed = (double)(addr.Timestamp - base.QuadPart) / (double)freq.QuadPart;
		printf("Packet [Timestamp=%.8g, Direction=%s IfIdx=%u SubIfIdx=%u Loopback=%u]\n", time_passed, (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND ? "outbound" : "inbound"), addr.IfIdx, addr.SubIfIdx, addr.Loopback);
		if (ip_header != NULL)
		{
			UINT8 *src_addr = (UINT8 *)&ip_header->SrcAddr;
			UINT8 *dst_addr = (UINT8 *)&ip_header->DstAddr;
			SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED);
			printf("IPv4 [Version=%u HdrLength=%u TOS=%u Length=%u Id=0x%.4X Reserved=%u DF=%u MF=%u FragOff=%u TTL=%u Protocol=%u Checksum=0x%.4X SrcAddr=%u.%u.%u.%u DstAddr=%u.%u.%u.%u]\n",
				ip_header->Version, ip_header->HdrLength, ntohs(ip_header->TOS), ntohs(ip_header->Length), ntohs(ip_header->Id), WINDIVERT_IPHDR_GET_RESERVED(ip_header),
				WINDIVERT_IPHDR_GET_DF(ip_header), WINDIVERT_IPHDR_GET_MF(ip_header), ntohs(WINDIVERT_IPHDR_GET_FRAGOFF(ip_header)), ip_header->TTL,
				ip_header->Protocol, ntohs(ip_header->Checksum), src_addr[0], src_addr[1], src_addr[2], src_addr[3], dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3]);
		}
/*
		if (ipv6_header != NULL)
		{
			UINT16 *src_addr = (UINT16 *)&ipv6_header->SrcAddr;
			UINT16 *dst_addr = (UINT16 *)&ipv6_header->DstAddr;
			SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED);
			printf("IPv6 [Version=%u TrafficClass=%u FlowLabel=%u Length=%u NextHdr=%u HopLimit=%u SrcAddr=",
				ipv6_header->Version,
				WINDIVERT_IPV6HDR_GET_TRAFFICCLASS(ipv6_header),
				ntohl(WINDIVERT_IPV6HDR_GET_FLOWLABEL(ipv6_header)),
				ntohs(ipv6_header->Length), ipv6_header->NextHdr,
				ipv6_header->HopLimit);
			for (i = 0; i < 8; i++)
			{
				printf("%x%c", ntohs(src_addr[i]), (i == 7 ? ' ' : ':'));
			}
			fputs("DstAddr=", stdout);
			for (i = 0; i < 8; i++)
			{
				printf("%x", ntohs(dst_addr[i]));
				if (i != 7)
				{
					putchar(':');
				}
			}
			fputs("]\n", stdout);
		}
		if (icmp_header != NULL)
		{
			SetConsoleTextAttribute(console, FOREGROUND_RED);
			printf("ICMP [Type=%u Code=%u Checksum=0x%.4X Body=0x%.8X]\n",
				icmp_header->Type, icmp_header->Code,
				ntohs(icmp_header->Checksum), ntohl(icmp_header->Body));
		}
		if (icmpv6_header != NULL)
		{
			SetConsoleTextAttribute(console, FOREGROUND_RED);
			printf("ICMPV6 [Type=%u Code=%u Checksum=0x%.4X Body=0x%.8X]\n",
				icmpv6_header->Type, icmpv6_header->Code,
				ntohs(icmpv6_header->Checksum), ntohl(icmpv6_header->Body));
		}
		if (udp_header != NULL)
		{
			SetConsoleTextAttribute(console, FOREGROUND_GREEN);
			printf("UDP [SrcPort=%u DstPort=%u Length=%u Checksum=0x%.4X]\n", ntohs(udp_header->SrcPort), ntohs(udp_header->DstPort), ntohs(udp_header->Length), ntohs(udp_header->Checksum));
		}
*/
		//we only care about TCP packets
		if (tcp_header != NULL)
		{
			SetConsoleTextAttribute(console, FOREGROUND_GREEN);
			printf("TCP [SrcPort=%u DstPort=%u SeqNum=%u AckNum=%u HdrLength=%u Reserved1=%u Reserved2=%u Urg=%u Ack=%u Psh=%u Rst=%u Syn=%u Fin=%u Window=%u Checksum=0x%.4X UrgPtr=%u]\n",
				ntohs(tcp_header->SrcPort), ntohs(tcp_header->DstPort),
				ntohl(tcp_header->SeqNum), ntohl(tcp_header->AckNum),
				tcp_header->HdrLength, tcp_header->Reserved1,
				tcp_header->Reserved2, tcp_header->Urg, tcp_header->Ack,
				tcp_header->Psh, tcp_header->Rst, tcp_header->Syn,
				tcp_header->Fin, ntohs(tcp_header->Window),
				ntohs(tcp_header->Checksum), ntohs(tcp_header->UrgPtr));
		}

		//print the content of the packet
		{
			SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE);
			for (i = 0; i < packet_len; i++)
			{
				if (i % 20 == 0)
					printf("\n\t");
				printf("%.2X", (UINT8)packet[i]);
			}
			SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_BLUE);
			for (i = 0; i < packet_len; i++)
			{
				if (i % 40 == 0)
					printf("\n\t");
				if (isprint(packet[i]))
					putchar(packet[i]);
				else
					putchar('.');
			}
			putchar('\n');
		}

		//modify the packet
		if(tcp_header->SrcPort == 8081)
		{
			//the data in the packet starts at 
			char *Payload = ((char *)tcp_header) + tcp_header->HdrLength * 4;
			//chek if the packet contains data we are looking for
			char *ContentPos = strstr(Payload, "lightweight");
			if (ContentPos != NULL)
			{
				strcpy(ContentPos, "RESEACRH");
				WinDivertHelperCalcChecksums(packet, packet_len, &addr, 0);
			}
		}
		//send the packet
		SetConsoleTextAttribute(console,FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	//cleanup
	WinDivertClose(handle);
	handle = NULL;
}
