#ifdef USE_NONBLOCKING_SOCKETS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "proxy.h"
#include "Utils.h"
#include "Socks5Proxy.h"
#include "FlowManager.h"
#include "ConfigHandler.h"
#include "Logger.h"

#include "Socket.h"
#include "ListenSocketWin32.h"
#include "proxy_socket.h"

DWORD proxy(LPVOID arg)
{
	//needs to be done dynamically or else threadpool is not ready to accept the threads
	sSocketMgr.SpawnWorkerThreads();

	ListenSocket<AppToProxySocket>* ls = new ListenSocket<AppToProxySocket>("0.0.0.0", GetOurProxyPort());
	if (ls == NULL)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create listen socket (%d)", GetLastError());
		SetProgramTerminated();
		return -1;
	}
	bool listnersockcreate = ls->IsOpen();
	if (listnersockcreate)
		ThreadPool.ExecuteTask(ls);
	while (IsWaitingForUserExitProgram() && listnersockcreate)
	{
		sSocketGarbageCollector.Update();	//handled by garbage collector in worldserver 
		Sleep(100);
	}
	//stop listening to incomming connections
	ls->Close();
	//kill threads and sockets
	sSocketMgr.ShutdownThreads();
	sSocketMgr.CloseAll();
	ThreadPool.Shutdown();
	return 0;
}


void ShutdownProxyThread()
{
}
#endif