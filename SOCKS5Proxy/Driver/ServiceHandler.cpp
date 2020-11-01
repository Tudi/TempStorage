#ifdef _SERVICE_BUILD
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "Logger.h"
#include "ConfigHandler.h"
#include "main.h"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME "xmwfps_service"

int StartServiceDispatcher()
{
    char ServiceName[50];
    strcpy_s(ServiceName, sizeof(ServiceName), SERVICE_NAME);
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {ServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "StartServiceCtrlDispatcher returned error %d", GetLastError());
        return GetLastError();
    }
    return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    DWORD Status = E_FAIL;
    HANDLE hThread = NULL;

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "RegisterServiceCtrlHandler returned error %d", GetLastError());
        goto EXIT;
    }

    // Tell the service controller we are starting
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "SetServiceStatus returned error %d", GetLastError());

     //Perform tasks neccesary to start the service here

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "CreateEvent(g_ServiceStopEvent) returned error %d", GetLastError());

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
            sLog.Log(LL_ERROR, __FILE__, __LINE__, "SetServiceStatus returned error %d", GetLastError());
        goto EXIT;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "SetServiceStatus returned error %d", GetLastError());

    // Start the thread that will perform the main task of the service
    hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject(hThread, INFINITE);


    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "SetServiceStatus returned error %d", GetLastError());

EXIT:
    return;
}


VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        //signall all our threads that it's time to stop performing their stuff
        SetProgramTerminated();

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
            sLog.Log(LL_ERROR, __FILE__, __LINE__, "SetServiceStatus returned error %d", GetLastError());

        // This will signal the worker thread to start shutting down
        SetEvent(g_ServiceStopEvent);
        break;
    default:
        break;
    }
}


DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    //read config file
    //load driver
    //start watchdog threads
    RunApplication();

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0
        && IsWaitingForUserExitProgram()
        )
    {
        Sleep(100);
    }

    //try to gracefull shutdown
    int ShutdownRetrys = 100;
    while (GetRunningThreadCount() && ShutdownRetrys > 0)
    {
        Sleep(100);
        ShutdownRetrys--;
    }

    return ERROR_SUCCESS;
}
#endif