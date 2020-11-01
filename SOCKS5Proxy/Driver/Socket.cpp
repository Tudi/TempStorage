#ifdef USE_NONBLOCKING_SOCKETS
#include "Network.h"

#define SOCKET_BUFFER_SIZE 0xFFFF

SocketGarbageCollector sSocketGarbageCollector;

Socket::Socket(SOCKET fd) : m_fd(fd), m_connected(false),	m_deleted(false)
{
	// Allocate Buffers
	readBuffer.Allocate(SOCKET_BUFFER_SIZE);
	writeBuffer.Allocate(SOCKET_BUFFER_SIZE);

	// IOCP Member Variables
	m_writeLock = 0;
	m_completionPort = 0;

	// Check for needed fd allocation.
	if(m_fd == 0)
		m_fd = SocketOps::CreateTCPFileDescriptor();
	m_empty_reads = 0;
	InitializeCriticalSection(&m_writeMutex);
	InitializeCriticalSection(&m_readMutex);
}

Socket::~Socket()
{
	if( m_fd )
	{
		SocketOps::CloseSocket(m_fd);
		m_fd = 0;	//this is closed already ?
	}
	DeleteCriticalSection(&m_writeMutex);
	DeleteCriticalSection(&m_readMutex);
}

bool Socket::TryRudeDisconnect()
{
	if( m_fd )
	{
		SocketOps::CloseSocket(m_fd);
		m_fd = 0;	//this is closed already ?
		return true;
	}
	return false;
}

bool Socket::Connect(in_addr Address, unsigned int Port)
{
//	struct hostent * ci = gethostbyname(Address);
//	if(ci == 0)
//		return false;

	m_client.sin_family = AF_INET;
	m_client.sin_port = ntohs((u_short)Port);
	memcpy(&m_client.sin_addr.s_addr, &Address, sizeof(Address));

	SocketOps::Blocking(m_fd);
	if(connect(m_fd, (const sockaddr*)&m_client, sizeof(m_client)) == -1)
		return false;

	// at this point the connection was established
	m_completionPort = sSocketMgr.GetCompletionPort();
	_OnConnect();
	return true;
}/**/

void Socket::Accept(sockaddr_in * address)
{
//	memcpy(&m_client, address, sizeof(*address));
	memcpy(&m_client, address, sizeof(sockaddr_in));
	_OnConnect();
}

void Socket::_OnConnect()
{
	// set common parameters on the file descriptor
	SocketOps::Nonblocking(m_fd);
	SocketOps::DisableBuffering(m_fd);
	m_connected = true;

	// IOCP stuff
	AssignToCompletionPort();
	SetupReadEvent();	
	//small chance that SetupReadEvent will delete this socket
	if( IsConnected() )
	{
		sSocketMgr.AddSocket(this);
		// Call virtual onconnect
		OnConnect();
	}
}

bool Socket::Send(const unsigned char * Bytes, unsigned int Size)
{
	bool rv;

	// This is really just a wrapper for all the burst stuff.
	BurstBegin();
	rv = BurstSend(Bytes, Size);
	if(rv)
		BurstPush();
	BurstEnd();

	return rv;
}

bool Socket::BurstSend(const unsigned char * Bytes, unsigned int Size)
{
	return writeBuffer.Write(Bytes, Size);
}

void Socket::Disconnect()
{
	if( m_connected )
	{
		m_connected = false;

		// remove from mgr
		sSocketMgr.RemoveSocket(this);

		SocketOps::CloseSocket(m_fd);
		m_fd = 0;	//this is closed already ?

		// Call virtual ondisconnect
		OnDisconnect();
	}

	if(!m_deleted) 
		Delete();
}

void Socket::Delete()
{
	if(m_deleted) 
		return;
	m_deleted = true;

	if(m_connected) 
		Disconnect();
	sSocketGarbageCollector.QueueSocket(this);
}


#endif