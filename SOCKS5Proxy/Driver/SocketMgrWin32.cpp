#ifdef USE_NONBLOCKING_SOCKETS
#include "Network.h"
#include <list>
#include "SocketMgrWin32.h"

SocketMgr sSocketMgr;

SocketMgr::SocketMgr()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
	InitializeCriticalSection(&socketLock);
	//!! needs to be called dynamically because it depends on threadpool
//	SpawnWorkerThreads();
}

SocketMgr::~SocketMgr()
{
	DeleteCriticalSection(&socketLock);
}

void SocketMgr::SpawnWorkerThreads()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	threadcount = si.dwNumberOfProcessors;

//	printf("IOCP: Spawning %u worker threads.\n", threadcount);
	for(long x = 0; x < threadcount; ++x)
		ThreadPool.ExecuteTask(new SocketWorkerThread());
}

#define SOCKET_EVENT_WAIT_TIMOUT	1000

bool SocketWorkerThread::run()
{
	HANDLE cp = sSocketMgr.GetCompletionPort();
	DWORD len;
	Socket * s;
	OverlappedStruct * ov;
	LPOVERLAPPED ol_ptr;

	while(true)
	{
		if(!GetQueuedCompletionStatus(cp, &len, (PULONG_PTR)&s, &ol_ptr, SOCKET_EVENT_WAIT_TIMOUT))
			continue;

		ov = CONTAINING_RECORD(ol_ptr, OverlappedStruct, m_overlap);

		if(ov->m_event == SOCKET_IO_THREAD_SHUTDOWN)
		{
			delete ov;
			ov = NULL;
			return true;
		}

		if(ov->m_event < NUM_SOCKET_IO_EVENTS)
			ophandlers[ov->m_event](s, len);
	}

	return true;
}

void HandleReadComplete(Socket * s, unsigned int len)
{
	//s->m_readEvent=NULL;
	if(!s->IsDeleted())
	{
		s->m_readEvent.Unmark();
		if(len)
		{
			s->GetReadBuffer().IncrementWritten(len);
			s->OnRead();
			s->SetupReadEvent();
		}
		else
		{
			//this is required due to the very slow connecting connections that get timed out on read even and might get deleted even before they finish connection
			//best would be to know when all registered callbacks will expire and then delete ourself
			s->m_empty_reads++;
			if( s->m_empty_reads > 100000 / SOCKET_EVENT_WAIT_TIMOUT )
				s->Delete();	  // Queue deletion.
		}
	}
}

void HandleWriteComplete(Socket * s, unsigned int len)
{
	if(!s->IsDeleted())
	{
		s->m_writeEvent.Unmark();
		s->BurstBegin();					// Lock
		s->GetWriteBuffer().Remove(len);
		if( s->GetWriteBuffer().GetSize() > 0 )
			s->WriteCallback();
		else
			s->DecSendLock();
		s->BurstEnd();					  // Unlock
	}
}

void HandleShutdown(Socket * s, unsigned int len)
{
	
}

void SocketMgr::CloseAll()
{
	list<Socket*> tokill;

	EnterCriticalSection(&socketLock);
	for(set<Socket*>::iterator itr = _sockets.begin(); itr != _sockets.end(); ++itr)
		tokill.push_back(*itr);
	LeaveCriticalSection(&socketLock);
	
	for(list<Socket*>::iterator itr = tokill.begin(); itr != tokill.end(); ++itr)
		(*itr)->Disconnect();

	size_t size;
	do
	{
		EnterCriticalSection(&socketLock);
		size = _sockets.size();
		LeaveCriticalSection(&socketLock);
	}while(size);
}

void SocketMgr::ShutdownThreads()
{
	for(int i = 0; i < threadcount; ++i)
	{
		OverlappedStruct * ov = new OverlappedStruct(SOCKET_IO_THREAD_SHUTDOWN);
		PostQueuedCompletionStatus(m_completionPort, 0, (ULONG_PTR)0, &ov->m_overlap);
	}
}
#endif