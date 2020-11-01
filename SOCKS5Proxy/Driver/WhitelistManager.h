#pragma once
#if 0
/*********************************************
* Some connections should not be redirected
* Ex : output of ssh.exe sending data to the ssh server
*********************************************/

struct IPV4ConnInfo{
	unsigned int IP;
	unsigned short Port;
	char Direction;
	char* ProgramName;
};

std::list<IPV4ConnInfo*> GetWhiteList(std::list<IPV4ConnInfo*>* KnownConnections);

int IsConnectionWhitelisted(WINDIVERT_TCPHDR* tcp_header, WINDIVERT_IPHDR* ip_hdr);
#endif