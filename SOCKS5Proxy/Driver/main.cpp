#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ConfigHandler.h"
#include "DriverHandler.h"
#include "TunnelStatus.h"
#include "Logger.h"
#include "RestartConnections.h"
#include "CommHandler.h"
#include "ServiceHandler.h"
#include "FirewallUtil.h"
#include "Licensing.h"

void RunApplication()
{
    //load values from config
    char FullPath[MAX_PATH];
    sprintf_s(FullPath, sizeof(FullPath), "%stunnel.cfg", GetFullPath());
    LoadConfigValues(FullPath);
    if (ConfigFileContainedSetupValues() == 0)
        sLog.Log(LL_Warning, __FILE__, __LINE__, "tunnel.cfg did not contain all required config values");

    //make sure we unblock ourself from firewall
    AddFirewallRule();
    
    //background thread to see if external commands are comming in
    StartCommandsListener();

    //wait for the SOCKS 5 tunnel to come online
    //no longer valid since we can have multiple socks tunnels
    StartTunnelCheckerThread();

    //this will load the driver 
    sLog.Log(LL_Info, __FILE__, __LINE__, "Starting driver");
    SetupDriver();

    //start redirecting packets
    sLog.Log(LL_Info, __FILE__, __LINE__, "Start redirecting packets");
    StartRedirectLoop();

    //start licensing monitor thread
    StartLicenseMonitorThread();

#ifndef _SERVICE_BUILD // service is already running in a thread that is waiting for the stop event
    //when we exit this loop, everything should shut down
    while (IsWaitingForUserExitProgram())
    {
        Sleep(100);
    }
    //wait for all threads to exit
    while (GetRunningThreadCount())
        Sleep(100);
#endif
}

int __cdecl main(int argc, char** argv)
{
    //do not allow multiple instances of the same process to be run
    //This protection is not global. If you get to a point where you fight a protection, it's bad
    FILE* UniqueInstance;
    errno_t er = fopen_s(&UniqueInstance, "PID", "wt");
    if (UniqueInstance == NULL)
        return -1;
    fprintf(UniqueInstance, "%d", GetProcessId(NULL));

#ifdef _SERVICE_BUILD
    StartServiceDispatcher();
#else
    RunApplication();
#endif

    fclose(UniqueInstance);
    remove("PID");
    return 0;
}
