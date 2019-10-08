#include "CapturePackets.h"
#include <pcap.h>
#include "ParsePackets.h"
#include "LordsMobileControl.h"

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

FILE *FCONTENT = NULL;
void DumpContent(unsigned char *data, unsigned int size)
{
	if (FCONTENT == NULL)
		errno_t er = fopen_s(&FCONTENT, "p_good", "wb");
	// might need to reassamble segmented packets later. TCP is a bytestream. The beggining of the packet should be a number indicating how much we need to read until the next packet
	if (FCONTENT)
	{
		fwrite(data, 1, size, FCONTENT);
		fflush(FCONTENT);
	}
}

unsigned char *TempPacketStore = NULL;
unsigned int WriteIndex = 0;
unsigned int ReadIndex = 0;
unsigned int ThrowAwayPacketsUntilSmallPackets = 1;
#define MAX_PACKET_SIZE					(10 * 1024 * 1024)
#define WAITING_FOR_X_BYTES				(*(unsigned short*)&TempPacketStore[ReadIndex])
#define MAX_PACKET_SIZE_SERVER_SENDS	15000
int ThrowAwayCount = 0;
void QueuePacketForMore(unsigned char *data, unsigned int size)
{
	//our temp store
	if (TempPacketStore == NULL)
		TempPacketStore = (unsigned char*)malloc(MAX_PACKET_SIZE); //10 MB should suffice i hope. Best i seen was about 10k

	//internal buffer is in a fucked up state !
	if (ThrowAwayPacketsUntilSmallPackets > 0)
	{
		printf("%d)Throwing away packet with size %d\n", ThrowAwayCount++, size);
		return;
	}

	//seems like we can panic. At this point we should try to resync to the next packet start. But how to do that ?
	if (MAX_PACKET_SIZE-WriteIndex <= size
		|| WriteIndex > MAX_PACKET_SIZE_SERVER_SENDS
		|| size > MAX_PACKET_SIZE_SERVER_SENDS
		)
	{
		printf("!!!ERROR:Packet did not fit into our buffer. Write index %d, Size %d, have %d\n", WriteIndex, size, MAX_PACKET_SIZE - WriteIndex);
		WriteIndex = 0;
		ReadIndex = 0;
		ThrowAwayPacketsUntilSmallPackets = 1;
		ThrowAwayCount = 0;
		return;
	}

	//add to our queue buffer. If we have enough data, process those
	memcpy(&TempPacketStore[WriteIndex], data, size);
	WriteIndex += size;

	//can we pop packets ?
	while (WriteIndex - ReadIndex >= WAITING_FOR_X_BYTES && WAITING_FOR_X_BYTES>0 && WriteIndex - ReadIndex != 0)
	{
//		ProcessPacket1(&TempPacketStore[ReadIndex + 2], WAITING_FOR_X_BYTES);
		QueuePacketToProcess(&TempPacketStore[ReadIndex + 2], WAITING_FOR_X_BYTES);
		ReadIndex += WAITING_FOR_X_BYTES;
	}

	//did we pop all packets ?
	if (ReadIndex >= WriteIndex)
	{
		ReadIndex = 0;
		WriteIndex = 0;
	}
}

void Wait1FullPacketThenParse(unsigned char *data, unsigned int size)
{
	if (size == 0)
		return;
	//theoretical size of a full packet. This is GAME specific !
	if (WriteIndex == 0)
	{
		unsigned short FullPacketSize = *(unsigned short*)data;
		if (size == FullPacketSize)
		{
			//ProcessPacket1(data, size);
			QueuePacketToProcess(&data[2], size - 2);
			//this could be a full packet. Consider ourself syncronized
			if (ThrowAwayPacketsUntilSmallPackets > 0)
				ThrowAwayPacketsUntilSmallPackets--;
			return;
		}
		//more than 1 server packet inside a single network packet
		if ( size>FullPacketSize && ThrowAwayPacketsUntilSmallPackets == 0 )
		{
			//ProcessPacket1(&data[2], FullPacketSize - 2);
			QueuePacketToProcess(&data[2], FullPacketSize - 2);
			QueuePacketForMore(&data[FullPacketSize], size - FullPacketSize); // should never happen
			return;
		}
	}
	//if we got here than this is a fragmented packet with first fragment
	QueuePacketForMore(data, size);
}

#define PROTOCOL_TCPIP	6

static unsigned int ClientBytesSent = 0;
static unsigned int ServerBytesSent = 0;

// Callback function invoked by libpcap for every incoming packet 
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	ip_hdr *ih;
	u_int ip_len;

	//Unused variable
	(VOID)(param);

	// retireve the position of the ip header
	ih = (ip_hdr *)(pkt_data + 14); //length of ethernet header

	// retireve the position of the udp header 
	ip_len = ih->ip_header_len * 4;

	unsigned int ServerIP[4] = { 192, 243, 0, 0 };
	FILE *f;
	errno_t opener = fopen_s(&f, "ServerIP.txt", "rt");
	if (f)
	{
		fscanf_s(f, "%d %d %d %d", &ServerIP[0], &ServerIP[1], &ServerIP[2], &ServerIP[3]);
		fclose(f);
	}
	//capturing all TCP packets from IP
	if (ih->ip_protocol == PROTOCOL_TCPIP)
	{
		// client to server
		// len 11, click on player owned tile : 0000   0b 00 9a 08 75 00 00 00 ff 03 ed
		if ( (ServerIP[0] == 0 || ((unsigned char*)&ih->ip_destaddr)[0] == ServerIP[0])
			&& (ServerIP[1] == 0 || ((unsigned char*)&ih->ip_destaddr)[1] == ServerIP[1])
			&& (ServerIP[2] == 0 || ((unsigned char*)&ih->ip_destaddr)[2] == ServerIP[2])
			&& (ServerIP[3] == 0 || ((unsigned char*)&ih->ip_destaddr)[3] == ServerIP[3]))
		{
			tcp_header *tcph = (tcp_header *)((u_char*)pkt_data + 14 + ip_len);
			int tcp_len = tcph->data_offset * 4;
			unsigned char *DataStart = (u_char*)pkt_data + 14 + ip_len + tcp_len;
			int TotalHeaderSize = (unsigned int)(DataStart - pkt_data);
			int BytesToDump = header->len - TotalHeaderSize;
            ClientBytesSent = htonl(tcph->sequence) + BytesToDump;
            if (BytesToDump > 0)
			{
				OnLordsClientPacketReceived(pkt_data, header->len, DataStart, BytesToDump);
//#define _DUMPPACKET_TO_FILE
#ifdef _DUMPPACKET_TO_FILE
                static FILE *FCONTENT = NULL;
                if (FCONTENT == NULL)
                    errno_t er = fopen_s(&FCONTENT, "client_to_server", "ab");
                // might need to reassamble segmented packets later. TCP is a bytestream. The beggining of the packet should be a number indicating how much we need to read until the next packet
                if (FCONTENT)
                {
                    //                fwrite(&BytesToDump, 1, 4, FCONTENT); //already present in the packet as the first 2 bytes
                    fwrite(DataStart, 1, BytesToDump, FCONTENT);
                    fflush(FCONTENT);
                }
#endif
			}

		}
		// server to client
		if ( (ServerIP[0] == 0 || ((unsigned char*)&ih->ip_srcaddr)[0] == ServerIP[0])
			&& (ServerIP[1] == 0 || ((unsigned char*)&ih->ip_srcaddr)[1] == ServerIP[1])
			&& (ServerIP[2] == 0 || ((unsigned char*)&ih->ip_srcaddr)[2] == ServerIP[2])
			&& (ServerIP[3] == 0 || ((unsigned char*)&ih->ip_srcaddr)[3] == ServerIP[3]))
		{
			tcp_header *tcph = (tcp_header *)((u_char*)pkt_data + 14 + ip_len);
			int tcp_len = tcph->data_offset * 4;
			unsigned char *DataStart = (u_char*)pkt_data + 14 + ip_len + tcp_len;
			int TotalHeaderSize = (unsigned int)(DataStart - pkt_data);
			int BytesToDump = header->len - TotalHeaderSize;
            ServerBytesSent = htonl(tcph->sequence) + BytesToDump;
            if (BytesToDump > 0)
			{
				DumpContent(DataStart, BytesToDump);
				Wait1FullPacketThenParse(DataStart, BytesToDump);
			}
		}
	}
}

pcap_t				* adapterHandle = NULL;
char                 errorBuffer[PCAP_ERRBUF_SIZE];
int StartCapturePackets(int AutoPickAdapter)
{
	pcap_if_t           * allAdapters;
	pcap_if_t           * adapter;

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
	if (AutoPickAdapter == -1)
	{
		printf("Enter the adapter number between 1 and %d:", crtAdapter);
		scanf_s("%d", &adapterNumber);

		if (adapterNumber < 1 || adapterNumber > crtAdapter)
		{
			printf("\nAdapter number out of range.\n");

			// Free the adapter list
			pcap_freealldevs(allAdapters);

			return -1;
		}
	}
	else
		adapterNumber = AutoPickAdapter; //this is my default wireless adapter

	// parse the list until we reach the desired adapter
	adapter = allAdapters;
	for (crtAdapter = 0; crtAdapter < adapterNumber - 1; crtAdapter++)
		adapter = adapter->next;

	// open the adapter
	adapterHandle = pcap_open_live(adapter->name, // name of the adapter
		6000,         // portion of the packet to capture
		// 65536 guarantees that the whole 
		// packet will be captured
		PCAP_OPENFLAG_PROMISCUOUS, // promiscuous mode
		-1,             // read timeout - 1 millisecond
		errorBuffer    // error buffer
		);

	if (adapterHandle == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter : %s\n", adapter->name);

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

	// free the adapter list
	pcap_freealldevs(allAdapters);

	//capture packets using callback
	pcap_loop(adapterHandle, 0, packet_handler, NULL);

	printf("Done creating background thread to monitor network trafic\n");

	return 0;
}

void StopCapturePackets()
{
	if (adapterHandle == NULL)
		return;
	pcap_breakloop(adapterHandle);
	pcap_close(adapterHandle);
	adapterHandle = NULL;
}

unsigned short ComputeChecksum(unsigned char *data, int plen)
{
    unsigned long sum = 0;

    for (int len = 0; len < plen; len+=2)
        sum += ((unsigned short)data[len] << 8) + data[len + 1];

    if (plen & 1)    
        sum += ((unsigned long)data[plen - 1]) << 8;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum = (~sum & 0xFFFF);
    return (unsigned short)sum;
}


unsigned short ComputeChecksum2(unsigned char *data, int plen)
{
    unsigned long sum = 0;

    unsigned short *temp = (unsigned short *)data;
    for (int len = 0; len < plen / 2; len++)
        sum += temp[len];

    if (plen & 1)
        sum += (unsigned long)data[plen - 1];

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum = (~sum & 0xFFFF);
    return (unsigned short)sum;
}


//ip.dst == 192.243.40.53 || ip.src == 192.243.40.53
void SendPacket(unsigned char *Data, int Len, int PayloadSize)
{
    if (ClientBytesSent == 0 || ServerBytesSent == 0)
        return;
    ip_hdr *ih = (ip_hdr *)(Data + 14); //length of ethernet header

    //check if i can generate correct checksum
 /*   {
        unsigned short oldIPChecksum = ih->ip_checksum;
        ih->ip_checksum = 0;
        unsigned short myIPChecksum = ComputeChecksum(Data + 14, ih->ip_header_len * 4);
        unsigned short myIPChecksum2 = htons(myIPChecksum);
        ih->ip_checksum = oldIPChecksum;
    }*/

    tcp_header *tcph = (tcp_header *)((u_char*)Data + 14 + ih->ip_header_len * 4);
    tcph->sequence = htonl(ClientBytesSent);
    tcph->acknowledge = htonl(ServerBytesSent);
    ClientBytesSent += PayloadSize;

    typedef struct PseudoHeader{
        unsigned long int source_ip;
        unsigned long int dest_ip;
        unsigned char reserved;
        unsigned char protocol;
        unsigned short int tcp_length;
    }PseudoHeader;
    PseudoHeader psh;

    tcph->checksum = 0;
//    tcph->checksum = htons(ComputeChecksum(Data, Len));
    tcph->checksum = htons(ComputeChecksum((unsigned char *)tcph, &Data[Len] - (unsigned char*)tcph));
//    tcph->checksum = ComputeChecksum2(Data, Len);

	if (pcap_sendpacket(adapterHandle, Data, Len) != 0)
	{
		printf("CapturePackets.cpp : Error sending the packet: %s\n", pcap_geterr(adapterHandle));
		return;
	}
}