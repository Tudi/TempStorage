#include "HTTPSendData.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <locale>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

string url_encode(const string value) 
{
	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << uppercase;
		escaped << '%' << setw(2) << int((unsigned char)c);
		escaped << nouppercase;
	}

	return escaped.str();
}

void AppendURLQuery(string &url, const char *name, int val, int IsFirst=0)
{
	char temp[2000];
	if (IsFirst==0)
		sprintf_s(temp, "&%s=%d", name, val);
	else
		sprintf_s(temp, "%s=%d", name, val);
	url += temp;
}

void AppendURLQuery(string &url, const char *name, const char *val, int IsFirst = 0)
{
	if (name == NULL || val == NULL)
		return;
	char temp[2000];
	string tval = url_encode(val);
	if (IsFirst == 0)
		sprintf_s(temp, "&%s=%s", name, tval.c_str());
	else
		sprintf_s(temp, "%s=%s", name, tval.c_str());
	url += temp;
}

int HTTPPostData(int k, int x, int y, char *name, char *guild, char *guildf, int clevel, __int64 kills, int vip, int grank, __int64 might, int HasPrisoners, int plevel)
{
	WSADATA wsaData;
	SOCKET Socket;
	SOCKADDR_IN SockAddr;
	int lineCount = 0;
	int rowCount = 0;
	struct hostent *host;
	locale local;
	int i = 0;

	// website url
	string url = "127.0.0.1";
	int URLPort = 8081;

	//HTTP GET
	string get_http = "GET /ImportPlayerInfoFromNetwork.php?";
	AppendURLQuery(get_http, "k", k, 1);
	AppendURLQuery(get_http, "x", x);
	AppendURLQuery(get_http, "y", y);
	AppendURLQuery(get_http, "CLevel", clevel);
	AppendURLQuery(get_http, "kills", (int)kills);
	AppendURLQuery(get_http, "vip", vip);
	AppendURLQuery(get_http, "GuildRank", grank);
	AppendURLQuery(get_http, "might", (int)might);
	AppendURLQuery(get_http, "HasPrisoners", HasPrisoners);
	AppendURLQuery(get_http, "PLevel", plevel);
	AppendURLQuery(get_http, "name", name);
	AppendURLQuery(get_http, "guild", guild);
	AppendURLQuery(get_http, "guildF", guildf);
	get_http += " HTTP / 1.1\r\n";
	get_http += "Host: " + url;
	get_http += " : " + URLPort;
	get_http += " \r\n";
	get_http += "Connection: close\r\n\r\n";


	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup failed.\n";
		system("pause");
		//return 1;
	}

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	host = gethostbyname(url.c_str());

	SockAddr.sin_port = htons(URLPort);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		cout << "Could not connect";
		system("pause");
		//return 1;
	}

	// send GET / HTTP
	send(Socket, get_http.c_str(), strlen(get_http.c_str()), 0);

	// recieve html
#ifdef DEBUG_HTTP_BEHAVIOR
	char buffer[10000];
	int nDataLength;
	string website_HTML;
	while ((nDataLength = recv(Socket, buffer, 10000, 0)) > 0)
	{
		int i = 0;
		while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r')
		{
			website_HTML += buffer[i];
			i += 1;
		}
	}

	// Display HTML source 
	cout << website_HTML;
#endif

	closesocket(Socket);
	WSACleanup();
	return 0;
}