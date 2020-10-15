#include "windivert.h"
#include <Windows.h>
#include <list>

class HistoryStore
{
public:
	HistoryStore()
	{
		memset(&tcp_hdr, 0, sizeof(tcp_hdr));
		memset(&ip_hdr, 0, sizeof(ip_hdr));
		Stamp = 0;
	}
	WINDIVERT_TCPHDR tcp_hdr;
	WINDIVERT_IPHDR ip_hdr;
	unsigned __int64 Stamp;
};

class HistoryManager
{
public:
	HistoryManager()
	{
		HistorySrcPortLookup = new HistoryStore[0xFFFF];
	}
	HistoryStore* GetStore(unsigned short Port)
	{
		return &HistorySrcPortLookup[Port];
	}
private:
	HistoryStore* HistorySrcPortLookup;
};

//singleton to handle our redirect history
HistoryManager sHistoryManager;

void AddRedirection(PWINDIVERT_TCPHDR tcp_header, WINDIVERT_IPHDR* ip_hdr)
{
	HistoryStore* hs = sHistoryManager.GetStore(tcp_header->SrcPort);
	if (hs->tcp_hdr.DstPort == tcp_header->DstPort)
		return;
	hs->Stamp = GetTickCount64();
	memcpy(&hs->tcp_hdr, tcp_header, sizeof(WINDIVERT_TCPHDR));
	memcpy(&hs->ip_hdr, ip_hdr, sizeof(WINDIVERT_IPHDR));
}

unsigned short GetRedirectedPacketOriginalPort(unsigned short SrcPort)
{
	HistoryStore* hs = sHistoryManager.GetStore(SrcPort);
	return hs->tcp_hdr.DstPort;
}

unsigned long GetRedirectedPacketOriginalIP(unsigned short SrcPort)
{
	HistoryStore* hs = sHistoryManager.GetStore(SrcPort);
	return htonl(hs->ip_hdr.DstAddr);
}