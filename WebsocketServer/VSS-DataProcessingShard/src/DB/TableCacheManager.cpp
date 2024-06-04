#include <mutex>
#include <string>
#include <vector>
#include <array>
#include <Windows.h>
#include "Util/Allocator.h"
#include "TableCacheManager.h"
#include "ResourceManager/DataSourceManager.h"
#include "DB/MysqlManager.h"
#include "Session/ApplicationSession.h"

TableCacheManager::DBIntTableCache::DBIntTableCache()
{
	m_sQuery = "";
	m_dMaxKeyValue = 0;
	m_pValues = NULL;
	m_dNextRefreshStamp = 0;
	m_dRefreshPeriod = DEFAULT_TABLE_CACHE_RESFRESH_PERIOD;
}

TableCacheManager::DBIntTableCache::~DBIntTableCache()
{
	DestructorCheckMemLeaks();
}

void TableCacheManager::DBIntTableCache::DestructorCheckMemLeaks()
{
	if (m_pValues != NULL)
	{
		InternalFree(m_pValues);
	}
	m_dMaxKeyValue = 0;
}

void TableCacheManager::DBIntTableCache::SetQuery(const char* query)
{
	m_sQuery = query;
}

void TableCacheManager::DBIntTableCache::SetQueryUncached(const char* query)
{
	m_sQueryUncachable = query;
}

__int64 TableCacheManager::DBIntTableCache::GetValueUncached(__int64 Index)
{
	__int64 res;
	if (sDBManager.RunCacheQuery1Val(m_sQueryUncachable.c_str(), Index, &res))
	{
		return res;
	}
	return 0;
}

void TableCacheManager::DBIntTableCache::RefreshCache()
{
	std::unique_lock<std::mutex> lock(m_RefreshLock);
	m_dNextRefreshStamp = GetTickCount64() + m_dRefreshPeriod;

	unsigned int* newValues;
	__int64 newMaxIndex;
	if (sDBManager.RunCacheQuery(m_sQuery.c_str(), TABLE_CACHE_MAX_ACCEPTED_ID, &newValues, &newMaxIndex) == false)
	{
		return;
	}

	unsigned int* freeme = m_pValues;
	m_pValues = newValues;
	m_dMaxKeyValue = newMaxIndex;

	InternalFree(freeme);
}

TableCacheManager::~TableCacheManager()
{
	DestructorCheckMemLeaks();
}

void TableCacheManager::DestructorCheckMemLeaks()
{
	for (size_t i = 0; i < m_TableCaches.size(); i++)
	{
		if (m_TableCaches[i] == NULL)
		{
			continue;
		}
		m_TableCaches[i]->DestructorCheckMemLeaks();
		InternalDelete(m_TableCaches[i]);
		m_TableCaches[i] = NULL;
	}
}

void TableCacheManager_AsyncExecuteThread(WatchdogThreadData* wtd);
void TableCacheManager::Init()
{
	InternalNew(m_TableCaches[DbTableCacheTypes::DBTCT_ModuleId_LocationId], TableCacheManager::DBIntTableCache);
	m_TableCaches[DbTableCacheTypes::DBTCT_ModuleId_LocationId]->SetQuery(
		"Select ModuleInstanceID, ModuleLocationID from ModuleInstances");
	m_TableCaches[DbTableCacheTypes::DBTCT_ModuleId_LocationId]->SetQueryUncached(
		"Select ModuleLocationID from ModuleInstances where ModuleInstanceID=");
	m_TableCaches[DbTableCacheTypes::DBTCT_ModuleId_LocationId]->RefreshCache();

	// create worker thread
	unsigned __int64 minSleep = DEFAULT_TABLE_CACHE_RESFRESH_PERIOD;
	for (size_t i = 0; i < m_TableCaches.size(); i++)
	{
		if (m_TableCaches[i]->GetSleepTime() < minSleep)
		{
			minSleep = m_TableCaches[i]->GetSleepTime();
		}
	}
	sAppSession.CreateWorkerThreadGroup(WTG_DBTableCache, 1, "DBTableCacheManager", TableCacheManager_AsyncExecuteThread, minSleep);
}

unsigned int TableCacheManager::GetModuleLocation(__int64 ModuleId)
{
	return m_TableCaches[DbTableCacheTypes::DBTCT_ModuleId_LocationId]->GetValue((unsigned int)ModuleId);
}

void TableCacheManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() &&
		wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		unsigned __int64 TickNow = GetTickCount64();
		{
			for (size_t i = 0; i < sTableCacheManager.m_TableCaches.size(); i++)
			{
				if (sTableCacheManager.m_TableCaches[i]->ShouldRefresh(TickNow))
				{
					sTableCacheManager.m_TableCaches[i]->RefreshCache();
				}
			}
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}