#pragma once

// Periodically report health of the DPS to know when to launch new instances

#include <array>

// value is in ms
#define SHCK_TIME_INTERVAL 1*60*1000		// Avg values are reset every X ms
#define SHCK_THREAD_SLEEP_PERIOD 10*1000	// fetch CPU usage..

class WatchdogThreadData;

class ServerHealthReportingManager
{
public:
	inline static ServerHealthReportingManager& getInstance()
	{
		static ServerHealthReportingManager instance;
		return instance;
	}
	~ServerHealthReportingManager();
	void DestructorCheckMemLeaks();
	// start worker threads. Get the list of alerts to manage
	void Init(__int64 DPSId);
	// WS clients
	inline void ReportPacketSent(__int64 size)
	{
		m_dIntervalBytesTx += size;
		m_dIntervalPktTx++;
	}
	// UDP and WS clients
	inline void ReportPacketRecv(__int64 size)
	{
		m_dIntervalBytesRx += size;
		m_dIntervalPktRx++;
	}
private:
	ServerHealthReportingManager() {};
	ServerHealthReportingManager(const ServerHealthReportingManager&) = delete;
	ServerHealthReportingManager& operator=(const ServerHealthReportingManager&) = delete;

	// periodically update the cache content
	friend void ServerHealthReportingManager_AsyncExecuteThread(WatchdogThreadData* wtd);
	void ResetInervalValues();
	__int64 m_dDPSId;
	unsigned __int64 m_dStartupStamp;
	unsigned __int64 m_dIntervalEndStamp;
	__int64 m_dIntervalValuesAdded;

	__int64 m_dIntervalBytesRx;
	__int64 m_dIntervalBytesTx;
	__int64 m_dIntervalPktRx;
	__int64 m_dIntervalPktTx;

	float m_fIntervalCPUUsage;
	float m_fPeakCPUUsage;
};

#define sServerHealthReportingManager ServerHealthReportingManager::getInstance()
