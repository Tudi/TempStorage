#include <Windows.h>
#include "ResourceManager/DataSourceManager.h"
#include "Network/NetworkSession.h"
#include "Allocator.h"
#include "LogManager.h"
#include "ApplicationSession.h"
#include "Network/NetworkSessionManager.h"

NetworkSessionManager::~NetworkSessionManager()
{
	DestructorCheckMemLeaks();
}

void NetworkSessionManager::DestructorCheckMemLeaks()
{
	m_Sessions.WriteLock();
	for (size_t i = 0; i < m_Sessions.GetVect().size(); i++)
	{
		InternalDelete(m_Sessions.GetVect()[i]);
	}
	m_Sessions.GetVect().clear();
	m_Sessions.WriteUnlock();
}

void NetworkManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		// update every network session
		std::vector<NetworkSession*>& sess = sNetworkSessionManager.m_Sessions.GetVect();
		if (!sess.empty())
		{
			sNetworkSessionManager.m_Sessions.ReadLock();
			{
				std::unique_lock<std::mutex> lock(sNetworkSessionManager.m_ReadIndexLock, std::defer_lock);
				for (auto itr : sess)
				{
					if (itr->HasPacketsQueued())
					{
						if (itr->OnPeriodicUpdate())
						{
							break;
						}
					}
				}
			}
			// allow server to create new sessions
			sNetworkSessionManager.m_Sessions.ReadUnlock();
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

void NetworkSessionManager::Init(size_t networkThreads)
{
	// create worker thread
	sAppSession.CreateWorkerThreadGroup(WTG_WS_Packet_Parser, (int)networkThreads, "WSParser", NetworkManager_AsyncExecuteThread, NETWORK_THREAD_SLEEP);
}

NetworkSession* NetworkSessionManager::CreateSession()
{
	static __int64 GlobalNetworkSessionUniqueId = 0;
	__int64 thisID = ++GlobalNetworkSessionUniqueId;

	NetworkSession* session;
	InternalNew(session, NetworkSession, thisID);
	session->AddRef();

	// Store the session in the vector
	m_Sessions.WriteLock();
	m_Sessions.GetVect().push_back(session);
	m_Sessions.WriteUnlock();

	return session;
}

void NetworkSessionManager::DestroySession(NetworkSession* session)
{
	// Lock the mutex when modifying shared data
	m_Sessions.WriteLock();
	for (auto itr = m_Sessions.GetVect().begin(); itr != m_Sessions.GetVect().end(); itr++)
	{
		if (*itr == session)
		{
			int refCount = (*itr)->DecRef();
			if (refCount == 0)
			{
				InternalDelete((*itr));
				m_Sessions.GetVect().erase(itr);
			}
			break;
		}
	}
	m_Sessions.WriteUnlock();
}

void NetworkSessionManager::WakeUpWorkerThread()
{
	sAppSession.WakeUpWorkerThread(WTG_WS_Packet_Parser);
}

void NetworkSessionManager::WakeUpAllWorkerThreads()
{
	sAppSession.WakeUpWorkerThread(WTG_WS_Packet_Parser, true);
}
