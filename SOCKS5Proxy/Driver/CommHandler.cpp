#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "Logger.h"
#include "ConfigHandler.h"
#include "Socks5Proxy.h"
#include "CommHandler.h"
#include "RestartConnections.h"
#include "Licensing.h"

#pragma pack(push,1)
struct CommandPacket
{
    unsigned short Size;
    unsigned short Cmd;
};
#pragma pack(pop)

int ReadAndHandleCommandFromSocket(SOCKET s)
{
    CommandPacket cmd;
    int len;
    len = recvData(s, &cmd, sizeof(CommandPacket), true);
    if (len != sizeof(CommandPacket))
    {
        //sLog.Log(LL_Warning, __FILE__, __LINE__, "failed to recv from socket (%d)", WSAGetLastError());
        return -1;
    }

    switch (cmd.Cmd)
    {
        case ECT_StartRedirecting:
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : start");
            SetPacketRedirectionStatus(1);
        }break;
        case ECT_StopRedirecting:
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : stop");
            SetPacketRedirectionStatus(0);
        }break;
        case ECT_DropConnections:
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : drop");
            RestartNetworkConnections(0);
        }break;
        case ECT_DropStartConnections:
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : drop start");
            RestartNetworkConnections(1);
        }break;
        case ECT_LoadConfigFile:
        {
            unsigned short PathSize;
            len = recvData(s, &PathSize, sizeof(PathSize), false);
            if (len != sizeof(PathSize))
                return -1;
            char* String = (char*)malloc(PathSize+32);
            len = recvData(s, String, PathSize, false);
            if (len != PathSize)
                return -1;
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : config=%s", String);
            LoadConfigValues(String);
            if (ConfigFileContainedSetupValues() == 0)
                sLog.Log(LL_Warning, __FILE__, __LINE__, "%s did not contain all required config values", String);
            free(String);
        }break;
        case ECT_ExitDriver:
        {
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : exit");
            SetProgramTerminated();
        }break;
        case ECT_GetLicenseStatus:
        {
            unsigned short DataSize;
            len = recvData(s, &DataSize, sizeof(DataSize), false);
            if (len != sizeof(DataSize))
                return -1;
            char* Data = (char*)malloc(DataSize + 32);
            len = recvData(s, Data, DataSize, false);
            if (len != DataSize)
                return -1;
            LicenseStatusCodes ls = GetLicenseStatus();
            //if license status of the driver is not the same as the license status of the tunnel.exe, something is strange and we should try to reset our status
            if(Data[0] != 0xFF && ls != (LicenseStatusCodes)Data[0])
                SetProgramTerminated();
            len = sendData(s, &ls, 1);
            sLog.Log(LL_Info, __FILE__, __LINE__, "Received command : license status");
        }break;
        default:
        break;
    }

    return 0;
}

DWORD ListenExternalCommandsThread(LPVOID arg)
{
    UINT16 port = GetCommandsPort();
    int on = 1;
    WSADATA wsa_data;
    WORD wsa_version = MAKEWORD(2, 2);
    struct sockaddr_in addr;
    SOCKET s;
    AutoMonitorThreadExit AM; //don't delete me

    if (WSAStartup(wsa_version, &wsa_data) != 0)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to start WSA (%d)", GetLastError());
        SetProgramTerminated();
        return -1;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create socket (%d)", WSAGetLastError());
        SetProgramTerminated();
        return -1;
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == SOCKET_ERROR)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to re-use address (%d)", GetLastError());
        SetProgramTerminated();
        return -1;
    }

    sLog.Log(LL_Info, __FILE__, __LINE__, "Command server listening on port : %d", port);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (bind(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to bind socket (%d)", WSAGetLastError());
        SetProgramTerminated();
        return -1;
    }

    if (listen(s, 16) == SOCKET_ERROR)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to listen socket (%d)", WSAGetLastError());
        SetProgramTerminated();
        return -1;
    }

    while (IsWaitingForUserExitProgram())
    {
        // Wait for a new connection.
        int size = sizeof(addr);
        SOCKET t = accept(s, (SOCKADDR*)&addr, &size);
        if (t == INVALID_SOCKET)
        {
            sLog.Log(LL_Warning, __FILE__, __LINE__, "CommHandler:failed to accept socket (%d)", WSAGetLastError());
            continue;
        }
        //read the command. Should run this in a separate thread, but i do no expect a lot of traffic here
        while(ReadAndHandleCommandFromSocket(t) != -1);
        //kill connection
        shutdown(t, SD_BOTH);
        closesocket(t);
    }
    return 0;
}

void StartCommandsListener()
{
    HANDLE thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)ListenExternalCommandsThread, (LPVOID)NULL, 0, NULL);
    if (thread == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create commands listener thread (%d)", GetLastError());
        SetProgramTerminated();
        return;
    }
    CloseHandle(thread);
}