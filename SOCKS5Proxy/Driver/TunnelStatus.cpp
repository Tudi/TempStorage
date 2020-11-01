#include <Windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "Utils.h"
#include "ConfigHandler.h"
#include "RulesManager.h"
#include "Logger.h"

//loop as long as the program is running and periodically check if it could connect to the SOCKS 5 tunnel
DWORD MonitorTunnelStatusLoopThread(LPVOID arg)
{
    WSADATA wsa_data;
    WORD wsa_version = MAKEWORD(2, 2);
    SOCKET s;
    struct sockaddr_in addr;
    int on = 1;
    AutoMonitorThreadExit AM; //don't delete me
    //enough to start it once, not sure we are the first to use it though
    if (WSAStartup(wsa_version, &wsa_data) != 0)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to start WSA (%d)", GetLastError());
        SetProgramTerminated();
        return -1;
    }
    //do the periodic connection check
    while (IsWaitingForUserExitProgram())
    {
        std::list<TunnelEntryStore*> *Tunnels = (std::list<TunnelEntryStore*> * )GetTunnelsToMonitor();
        for (auto itr = Tunnels->begin(); itr != Tunnels->end(); itr++)
        {
            //create a new socket
            s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (s == INVALID_SOCKET)
            {
                sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to create socket (%d)", WSAGetLastError());
                continue;
            }
            //do this in every loop, in case the config file got reloaded, we update these settings also
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons((*itr)->Port);
            addr.sin_addr.s_addr = htonl((*itr)->IP);
            //try to bind it to the port that should be listening
            if (bind(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
            {
                //we expect that the address is already in use by the tunnel
                if (WSAGetLastError() == WSAEADDRINUSE)
                {
                    if (*(*itr)->StatusWriteback == 0)
                    {
                        char SrcIP[50];
                        IPv4Tostr(htonl((*itr)->IP), SrcIP, sizeof(SrcIP));
                        sLog.Log(LL_Info, __FILE__, __LINE__, "Tunnel %s:%d came online. Rule : %s", SrcIP, (*itr)->Port, (*itr)->RuleName);
                    }
                    *(*itr)->StatusWriteback = 1;
                }
                else
                {
                    if (*(*itr)->StatusWriteback == 1)
                    {
                        char SrcIP[50];
                        IPv4Tostr(htonl((*itr)->IP), SrcIP, sizeof(SrcIP));
                        sLog.Log(LL_Info, __FILE__, __LINE__, "Tunnel %s:%d went offline. Rule : %s", SrcIP, (*itr)->Port, (*itr)->RuleName);
                    }
                    *(*itr)->StatusWriteback = 0;
                }
            }
            else
            {
                if (*(*itr)->StatusWriteback == 1)
                {
                    char SrcIP[50];
                    IPv4Tostr(htonl((*itr)->IP), SrcIP, sizeof(SrcIP));
                    sLog.Log(LL_Info, __FILE__, __LINE__, "Tunnel %s:%d went offline. Rule : %s", SrcIP, (*itr)->Port, (*itr)->RuleName);
                }
                *(*itr)->StatusWriteback = 0;
            }
            //we are done with the test for now. No longer require this socket
            closesocket(s);
            //delete the value
            *(*itr)->RefCounter -= 1;
            delete (*itr);
        }
        //delete the list that stores the tunnel IPs
        delete Tunnels;
        Tunnels = NULL;

        //sleep for a while
        Sleep(GetWatchdogTunnelSleepMs());
    }
    //we might be the last thread to clean up
    WSACleanup();
    return 0;
}

//create a watchdog thread to monitor the status of the SOCKS 5 tunnel
void StartTunnelCheckerThread()
{
    HANDLE thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)MonitorTunnelStatusLoopThread, (LPVOID)NULL, 0, NULL);
    if (thread == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create redirectloop thread (%d)", GetLastError());
        SetProgramTerminated();
        return;
    }
    CloseHandle(thread);
}