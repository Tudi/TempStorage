#define PROXY_PORT      5557 // this is my local proxy that should convert any connection to socks5 connection

unsigned short GetOurProxyPort()
{
	return PROXY_PORT;
}

unsigned long GetOurProxyIP()
{
	//	return htonl(0x7F000001);
	//return 0x0100007F;
	return 0x7F000001;
}

unsigned short GetSOCKSTunnelPort()
{
	return 5556;
}

unsigned long GetSOCKSTunnelIP()
{
//	return inet_addr("127.0.0.1");
//	return htonl(0x7F000001);
//	return 0x0100007F;
	return 0x7F000001;
}

int SkipRedirectOnOutboundDestPort(unsigned short Port)
{
	if (Port == GetOurProxyPort())
		return 1;
	if (Port == GetSOCKSTunnelPort())
		return 1;
	return 0;
}