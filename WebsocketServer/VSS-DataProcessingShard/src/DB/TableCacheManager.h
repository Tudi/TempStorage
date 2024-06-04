#pragma once

// simple value matching for quick lookup. Made to look up location of a module instance
// module locations might be looked up at every single packet that arrives to DPS

#include <array>

// value is in ms
#define DEFAULT_TABLE_CACHE_RESFRESH_PERIOD 5*60*1000		// every 5 minutes ? Run a remote command to force quicker refresh
#define TABLE_CACHE_MAX_ACCEPTED_ID			100*1000*1000 // 400 MB memory / table

enum DbTableCacheTypes
{
	DBTCT_ModuleId_LocationId = 0,
	DBTCT_Max
};

class WatchdogThreadData;

class TableCacheManager
{
public:
	inline static TableCacheManager& getInstance()
	{
		static TableCacheManager instance;
		return instance;
	}
	~TableCacheManager();
	void DestructorCheckMemLeaks();
	// start worker threads. Get the list of alerts to manage
	void Init();

	unsigned int GetModuleLocation(__int64 ModuleId);
private:
	TableCacheManager() {};
	TableCacheManager(const TableCacheManager&) = delete;
	TableCacheManager& operator=(const TableCacheManager&) = delete;

	// periodically update the cache content
	friend void TableCacheManager_AsyncExecuteThread(WatchdogThreadData* wtd);

	class DBIntTableCache;
	std::array<TableCacheManager::DBIntTableCache *, DbTableCacheTypes::DBTCT_Max> m_TableCaches;
};

class TableCacheManager::DBIntTableCache
{
public:
	DBIntTableCache();
	~DBIntTableCache();
	void DestructorCheckMemLeaks();

	inline unsigned int GetValue(unsigned int Index)
	{
		if (Index <= m_dMaxKeyValue)
		{
			return m_pValues[Index];
		}
		return (unsigned int)GetValueUncached(Index);
	}
protected:
	friend class TableCacheManager;
	friend void TableCacheManager_AsyncExecuteThread(WatchdogThreadData* wtd);
	__int64 GetValueUncached(__int64 Index);
	void SetQuery(const char* query);
	void SetQueryUncached(const char* query);
	void RefreshCache();
	inline bool ShouldRefresh(unsigned __int64 tickNow) { return m_dNextRefreshStamp < tickNow; }
	unsigned __int64 GetSleepTime() { return m_dRefreshPeriod; }
private:
	std::string m_sQuery; // run this query in case value is missing and we are needing it
	std::string m_sQueryUncachable; // if this table has some strangely large values
	__int64 m_dMaxKeyValue; // can return anything below this value
	unsigned int* m_pValues;
	std::mutex m_RefreshLock;
	unsigned __int64 m_dNextRefreshStamp;
	unsigned __int64 m_dRefreshPeriod;
};

#define sTableCacheManager TableCacheManager::getInstance()
