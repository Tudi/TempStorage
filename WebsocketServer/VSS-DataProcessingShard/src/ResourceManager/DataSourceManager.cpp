#include <assert.h>
#include "DataSourceManager.h"
#include "DB/MysqlManager.h"
#include "LogManager.h"
#include "magic_enum/magic_enum_all.hpp"
#include "AlertLifeCycleManager.h"

VSSDSManager::VSSDSManager()
{
}

VSSDSManager::~VSSDSManager()
{
	DestructorCheckMemLeaks();
}

void VSSDSManager::DestructorCheckMemLeaks()
{
}

void VSSDSManager::Init(__int64 DPS_Id)
{
    std::vector<ModuleInstanceQueryData> rows;
    sDBManager.Query_DPSModules(DPS_Id, rows);

    size_t ModuleDefinitionCount = rows.size();
    if (ModuleDefinitionCount == 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
            "VSSDSManager:DPS %lld has not modules assigned. Nothing to do.", DPS_Id);
    }
    else
    {
        // convert the SQL rows to internal storage
        // Create the connections, initialize session data ...
        m_ModuleConnectionData.reserve(ModuleDefinitionCount);
        for (const auto& it : rows)
        {
            AddModuleConnectionInfo(it.InstanceId, it.ConnectionURL, it.Status);
        }
    }
}

void VSSDSManager::AddModuleConnectionInfo(const __int64 ModuleId, const std::string& ConnectionURL, const int AdminStatus)
{
    if (ModuleId <= 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
            "VSSDSManager:Module %%s has no ID. Can't subscribe to it", ConnectionURL.c_str());
        return;
    }
    if (ConnectionURL.length() == 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
            "VSSDSManager:Module %lld has no connection URL. Can't add it to connections", ModuleId);
        return;
    }
    ModuleConnectionData& row = m_ModuleConnectionData.emplace_back();
    row.InstanceId = ModuleId;
    row.ConnectionURL = ConnectionURL;
    row.Status = AdminStatus;
}

void VSSDSManager::SubscribeToModule(VSS_N_SubscribeModules::SubscriptionType SubType, const __int64 ModuleId, const DS_CB_OnDataArrived CB_func, void* UserData1, void* UserData2)
{
    std::unordered_map<__int64, std::vector<ModuleSubscriptionData>> *dstModules = &m_ModuleSubscriptionsAlert;
    if (SubType == VSS_N_SubscribeModules::SubscriptionType::SubscribeFeed)
    {
        dstModules = &m_ModuleSubscriptionsFeed;
    }

    // check if we already have the data
    const std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator &itr = 
        dstModules->find(ModuleId);
    if (itr != dstModules->end())
    {
        for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin();
            itr2 != itr->second.end();itr2++)
        {
            if (itr2->UserData1 == UserData1 && itr2->CB_func == CB_func)
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
                    "VSSDSManager:Warning. Possible multiple same subscriptions to the same module %lld by same listener", ModuleId);
            }
        }
    }

    m_MutexListLock.WriteLock();

    // do the actual subscription
    ModuleSubscriptionData &cbd = (*dstModules)[ModuleId].emplace_back();
    cbd.CB_func = CB_func;
    cbd.UserData1 = UserData1;
    cbd.UserData2 = UserData2;

    m_MutexListLock.WriteUnlock();

    // euuugh, this is ugly
    std::string_view subsTypeName = magic_enum::enum_name<VSS_N_SubscribeModules::SubscriptionType>(SubType);
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
        "VSSDSManager:New data listen subscription type %s for module %lld. Total %lld", subsTypeName.data(), ModuleId, (*dstModules)[ModuleId].size());
}

void VSSDSManager::UnSubscribeFromModule(VSS_N_SubscribeModules::SubscriptionType SubType, const __int64 ModuleId, const DS_CB_OnDataArrived CB_func, void* UserData1, void* UserData2)
{
    m_MutexListLock.WriteLock();

    std::unordered_map<__int64, std::vector<ModuleSubscriptionData>> *dstModules = &m_ModuleSubscriptionsAlert;
    if (SubType == VSS_N_SubscribeModules::SubscriptionType::UnsubscribeFeed)
    {
        dstModules = &m_ModuleSubscriptionsFeed;
    }
    // check if we already have the data
    const std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator& itr =
        dstModules->find(ModuleId);
    if (itr != dstModules->end())
    {
        bool FoundAnySubscriptions = false;
        for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end();)
        {
            if (itr2->UserData1 == UserData1 &&
                itr2->CB_func == CB_func &&
                itr2->UserData2 == UserData2)
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
                    "VSSDSManager:Subscription removed from module %lld. Total %lld", ModuleId, (*dstModules)[ModuleId].size());
                FoundAnySubscriptions = true;
                itr2 = itr->second.erase(itr2);
            }
            else
            {
                ++itr2;
            }
        }

        // just for the sake of debugging
        if (FoundAnySubscriptions == false)
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDPSDataSource, 0, 0,
                "VSSDSManager:Expected to remove subscription from module %lld. Found none", ModuleId);
        }
    }

    m_MutexListLock.WriteUnlock();
}

void VSSDSManager::UnSunscribeAll(void* UserData1)
{
    m_MutexListLock.WriteLock();

    // check if we already have the data
    for(std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator itr =
        m_ModuleSubscriptionsAlert.begin(); itr != m_ModuleSubscriptionsAlert.end(); itr++)
    {
        for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end();)
        {
            if (itr2->UserData1 == UserData1)
            {
                itr2 = itr->second.erase(itr2);
            }
            else
            {
                ++itr2;
            }
        }
    }
    for (std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator itr =
        m_ModuleSubscriptionsFeed.begin(); itr != m_ModuleSubscriptionsFeed.end(); itr++)
    {
        for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end();)
        {
            if (itr2->UserData1 == UserData1)
            {
                itr2 = itr->second.erase(itr2);
            }
            else
            {
                ++itr2;
            }
        }
    }

    m_MutexListLock.WriteUnlock();
}

void VSSDSManager::OnModuleFeedArrived(const __int64 InstanceId, DSModuleData* md)
{
    const std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator& itr =
        m_ModuleSubscriptionsFeed.find(InstanceId);
    // if we have no subscriptions to this module, exit this function
    if (itr != m_ModuleSubscriptionsFeed.end())
    {
        if (itr->second.empty())
        {
            return;
        }
    }
    // parse the input and format it so it's good for callbacks
    if (itr != m_ModuleSubscriptionsFeed.end())
    {
        m_MutexListLock.ReadLock();

        for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
        {
            itr2->CB_func(md, itr2->UserData1, itr2->UserData2);
        }

        m_MutexListLock.ReadUnlock();
    }
    // we might want to run alert script on this data
    // give it to some AI to analize it ?
}

void VSSDSManager::OnModuleAlertArrived(const __int64 InstanceId, DSModuleAlert* ma)
{
    const std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator& itr =
        m_ModuleSubscriptionsAlert.find(InstanceId);
    // if we have no subscriptions to this module, exit this function
    // Todo : so who should handle this alert ?
    if (itr != m_ModuleSubscriptionsAlert.end())
    {
        if (itr->second.empty())
        {
            return;
        }
    }
    // parse the input and format it so it's good for callbacks
    if (itr != m_ModuleSubscriptionsAlert.end())
    {
        std::vector<DSModuleAlert> softwareAlerts;
        sAlertManager.CreateAlertsFromModuleAlert(ma, softwareAlerts);

        // the hardware alert was not converted to any software alerts
        if (softwareAlerts.empty())
        {
            return;
        }

        m_MutexListLock.ReadLock();
        for (auto itr3 = softwareAlerts.begin(); itr3 != softwareAlerts.end(); itr3++)
        {
            DSModuleAlert &softAlert = (*itr3);
            // call each network session that wants this data to be relayed to
            for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
            {
                itr2->CB_func(&softAlert, itr2->UserData1, itr2->UserData2);
            }
        }

        m_MutexListLock.ReadUnlock();
    }
}

void VSSDSManager::OnModuleAlertUpdate(DSModuleAlert* ma)
{
    const std::unordered_map<__int64, std::vector<ModuleSubscriptionData>>::iterator& itr =
        m_ModuleSubscriptionsAlert.find(ma->ModuleID);
    // if we have no subscriptions to this module, exit this function
    // Todo : so who should handle this alert ?
    if (itr != m_ModuleSubscriptionsAlert.end())
    {
        if (itr->second.empty())
        {
            return;
        }
    }
    // parse the input and format it so it's good for callbacks
    if (itr != m_ModuleSubscriptionsAlert.end())
    {
        m_MutexListLock.ReadLock();

        // call each network session that wants this data to be relayed to
        for (std::vector<ModuleSubscriptionData>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
        {
            itr2->CB_func(ma, itr2->UserData1, itr2->UserData2);
        }

        m_MutexListLock.ReadUnlock();
    }
}