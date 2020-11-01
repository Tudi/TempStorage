#ifndef SOCKETMGR_H_WIN32
#define SOCKETMGR_H_WIN32

/*********************************************
* AsyncSocket thread to listen on socket events and call callback functions based on events
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
#include <set>
#include "Network.h"
#include "Threading.h"

class Socket;
class SocketMgr
{
public:
	SocketMgr();
	~SocketMgr();

	HANDLE GetCompletionPort() { return m_completionPort; }
	void SpawnWorkerThreads();
	void CloseAll();
	void AddSocket(Socket * s)
	{
		EnterCriticalSection(&socketLock);
		_sockets.insert(s);
		LeaveCriticalSection(&socketLock);
	}

	void RemoveSocket(Socket * s)
	{
		EnterCriticalSection(&socketLock);
		_sockets.erase(s);
		LeaveCriticalSection(&socketLock);
	}

	void ShutdownThreads();
	long threadcount;

private:
	HANDLE m_completionPort;
	set<Socket*> _sockets;
	CRITICAL_SECTION socketLock;
};

typedef void(*OperationHandler)(Socket * s, unsigned int len);

class SocketWorkerThread : public ThreadBase
{
public:
	bool run();
};

void HandleReadComplete(Socket * s, unsigned int len);
void HandleWriteComplete(Socket * s, unsigned int len);
void HandleShutdown(Socket * s, unsigned int len);

static OperationHandler ophandlers[NUM_SOCKET_IO_EVENTS] = {
	&HandleReadComplete,
	&HandleWriteComplete,
	&HandleShutdown };

extern SocketMgr sSocketMgr;
#endif
#endif