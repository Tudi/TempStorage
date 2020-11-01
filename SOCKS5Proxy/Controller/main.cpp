#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

#include <string>
#include <vector>
#include <iostream>
#include "StdAfx.h"
#include "../RedirectToSocks5/FirewallUtil.h"
#include "../RedirectToSocks5/Licensing.h"
#include "../RedirectToSocks5/CommHandler.h"

//partial compare. If one of the strings end, consider it a match
int strcmpp(const char* s1, const char* s2)
{
	while (s1[0] != 0 && s2[0] != 0 && tolower(s1[0]) == tolower(s2[0]))
	{
		s1++;
		s2++;
	}
	if (s1[0] == 0 || s2[0] == 0)
		return 0;
	return -1;
}

void SendCMD(SOCKET s, int cmd, char *data)
{
	struct CommandPacket
	{
		unsigned short Size;
		unsigned short Cmd;
	};
	CommandPacket p;
	p.Size = sizeof(CommandPacket);
	p.Cmd = cmd;
	int count = send(s, (char*)&p, sizeof(p), 0);
	if (data != NULL)
	{
		unsigned short Size = (unsigned short)(strlen(data)+1);
		int count = send(s, (char*)&Size, sizeof(Size), 0);
		count = send(s, data, Size, 0);
	}
}

int __cdecl main(int argc, char** argv)
{
	if (argc == 1)
	{
		printf("Usage:tunnel.exe port=[port] [option]\n");
		printf("Example:tunnel.exe port=5558 start\n");
		printf("options : \n");
		printf("port=[port] \t\t port number to connect to the driver. Default 5558 \n");
		printf("start \t\t start redirecting packets \n");
		printf("stop \t\t stop redirecting packets \n");
		printf("drop \t\t restart network adapters \n");
		printf("config=[filename] \t\t reload adapter configurations \n");
		printf("exit \t\t unload driver \n");
//		printf("restart \t\t full reload driver \n");
		return 0;
	}

	int OptionRestart = 0;
	int OptionStart = 0;
	int OptionStop = 0;
	int OptionReload = 0;
	int OptionDrop = 0;
	int OptionExit = 0;
	int OptionCheckLicense = 1;
	char* ReloadPath = NULL;
	for (int i = 1; i < argc; i++)
		if (strcmpp(argv[i], "start") == 0)
			OptionStart = 1;
		else if (strcmpp(argv[i], "stop") == 0)
			OptionStop = 1;
		else if (strcmpp(argv[i], "drop") == 0)
			OptionDrop = 1;
		else if (strcmpp(argv[i], "exit") == 0)
			OptionExit = 1;
		else if (strcmpp(argv[i], "restart") == 0)
			OptionRestart = 1;
		else if (strcmpp(argv[i], "port=") == 0)
			CommPort = atoi(argv[i] + strlen("port="));
		else if (strcmpp(argv[i], "config=") == 0)
		{
			OptionReload = 1;
			ReloadPath = argv[i] + strlen("config=");
			//printf("cfg:%s\n", ReloadPath);
		}

	//try to make sure OS is not blocking us to connect to the driver
	AddFirewallRule();

	//first, check license status 
	LicenseStatusCodes ls = GetLicenseStatus();

	//create connection to our driver to send the commands
	WSADATA wsaData;
	int rv = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (rv != 0)
	{
		printf("WSAStartup() error: %d", rv);
		return 1;
	}

	SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
	{
		printf("socket() error: %d", WSAGetLastError());
		return 1;
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl((127<<24)|1);
	saddr.sin_port = htons(CommPort);

	if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
	{
		//maybe service is not yet started. Should have name of the service dynamic
		printf("Trying to start driver\n");
		system("net start xmwfps");
		if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
		{
			printf("connect() error: %d", WSAGetLastError());
			return 1;
		}
	}

	//send commands one by 1
	if (OptionStop)
		SendCMD(fd, ECT_StopRedirecting, NULL);
	if(OptionReload)
		SendCMD(fd, ECT_LoadConfigFile, ReloadPath);
	if(OptionStart && OptionDrop)
		SendCMD(fd, ECT_DropStartConnections, NULL);
	else if (OptionDrop)
		SendCMD(fd, ECT_DropConnections, NULL);
	else if (OptionStart)
		SendCMD(fd, ECT_StartRedirecting, NULL);
	if (OptionExit)
		SendCMD(fd, ECT_ExitDriver, NULL);
	else if (OptionCheckLicense && ls == LS_CHECKED_INVALID)
	{
		printf("Driver is running in demo mode\n");
		SendCMD(fd, ECT_GetLicenseStatus, (char*)&ls);
	}
	if (OptionRestart)
	{
		//stop the drive with system command
		printf("sending service stop xmwfps command\n");
		system("net stop xmwfps");
		Sleep(5000);
		printf("sending service start xmwfps command\n");
		system("net start xmwfps");
	}

	//should wait for server to give us a reply
	Sleep(500);

	//kill connection
	shutdown(fd, SD_BOTH);
	closesocket(fd);

	return 0;
}