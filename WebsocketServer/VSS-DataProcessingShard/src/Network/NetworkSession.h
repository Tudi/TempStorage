#pragma once

#include <set>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "Network/VSSPacketDefines.h"
#include "Util/VectRW.h"
#include "Util/ObjDescriptor.h"

// only for non active threads
#define NETWORK_THREAD_SLEEP	5000
#define NETWORK_PACKET_SLOTS_RESERVED	3 // handshake, subscribe, unsubscribe ..

typedef struct ModuleSubscription
{
	void Init(__int64 ModuleId, __int64 DataRate);
	__int64 m_dModuleId;
//	__int64 m_dDataRateRequested, m_dDataSent, m_dInitStamp;
}Modulesubscription;

class NetworkSession
{
public:
	REFLECT_TYPE(NetworkSession);
	NetworkSession(__int64 ID);
	~NetworkSession();
	/// <summary>
	/// Client sent us all the details so we can do DB queries
	/// </summary>
	bool IsInitialized() { return m_bInitConfirmed; }
	/// <summary>
	/// Received from SessionManager. This object can be ontained from Session manager using this ID
	/// </summary>
	__int64 GetSessionId() { return m_dUUID; }
	/// <summary>
	/// Right now there is 1 server thread. It will queue up packets to sessions
	/// NetworkManager will have a threadpool that will search for jobs and parse packets
	/// A different approach could be to queue all packets to network manager, threads would pop packets 1 by 1. Should benchmark which is better
	/// </summary>
	void OnMessage(std::string& msg);
	/// <summary>
	/// A worker thread is parsing received packets
	/// returns false if thread failed to aquire lock
	/// </summary>
	bool OnPeriodicUpdate();
	/// <summary>
	/// Client handshake data were valid. We save them internally
	/// </summary>
	void OnLoginSuccess(__int64 SessionId, __int64 UserId, __int64 Salt);
	/// <summary>
	/// Receive Alerts, Feed from this Module
	/// </summary>
	void OnSubscribeToModule(VSS_N_SubscribeModules::SubscriptionType SubType, const VSS_N_SubscribeModules::ModuleSubscriptionDetail &details);
	/// <summary>
	/// No longer wish to receive data from this Module
	/// </summary>
	void OnUnSubscribeFromModule(VSS_N_SubscribeModules::SubscriptionType SubType, __int64 ModuleId);
	// right now this is used in 1 place. Thread concurency is not an issue
	void AddRef() { m_dReferenceCounter++; }
	int DecRef() { return --m_dReferenceCounter; }
	/// <summary>
	/// Non threadsafe way to check for a worker thread if it should lock the object to process packets
	/// </summary>
	bool HasPacketsQueued() const { return !m_packetQueue[m_packetQueueWriteIndex].empty(); }	
	/// <summary>
	/// In a normal implementation this could send data directly, but out WS library forces us to relay it to the server
	/// </summary>
	bool SendMsg(const char* data, const size_t len);
private:
	// when data source manager receives a module data packet, it will call this function with the new data
	// depending on DS implementation, our callback function should not waste too much time 
	static void CB_OnModuleDataArrived(void* CBData, void* self, void* unused);
	static void CB_OnModuleAlertArrived(void* CBData, void* self, void* unused);
	bool m_bInitSent, m_bInitConfirmed;
	int m_dHeartbeatSnt, m_dHeartbeatRcv;
	__int64 m_dUserId;
	__int64 m_lldSessionId; // this is the ID stored in DB to identify a session
	__int64 m_dSessionSalt;	// used to call backend API calls
	__int64 m_dDataRateRequested, m_dDataSent, m_dInitStamp;
	std::vector<Modulesubscription*> m_SubscribedModulesAlert;
	std::vector<Modulesubscription*> m_SubscribedModulesFeed;
	__int64 m_dUUID; // session id. Internal for the sake of logging
	int m_dReferenceCounter; // can only be destroyed if reference counter reaches 0
	std::vector<std::string> m_packetQueue[2];
	std::mutex m_packetQueueLock[2];
	__int64 m_packetQueueWriteIndex; // one is always written by Server. One is read by worker threads
	class ServerNetworkContext *m_SrvCtx; // this is just for the sake of code consistency
};
