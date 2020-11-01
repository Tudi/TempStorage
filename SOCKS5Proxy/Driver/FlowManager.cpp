#include "WinDivert/windivert.h"
#include <Windows.h>
#include <list>
#include <iphlpapi.h>
#include <string>
#include "RulesManager.h"
#include "FlowManager.h"
#include "Logger.h"
#include "Utils.h"

class FlowStatusManager
{
public:
	FlowStatusManager()
	{
		HistorySrcPortLookup = new FlowStatusStore[0xFFFF];
	}
	FlowStatusStore* GetStore(unsigned short Port)
	{
		return &HistorySrcPortLookup[Port];
	}
private:
	FlowStatusStore* HistorySrcPortLookup;
};

//singleton to handle our redirect history
FlowStatusManager sFlowStatusManager;

#pragma comment(lib, "psapi.lib")
std::string ProcessIdToName(DWORD processId)
{
	std::string ret;
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,processId);
	if (handle)
	{
		DWORD buffSize = 1024;
		CHAR buffer[1024];
		if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
			ret = buffer;
		else
			sLog.Log(LL_Warning, __FILE__, __LINE__, "Error GetModuleBaseNameA : %lu", GetLastError());
		CloseHandle(handle);
	}
	else
		sLog.Log(LL_Warning, __FILE__, __LINE__, "Error OpenProcess : %lu", GetLastError());
	return ret;
}

HMODULE hLib = NULL;
char *GetImageNameForPort(unsigned short Port)
{
	DWORD(WINAPI * pGetExtendedTcpTable)(
		PVOID pTcpTable,
		PDWORD pdwSize,
		BOOL bOrder,
		ULONG ulAf,
		TCP_TABLE_CLASS TableClass,
		ULONG Reserved
		);
	MIB_TCPTABLE_OWNER_PID* pTCPInfo;
	MIB_TCPROW_OWNER_PID* owner;
	DWORD size;
	DWORD dwResult;

	if(hLib == NULL)
		hLib = LoadLibrary("iphlpapi.dll");

	pGetExtendedTcpTable = (DWORD(WINAPI*)(PVOID, PDWORD, BOOL, ULONG, TCP_TABLE_CLASS, ULONG))
		GetProcAddress(hLib, "GetExtendedTcpTable");

	dwResult = pGetExtendedTcpTable(NULL, &size, false, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
	pTCPInfo = (MIB_TCPTABLE_OWNER_PID*)malloc(size);
	dwResult = pGetExtendedTcpTable(pTCPInfo, &size, false, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
	for (DWORD dwLoop = 0; dwLoop < pTCPInfo->dwNumEntries; dwLoop++)
	{
		owner = &pTCPInfo->table[dwLoop];
		if (owner->dwLocalPort == Port)
		{
			char* ret = _strdup(ProcessIdToName(owner->dwOwningPid).c_str());
			return ret;
		}
	}
	return NULL;
}

int IsNewFlow(FlowStatusStore* hs, PWINDIVERT_TCPHDR tcp_header, WINDIVERT_IPHDR* ip_hdr)
{
	if (hs->tcp_hdr.DstPort == tcp_header->DstPort
		&& hs->ip_hdr.DstAddr == ip_hdr->DstAddr
		&& hs->ip_hdr.Protocol == ip_hdr->Protocol
		)
		return 0;
	return 1;
}

void FlowStatusStore::UpdateImageName()
{
	//if too much time passed ( or conditions changed ), refresh image name. Required for rules to function
//	if (HasProgramBasedFiltering())
	{
		//		__int64 TimePassed = GetTickCount64() - hs->Stamp;
		//		if (TimePassed > 10000)
		{
			if (ImageName != NULL)
			{
				free(ImageName);
				ImageName = NULL;
			}
			ImageName = GetImageNameForPort(tcp_hdr.SrcPort);
		}
	}
/*	if (hs->ImageName == NULL)
	{
		hs->ImageName = (char*)malloc(2);
		hs->ImageName[0] = 0;
	}*/
}

FlowStatusStore* GetFlowStatus(PWINDIVERT_TCPHDR tcp_header, WINDIVERT_IPHDR* ip_hdr, int SkipCreateNew)
{
	FlowStatusStore* hs = GetFlowStatus(tcp_header->SrcPort);
	if (IsNewFlow(hs, tcp_header, ip_hdr) == 0)
		return hs;

	if (SkipCreateNew == 1)
		return NULL;

//	if (hs->RedirectStatus == TFRS_SettingUp)
//		return NULL;

//	if (tcp_header->DstPort == htons(5557))
//		return NULL;

	if (sLog.LogConnections())
	{
		char SrcIP[50], DstIP[50];
		IPv4Tostr(ip_hdr->SrcAddr, SrcIP, sizeof(SrcIP));
		IPv4Tostr(ip_hdr->DstAddr, DstIP, sizeof(DstIP));
		sLog.Log(LL_Connections, __FILE__, __LINE__, "%X Start new session from %s:%d to %s:%d", hs, SrcIP, htons(tcp_header->SrcPort), DstIP, htons(tcp_header->DstPort));
	}

//	hs->Stamp = GetTickCount64();
	memcpy(&hs->tcp_hdr, tcp_header, sizeof(WINDIVERT_TCPHDR));
	memcpy(&hs->ip_hdr, ip_hdr, sizeof(WINDIVERT_IPHDR));
	hs->RedirectStatus = TFRS_NotSet;

	return hs;
}

FlowStatusStore* GetFlowStatus(unsigned short SrcPort)
{
	FlowStatusStore* hs = sFlowStatusManager.GetStore(SrcPort);
	return hs;
}
