#ifndef SOCKET_DEFINES_H
#define SOCKET_DEFINES_H

/*********************************************
* AsyncSocket implementation
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
using namespace std;

enum SocketIOEvent
{
	SOCKET_IO_EVENT_READ_COMPLETE   = 0,
	SOCKET_IO_EVENT_WRITE_END		= 1,
	SOCKET_IO_THREAD_SHUTDOWN		= 2,
	NUM_SOCKET_IO_EVENTS			= 3,
};

class OverlappedStruct
{
public:
	OverlappedStruct(SocketIOEvent ev) : m_event(ev)
	{
		memset(&m_overlap, 0, sizeof(OVERLAPPED));
		m_inUse = 0;
	};

	OverlappedStruct()
	{
		memset(&m_overlap, 0, sizeof(OVERLAPPED));
		m_inUse = 0;
	}

	__forceinline void Reset(SocketIOEvent ev)
	{
		memset(&m_overlap, 0, sizeof(OVERLAPPED));
		m_event = ev;
	}

	void Mark()
	{
		long val = InterlockedCompareExchange(&m_inUse, 1, 0);
	}

	void Unmark()
	{
		InterlockedExchange(&m_inUse, 0);
	}
//private:
	OVERLAPPED m_overlap;
	SocketIOEvent m_event;
	volatile long m_inUse;
};

#endif
#endif