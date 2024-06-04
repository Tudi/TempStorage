#pragma once

#define KPI_PERIOD_UPDATE_LATENCY 60000
#define KPI_PERIOD_MEASURE_LATENCY_RAW 30000
#define KPI_PERIOD_MEASURE_LATENCY_API 30000
#define KPI_PERIOD_SAVE_TO_SERVER_HOURS 24
#define KPI_FILE_NAME "./Data/kpi.db"

class WatchdogThreadData;

/// <summary>
/// Module to generate KPI data. 
/// Periodically sends data to server
/// </summary>
class KPIManager
{
public:
    inline static KPIManager& getInstance() {
#ifdef _DEBUG
        static KPIManager* instance = new KPIManager;
        return *instance;
#else
        static KPIManager instance;
        return instance;
#endif
    }
    /// <summary>
    /// set internal states once application started up
    /// </summary>
    void Init();
    /// <summary>
    /// Someone Called a server API. Monitor how many API calls / s this user has
    /// </summary>
    void IncreaseAPICallCount();
    /// <summary>
    /// Client experienced FPS. This value will come every 1 second and represents and average
    /// </summary>
    void UpdateFPSStat(__int64 AvgFPSEveryMinute);
    /// <summary>
    /// Async backend API call durations should not be too large
    /// </summary>
    void UpdateAsyncDurationStat(__int64 CurCallTime);
    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks(); 
private:
    KPIManager();
    ~KPIManager();
    void UpdatePersistentData();
    void LoadPersistentData();
    KPIManager(const KPIManager&) = delete;
    KPIManager& operator=(const KPIManager&) = delete;

    friend void KPIManager_AsyncExecuteThread(WatchdogThreadData* wtd);

    unsigned __int64 m_ullLastRawLatencyCheckStamp;
    unsigned __int64 m_ullLastAPILatencyCheckStamp;
    int m_dLastRawLatency;
    int m_dLastAPILatency;

    struct PersistentKPIData* m_PersistentData;
    struct PersistentKPIData* m_PersistentDataNow;
};

#define sKPI KPIManager::getInstance()