#pragma once

/*
* Connects to single/multiple DataProcessingShards to fetch Module Alerts and Feed
* Should keep track which Module is visualized and only fetch feed for actively viewed Module
* Always subscribe to all DPS servers for Alerts
* In case a DPS server is no longer reachable, call Backend API to fetch a new list of DPS for this organization
* DPS instances might come online / go offline. Every time heartbeat is not received, refetch a new DPS list
*/

// X milliseconds since connection is down
#define DPS_RECONNECT_INTERVAL_MIN 1000
#define DPS_RECONNECT_INTERVAL_DEFAULT 5000
#define DPS_RECONNECT_INTERVAL_MAX 10000

#include "Util/ObjDescriptor.h"

class VSSWebSocketClientEx;
class ModuleDataSourceManager
{
public:
    inline static ModuleDataSourceManager& getInstance() {
#ifdef _DEBUG
        static ModuleDataSourceManager* instance = new ModuleDataSourceManager;
        return *instance;
#else
        static ModuleDataSourceManager instance;
        return instance;
#endif
    }

    // keep a list of RDP instances that we should listen to for events : Alerts, Feed
    typedef struct DPSConnectionInfo
    {
        REFLECT_TYPE(ModuleDataSourceManager::DPSConnectionInfo);
        DPSConnectionInfo()
        {
            DPSID = 0;
            ConnectionURL[0] = 0;
            wsClient = NULL;
            InitTypeInfo();
        }
        unsigned __int64 DPSID;
        char ConnectionURL[MAX_DB_STRING_LENGTH];
        VSSWebSocketClientEx* wsClient; // connection to the DPS
        std::set<unsigned __int64> SupportedModuleIds; // all moduleIds available
        // periodically syncronize these lists from global manager list
        std::set<unsigned __int64> SubscribedModuleIdsAlerts; // probably all modules
        std::set<unsigned __int64> SubscribedModuleIdsFeed; // probably just 1 ID
    }DPSConnectionInfo;

    /// <summary>
    /// set internal states once application started up
    /// </summary>
    void Init();
    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();
    /// <summary>
    /// Query Backend loadbalancer to which DPS should this organization connect to
    /// </summary>
    void RefreshDPSList();
    /// <summary>
    /// Fetch the latest Organization DPS list
    /// </summary>
    void OnUserLogin() { RefreshDPSList(); }
    /// <summary>
    /// Disconnect all connections
    /// </summary>
    void OnUserLogout();
    /// <summary>
    /// Tell DPS server that we want to recieve Module feed from them
    /// Function might fail if DPS connection is offline
    /// </summary>
    bool SubscribeModuleFeed(__int64 ModuleId, bool bIsUnsubscribe = false);
    /// <summary>
    /// Tell DPS server that we no longer want to recieve Module feed from them
    /// </summary>
    void UnSubscribeModuleFeed(__int64 ModuleId);
    /// <summary>
    /// Refresh the DPS list
    /// </summary>
    void OnUserLoggedIn();
    /// <summary>
    /// Check if UI is subscribed to a specific ModuleFeed
    /// </summary>
    bool IsSubscribedToModuleFeed(unsigned __int64 ModuleId)
    {
        return m_FeedSubscribedModuleIds.find(ModuleId) != m_FeedSubscribedModuleIds.end();
    }
private:
    ModuleDataSourceManager();
    ~ModuleDataSourceManager();
    ModuleDataSourceManager(const ModuleDataSourceManager&) = delete;
    ModuleDataSourceManager& operator=(const ModuleDataSourceManager&) = delete;

    // periodically check for reconnections
    friend void RDSManager_AsyncExecuteThread(WatchdogThreadData* wtd);
    // backend replied with DPS list. Parse it and check if connection list is in sync with expectations
    friend void CB_AsyncDPSListDataArived(int CurlErr, char* response, void* userData);
    friend bool CB_OnDBRowDPSListExtractFinished(int rowIndex, class ExtractDBColumnToBinary* rowColDataArr);
    unsigned __int64 m_PrevValuesDPSList;

    // Extracted functions for the sake of code readability
    void RefreshDPSListIfNeeded();
    void ApplyNewDPSListIfNeeded();
    void CheckOfflineConnections();
    void DisconnectAllConnections();
    void PeriodicUpdateAllConnections(); // send init packets...

    bool m_bRefreshDPSList;
    WatchdogThreadData* m_pWorkerThread;
    // avoid spamming reconnections. UI might not have enough time to connect before it receives a timeout event
    unsigned __int64 m_dQueuedDPSRefreshStamp;

    // the active WS connections where we will receive alerts and Module feed
    std::mutex m_DPSListRefreshMutex;
    std::vector<DPSConnectionInfo> m_RequiredConnections;
    std::vector<DPSConnectionInfo> m_HaveConnections;

    // I wonder what is the best approach is. Have a list of modules with possible subscriptions. Might remake the implementation
    std::set<unsigned __int64> m_AlertSubscribedModuleIds;
    std::set<unsigned __int64> m_FeedSubscribedModuleIds;
};

#define sDataSourceManager ModuleDataSourceManager::getInstance()