#include <stddef.h>
#include "Network/VssPacketDefines.h"
#include "AlertLifeCycleManager.h"
#include "DataSourceManager.h"
#include "DB/MysqlManager.h"
#include "DB/TableCacheManager.h"
#include "Session/ApplicationSession.h"
#include "Util/Allocator.h"
#include "ResourceManager/LogManager.h"
#include "Web/WebApiInterface.h"
#include "Util/VariousFuncs.h"

AlertLifeCycleManager::AlertLifeCycleManager()
{
}

AlertLifeCycleManager::~AlertLifeCycleManager()
{
	DestructorCheckMemLeaks();
}

void AlertLifeCycleManager::DestructorCheckMemLeaks()
{
	std::unique_lock<std::mutex> lock(m_ListLock);
	m_MonitoredAlerts.clear();
}

void AlertManager_AsyncExecuteThread(WatchdogThreadData* wtd);
void AlertLifeCycleManager::Init(__int64 DPSID, int workerThreads)
{
	// load alerts from DB that should be managed by this DPS
	std::vector<AlertQueryData> alertQueryResults;
	sDBManager.Query_DPSAlerts(DPSID, alertQueryResults);
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
		"AlertLifeCycleManager:Queue up %lld alerts that still require notifications", alertQueryResults.size());
	for (auto itr = alertQueryResults.begin(); itr != alertQueryResults.end(); itr++)
	{
		CreateAlert(&(*itr));
	}

	// create worker thread
	sAppSession.CreateWorkerThreadGroup(WTG_Alert_Lifecylce, workerThreads, "AlertHandle", AlertManager_AsyncExecuteThread, ALERTLIFECYCLE_WORKERTHREAD_SLEEP);
}

void AlertLifeCycleManager::CreateAlert(struct AlertQueryData* ma)
{
	{
		std::unique_lock<std::mutex> lock(m_ListLock);
		AlertQueueData& aqd = m_MonitoredAlerts.emplace_back();
		aqd.AlertId = ma->AlertId;
		aqd.StatusFlags = (VSS_N_ModuleAlertState::AlertStateFlags)(ma->AlertStatusTypeId | (unsigned int)VSS_N_ModuleAlertState::AlertStateFlags::Triggered);
		aqd.ModuleId = ma->ModuleId;
		aqd.LocationId = ma->LocationId;
		aqd.OrganizationId = ma->OrganizationId;
		aqd.UserId = ma->UserId;
		aqd.AlertTypeId = (VSS_N_ModuleAlertState::AlertTypes)ma->AlertTypeId;
	}

	sAppSession.WakeUpWorkerThread(WTG_Alert_Lifecylce);
}

void AlertLifeCycleManager::CreateAlertsFromModuleAlert(struct DSModuleAlert* ma, std::vector<DSModuleAlert>& out_Alerts)
{
	ma->StateFlags = VSS_N_ModuleAlertState::AlertStateFlags::Triggered;

	// get the list of Organizations and users that are subscribed to this module + alert type
	sDBManager.Query_SoftwareAlerts(ma->ModuleID, (int)ma->AlertType, out_Alerts);

	// No software alerts are needed for this HW alert
	if (out_Alerts.empty() == true)
	{
		return;
	}

	// fetch location info for this Module. Could cache this
	ma->LocationId = sTableCacheManager.GetModuleLocation(ma->ModuleID);

	// create a software alert for each company that wishes to receive this HW alert
	std::unique_lock<std::mutex> lock(m_ListLock, std::defer_lock);
	lock.lock();
	for (auto itr = out_Alerts.begin(); itr != out_Alerts.end(); itr++)
	{
		// copy HW alert details to SW Alert details
		(*itr).AlertType = ma->AlertType;
		(*itr).TriggerStamp = ma->TriggerStamp;
		(*itr).LocationId = ma->LocationId;
		(*itr).ModuleID = ma->ModuleID;
		(*itr).AlertType = ma->AlertType;

		sDBManager.Create_AlertInstance((*itr).AlertDefinitionId, (int)(*itr).AlertType, (*itr).TriggerStamp, (*itr).LocationId, (*itr).ModuleID, (*itr).OrganizationId, (*itr).UserId, (*itr).AlertId);

		AlertQueueData& aqd = m_MonitoredAlerts.emplace_back();
		aqd.AlertId = (*itr).AlertId;
		aqd.StatusFlags = VSS_N_ModuleAlertState::AlertStateFlags::Triggered;
		aqd.ModuleId = (*itr).ModuleID;
		aqd.LocationId = (*itr).LocationId;
		aqd.OrganizationId = (*itr).OrganizationId;
		aqd.UserId = (*itr).UserId;
		aqd.AlertTypeId = (*itr).AlertType;

		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
			"AlertLifeCycleManager:Created new Alert with ID %lld", (*itr).AlertId);

		// wake up a worker thread if there are none already working
		sAppSession.WakeUpWorkerThread(WTG_Alert_Lifecylce);
	}
	lock.unlock();
}

bool AlertLifeCycleManager::HandleWorkSendEmail(__int64 AlertId)
{
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
		"AlertLifeCycleManager:Sending email notification for Alert %lld", AlertId);

	DSModuleAlert ma;
	std::unique_lock<std::mutex> lock(m_ListLock, std::defer_lock);
	lock.lock();
	for (auto itr = m_MonitoredAlerts.begin(); itr != m_MonitoredAlerts.end(); ++itr)
	{
		if (itr->AlertId == AlertId)
		{
			ma.ModuleID = itr->ModuleId;
			ma.LocationId = itr->LocationId;
			ma.AlertId = itr->AlertId;
			ma.StateFlags = itr->StatusFlags;
			ma.OrganizationId = itr->OrganizationId;
			ma.UserId = itr->UserId;
			ma.AlertType = itr->AlertTypeId;
			break;
		}
	}
	lock.unlock();

	// notify connected UI client about the status update
	ma.StateFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags(ma.StateFlags, VSS_N_ModuleAlertState::AlertStateFlags::EmailPending);
	sDSManager.OnModuleAlertUpdate(&ma);

	// call web API to send notification
	WebApiErrorCodes er = WebApi_CreateEmailAlert(0, 0, AlertId);
	if (er != WebApiErrorCodes::WAE_NoError && er != WebApiErrorCodes::WAE_EmptyResponse)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
			"AlertLifeCycleManager:Failed to send Email for Alert %lld", AlertId);

		// maybe notify UI that the alert is no longer in sending state ? Retry send ?

		return false;
	}

	// update DB and mark this Alert as it's notification was sent
	bool dbRes = sDBManager.Update_AlertStatus(AlertId, (int)VSS_N_ModuleAlertState::AlertStateFlags::EmailSent);
	if (dbRes != true)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
			"AlertLifeCycleManager:Failed to update DB with send Email status for Alert %lld", AlertId);
	}

	// notify the subscribed WS clients about the new status
	ma.StateFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumClearFlags(ma.StateFlags, VSS_N_ModuleAlertState::AlertStateFlags::EmailPending);
	ma.StateFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags(ma.StateFlags, VSS_N_ModuleAlertState::AlertStateFlags::EmailSent);
	sDSManager.OnModuleAlertUpdate(&ma);

	return true;
}

bool AlertLifeCycleManager::HandleWorkSendSMS(__int64 AlertId)
{
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
		"AlertLifeCycleManager:Sending SMS notification for Alert %lld", AlertId);

	DSModuleAlert ma;
	std::unique_lock<std::mutex> lock(m_ListLock, std::defer_lock);
	lock.lock();
	for (auto itr = m_MonitoredAlerts.begin(); itr != m_MonitoredAlerts.end(); ++itr)
	{
		if (itr->AlertId == AlertId)
		{
			ma.ModuleID = itr->ModuleId;
			ma.LocationId = itr->LocationId;
			ma.AlertId = itr->AlertId;
			ma.AlertType = itr->AlertTypeId;
			ma.StateFlags = itr->StatusFlags;
			ma.OrganizationId = itr->OrganizationId;
			ma.UserId = itr->UserId;
			ma.AlertType = itr->AlertTypeId;
			break;
		}
	}
	lock.unlock();

	// notify connected UI client about the status update
	ma.StateFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags(ma.StateFlags, VSS_N_ModuleAlertState::AlertStateFlags::SMSPending);
	sDSManager.OnModuleAlertUpdate(&ma);

	// call web API to send notification
	WebApiErrorCodes er = WebApi_CreateSMSAlert(0, 0, AlertId);
	if (er != WebApiErrorCodes::WAE_NoError && er != WebApiErrorCodes::WAE_EmptyResponse)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
			"AlertLifeCycleManager:Failed to send SMS for Alert %lld", AlertId);

		// maybe notify UI that the alert is no longer in sending state ? Retry send ?

		return false;
	}

	// update DB and mark this Alert as it's notification was sent
	bool dbRes = sDBManager.Update_AlertStatus(AlertId, (int)VSS_N_ModuleAlertState::AlertStateFlags::SMSSent);
	if (dbRes != true)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
			"AlertLifeCycleManager:Failed to update DB with send SMS status for Alert %lld", AlertId);
	}

	// notify the subscribed WS clients about the new status
	ma.StateFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumClearFlags(ma.StateFlags, VSS_N_ModuleAlertState::AlertStateFlags::SMSPending);
	ma.StateFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags(ma.StateFlags, VSS_N_ModuleAlertState::AlertStateFlags::SMSSent);
	sDSManager.OnModuleAlertUpdate(&ma);

	return false;
}

void AlertLifeCycleManager::SelectAlertJob(__int64& AlertId, VSS_N_ModuleAlertState::AlertStateFlags& setFlag, 
	VSS_N_ModuleAlertState::AlertStateFlags& clearFlag)
{
	std::unique_lock<std::mutex> lock(m_ListLock);
	for (auto itr = m_MonitoredAlerts.begin(); itr != m_MonitoredAlerts.end(); ++itr)
	{
		// can we send an email ?
		if (EnumHasAnyFlag((*itr).StatusFlags, VSS_N_ModuleAlertState::AlertStateFlags::ASF_EmailJobNotAvailable) == false)
		{
			AlertId = (*itr).AlertId;
			setFlag = VSS_N_ModuleAlertState::AlertStateFlags::EmailSent;
			clearFlag = VSS_N_ModuleAlertState::AlertStateFlags::EmailPending;
			// Mark this job as pending to be worked on
			(*itr).StatusFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags((*itr).StatusFlags, clearFlag);

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
				"AlertLifeCycleManager:Picked Email send job for Alert %lld", AlertId);

			return;
		}
		// can we send an SMS ?
		else if (EnumHasAnyFlag((*itr).StatusFlags, VSS_N_ModuleAlertState::AlertStateFlags::ASF_SMSJobNotAvailable) == false)
		{
			AlertId = (*itr).AlertId;
			setFlag = VSS_N_ModuleAlertState::AlertStateFlags::SMSSent;
			clearFlag = VSS_N_ModuleAlertState::AlertStateFlags::SMSPending;
			// Mark this job as pending to be worked on
			(*itr).StatusFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags((*itr).StatusFlags, clearFlag);

			AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
				"AlertLifeCycleManager:Picked SMS send job for Alert %lld", AlertId);

			return;
		}
	}
}

void AlertLifeCycleManager::MarkJobDone(__int64 AlertId, VSS_N_ModuleAlertState::AlertStateFlags setFlag, 
	VSS_N_ModuleAlertState::AlertStateFlags clearFlag)
{
	std::unique_lock<std::mutex> lock(m_ListLock);
	for (auto itr = m_MonitoredAlerts.begin(); itr != m_MonitoredAlerts.end(); ++itr)
	{
		if (itr->AlertId == AlertId)
		{
			(*itr).StatusFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumSetFlags((*itr).StatusFlags, setFlag);
			(*itr).StatusFlags = (VSS_N_ModuleAlertState::AlertStateFlags)EnumClearFlags((*itr).StatusFlags, clearFlag);

			// all work done for this Alert ? Remove it from work queue
			if (EnumHasAllFlags((*itr).StatusFlags, VSS_N_ModuleAlertState::AlertStateFlags::ASF_HAS_ALL_JOBS_DONE) == true &&
				EnumHasAnyFlag((*itr).StatusFlags, VSS_N_ModuleAlertState::AlertStateFlags::ASF_HAS_PENDING_JOBS) == false)
			{
				m_MonitoredAlerts.erase(itr);
				
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
					"AlertLifeCycleManager:All jobs are done for Alert %lld", AlertId);

				if (m_MonitoredAlerts.empty())
				{
					AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
						"AlertLifeCycleManager:No more queued alerts to process");
				}
			}

			return;
		}
	}

	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
		"AlertLifeCycleManager:Had a job for Alert %lld, but alert is no longer available", AlertId);
}

void AlertManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() &&
		wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		{
			VSS_N_ModuleAlertState::AlertStateFlags workType;
			VSS_N_ModuleAlertState::AlertStateFlags workTypePending;
			do{
				workType = VSS_N_ModuleAlertState::AlertStateFlags::None;
				__int64 AlertId = 0;

				// check if there is something to do
				sAlertManager.SelectAlertJob(AlertId, workType, workTypePending);

				// handle the work
				if (workType == VSS_N_ModuleAlertState::AlertStateFlags::EmailSent)
				{
					sAlertManager.HandleWorkSendEmail(AlertId);
					// no retry is supported
					sAlertManager.MarkJobDone(AlertId, workType, workTypePending);
				}
				else if (workType == VSS_N_ModuleAlertState::AlertStateFlags::SMSSent)
				{
					sAlertManager.HandleWorkSendSMS(AlertId);
					// no retry is supported
					sAlertManager.MarkJobDone(AlertId, workType, workTypePending);
				}
				else if (workType != VSS_N_ModuleAlertState::AlertStateFlags::None)
				{
					AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAlertLifeCycle, 0, 0,
						"AlertLifeCycleManager:Unexpected job type %d for Alert %lld", (int)workType, AlertId);
				}
				// if we found work, search again until we find no more work
			}while (workType != VSS_N_ModuleAlertState::AlertStateFlags::None);
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}