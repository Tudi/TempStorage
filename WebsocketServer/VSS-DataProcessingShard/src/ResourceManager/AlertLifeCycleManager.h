#pragma once

#include <vector>
#include <mutex>

#define ALERTLIFECYCLE_WORKERTHREAD_SLEEP 5000

class WatchdogThreadData;

/// <summary>
/// Create and Alert in DB, also make sure SMS is sent, Email is sent. Notify all subscribers about alert state
/// </summary>
class AlertLifeCycleManager
{
public:
	inline static AlertLifeCycleManager& getInstance()
	{
		static AlertLifeCycleManager instance;
		return instance;
	}
	~AlertLifeCycleManager();
	void DestructorCheckMemLeaks();
	// start worker threads. Get the list of alerts to manage
	void Init(__int64 DPSID, int workerThreads);
	// create a DB alert and queue up for Email + SMS sending
	void CreateAlertsFromModuleAlert(struct DSModuleAlert *ma, std::vector<DSModuleAlert> &out_Alerts);
	void CreateAlert(struct AlertQueryData* ma);
private:
	AlertLifeCycleManager();
	AlertLifeCycleManager(const AlertLifeCycleManager&) = delete;
	AlertLifeCycleManager& operator=(const AlertLifeCycleManager&) = delete;

	friend void AlertManager_AsyncExecuteThread(WatchdogThreadData* wtd);
	struct AlertQueueData
	{
		AlertQueueData() {}; // empty constructor. Set all data manually
		__int64 AlertId;
		VSS_N_ModuleAlertState::AlertTypes AlertTypeId;
		VSS_N_ModuleAlertState::AlertStateFlags StatusFlags;
		__int64 ModuleId;
		__int64 LocationId;
		__int64 OrganizationId;
		__int64 UserId;
	};
	void SelectAlertJob(__int64 &AlertId, VSS_N_ModuleAlertState::AlertStateFlags&setFlag, VSS_N_ModuleAlertState::AlertStateFlags&clearFlag);
	bool HandleWorkSendEmail(__int64 AlertId);
	bool HandleWorkSendSMS(__int64 AlertId);
	void MarkJobDone(__int64 AlertId, VSS_N_ModuleAlertState::AlertStateFlags setFlag, VSS_N_ModuleAlertState::AlertStateFlags clearFlag);

	std::mutex m_ListLock;
	std::vector<AlertQueueData> m_MonitoredAlerts;
};

#define sAlertManager AlertLifeCycleManager::getInstance()