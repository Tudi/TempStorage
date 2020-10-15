#pragma once

//our proxy will create a bridge compatible with SOCKS 5 tunnel
unsigned short GetOurProxyPort();
unsigned long GetOurProxyIP();
unsigned short GetSOCKSTunnelPort();
unsigned long GetSOCKSTunnelIP();
//we do not redirect traffic going into our : proxy, tunnel
int SkipRedirectOnOutboundDestPort(unsigned short Port);