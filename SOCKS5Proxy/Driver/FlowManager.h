#pragma once
#include "WinDivert/windivert.h"

/*********************************************
* Flows of traffic will have a "session" that remembers what was the original source and
* destination of the flow. This session is used to retrieve where to return packets.
* Session is also used to retrieve destination port for redirected packets
* 
* Should rename this file to FlowManager
* 
*********************************************/

enum TrafficFlowRedirectStatus
{
	TFRS_NotSet,
	TFRS_SettingUp, //client might spam us with reconnect 
	TFRS_NotRedirected,
	TFRS_Redirected,
	TFRS_ConnectionClosed, //on our side, we closed this "session"
};

class FlowStatusStore
{
public:
	FlowStatusStore()
	{
		memset(&tcp_hdr, 0, sizeof(tcp_hdr));
		memset(&ip_hdr, 0, sizeof(ip_hdr));
//		Stamp = 0;
		ImageName = NULL;
		RedirectStatus = TFRS_NotSet;
		TunnelStatus = NULL;
		RedirectToIP = 0;
		RedirectToPort = 0;
	}
	void UpdateImageName();
	WINDIVERT_TCPHDR tcp_hdr;
	WINDIVERT_IPHDR ip_hdr;
//	unsigned __int64 Stamp;
	char* ImageName; //program name using this port
	TrafficFlowRedirectStatus RedirectStatus;
	unsigned int RedirectToIP;
	unsigned short RedirectToPort;
	int* TunnelStatus;
};

//memorize original destination port, required for us to connect to the destination
FlowStatusStore *GetFlowStatus(WINDIVERT_TCPHDR *tcp_header, WINDIVERT_IPHDR *ip_hdr, int SkipCreateNew = 0);
FlowStatusStore* GetFlowStatus(unsigned short SrcPort);
