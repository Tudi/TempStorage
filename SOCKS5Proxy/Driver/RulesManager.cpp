#include <stdlib.h>
#include <string.h>
#include <list>
#include <set>
#include "WinDivert/windivert.h"
#include "FlowManager.h"
#include "RulesManager.h"
#include "ConfigHandler.h"
#include "Utils.h"
#include "Logger.h"
#include "WhitelistManager.h"

#define MISSING_NAME_SIZE	1

class IPRangeStore
{
public:
	unsigned int IP;
	unsigned int Mask; //only need to math IP where mask bits are 1
	unsigned short Port;
};

class ProgramNameStore
{
public:
	ProgramNameStore(const char* pName)
	{
		Name = _strdup(pName);
		Size = strlen(pName);
	}
	~ProgramNameStore()
	{
		free(Name);
		Name = NULL;
		Size = 0;
	}
	size_t Size;
	char* Name;
};

class FilterRuleGroup
{
public:
	FilterRuleGroup(const char* pName)
	{
		if (pName == NULL)
		{
			Name = (char*)malloc(MISSING_NAME_SIZE);
			Name[0] = 0;
		}
		else
			Name = _strdup(pName);
		TunnelStatus = 0;
		RefCounter = 0;
	}
	~FilterRuleGroup()
	{
		free(Name);
		Name = NULL;
		for (auto itr = SelectedPrograms.begin(); itr != SelectedPrograms.end(); itr++)
			delete (*itr);
		SelectedPrograms.clear();
		for (auto itr = SelectedDstIPs.begin(); itr != SelectedDstIPs.end(); itr++)
			free((*itr));
		SelectedDstIPs.clear();
	}
	bool HasName(const char* pName)
	{
		return strcmp(Name, pName)==0;
	}
	void AddSelectedProgram(const char* Program)
	{
		//check if already exists
		for (auto itr = SelectedPrograms.begin(); itr != SelectedPrograms.end(); itr++)
			if (strcmp(Program, (*itr)->Name) == 0)
				return;
		//if new, add it
		SelectedPrograms.push_front(new ProgramNameStore(Program));
	}
	void AddSelectedIP(const char* IP)
	{
		//convert string to IP + MASK
		unsigned int b[4]; //4 bytes for ipv4, but what if format is wrong ?
		int MaskBits = 33;
		int IPnum = 0;
		int Port = 0xFFFF;
		//special case when all IPs are selected for redirecting
		if (IP[0] == '*')
		{
			sscanf_s(IP, "*/%d:%d", &MaskBits, &Port);
			b[0] = b[1] = b[2] = b[3] = 0;
			MaskBits = 0; //select all IPs
		}
		else
			sscanf_s(IP, "%d.%d.%d.%d/%d:%d", b + 0, b + 1, b + 2, b + 3, &MaskBits, &Port);
		IPnum = 0;
		for (int i = 0; i < 4; i++)
			IPnum = (IPnum << 8) + b[i];
		int MaskWithBits = 0;
		for (int i = 0; i < 32; i++)
			if (i >= (32-MaskBits))
				MaskWithBits |= (1 << i);
		//check if already exists
		for (auto itr = SelectedDstIPs.begin(); itr != SelectedDstIPs.end(); itr++)
			if ((*itr)->IP == IPnum && (*itr)->Mask == MaskWithBits && (*itr)->Port == Port)
				return;
		//create new
		IPRangeStore *ret = (IPRangeStore*)malloc(sizeof(IPRangeStore));
		memset(ret, 0, sizeof(IPRangeStore));
		ret->IP = IPnum;
		ret->Mask = MaskWithBits;
		ret->Port = Port;

		SelectedDstIPs.push_front(ret);
	}
	void SetRedirectIP(const char* RedirectTo)
	{
		//convert string to IP + MASK
		unsigned int b[4]; //4 bytes for ipv4, but what if format is wrong ?
		sscanf_s(RedirectTo, "%d.%d.%d.%d:%d", b + 0, b + 1, b + 2, b + 3, &RedirectPort);
		RedirectAddr = 0;
		for (int i = 0; i < 4; i++)
			RedirectAddr = (RedirectAddr << 8) + b[i];
	}
	int HasProgramBasedFiltering()
	{
		return (SelectedPrograms.empty()==false);
	}
	int IsIPRedirected(unsigned int IP, unsigned short Port)
	{
		for (auto itr = SelectedDstIPs.begin(); itr != SelectedDstIPs.end(); itr++)
			if (((*itr)->IP & (*itr)->Mask) == (IP & (*itr)->Mask)
				&& ((*itr)->Port == 0xFFFF || (*itr)->Port == Port))
				return 1;
		return 0;
	}
	int strNcompare(const char* s1, const char* s2, int N)
	{
		__int64* i1 = (_int64*)s1;
		__int64* i2 = (_int64*)s2;
		for (int i = 0; i < N / sizeof(_int64); i++)
			if (i1[i] != i2[i])
				return -1;
		for(int i= N / sizeof(_int64);i<N;i++)
			if (s1[i] != s2[i])
				return -1;
		return 0;
	}
	int IsProgramRedirected(const char* ProgramName)
	{
		if (SelectedPrograms.empty())
			return 1;
		for (auto itr = SelectedPrograms.begin(); itr != SelectedPrograms.end(); itr++)
		{
			ProgramNameStore* rns = (*itr);
			if (strNcompare(ProgramName, rns->Name, (int)rns->Size) == 0)
				return 1;
		}
		return 0;
	}
	unsigned int GetRedirectIP() { return RedirectAddr; }
	unsigned short GetRedirectPort() { return RedirectPort; }
	int* GetStatusWritebackAddr() { return &TunnelStatus; }
	char* GetName() { return Name; }
	int* GetRefCounter() { return &RefCounter; }
	int IsValidFilter()
	{
		if (RedirectAddr == 0 || RedirectPort == 0)
			return 0;
		if (SelectedPrograms.empty() && SelectedDstIPs.empty())
			return 0;
		return 1;
	}
	int IsTunnelDestination(unsigned int ip, unsigned short port)
	{
		if (RedirectPort == port && RedirectAddr == ip)
			return 1;
		return 0;
	}
private:
	char* Name;
	std::list<ProgramNameStore*> SelectedPrograms;
	std::list<IPRangeStore*> SelectedDstIPs;
	unsigned int RedirectAddr;
	unsigned int RedirectPort;
	int TunnelStatus; //this is shared between multiple threads, always delay delete these !
	int RefCounter;
};

class FilterRuleManager
{
public:
	FilterRuleManager()
	{
	}
	~FilterRuleManager()
	{
		int RetryDelete = 10;
		while (FilterGroupsDeleteLater.empty() == false && RetryDelete > 0)
		{
			ClearRules();
			RetryDelete--;
			Sleep(100);
		}
	}
	FilterRuleGroup *GetGroup(const char* pName, int Whitelist, int DoNotCreate=0)
	{
		std::list<FilterRuleGroup*>* fg;
		if (Whitelist)
			fg = &WhitelistGroups;
		else
			fg = &FilterGroups;
		//group already exists
		for (auto itr = fg->begin(); itr != fg->end(); itr++)
			if ((*itr)->HasName(pName))
				return (*itr);
		//create a new group
		if (DoNotCreate == 0)
		{
			FilterRuleGroup* frg = new FilterRuleGroup(pName);
			fg->push_front(frg);
			return frg;
		}
		return NULL;
	}
	//if there is any method for fast lookup IP/Program...use it
	void CompileRules()
	{
		//cache which port+ip combo we use for tunnels
		for (int i = 0; i < 0xFFFF; i++)
			TunnelPortIps[i].clear();
		for (auto itr = FilterGroups.begin(); itr != FilterGroups.end(); itr++)
			TunnelPortIps[(*itr)->GetRedirectPort()].insert((*itr)->GetRedirectIP());
	}
	int IsIPRedirected(unsigned int IP, unsigned short Port, FlowStatusStore *fs)
	{
		int TryGetProgramName = 1; //if we fail once, don't try to get the name again
		for (auto itr = FilterGroups.begin(); itr != FilterGroups.end(); itr++)
			if ((*itr)->IsIPRedirected(IP, Port))
			{
				if (fs->ImageName == NULL && TryGetProgramName == 1)
				{
					TryGetProgramName = 0; // it's a slow process to get program name
					if ((*itr)->HasProgramBasedFiltering())
						fs->UpdateImageName();
				}
				if (fs->ImageName == NULL || (*itr)->IsProgramRedirected(fs->ImageName))
				{
					fs->RedirectToIP = (*itr)->GetRedirectIP();
					fs->RedirectToPort = (*itr)->GetRedirectPort();
					fs->TunnelStatus = (*itr)->GetStatusWritebackAddr();
					return 1;
				}
			}
		return 0;
	}
	std::list<TunnelEntryStore*>*GetTunnelsToMonitor()
	{
		std::list<TunnelEntryStore*> *ret = new std::list<TunnelEntryStore*>();
		for (auto itr = FilterGroups.begin(); itr != FilterGroups.end(); itr++)
		{
			TunnelEntryStore* t = new TunnelEntryStore();
			t->IP = (*itr)->GetRedirectIP();
			t->Port = (*itr)->GetRedirectPort();
			t->StatusWriteback = (*itr)->GetStatusWritebackAddr();
			t->RuleName = (*itr)->GetName();
			t->RefCounter = (*itr)->GetRefCounter();
			*t->RefCounter += 1;
			ret->push_back(t);
		}
		return ret;
	}
	//when we reload config file, we remove old rules
	void ClearRules()
	{
		//should add a mutex to make sure list is not accessed from another thread
		std::list<FilterRuleGroup*> tFilterGroups = FilterGroups;

		//other threads should no longer use the invalid list
		FilterGroups.clear();

		//can we delete values from referenced values ?
/*		int CanWeClearList = 1;
		for (auto itr = FilterGroupsDeleteLater.begin(); itr != FilterGroupsDeleteLater.end(); itr++)
			if (*(*itr)->GetRefCounter() != 0)
				CanWeClearList = 0;
		if (CanWeClearList)
		{
			for (auto itr = FilterGroupsDeleteLater.begin(); itr != FilterGroupsDeleteLater.end(); itr++)
				delete (*itr);
			FilterGroupsDeleteLater.clear();
		}/**/

		//try to delete the rules. Rules that are referenced will be delay deleted
		for (auto itr = tFilterGroups.begin(); itr != tFilterGroups.end(); itr++)
		{
			//mark this tunnel as offline. This is required since active sessions will still use the shared object
			*(*itr)->GetStatusWritebackAddr() = 0;
//			if (*(*itr)->GetRefCounter() != 0)
				FilterGroupsDeleteLater.push_back(*itr);
//			else
//				delete (*itr);
		}
	}
	int GetRuleCount()
	{
		int ret = 0;
		for (auto itr = FilterGroups.begin(); itr != FilterGroups.end(); itr++)
			if ((*itr)->IsValidFilter())
				ret++;
		return ret;
	}
	int IsConnectionWhitelisted(WINDIVERT_TCPHDR* tcp_header, WINDIVERT_IPHDR* ip_hdr, FlowStatusStore* fs)
	{
		if (IsTunnelDestination(htonl(ip_hdr->SrcAddr), htons(tcp_header->SrcPort)))
			return 1;
		else if (IsTunnelDestination(htonl(ip_hdr->DstAddr), htons(tcp_header->DstPort)))
			return 1;

		//special case, data going from proxy entrance to "app"
//		if (htonl(ip_hdr->SrcAddr) == IPFromBytes(127, 0, 0, 1) && htons(tcp_header->SrcPort) == GetOurProxyPort())
//			return 0;

		unsigned int IP = ntohl(ip_hdr->DstAddr);
		unsigned short Port = ntohs(tcp_header->DstPort);
		unsigned int IP2 = ntohl(ip_hdr->SrcAddr);
		unsigned short Port2 = ntohs(tcp_header->SrcPort);
		int TryGetProgramName = 1; //if we fail once, don't try to get the name again
		for (auto itr = WhitelistGroups.begin(); itr != WhitelistGroups.end(); itr++)
			if ((*itr)->IsIPRedirected(IP, Port) || (*itr)->IsIPRedirected(IP2, Port2))
			{
				if (fs->ImageName == NULL && TryGetProgramName == 1)
				{
					TryGetProgramName = 0; // it's a slow process to get program name
					if ((*itr)->HasProgramBasedFiltering())
					{
						fs->UpdateImageName();
						if (fs->ImageName == NULL)
							return 1;
					}
				}
				if((*itr)->IsProgramRedirected(fs->ImageName))
					return 1;
			}
		return 0;
	}
private:
	int IsTunnelDestination(unsigned int ip, unsigned short port)
	{
		if (TunnelPortIps[port].empty())
			return 0;
		auto itr = TunnelPortIps[port].find(ip);
		if (itr != TunnelPortIps[port].end())
			return 1;
		//		for (auto itr = FilterGroups.begin(); itr != FilterGroups.end(); itr++)
		//			if ((*itr)->IsTunnelDestination(ip, port))
		//				return 1;
		return 0;
	}
	std::list<FilterRuleGroup*> FilterGroups;
	std::list<FilterRuleGroup*> WhitelistGroups;
	std::list<FilterRuleGroup*> FilterGroupsDeleteLater;
	std::set<unsigned int> TunnelPortIps[0xFFFF];
};

FilterRuleManager sFRM;

void *GetTunnelsToMonitor()
{
	return sFRM.GetTunnelsToMonitor();
}

void FilterAddRule(const char* RuleName, const char* Program, const char* DestinationIP, const char* RedirectTo, int WhiteList)
{
	FilterRuleGroup* frg = sFRM.GetGroup(RuleName, WhiteList);
	if (Program != NULL && strlen(Program) > 0
		&& Program[0] != '*')
		frg->AddSelectedProgram(Program);
	if (DestinationIP != NULL && strlen(DestinationIP) > 0)
		frg->AddSelectedIP(DestinationIP);
	if (RedirectTo != NULL && strlen(RedirectTo) > 0)
		frg->SetRedirectIP(RedirectTo);
}

int IsRuleNameTaken(const char* RuleName)
{
	return (sFRM.GetGroup(RuleName, 1) != NULL);
}

void CompileFilterRules()
{
	sFRM.CompileRules();
}

//check if a specific source port should be redirected
int IsTrafficFlowSelectedForRedirection(WINDIVERT_TCPHDR* tcp_header, WINDIVERT_IPHDR* ip_hdr)
{
	//no need to handle it as a new connection
	if (tcp_header->SrcPort == htons(GetOurProxyPort()))
		return TFRS_Redirected;
	//if we redirect commands comming to us, we probably will fail to give us commands
//	if (htons(tcp_header->SrcPort) == GetOurCommandsPort() || htons(tcp_header->DstPort) == GetOurCommandsPort())
//		return TFRS_NotRedirected;

//	if (tcp_header->DstPort == htons(5557))
//		return TFRS_NotRedirected;

	//let's check if flow manager if we already know the status of the flow
	FlowStatusStore *fs = GetFlowStatus(tcp_header, ip_hdr);
	if (fs == NULL)
		return TFRS_NotRedirected;

	if (fs->RedirectStatus != TFRS_NotSet)
	{
		//is the tunnel up and running ?
		if (RedirectPacketsToDeadTunnels() == 0 && *fs->TunnelStatus == 0)
			return TFRS_NotRedirected;
		//handle the status as usual
		return fs->RedirectStatus;
	}

	//we presume this flow will not be redirected unless one of our rules will set it as redirected
	fs->RedirectStatus = TFRS_NotRedirected;

	//white list is more important than blacklist
	if (sFRM.IsConnectionWhitelisted(tcp_header, ip_hdr, fs))
		fs->RedirectStatus = TFRS_NotRedirected;
	//check if destination IP is in the list of redirected IPs
	else if (sFRM.IsIPRedirected(htonl(ip_hdr->DstAddr), htons(tcp_header->DstPort), fs) == 1)
		fs->RedirectStatus = TFRS_Redirected;
//	else if (sFRM.IsProgramRedirected(fs->ImageName, &fs->RedirectToIP, &fs->RedirectToPort, &fs->TunnelStatus) == 1)
//		fs->RedirectStatus = TFRS_Redirected;

	//is the tunnel up and running ?
	if (RedirectPacketsToDeadTunnels() == 0 && *fs->TunnelStatus == 0)
		return TFRS_NotRedirected;

	if (fs->RedirectStatus == TFRS_Redirected && sLog.LogConnections())
	{
		char SrcIP[50], DstIP[50];
		IPv4Tostr(ip_hdr->SrcAddr, SrcIP, sizeof(SrcIP));
		IPv4Tostr(ip_hdr->DstAddr, DstIP, sizeof(DstIP));
		char TunnelIP[50];
		IPv4Tostr(htonl(fs->RedirectToIP), TunnelIP, sizeof(TunnelIP));
		sLog.Log(LL_Connections, __FILE__, __LINE__, "%X New connection tunnel : %s:%d->127.0.0.1:%d->%s:%d->%s:%d for %s",
			fs, SrcIP, htons(tcp_header->SrcPort), GetOurProxyPort(), TunnelIP, fs->RedirectToPort, DstIP, htons(tcp_header->DstPort), fs->ImageName);
	}

	//from now on, we know the redirection status for this flow of traffic
	return fs->RedirectStatus;
}

void ClearFilterRules()
{
	sFRM.ClearRules();
}

int GetRuleCount()
{
	return sFRM.GetRuleCount();
}