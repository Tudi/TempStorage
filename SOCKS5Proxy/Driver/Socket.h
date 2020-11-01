#ifndef SOCKET_H
#define SOCKET_H

/*********************************************
* AsyncSocket implementation
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
#include <map>
#include <time.h>
#include "SocketDefines.h"
#include "StraightBuffer.h"

class Socket
{
public:
	// Constructor. If fd = 0, it will be assigned 
	Socket(SOCKET fd);
	
	// Destructor.
	virtual ~Socket();

	// Open a connection to another machine.
	bool Connect(in_addr Address, unsigned int Port);

	// Disconnect the socket.
	void Disconnect();
	bool TryRudeDisconnect();	//you would think this should never happen or be used

	// Accept from the already-set fd.
	void Accept(sockaddr_in * address);

/* Implementable methods */

	// Called when data is received.
	virtual void OnRead() {}

	// Called when a connection is first successfully established.
	virtual void OnConnect() {}

	// Called when the socket is disconnected from the client (either forcibly or by the connection dropping)
	virtual void OnDisconnect() {}

/* Sending Operations */

	// Locks sending mutex, adds bytes, unlocks mutex.
	bool Send(const unsigned char * Bytes, unsigned int Size);

	// Burst system - Locks the sending mutex.
	void BurstBegin() { EnterCriticalSection(&m_writeMutex); }

	// Burst system - Adds bytes to output buffer. If possible. Depends on storage buffer size. May fail
	bool BurstSend(const unsigned char * Bytes, unsigned int Size);

	// Burst system - Pushes event to queue - do at the end of write events.
	void BurstPush();

	// Burst system - Unlocks the sending mutex.
	void BurstEnd() { LeaveCriticalSection(&m_writeMutex); }

	// Get the client's ip in numerical form.
	SOCKET GetFd() { return m_fd; }

	void SetupReadEvent();
	void ReadCallback(unsigned int len);
	void WriteCallback();

	bool IsDeleted() { return m_deleted; }
	bool IsConnected() { return m_connected; }
	sockaddr_in & GetRemoteStruct() { return m_client; }
	StraightBuffer& GetReadBuffer() { return readBuffer; }
	StraightBuffer& GetWriteBuffer() { return writeBuffer; }

	void Delete();

	//would be best if we would simply mark how much longer the callback is registered to this struct instead of barbaric fixed values
	unsigned char m_empty_reads;	//number of times nothing was read and waiting timed out
protected:

	// Called when connection is opened.
	void _OnConnect();
  
	SOCKET m_fd;

	StraightBuffer	readBuffer;
	StraightBuffer	writeBuffer;

	CRITICAL_SECTION m_writeMutex;
	CRITICAL_SECTION m_readMutex;

	// we are connected? stop from posting events.
    bool m_connected;

    // We are deleted? Stop us from posting events.
    bool m_deleted;

	sockaddr_in m_client;

public:

	// Set completion port that this socket will be assigned to.
	void SetCompletionPort(HANDLE cp) { m_completionPort = cp; }
	
	// Atomic wrapper functions for increasing read/write locks
	void IncSendLock() { InterlockedIncrement(&m_writeLock); }
	void DecSendLock() { InterlockedDecrement(&m_writeLock); }
	bool AcquireSendLock()
	{
		if(m_writeLock)
			return false;
		else
		{
			IncSendLock();
			return true;
		}
	}
	OverlappedStruct m_readEvent;
	OverlappedStruct m_writeEvent;

private:
	// Completion port socket is assigned to
	HANDLE m_completionPort;
	
	// Write lock, stops multiple write events from being posted.
	volatile long m_writeLock;
	
	// Assigns the socket to his completion port.
	void AssignToCompletionPort();
};

/* Socket Garbage Collector */
#define SOCKET_GC_TIMEOUT 40

class SocketGarbageCollector
{
	map<Socket*, time_t> deletionQueue;
	CRITICAL_SECTION lock;
public:
	SocketGarbageCollector()
	{
		InitializeCriticalSection(&lock);
	}
	~SocketGarbageCollector()
	{
		DeleteCriticalSection(&lock);
		map<Socket*, time_t>::iterator i;
		for(i=deletionQueue.begin();i!=deletionQueue.end();++i)
			delete i->first;
		deletionQueue.clear();
	}
	void NoDeleteClear() { deletionQueue.clear(); }
	void Update()
	{
		map<Socket*, time_t>::iterator i, i2;
		time_t t = time(NULL);
		EnterCriticalSection(&lock);
		for(i = deletionQueue.begin(); i != deletionQueue.end();)
		{
			i2 = i++;
			if(i2->second <= t)
			{
				//maybe some morron forgot that callbacks are registered to sockets and deleting the data holder without the events is bad
				if( i2->first->IsConnected() == true 
//					|| i2->first->GetFd() //sockets that never manage to connect anywhere will probably have this non 0. Destructor should handle closing the handle
					|| i2->first->IsDeleted() == false 
					|| i2->first->m_readEvent.m_inUse || i2->first->m_writeEvent.m_inUse // wow, this will actually trigger a lot it seems.
					)
				{
//					if( i2->first->IsConnected() == true || i2->first->IsDeleted() == false ) 
//						ASSERT( false );
					//in case for some fucked up reason the socket was still open then give some more time then delete callback holder
					if( i2->first->TryRudeDisconnect() )
					{
						i2->second = time(NULL) + SOCKET_GC_TIMEOUT;
						continue; 
					}
				}
				delete i2->first;
//				sLog.outDebug("Deleteing socket : %p",i2->first);
				deletionQueue.erase(i2);
			}
		}
		LeaveCriticalSection(&lock);
	}

	void QueueSocket(Socket * s)
	{
		EnterCriticalSection(&lock);

		//check for double delete
		map<Socket*, time_t>::iterator i;
		for(i = deletionQueue.begin(); i != deletionQueue.end();i++)
			if( i->first == s )
			{
				LeaveCriticalSection(&lock);
				return;
			}

		deletionQueue.insert( map<Socket*, time_t>::value_type( s, time(NULL) + SOCKET_GC_TIMEOUT ) );
		LeaveCriticalSection(&lock);
	}
};

extern SocketGarbageCollector sSocketGarbageCollector;

#endif
#endif