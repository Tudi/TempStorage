#pragma once

#include "Util/VectRW.h"

class WatchdogThreadData;
class NetworkSession;

// used by the server to periodically update sessions
// Has N worker threads that will try to split the work
class NetworkSessionManager
{
public:
	inline static NetworkSessionManager& getInstance()
	{
		// This is a thread-safe way to create the instance
#ifdef _DEBUG
		static NetworkSessionManager* instance = new NetworkSessionManager();
		return *instance;
#else
		static NetworkSessionManager instance;
		return instance;
#endif
	}
	~NetworkSessionManager();
	void DestructorCheckMemLeaks();

	// Function to create a new session and return a shared pointer to it
	NetworkSession* CreateSession();
	void DestroySession(NetworkSession*);

	// Creates N worker threads that will process incomming packets
	void Init(size_t networkThreads);
	// when a session receives a packet, it will try to wake up a worker thread to get it processed
	void WakeUpWorkerThread();
	void WakeUpAllWorkerThreads();
private:
	NetworkSessionManager()
	{
		m_dReadIndex = 0;
	};
	NetworkSessionManager(const NetworkSessionManager&) = delete;
	NetworkSessionManager& operator=(const NetworkSessionManager&) = delete;

	// the worker function that will process sessions
	friend void NetworkManager_AsyncExecuteThread(WatchdogThreadData* wtd);
	// worker threads will iterate through all sessions and process all of them
	std::mutex m_ReadIndexLock;
	size_t m_dReadIndex;

	// session lifetime will be managed by us
	RWLockedVector<NetworkSession*> m_Sessions;
};

#define sNetworkSessionManager NetworkSessionManager::getInstance()
