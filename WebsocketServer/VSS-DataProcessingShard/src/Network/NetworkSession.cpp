#include <Windows.h>
#include "ResourceManager/DataSourceManager.h"
#include "NetworkSession.h"
#include "Allocator.h"
#include "LogManager.h"
#include "ApplicationSession.h"
#include "Network/WSServer.h"

NetworkSession::NetworkSession(__int64 ID)
{
	m_dUUID = ID;

	m_bInitSent = false;
	m_bInitConfirmed = false;
	m_dHeartbeatSnt = m_dHeartbeatRcv = 0;
	m_dUserId = 0;
	m_lldSessionId = 0;
	m_dSessionSalt = 0;
	m_dDataRateRequested = 0;
	m_dDataSent = 0;
	m_dInitStamp = 0;
	m_dReferenceCounter = 0;

	m_packetQueueWriteIndex = 0;
	m_packetQueue[0].reserve(NETWORK_PACKET_SLOTS_RESERVED);
	m_packetQueue[1].reserve(NETWORK_PACKET_SLOTS_RESERVED);

	InternalNew(m_SrvCtx, ServerNetworkContext, this);

	InitTypeInfo();
}

NetworkSession::~NetworkSession()
{
	std::unique_lock<std::mutex> lock1(m_packetQueueLock[0]);
	std::unique_lock<std::mutex> lock2(m_packetQueueLock[1]);
	m_packetQueue[0].clear();
	m_packetQueue[1].clear();

	// make no mistakes. Unsubscribe all callbacks to this object
	sDSManager.UnSunscribeAll(this);

	// free memory
	for (std::vector<Modulesubscription*>::iterator itr = m_SubscribedModulesAlert.begin(); itr != m_SubscribedModulesAlert.end(); itr++)
	{
		InternalFree(*itr);
	}
	m_SubscribedModulesAlert.clear();

	for (std::vector<Modulesubscription*>::iterator itr = m_SubscribedModulesFeed.begin(); itr != m_SubscribedModulesFeed.end(); itr++)
	{
		InternalFree(*itr);
	}
	m_SubscribedModulesFeed.clear();

	InternalDelete(m_SrvCtx);
}

void NetworkSession::OnMessage(std::string& msg)
{
	std::unique_lock<std::mutex> lock(m_packetQueueLock[m_packetQueueWriteIndex]);
	std::string& last = m_packetQueue[m_packetQueueWriteIndex].emplace_back();
	last = std::move(msg);
}

// need to make the queue thread safe !
bool NetworkSession::OnPeriodicUpdate()
{
	if (m_packetQueueLock[m_packetQueueWriteIndex].try_lock())
	{
		const size_t readIndex = m_packetQueueWriteIndex;
		m_packetQueueWriteIndex = 1 - m_packetQueueWriteIndex; // switch writing to the other storage

		for (auto itr : m_packetQueue[readIndex])
		{
			std::string& val = itr;
			VSS_N_PacketHeader* ph = (VSS_N_PacketHeader*)val.c_str();

			// valid packet ?
			if (ph->Size != val.length())
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
					"NetworkSession:Network Packet Size %d does not match internal size %lld. Aborting.", ph->Size, val.length());
				continue;
			}
			if (ph->Opcode >= VSSWSO_MAX)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
					"NetworkSession:Network Packet Opcode %d greater than max. Aborting.", ph->Opcode, VSSWSO_MAX);
				continue;
			}
			if (sPacketHandlers[ph->Opcode] == NULL)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
					"NetworkSession:Missing packet handler. Opcode %d. Aborting.", ph->Opcode);
				continue;
			}

			sPacketHandlers[ph->Opcode]->ParserFunc(val.c_str(), ph->Size, m_SrvCtx);
		}

		// we processed all the packets
		m_packetQueue[readIndex].clear();

		// we are done processing this packet array
		m_packetQueueLock[readIndex].unlock();

		return true;
	}

	// failed to process packet list
	return false;
}

void NetworkSession::OnLoginSuccess(__int64 SessionId, __int64 UserId, __int64 Salt)
{
	m_lldSessionId = SessionId;
	m_dUserId = UserId;
	m_dSessionSalt = Salt;
	m_bInitConfirmed = true;
	m_bInitSent = true;
}

void NetworkSession::OnSubscribeToModule(VSS_N_SubscribeModules::SubscriptionType SubType, 
	const VSS_N_SubscribeModules::ModuleSubscriptionDetail& details)
{
	std::vector<Modulesubscription*> *dstModules = &m_SubscribedModulesAlert;
	if (SubType == VSS_N_SubscribeModules::SubscriptionType::SubscribeFeed)
	{
		dstModules = &m_SubscribedModulesFeed;
	}
	// this list is changed when parsing packets. Only 1 thread can parse packets. No need for thread safety
	for (std::vector<Modulesubscription*>::iterator itr = dstModules->begin(); itr != dstModules->end(); itr++)
	{
		if ((*itr)->m_dModuleId == details.ModuleId)
		{
			return;
		}
	}

	Modulesubscription* rs = (Modulesubscription *)InternalMalloc(sizeof(Modulesubscription));
	rs->Init(details.ModuleId, 0);
	dstModules->push_back(rs);

	if (SubType == VSS_N_SubscribeModules::SubscriptionType::SubscribeFeed)
	{
		sDSManager.SubscribeToModule(SubType, details.ModuleId, NetworkSession::CB_OnModuleDataArrived, this, NULL);
	}
	else
	{
		sDSManager.SubscribeToModule(SubType, details.ModuleId, NetworkSession::CB_OnModuleAlertArrived, this, NULL);
	}

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
		"NetworkSession:Subscribed to Module %lld Alerts=%d, Feed=%d",
		details.ModuleId, SubType == VSS_N_SubscribeModules::SubscriptionType::SubscribeAlert,
		SubType == VSS_N_SubscribeModules::SubscriptionType::SubscribeFeed);
}

void NetworkSession::OnUnSubscribeFromModule(VSS_N_SubscribeModules::SubscriptionType SubType, __int64 ModuleId)
{
	// unsubscribe notifications first so that we would not need list locking
	if (SubType == VSS_N_SubscribeModules::SubscriptionType::UnsubscribeFeed)
	{
		sDSManager.UnSubscribeFromModule(SubType, ModuleId, CB_OnModuleDataArrived, this, NULL);
	}
	else
	{
		sDSManager.UnSubscribeFromModule(SubType, ModuleId, CB_OnModuleAlertArrived, this, NULL);
	}

	std::vector<Modulesubscription*> *dstModules = &m_SubscribedModulesAlert;
	if (SubType == VSS_N_SubscribeModules::SubscriptionType::UnsubscribeFeed)
	{
		dstModules = &m_SubscribedModulesFeed;
	}
	// this list is changed when parsing packets. Only 1 thread can parse packets. No need for thread safety
	for (std::vector<Modulesubscription*>::iterator itr = dstModules->begin(); itr != dstModules->end(); itr++)
	{
		if ((*itr)->m_dModuleId == ModuleId)
		{
			InternalFree(*itr);
			dstModules->erase(itr);

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDatabaseManager, 0, 0,
				"NetworkSession:Unsubscribed Module %lld. Alert=%d, Feed=%d", ModuleId,
				SubType == VSS_N_SubscribeModules::SubscriptionType::UnsubscribeAlert,
				SubType == VSS_N_SubscribeModules::SubscriptionType::UnsubscribeFeed);

			break;
		}
	}
}

void Modulesubscription::Init(__int64 ModuleId, __int64 DataRate)
{
	DataRate;
	m_dModuleId = ModuleId;
//	m_dDataRateRequested = DataRate;
//	m_dDataSent = 0;
//	m_dInitStamp = GetTickCount64();
}

void NetworkSession::CB_OnModuleDataArrived(void* CBData, void* self, void* unused)
{
	unused; self;
	DSModuleData* md = typecheck_castL(DSModuleData, CBData);
	NetworkSession* this_ = typecheck_castL(NetworkSession, self);

	if (this_->IsInitialized() == false)
	{
		return;
	}

#ifdef _DEBUG
	bool bConfirmedSubscription = false;
	for (auto itr = this_->m_SubscribedModulesFeed.begin(); itr != this_->m_SubscribedModulesFeed.end(); itr++)
	{
		if ((*itr)->m_dModuleId == md->ModuleID)
		{
			bConfirmedSubscription = true;
			break;
		}
	}
	if (bConfirmedSubscription == false)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceNetworkManager, 0, 0,
			"NetworkSession:Unexpected ModuleId %lld feed notification. Not subscribed", md->ModuleID);
	}
#endif

	// send this data to the UI connected through the network session
	GENERIC_PACKET_INIT(VSS_WS_Opcodes::VSSWSO_S_ModuleDataStatus, VSS_N_ModuleObjectState, md->size());
	pktOut->ModuleID = md->ModuleID;
	pktOut->Timestamp = md->Timestamp;
	for (size_t i = 0; i < pktOut->Counter; i++)
	{
		pktOut->ObjectStates[i].id = md->Objects[i].ObjectId;
		pktOut->ObjectStates[i].x = md->Objects[i].x;
		pktOut->ObjectStates[i].y = md->Objects[i].y;
//		strcpy_s(pktOut->ObjectStates[i].tags, md->Objects[i].c_str());
	}

	this_->SendMsg(packetBuff, pktSize);
}

void NetworkSession::CB_OnModuleAlertArrived(void* CBData, void* self, void* unused)
{
	unused; self;
	DSModuleAlert* ma = typecheck_castL(DSModuleAlert, CBData);
	NetworkSession* this_ = typecheck_castL(NetworkSession, self);

	if (this_->IsInitialized() == false)
	{
		return;
	}

#ifdef _DEBUG
	bool bConfirmedSubscription = false;
	for (auto itr = this_->m_SubscribedModulesAlert.begin(); itr != this_->m_SubscribedModulesAlert.end(); itr++)
	{
		if ((*itr)->m_dModuleId == ma->ModuleID)
		{
			bConfirmedSubscription = true;
			break;
		}
	}
	if (bConfirmedSubscription == false)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceNetworkManager, 0, 0,
			"NetworkSession:Unexpected ModuleId %lld Alert notification. Not subscribed", ma->ModuleID);
	}
#endif

	// we could filter so that only UI's with loggged in users will get the Alert popup
	if (ma->UserId != 0 && this_->m_dUserId != ma->UserId)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceNetworkManager, 0, 0,
			"NetworkSession:Logged in User is different than the alert type is created. Skip ?");
	}

	// send this data to the UI connected through the network session
	GENERIC_PACKET_INIT(VSS_WS_Opcodes::VSSWSO_S_ModuleAlertStatus, VSS_N_ModuleAlertState, 1);
	pktOut->AlertId = ma->AlertId;
	pktOut->AlertType = ma->AlertType;
	pktOut->TriggerStamp = ma->TriggerStamp;
	pktOut->StateFlags = ma->StateFlags;
	pktOut->LocationId = ma->LocationId;
	pktOut->ModuleID = ma->ModuleID;
	pktOut->AlertType = ma->AlertType;

	this_->SendMsg(packetBuff, pktSize);
}

bool NetworkSession::SendMsg(const char* data, const size_t len)
{
	return sWSServer.SendMsg(data, len, GetSessionId());
}