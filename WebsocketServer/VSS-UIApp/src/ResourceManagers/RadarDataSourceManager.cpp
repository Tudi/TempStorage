#include "stdafx.h"
#include "json/yyjson.h"
#include "json/yyjson2.h"
#include "Network/VSSPacketDefines.h"

ModuleDataSourceManager::ModuleDataSourceManager()
{
	m_bRefreshDPSList = true;
	m_dQueuedDPSRefreshStamp = 0;
	// in the beggining expecting 1-2 connections
	// 99% connections will idle with PING-PONG only
	m_RequiredConnections.reserve(2);
	m_HaveConnections.reserve(2);
	m_PrevValuesDPSList = 0;
	m_pWorkerThread = NULL;
}

ModuleDataSourceManager::~ModuleDataSourceManager()
{
#ifndef _DEBUG
	DestructorCheckMemLeaks();
#endif
}

void ModuleDataSourceManager::DestructorCheckMemLeaks()
{
	m_pWorkerThread = NULL; // will be handled by application session
	DisconnectAllConnections();
#ifdef _DEBUG
	delete& sDataSourceManager;
#endif
}

void RDSManager_AsyncExecuteThread(WatchdogThreadData* wtd);
void ModuleDataSourceManager::Init()
{
	int UpdatePeriod = sConfigManager.GetInt(DPS_ReconnectInterval, DPS_RECONNECT_INTERVAL_DEFAULT);
	if (UpdatePeriod < DPS_RECONNECT_INTERVAL_MIN)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
			"ModuleDataSourceManager:Strangely small reconnect interval %d", UpdatePeriod);
		UpdatePeriod = DPS_RECONNECT_INTERVAL_MIN;
	}
	if (UpdatePeriod > DPS_RECONNECT_INTERVAL_MAX)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
			"ModuleDataSourceManager:Strangely large reconnect interval %d", UpdatePeriod);
		UpdatePeriod = DPS_RECONNECT_INTERVAL_MAX;
	}

	m_pWorkerThread = sAppSession.CreateWorkerThread(RDSManager_AsyncExecuteThread, "ModuleDataSourceManager", UpdatePeriod);
}

void ModuleDataSourceManager::RefreshDPSList()
{ 
	m_bRefreshDPSList = true; 
//	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
//		"ModuleDataSourceManager:DPS refresh has been queued");
}

void ModuleDataSourceManager::OnUserLoggedIn()
{
	RefreshDPSList();
	m_pWorkerThread->WakeupThread();
}

void ModuleDataSourceManager::OnUserLogout()
{
	std::unique_lock<std::mutex> lock(m_DPSListRefreshMutex);
	// flush all subscriptions
	m_AlertSubscribedModuleIds.clear();
	m_FeedSubscribedModuleIds.clear();
	m_RequiredConnections.clear();
	// disconnect all clients
	m_pWorkerThread->WakeupThread();
}

static void CloseOutdatedConnections(std::vector<ModuleDataSourceManager::DPSConnectionInfo> &m_RequiredConnections,
	std::vector<ModuleDataSourceManager::DPSConnectionInfo> &m_HaveConnections)
{
	for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end();)
	{
		bool bConnectionShouldBeKept = false;
		for (auto itr2 = m_RequiredConnections.begin(); itr2 != m_RequiredConnections.end(); itr2++)
		{
			if (itr2->DPSID == itr->DPSID)
			{
				bConnectionShouldBeKept = true;
				break;
			}
		}
		if (bConnectionShouldBeKept == false)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
				"ModuleDataSourceManager:Closing connection to %s", itr->ConnectionURL);

			InternalDelete(itr->wsClient);
			itr = m_HaveConnections.erase(itr);
		}
		else
		{
			++itr;
		}
	}
}

static void UnsubscribeNoLongerRequiredModules(std::vector<ModuleDataSourceManager::DPSConnectionInfo>& m_HaveConnections,
	std::set<unsigned __int64>& m_AlertSubscribedModuleIds,
	std::set<unsigned __int64>& m_FeedSubscribedModuleIds)
{
	// maybe it's not a full connection, just  asingle module that got unsubscribed
	for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end(); itr++)
	{
		std::set<unsigned __int64> unsubAlerts;
		for (auto itr2 = itr->SubscribedModuleIdsAlerts.begin(); itr2 != itr->SubscribedModuleIdsAlerts.end(); itr2++)
		{
			// this happens if user buys/unsubscribes a module while we are logged in
			const unsigned __int64 SubscribedModuleId = *itr2;
			if (m_AlertSubscribedModuleIds.find(SubscribedModuleId) == m_AlertSubscribedModuleIds.end())
			{
				unsubAlerts.insert(SubscribedModuleId);
				itr->SubscribedModuleIdsAlerts.erase(SubscribedModuleId);
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
					"ModuleDataSourceManager:Unsubscribe Alerts from module %lld", SubscribedModuleId);
				break;
			}
		}
		if (unsubAlerts.empty() == false)
		{
			itr->wsClient->UnSubscribeFromModules(unsubAlerts, VSS_N_SubscribeModules::SubscriptionType::UnsubscribeAlert);
		}

		std::set<unsigned __int64> unsubFeed;
		for (auto itr2 = itr->SubscribedModuleIdsFeed.begin(); itr2 != itr->SubscribedModuleIdsFeed.end(); itr2++)
		{
			const unsigned __int64 SubscribedModuleId = *itr2;
			// this happens if user buys/unsubscribes a module while we are logged in
			if (m_FeedSubscribedModuleIds.find(SubscribedModuleId) == m_FeedSubscribedModuleIds.end())
			{
				unsubFeed.insert(SubscribedModuleId);
				itr->SubscribedModuleIdsFeed.erase(SubscribedModuleId);
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
					"ModuleDataSourceManager:Unsubscribe Feed from module %lld", SubscribedModuleId);
				break;
			}
		}
		if (unsubFeed.empty() == false)
		{
			itr->wsClient->UnSubscribeFromModules(unsubFeed, VSS_N_SubscribeModules::SubscriptionType::UnsubscribeFeed);
		}
	}
}

static void OpenMissingConnections(std::vector<ModuleDataSourceManager::DPSConnectionInfo>& m_RequiredConnections,
	std::vector<ModuleDataSourceManager::DPSConnectionInfo>& m_HaveConnections)
{
	for (auto itr2 = m_RequiredConnections.begin(); itr2 != m_RequiredConnections.end(); itr2++)
	{
		bool bAlreadyHaveConnection = false;
		for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end(); itr++)
		{
			if (itr2->DPSID == itr->DPSID)
			{
				bAlreadyHaveConnection = true;
				// maybe it's faster to check if they match ?
				itr->SupportedModuleIds = itr2->SupportedModuleIds;
				break;
			}
		}
		if (bAlreadyHaveConnection == false)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
				"ModuleDataSourceManager:Creating new connection to %s", itr2->ConnectionURL);
			
			ModuleDataSourceManager::DPSConnectionInfo& ci = m_HaveConnections.emplace_back();
			memcpy(ci.ConnectionURL, (*itr2).ConnectionURL, MIN(sizeof(ci.ConnectionURL), sizeof((*itr2).ConnectionURL)));
			ci.DPSID = (*itr2).DPSID;
			InternalNew(ci.wsClient, VSSWebSocketClientEx);
			ci.wsClient->ConnectToServer(ci.ConnectionURL);
			ci.SupportedModuleIds = std::move((*itr2).SupportedModuleIds);
		}
	}
}


static void SubscribeMissingModules(std::vector<ModuleDataSourceManager::DPSConnectionInfo>& m_HaveConnections,
	std::set<unsigned __int64>& m_AlertSubscribedModuleIds,
	std::set<unsigned __int64>& m_FeedSubscribedModuleIds)
{
	// maybe it's not a full connection, just  asingle module that got unsubscribed
	for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end(); itr++)
	{
		std::set<unsigned __int64> subAlerts;
		for (auto itr2 = m_AlertSubscribedModuleIds.begin(); itr2 != m_AlertSubscribedModuleIds.end(); itr2++)
		{
			if (itr->wsClient->CanSendPackets() == false)
			{
				continue;
			}
			// this happens if user buys/unsubscribes a module while we are logged in
			if (itr->SupportedModuleIds.find(*itr2) != itr->SupportedModuleIds.end() &&
				itr->SubscribedModuleIdsAlerts.find(*itr2) == itr->SubscribedModuleIdsAlerts.end())
			{
				subAlerts.insert(*itr2);
				itr->SubscribedModuleIdsAlerts.insert(*itr2);
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
					"ModuleDataSourceManager:Subscribe Alerts from module %lld", *itr2);
			}
		}
		if (subAlerts.empty() == false)
		{
			itr->wsClient->SubscribeToModules(subAlerts, VSS_N_SubscribeModules::SubscriptionType::SubscribeAlert);
		}

		std::set<unsigned __int64> subFeed;
		for (auto itr2 = m_FeedSubscribedModuleIds.begin(); itr2 != m_FeedSubscribedModuleIds.end(); itr2++)
		{
			if (itr->wsClient->CanSendPackets() == false)
			{
				continue;
			}
			// this happens if user buys/unsubscribes a module while we are logged in
			if (itr->SupportedModuleIds.find(*itr2) != itr->SupportedModuleIds.end() &&
				itr->SubscribedModuleIdsFeed.find(*itr2) == itr->SubscribedModuleIdsFeed.end())
			{
				subFeed.insert(*itr2);
				itr->SubscribedModuleIdsFeed.insert(*itr2);
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
					"ModuleDataSourceManager:Subscribe Feed from module %lld", *itr2);
			}
		}
		if (subFeed.empty() == false)
		{
			itr->wsClient->SubscribeToModules(subFeed, VSS_N_SubscribeModules::SubscriptionType::SubscribeFeed);
		}
	}
}

void ModuleDataSourceManager::CheckOfflineConnections()
{
	for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end();)
	{
		// for some reason this connection keeps failing. Maybe the DPS went down ( or rebalanced )
		// fetch a new list of RDPS
		if (itr->wsClient->IsConsideredUnusable() == true)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
				"ModuleDataSourceManager:Server offline for too long : %s", itr->ConnectionURL);

			RefreshDPSList();

			InternalDelete(itr->wsClient);
			itr = m_HaveConnections.erase(itr);

			// force reparse the next DPS list to be able to connect to maybe this server again later
			m_PrevValuesDPSList = 0;
		}
		else
		{
			itr++;
		}
	}
}

void ModuleDataSourceManager::PeriodicUpdateAllConnections()
{
	for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end(); itr++)
	{
		// init / heartbeat ..
		itr->wsClient->PeriodicUpdate();
	}
}

void ModuleDataSourceManager::DisconnectAllConnections()
{
	m_PrevValuesDPSList = 0;

	if (!m_HaveConnections.empty())
	{
		for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end(); itr++)
		{
			InternalDelete(itr->wsClient);
		}

		m_HaveConnections.clear();
	}

	// force reparse the next DPS list to be able to connect to maybe this server again later
	m_PrevValuesDPSList = 0;

	RefreshDPSList();
}

void CB_AsyncDPSListDataArived(int CurlErr, char* response, void* userData);
void ModuleDataSourceManager::RefreshDPSListIfNeeded()
{
	if (m_bRefreshDPSList == true &&
		m_dQueuedDPSRefreshStamp < GetTickCount64())
	{
		// we issued a list refresh, no need to spam it
		m_bRefreshDPSList = false;

		// if we update too often, we might miss the chance to actually make connections
		m_dQueuedDPSRefreshStamp = GetTickCount64() + DPS_RECONNECT_INTERVAL_DEFAULT;

		// queue up the backend query
		WebApi_GetDPSListAsync(sUserSession.GetOrganizationId(), CB_AsyncDPSListDataArived, NULL);
	}
}

void ModuleDataSourceManager::ApplyNewDPSListIfNeeded()
{
	if (!m_RequiredConnections.empty())
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
			"ModuleDataSourceManager:Applying new DPS list size %d", m_RequiredConnections.size());

		// disconnect connections we should not have
		CloseOutdatedConnections(m_RequiredConnections, m_HaveConnections);

		// connect to connections we do not yet have
		OpenMissingConnections(m_RequiredConnections, m_HaveConnections);

		// had values or not, safe to clear content at this point
		m_RequiredConnections.clear();
	}

	// in case subscription expired while user was logged in
	UnsubscribeNoLongerRequiredModules(m_HaveConnections, m_AlertSubscribedModuleIds, m_FeedSubscribedModuleIds);

	// we want to recieve feed for these modules. Maybe we do not have them yet
	SubscribeMissingModules(m_HaveConnections, m_AlertSubscribedModuleIds, m_FeedSubscribedModuleIds);
}

void RDSManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		if (sAppSession.IsUserLoggedIn())
		{
			std::lock_guard<std::mutex> lock(sDataSourceManager.m_DPSListRefreshMutex);

			// do we have an active connection ? Maybe we should
			sDataSourceManager.RefreshDPSListIfNeeded();

			// update list of active connections. Might happen on load balancer rebalance
			sDataSourceManager.ApplyNewDPSListIfNeeded();

			// check if we have all connections working. If something disconnected, refetch list and reconnect
			sDataSourceManager.CheckOfflineConnections();

			// init, heartbeat
			sDataSourceManager.PeriodicUpdateAllConnections();
		}
		else
		{
			sDataSourceManager.DisconnectAllConnections();
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

typedef struct DBRowDPSConnectionInfo
{
	REFLECT_TYPE(DBRowDPSConnectionInfo);
	DBRowDPSConnectionInfo() { InitTypeInfo(); }

	unsigned __int64 DPSID;
	char ConnectionURL[MAX_DB_STRING_LENGTH];
	unsigned __int64 ModuleId; // if this is non 0, we should let the DPS know we need data for it
}DBRowDPSConnectionInfo;

bool CB_OnDBRowDPSListExtractFinished(int rowIndex, ExtractDBColumnToBinary* rowColDataArr)
{
	rowIndex;
	DBRowDPSConnectionInfo* ci = typecheck_castL(DBRowDPSConnectionInfo,rowColDataArr[0].cbDRF_userData1);

	// !! might need to change this : Auto subscribe for alerts 
	sDataSourceManager.m_AlertSubscribedModuleIds.insert(ci->ModuleId);

	bool bAlreadyPresent = false;
	std::vector<ModuleDataSourceManager::DPSConnectionInfo>& m_RequiredConnections = sDataSourceManager.m_RequiredConnections;
	for (auto itr : m_RequiredConnections)
	{
		if (itr.DPSID == ci->DPSID)
		{
			bAlreadyPresent = true;
			if (itr.SupportedModuleIds.find(ci->ModuleId) == itr.SupportedModuleIds.end())
			{
				itr.SupportedModuleIds.insert(ci->ModuleId);

				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
					"ModuleDataSourceManager:DPS %s should also send us data for Module %lld", ci->ConnectionURL, ci->ModuleId);
			}
		}
	}

	if (bAlreadyPresent == false)
	{
		ModuleDataSourceManager::DPSConnectionInfo &nci = m_RequiredConnections.emplace_back();
		nci.SupportedModuleIds.insert(ci->ModuleId);
		nci.DPSID = ci->DPSID;
		strcpy_s(nci.ConnectionURL, ci->ConnectionURL);

		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
			"ModuleDataSourceManager:DPS %s should send us data for Module %lld", ci->ConnectionURL, ci->ModuleId);
	}

	return true;
}

void CB_AsyncDPSListDataArived(int CurlErr, char* response, void* userData)
{
	userData;
	std::lock_guard<std::mutex> lock(sDataSourceManager.m_DPSListRefreshMutex);

	// thread concurency on logout + refresh
	if (sAppSession.IsUserLoggedIn() == false ||
		response == NULL || 
		CurlErr != 0)
	{
		return;
	}

	// refreshing many values induces a monitor flickering.
	// If we got the same values as before, do not refresh
	uint64_t crc64Val = crc64(0, response, strlen(response));
	if (sDataSourceManager.m_PrevValuesDPSList == crc64Val)
	{
		return;
	}
	sDataSourceManager.m_PrevValuesDPSList = crc64Val;

	yyJSON(yydoc);
	if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModulesWindow, "ModulesGrid") != WebApiErrorCodes::WAE_NoError)
	{
		return;
	}

	// flush previous list of required connections
	sDataSourceManager.m_RequiredConnections.clear();

	DBRowDPSConnectionInfo ci;
	const char* arrayName = "DPSInstances";
	ExtractDBColumnToBinary extractColumns[] = {
		{"DPSID", &ci.DPSID},
		{"ConnectionURL", ci.ConnectionURL, sizeof(ci.ConnectionURL)},
		{"ModuleInstanceID", &ci.ModuleId},
		{NULL} };
	extractColumns[0].SetDataRowFinishedFunc(CB_OnDBRowDPSListExtractFinished, &ci, NULL);

	ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceModulesWindow);

	// wake up the worker thread to process this new list
	sDataSourceManager.m_pWorkerThread->WakeupThread();
}

bool ModuleDataSourceManager::SubscribeModuleFeed(__int64 ModuleId, bool bIsUnsubscribe)
{
	std::lock_guard<std::mutex> lock(sDataSourceManager.m_DPSListRefreshMutex);

	bool bFoundDPSWithModule = false;
#ifdef _DEBUG
	for (auto itr = m_HaveConnections.begin(); itr != m_HaveConnections.end(); itr++)
	{
		// if there is an alert, there is a chance there is a feed also
		// checking alert set is just a sanity check
		auto findres = itr->SupportedModuleIds.find(ModuleId);
		if (findres != itr->SupportedModuleIds.end())
		{
			bFoundDPSWithModule = true;
#if 0
			std::set<unsigned __int64> feedSet;
			feedSet.insert(ModuleId);
			if (bIsUnsubscribe == true)
			{
				if (itr->SubscribedModuleIdsFeed.find(ModuleId) != itr->SubscribedModuleIdsFeed.end())
				{
					itr->SubscribedModuleIdsFeed.erase(ModuleId);
					itr->wsClient->UnSubscribeFromModules(feedSet, VSS_N_SubscribeModules::SubscriptionType::UnsubscribeFeed);
				}
			}
			else if (itr->SubscribedModuleIdsFeed.find(ModuleId) == itr->SubscribedModuleIdsFeed.end())
			{
				// if socket did not finish handshake, wait for it. Queue this subscription
				if (itr->wsClient->CanSendPackets() == true)
				{
					itr->SubscribedModuleIdsFeed.insert(ModuleId);
					itr->wsClient->SubscribeToModules(feedSet, VSS_N_SubscribeModules::SubscriptionType::SubscribeFeed);
				}
			}
#endif
		}
	}

	// might happen if DPS connection breaks. Not an error, but should retry later
	if (bFoundDPSWithModule == false && bIsUnsubscribe == false)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceModuleDataSourceManager, 0, 0,
			"ModuleDataSourceManager:Failed to subscribe to Module %lld feed", ModuleId);
	}
#endif

	// in case client disconnects, we want to resend this list
	if (bIsUnsubscribe == true)
	{
		m_FeedSubscribedModuleIds.erase(ModuleId);
	}
	else
	{
		m_FeedSubscribedModuleIds.insert(ModuleId);
	}

	m_pWorkerThread->WakeupThread();

	return bFoundDPSWithModule;
}

void ModuleDataSourceManager::UnSubscribeModuleFeed(__int64 ModuleId)
{
	SubscribeModuleFeed(ModuleId, true);
}