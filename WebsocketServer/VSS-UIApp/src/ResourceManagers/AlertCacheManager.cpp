#include "stdafx.h"
#include "json/yyjson.h"
#include "json/yyjson2.h"

void AlertCacheManager::RecentAlerts_AsyncExecuteThread(WatchdogThreadData* wtd, AlertCacheManager *wnd)
{
	while (sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
	{
		ULONGLONG startStamp = GetTickCount64();

		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		if (sAppSession.IsUserLoggedIn())
		{
			// in case there are data to be processed from previous reply
			wnd->ProcessAPIReply();

			// time to refresh cache state
			if (wnd->m_dNextUpdateStamp < startStamp &&
				sLocalization.GetAlertStateIdString(1)[0] != 0 // because async API calls
				)
			{
				wnd->m_dNextUpdateStamp = startStamp + ALERT_POLL_INTERVAL_MS;
				wnd->QueRefreshCache();
			}
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

AlertCacheManager::AlertCacheManager()
{
	InitTypeInfo();
	m_AlertCache.reserve(MAX_ALERT_CACHE_COUNT);
}

void AlertCacheManager::Init()
{
	m_dLastCachedId = 0;
	m_sHttpResponse = NULL;
	m_dNextUpdateStamp = 0;

	// create watchdog data
	WatchdogThreadData* wtd;
	InternalNew(wtd, WatchdogThreadData);
	if (wtd == NULL)
	{
		return;
	}

	// start worker thread to push log messages to server
	std::thread* myThread = new std::thread([wtd, this]() { RecentAlerts_AsyncExecuteThread(wtd, this); });
	wtd->Init(myThread, ALERT_POLL_INTERVAL_MS, "RecentAlerts");

	// Make the application wait until this thread also exits
	sAppSession.AddModuleThread(wtd);

	m_WorkerThread = wtd;
}

void AlertCacheManager::DestructorCheckMemLeaks()
{
	for (auto itr = m_AlertCache.begin(); itr != m_AlertCache.end(); ++itr)
	{
		// can this even happen ?
		if (*itr == NULL)
		{
			continue;
		}

		InternalFree(*itr);
	}
	m_AlertCache.clear();

	m_WorkerThread = NULL;

	InternalFree(m_sHttpResponse);

#ifdef _DEBUG
	delete& sAlertsCache;
#endif
}

void AlertCacheManager::QueRefreshCache()
{
	std::lock_guard<std::mutex> lock(m_AlertCacheMutex);
	WebApi_GetAlertsAsync(0, 0, MAX_ALERT_CACHE_COUNT, (int)m_dLastCachedId, AlertCacheManager::CB_AsyncDataArived, this);
}

void AlertCacheManager::OnUserLoggedIn()
{
	QueRefreshCache();
}

void AlertCacheManager::OnUserLoggedOut()
{
	std::lock_guard<std::mutex> lock(m_AlertCacheMutex);
	for (auto itr = m_AlertCache.begin(); itr != m_AlertCache.end(); ++itr)
	{
		// can this even happen ?
		if (*itr == NULL)
		{
			continue;
		}

		InternalFree(*itr);
	}
	m_AlertCache.clear();
	InternalFree(m_sHttpResponse);
	m_dLastCachedId = 0;
}

void AlertCacheManager::CB_AsyncDataArived(int CurlErr, char* response, void* userData)
{
	CurlErr;
	AlertCacheManager* wnd = (AlertCacheManager*)userData;
	std::lock_guard<std::mutex> lock(wnd->m_AlertCacheMutex);
	if (wnd->m_sHttpResponse != NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsCacheManager, 0, 0,
			"AlertCacheManager:Multiple backend replies before processing");
		return;
	}

	// duplicate the response
	wnd->m_sHttpResponse = InternalStrDup(response);

	// wake up the worker thread so it will process this new response
	wnd->m_WorkerThread->WakeupThread();
}

bool AlertCacheManager::CB_OnDBRowExtractFinished(int rowIndex, ExtractDBColumnToBinary* rowColDataArr)
{
	rowIndex;
	AlertHistoryData* ahd = typecheck_castL(AlertHistoryData, rowColDataArr[0].cbDRF_userData1);
	AlertCacheManager* wnd = typecheck_castL(AlertCacheManager, rowColDataArr[0].cbDRF_userData2);

	// check if this alert ID is already cached
	for (auto itr = wnd->m_AlertCache.begin(); itr != wnd->m_AlertCache.end(); itr++)
	{
		// we found it, maybe it's an update ?
		if ((*itr)->alertId == ahd->alertId)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsCacheManager, 0, 0,
				"AlertCacheManager:AlertId %lld already found in cache. Updating it.", ahd->alertId);
			*(*itr) = *ahd;
			return true;
		}
	}

	// don't cache too many alerts. Probably not going to show them all anyway
	if (wnd->m_AlertCache.size() >= MAX_ALERT_CACHE_COUNT)
	{
		AlertHistoryData* todel = wnd->m_AlertCache.pop();
		InternalFree(todel);
	}

	// duplicate the temp store
	AlertHistoryData* toadd = (AlertHistoryData*)InternalMalloc(sizeof(AlertHistoryData));
	if (toadd == NULL)
	{
		return false; // break parsing rest of the result
	}
	memcpy(toadd, ahd, sizeof(AlertHistoryData));

	// add this new row
	wnd->m_AlertCache.push(toadd);

	// no longer request this row
	if (toadd->alertId > wnd->m_dLastCachedId)
	{
		wnd->m_dLastCachedId = toadd->alertId;
	}

	return true;
}

void AlertCacheManager::ProcessAPIReply()
{
	std::lock_guard<std::mutex> lock(m_AlertCacheMutex);

	// 99.9% of cases this should be anything than null
	if (m_sHttpResponse == NULL)
	{
		return;
	}

	// is this a valid JSON ?
	yyJSON(yydoc);
	if (ExtractDBColumnToBinary::DBH_APIResultValid(0, m_sHttpResponse, yydoc, LogSourceGroups::LogSourceAlertsCacheManager, "ProcessAPIReply") != WebApiErrorCodes::WAE_NoError)
	{
		InternalFree(m_sHttpResponse);
		return;
	}

	// convert JSON to internal store
	const char* arrayName = "Alerts";
	AlertHistoryData ahd; // temp store for extracted data from DB
	ExtractDBColumnToBinary extractColumns[] = {
		{"AlertId", &ahd.alertId},
		{"AlertDefinitionName", ahd.alertName, sizeof(ahd.alertName)},
		{"AlertTypeId", &ahd.alertTypeId },
		{"LocationId", &ahd.locationId },
		{"LocationName", ahd.locationName, sizeof(ahd.locationName)},
		{"CreatedTimestamp", &ahd.stamp},
		{"CreatedTimestamp", ahd.alertStampStr, sizeof(ahd.alertStampStr), TimeStampToSortableStr},
		{"CreatedTimestamp", ahd.alertStampStrCard, sizeof(ahd.alertStampStrCard), TimeStampToStrAlertCard},
		{"CreatedTimestamp", ahd.alertDateStr, sizeof(ahd.alertDateStr), TimeStampToDateStr},
		{"CreatedTimestamp", ahd.alertTimeStr, sizeof(ahd.alertTimeStr), TimeStampToTimeStr},
		{"AlertStatusTypeId", (unsigned char*)&ahd.statusFlags},
		{"AlertStatusTypeId", ahd.alertStatusStr, sizeof(ahd.alertStatusStr), AlertStatusIdToStr},
		{"AlertTypeName", ahd.alertTypeName, sizeof(ahd.alertTypeName)},
		{"AlertTypeDescription", ahd.alertTypeDescription, sizeof(ahd.alertTypeDescription)},
		{NULL} };
	extractColumns[0].SetDataRowFinishedFunc(CB_OnDBRowExtractFinished, &ahd, this);
	extractColumns[0].SetInitFunction(InitDatagridToStoreRows);

	ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceAlertsCacheManager);

	// we are done with this respon se
	InternalFree(m_sHttpResponse);

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertsCacheManager, 0, 0,
		"AlertCacheManager:Processed API reply. Now have %d entries", (int)m_AlertCache.size());
}

const AlertHistoryData* AlertCacheManager::GetCachedData(int index)
{
	std::lock_guard<std::mutex> lock(m_AlertCacheMutex);
	if (m_AlertCache.size() <= index || index < 0)
	{
		return NULL;
	}
	return m_AlertCache[index];
}

const bool AlertCacheManager::GetCachedData(int index, AlertHistoryData& out_data)
{
	std::lock_guard<std::mutex> lock(m_AlertCacheMutex);
	if (m_AlertCache.size() <= index || index < 0)
	{
		return false;
	}
	memcpy(&out_data, m_AlertCache[index], sizeof(AlertHistoryData));
	return true;
}

const AlertHistoryData* AlertCacheManager::GetCachedData(int index, int locationId)
{
	size_t skippedIndex = 0;
	for (auto itr : m_AlertCache)
	{
		if (itr->locationId == locationId)
		{
			if (skippedIndex == index)
			{
				return itr;
			}
			skippedIndex++;
		}
	}
	return NULL;
}

void AlertCacheManager::OnDPSAlertArrived(__int64 alertId, __int64 alertTypeId, unsigned __int64 stamp, 
	VSS_N_ModuleAlertState::AlertStateFlags StatusFlags, __int64 locationId)
{
	AlertHistoryData* toadd = NULL;
	bool bAlreadyExisted = false;
	// check if this is a simple update notification
	for (auto itr = m_AlertCache.begin(); itr != m_AlertCache.end(); itr++)
	{
		// we found it, maybe it's an update ?
		if ((__int64)(*itr)->alertId == alertId)
		{
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsCacheManager, 0, 0,
				"AlertCacheManager:AlertId %lld already found in cache. Updating it.", alertId);
			toadd = (*itr);
			bAlreadyExisted = true;
		}
	}

	if (toadd == NULL)
	{
		toadd = (AlertHistoryData*)InternalMalloc(sizeof(AlertHistoryData));
	}
	if (toadd == NULL)
	{
		return;
	}
	std::lock_guard<std::mutex> lock(m_AlertCacheMutex);

	if (bAlreadyExisted == false)
	{
		toadd->InitTypeInfo();
		toadd->alertId = alertId;
		toadd->alertTypeId = alertTypeId;
		strcpy_s(toadd->alertName, sLocalization.GetAlertTypeIdString((int)toadd->alertTypeId));
		toadd->locationId = locationId;
		strcpy_s(toadd->locationName, sLocalization.GetLocationIdString((int)toadd->locationId));
		toadd->stamp = stamp;

		time_t nVal = stamp;
		struct tm timeinfo;
		errno_t convErr = localtime_s(&timeinfo, &nVal); // Convert to local time
		if (convErr == NO_ERROR)
		{
			strftime(toadd->alertStampStr, sizeof(toadd->alertStampStr), "%m/%d/%Y %I:%M:%S %p", &timeinfo);
		}
		else
		{
			toadd->alertStampStr[0] = 0;
			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertsCacheManager, 0, 0,
				"AlertCacheManager:Failed to convert timestamp %lld == %lld to string", nVal, stamp);
		}
	}

	toadd->statusFlags = StatusFlags;

	strcpy_s(toadd->alertStatusStr, sLocalization.GetAlertStateIdString((int)toadd->statusFlags));

	// add this new row
	if (bAlreadyExisted == false)
	{
		m_AlertCache.push(toadd);
	}
}