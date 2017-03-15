#include "CapturePackets.h"
#include <pcap.h>
#include <time.h>

/* 4 bytes IP address */
typedef struct ip_address{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header{
	u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
	u_char  tos;            // Type of service 
	u_short tlen;           // Total length 
	u_short identification; // Identification
	u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
	u_char  ttl;            // Time to live
	u_char  proto;          // Protocol
	u_short crc;            // Header checksum
	ip_address  saddr;      // Source address
	ip_address  daddr;      // Destination address
	u_int   op_pad;         // Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header{
	u_short sport;          // Source port
	u_short dport;          // Destination port
	u_short len;            // Datagram length
	u_short crc;            // Checksum
}udp_header;

typedef struct ip_hdr
{
	unsigned char ip_header_len : 4; // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
	unsigned char ip_version : 4; // 4-bit IPv4 version
	unsigned char ip_tos; // IP type of service
	unsigned short ip_total_length; // Total length
	unsigned short ip_id; // Unique identifier

	unsigned char ip_frag_offset : 5; // Fragment offset field

	unsigned char ip_more_fragment : 1;
	unsigned char ip_dont_fragment : 1;
	unsigned char ip_reserved_zero : 1;

	unsigned char ip_frag_offset1; //fragment offset

	unsigned char ip_ttl; // Time to live
	unsigned char ip_protocol; // Protocol(TCP,UDP etc)
	unsigned short ip_checksum; // IP checksum
	unsigned int ip_srcaddr; // Source address
	unsigned int ip_destaddr; // Source address
} IPV4_HDR;

// TCP header
typedef struct tcp_header
{
	unsigned short source_port; // source port
	unsigned short dest_port; // destination port
	unsigned int sequence; // sequence number - 32 bits
	unsigned int acknowledge; // acknowledgement number - 32 bits

	unsigned char ns : 1; //Nonce Sum Flag Added in RFC 3540.
	unsigned char reserved_part1 : 3; //according to rfc
	unsigned char data_offset : 4; /*The number of 32-bit words in the TCP header.
								   This indicates where the data begins.
								   The length of the TCP header is always a multiple
								   of 32 bits.*/

	unsigned char fin : 1; //Finish Flag
	unsigned char syn : 1; //Synchronise Flag
	unsigned char rst : 1; //Reset Flag
	unsigned char psh : 1; //Push Flag
	unsigned char ack : 1; //Acknowledgement Flag
	unsigned char urg : 1; //Urgent Flag

	unsigned char ecn : 1; //ECN-Echo Flag
	unsigned char cwr : 1; //Congestion Window Reduced Flag

	////////////////////////////////

	unsigned short window; // window
	unsigned short checksum; // checksum
	unsigned short urgent_pointer; // urgent pointer
} TCP_HDR;


#ifdef WIN32
unsigned short ntohs1(const unsigned short net)
{
	unsigned char data[2] = {};
	memcpy(&data, &net, sizeof(data));

	return ((unsigned int)data[1] << 0)
		| ((unsigned int)data[0] << 8);
}

unsigned int ntohl1(unsigned int const net) {
	unsigned char data[4] = {};
	memcpy(&data, &net, sizeof(data));

	return ((unsigned int)data[3] << 0)
		| ((unsigned int)data[2] << 8)
		| ((unsigned int)data[1] << 16)
		| ((unsigned int)data[0] << 24);
}

#endif

FILE *FCONTENT = NULL;
void DumpContent(unsigned char *data, unsigned int size, unsigned int off1, unsigned int off2)
{
	if (FCONTENT == NULL)
		errno_t er = fopen_s(&FCONTENT, "p_good", "wb");
	// might need to reassamble segmented packets later. TCP is a bytestream. The beggining of the packet should be a number indicating how much we need to read until the next packet
	if (FCONTENT)
	{
//		unsigned int curtime = time(NULL);
		//		fwrite(&off1, 1, sizeof(off1), FCONTENT);
		//		fwrite(&off2, 1, sizeof(off2), FCONTENT);
		//		fwrite(&curtime, 1, sizeof(curtime), FCONTENT);
		//		fwrite(&size, 1, sizeof(size), FCONTENT);
		fwrite(data, 1, size, FCONTENT);
		fflush(FCONTENT);
	}
}

#define PROTOCOL_TCPIP	6
// Callback function invoked by libpcap for every incoming packet 
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	//	struct tm ltime;
	//	char timestr[16];
	ip_hdr *ih;
	u_int ip_len;
	//	time_t local_tv_sec;

	//Unused variable
	(VOID)(param);

	// convert the timestamp to readable format
	//	local_tv_sec = header->ts.tv_sec;
	//	localtime_s(&ltime, &local_tv_sec);
	//	strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);

	// print timestamp and length of the packet 
	//	printf("%s.%.6d len:%d ", timestr, header->ts.tv_usec, header->len);

	// retireve the position of the ip header
	ih = (ip_hdr *)(pkt_data + 14); //length of ethernet header

	// retireve the position of the udp header 
	ip_len = ih->ip_header_len * 4;

	//capturing all TCP packets from IP
	if (ih->ip_protocol == PROTOCOL_TCPIP
		&& (((unsigned char*)&ih->ip_srcaddr)[0] == 192 && ((unsigned char*)&ih->ip_srcaddr)[1] == 243 && ((unsigned char*)&ih->ip_srcaddr)[2] == 47 && ((unsigned char*)&ih->ip_srcaddr)[3] == 118)
		)
	{
		tcp_header *tcph = (tcp_header *)((u_char*)pkt_data + 14 + ip_len);
		int tcp_len = tcph->data_offset * 4;
		unsigned char *DataStart = (u_char*)pkt_data + 14 + ip_len + tcp_len;
		//		printf("Ack number : %08X, more fragments %d, fo1 %d, fo2 %d\n", tcph->acknowledge, ih->ip_more_fragment, ih->ip_frag_offset, ih->ip_frag_offset1);
		//		printf("Ack number : %08X, len %d, total len %d\n", ntohl1(tcph->acknowledge), header->len, ntohs1(ih->ip_total_length));
		//		printf("Ack number : %08X, len %X, total len %X\n", ntohl1(tcph->acknowledge), header->len, ntohs1(ih->ip_total_length));
		printf("Ack number : %08X, %d, len %d, total len %d\n", tcph->acknowledge, tcph->acknowledge - PrevAck2, header->len, ntohs1(ih->ip_total_length));
		PrevAck2 = tcph->acknowledge;
		int TotalHeaderSize = (unsigned int)(DataStart - pkt_data);
		int BytesToDump = header->len - TotalHeaderSize;
		DumpContent(DataStart, BytesToDump, ih->ip_frag_offset, ih->ip_frag_offset1);
		//		DumpContentFull((unsigned char*)pkt_data, header->len);
		//		if (ih->ip_more_fragment || ih->ip_frag_offset != 0 || ih->ip_frag_offset1 != 0)
		//			DumpContentMaybeFragmented((unsigned char*)pkt_data, header->len);
	}
	/*	else
	{
	udp_header *uh;
	u_short sport, dport;
	uh = (udp_header *)((u_char*)ih + ip_len);

	// convert from network byte order to host byte order
	sport = ntohs1(uh->sport);
	dport = ntohs1(uh->dport);

	// print ip addresses and udp ports
	printf("%d.%d.%d.%d.%d -> %d.%d.%d.%d.%d\n", ih->saddr.byte1, ih->saddr.byte2, ih->saddr.byte3, ih->saddr.byte4, sport, ih->daddr.byte1, ih->daddr.byte2, ih->daddr.byte3, ih->daddr.byte4, dport);
	}/**/
}

int InitCapturePackets()
{
	pcap_if_t           * allAdapters;
	pcap_if_t           * adapter;
	pcap_t           * adapterHandle;
	char                 errorBuffer[PCAP_ERRBUF_SIZE];

	// retrieve the adapters from the computer
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &allAdapters, errorBuffer) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs_ex function: %s\n", errorBuffer);
		return -1;
	}

	// if there are no adapters, print an error
	if (allAdapters == NULL)
	{
		printf("\nNo adapters found! Make sure WinPcap is installed.\n");
		return 0;
	}

	// print the list of adapters along with basic information about an adapter
	int crtAdapter = 0;
	for (adapter = allAdapters; adapter != NULL; adapter = adapter->next)
	{
		printf("\n%d.%s ", ++crtAdapter, adapter->name);
		printf("-- %s\n", adapter->description);
	}

	printf("\n");

	int adapterNumber;

	printf("Enter the adapter number between 1 and %d:", crtAdapter);
	scanf_s("%d", &adapterNumber);

	if (adapterNumber < 1 || adapterNumber > crtAdapter)
	{
		printf("\nAdapter number out of range.\n");

		// Free the adapter list
		pcap_freealldevs(allAdapters);

		return -1;
	}

	// parse the list until we reach the desired adapter
	adapter = allAdapters;
	for (crtAdapter = 0; crtAdapter < adapterNumber - 1; crtAdapter++)
		adapter = adapter->next;

	// open the adapter
	adapterHandle = pcap_open(adapter->name, // name of the adapter
		65536,         // portion of the packet to capture
		// 65536 guarantees that the whole 
		// packet will be captured
		PCAP_OPENFLAG_PROMISCUOUS, // promiscuous mode
		1000,             // read timeout - 1 millisecond
		NULL,          // authentication on the remote machine
		errorBuffer    // error buffer
		);

	if (adapterHandle == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter\n", adapter->name);

		// Free the adapter list
		pcap_freealldevs(allAdapters);

		return -1;
	}

	/* Check the link layer. We support only Ethernet for simplicity. */
	if (pcap_datalink(adapterHandle) != DLT_EN10MB)
	{
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		/* Free the device list */
		pcap_freealldevs(allAdapters);
		return -1;
	}
	printf("\nCapture session started on  adapter %s...\n", adapter->name);

	//create a capture filter
	/*	{
	u_int netmask = 0xffffff;
	char packet_filter[] = "ip.src == 192.243.47.118 && tcp.len > 0";
	struct bpf_program fcode;
	//compile the filter
	if (pcap_compile(adapterHandle, &fcode, packet_filter, 1, netmask) <0)
	{
	fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
	// Free the device list
	pcap_freealldevs(allAdapters);
	return -1;
	}

	//set the filter
	if (pcap_setfilter(adapterHandle, &fcode)<0)
	{
	fprintf(stderr, "\nError setting the filter.\n");
	// Free the device list
	pcap_freealldevs(allAdapters);
	return -1;
	}
	}/**/

	// free the adapter list
	pcap_freealldevs(allAdapters);

	//listen 1 by 1 
	/*	{
	struct pcap_pkthdr * packetHeader;
	const u_char       * packetData;
	// this is the most important part of the application
	// here we start receiving packet traffic
	int retValue;
	while ((retValue = pcap_next_ex(adapterHandle, &packetHeader, &packetData)) >= 0)
	{
	// timeout elapsed if we reach this point
	if (retValue == 0)
	continue;

	// we print some information about the captured packet
	// we print only the length of the packet here
	printf("%c.%c.%c.%c length of packet: %d\n", packetHeader->len);
	}

	// if we get here, there was an error reading the packets
	if (retValue == -1)
	{
	printf("Error reading the packets: %s\n", pcap_geterr(adapterHandle));
	return -1;
	}
	}/**/

	//capture packets using callback
	{
		pcap_loop(adapterHandle, 0, packet_handler, NULL);
	}/**/

	//	system("PAUSE");
	return 0;
}
