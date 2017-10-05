#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
//#include <Windows.h>
#include <stdlib.h>
#include <vector>

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN        
#include <intrin.h>   

#pragma comment(lib, "iphlpapi.lib")

void GetMacsWithIP(std::vector<_int64> &vMac)
{
	//	vMac.clear();
	char data[4096];
	memset(data, 0, sizeof(data));
//	unsigned long  len = 4000;
//	PIP_ADAPTER_INFO pinfo = (PIP_ADAPTER_INFO)data;

	PIP_ADAPTER_INFO pAdapterInfo;
	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	ULONG buflen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(buflen);
	}

	if (GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR)
	{
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			if (
				//				pAdapter->Type == MIB_IF_TYPE_ETHERNET && 
				strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				//				printf("would use\n");
				vMac.push_back(*(__int64*)pAdapter->Address);
			}
#ifdef TRACE_WHAT_HAPPENED
			printf("\tType: \t%d\n", pAdapter->Type);
			printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
			printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
			printf("\tAdapter Addr: \t%ld\n", pAdapter->Address);
			printf("\tIP Address: \t%s\n", pAdapter->IpAddressList.IpAddress.String);
			printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);
			printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
			if (pAdapter->DhcpEnabled)
			{
				printf("\tDHCP Enabled: Yes\n");
				printf("\t\tDHCP Server: \t%s\n", pAdapter->DhcpServer.IpAddress.String);
				printf("\tLease Obtained: %ld\n", pAdapter->LeaseObtained);
			}
			else
			{
				printf("\tDHCP Enabled: No\n");
			}
			if (pAdapter->HaveWins)
			{
				printf("\tHave Wins: Yes\n");
				printf("\t\tPrimary Wins Server: \t%s\n", pAdapter->PrimaryWinsServer.IpAddress.String);
				printf("\t\tSecondary Wins Server: \t%s\n", pAdapter->SecondaryWinsServer.IpAddress.String);
			}
			else
			{
				printf("\tHave Wins: No\n");
			}
			printf("\n\n");
#endif
			pAdapter = pAdapter->Next;
		}
	}
#ifdef TRACE_WHAT_HAPPENED
	else
	{
		printf("Call to GetAdaptersInfo failed.\n");
	}
#endif
	free(pAdapterInfo);
}

#endif