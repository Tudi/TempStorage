#pragma once
#include "WinDivert/windivert.h"

/*********************************************
* Rules loaded from the config file. Based on rules we will select which flows of traffic
* will get redirected t our proxy
*********************************************/

//as config file is read line by line, new info is added to rule manager
void FilterAddRule(const char *RuleName, const char* Program, const char* DestinationIP, const char *RedirectTo, int WhiteList);
//when we wish to load a new config file
void ClearFilterRules();
//perform actions that might lead up to faster rule usage
void CompileFilterRules();
//check if config file rule set name is already taken. We group rules based on name. 
//If rule group name is not unique, multiple groups will get merged into a single one
int IsRuleNameTaken(const char* RuleName);
//check if a specific source port should be redirected
int IsTrafficFlowSelectedForRedirection(WINDIVERT_TCPHDR* tcp_header, WINDIVERT_IPHDR* ip_hdr);
//monitor these IPs to check if tunnel is up for forwarding
struct TunnelEntryStore
{
	unsigned int IP;
	unsigned short Port;
	int* StatusWriteback;
	char* RuleName;
	int* RefCounter;
};
//void * because it starts to conflict with includes
void *GetTunnelsToMonitor();
//get the number of active rule groups
int GetRuleCount();
