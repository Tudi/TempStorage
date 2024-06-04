#pragma once

#include <vector>

#define UDP_WORKERTHREAD_SLEEP	1000
#define INVALID_LARGET_THREADCOUNT	100
#define EXPECTED_NORMAL_PACKET_PER_SECOND	4
#define SPAWN_WORKER_THREAD_AFTER_X_ALERTS	10

class WatchdogThreadData;

// used by the server to periodically update sessions
// Has N worker threads that will try to split the work
class UDPServerManager
{
public:
	inline static UDPServerManager& getInstance()
	{
		// This is a thread-safe way to create the instance
#ifdef _DEBUG
		static UDPServerManager* instance = new UDPServerManager();
		return *instance;
#else
		static UDPServerManager instance;
		return instance;
#endif
	}
	~UDPServerManager();
	void DestructorCheckMemLeaks();

	// Creates N worker threads that will process incomming packets
	void Init(size_t maxNetworkThreads, unsigned short listenPort);
	void ShutDownServer();
private:
	UDPServerManager();
	UDPServerManager(const UDPServerManager&) = delete;
	UDPServerManager& operator=(const UDPServerManager&) = delete;

	// the worker function that will process sessions
	static void UDPServerManager_AsyncExecuteThread(WatchdogThreadData* wtd);

#ifdef _WIN32
	SOCKET m_dSocket;
#else
	int m_dSocket;
#endif
};

#define sUDPServerManager UDPServerManager::getInstance()
