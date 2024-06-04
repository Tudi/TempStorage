#pragma once

#include "Util/VariousFuncs.h"
#include "Util/ObjDescriptor.h"
#include "Network/VssPacketDefines.h"

// keeps track of recent alerts
// has background worker thread that periodically query server for new alerts
// is able to serve locations with new location based alerts
// is able to serve windows notification system with new alerts

#define MAX_ALERT_CACHE_COUNT 1000

#ifdef _DEBUG
	#define ALERT_POLL_INTERVAL_MS (uint64_t)500000
#else
	#define ALERT_POLL_INTERVAL_MS (uint64_t)500
#endif

// all the fields that we might want to show on UI side
typedef struct AlertHistoryData
{
	REFLECT_TYPE(AlertHistoryData);
	AlertHistoryData() { InitTypeInfo(); }

	unsigned __int64 alertId;
	char alertName[MAX_DB_STRING_LENGTH]; // this is the UI name when we create an Organization specific alert
	unsigned __int64 alertTypeId;
	char alertTypeName[MAX_DB_STRING_LENGTH];
	char alertTypeDescription[MAX_DB_STRING_LENGTH];
	unsigned __int64 locationId;
	char locationName[MAX_DB_STRING_LENGTH];
	unsigned __int64 stamp;
	char alertStampStr[MAX_DB_STRING_LENGTH];
	char alertStampStrCard[MAX_DB_STRING_LENGTH];
	char alertDateStr[MAX_DB_STRING_LENGTH];
	char alertTimeStr[MAX_DB_STRING_LENGTH];
	VSS_N_ModuleAlertState::AlertStateFlags statusFlags;
	char alertStatusStr[MAX_DB_STRING_LENGTH];
}AlertHistoryData;

class WatchdogThreadData;
class ExtractDBColumnToBinary;

class AlertCacheManager
{
public:
	REFLECT_TYPE(AlertCacheManager);
	inline static AlertCacheManager& getInstance() {
#ifdef _DEBUG
		static AlertCacheManager* instance = new AlertCacheManager;
		return *instance;
#else
		static AlertCacheManager instance;
		return instance;
#endif
	}
	AlertCacheManager();
	void Init();
	void DestructorCheckMemLeaks();
	void OnUserLoggedIn();
	void OnUserLoggedOut();
//	std::mutex &GetLock() { return m_AlertCacheMutex; } // lock list while reading data to avoid accesing bad data
	const AlertHistoryData* GetCachedData(int index);
	const AlertHistoryData* GetCachedData(int index, int locationId);
	const bool GetCachedData(int index, AlertHistoryData& out_data);
	size_t GetSize() { return m_AlertCache.size(); }
	void OnDPSAlertArrived(__int64 alertId, __int64 alertTypeId, unsigned __int64 stamp, 
		VSS_N_ModuleAlertState::AlertStateFlags StatusFlags, __int64 locationId);
	std::mutex m_AlertCacheMutex;
private:
	AlertCacheManager(const AlertCacheManager&) = delete;
	AlertCacheManager& operator=(const AlertCacheManager&) = delete;

	// periodically check for new alerts
	static void RecentAlerts_AsyncExecuteThread(WatchdogThreadData* wtd, AlertCacheManager* wnd);
	// when response arrives from backen
	static void CB_AsyncDataArived(int CurlErr, char* response, void* userData);
	// when DBHelper extracted a full row, we will add it to our cache list
	static bool CB_OnDBRowExtractFinished(int rowIndex, ExtractDBColumnToBinary* rowColDataArr);

	// query backend if new Alerts got added
	void QueRefreshCache();
	// the HTTP result will be processed by it's own thread to not block CURL thread
	void ProcessAPIReply();

	uint64_t m_dLastCachedId;
	char* m_sHttpResponse; // last http response to not block http query interface. We will parse it in our thread
	CircularBuffer<AlertHistoryData*> m_AlertCache;
	WatchdogThreadData* m_WorkerThread;
	uint64_t m_dNextUpdateStamp;
};

#define sAlertsCache AlertCacheManager::getInstance()