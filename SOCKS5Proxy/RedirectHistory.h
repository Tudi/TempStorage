#pragma once
#include "windivert.h"

//memorize original destination port, required for us to connect to the destination
void AddRedirection(WINDIVERT_TCPHDR *tcp_header, WINDIVERT_IPHDR *ip_hdr);
//retrieve a destination port based on the source port
//!expects/returns port format as in packet. Make sure you htons(humanport)
unsigned short GetRedirectedPacketOriginalPort(unsigned short SrcPort);
unsigned long GetRedirectedPacketOriginalIP(unsigned short SrcPort);
