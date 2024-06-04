#include <mutex>
#include <string>
#include <vector>
#include <array>
#include <Windows.h>
#include "Util/Allocator.h"
#include "ServerHealthReportingManager.h"
#include "ResourceManager/DataSourceManager.h"
#include "DB/MysqlManager.h"
#include "Session/ApplicationSession.h"
#include "Network/WsServer.h"

ServerHealthReportingManager::~ServerHealthReportingManager()
{
	DestructorCheckMemLeaks();
}

void ServerHealthReportingManager::DestructorCheckMemLeaks()
{
}

void ServerHealthReportingManager_AsyncExecuteThread(WatchdogThreadData* wtd);
void ServerHealthReportingManager::Init(__int64 DPSId)
{
	m_dDPSId = DPSId;
	m_dStartupStamp = time(NULL);
	m_fPeakCPUUsage = 0.0f;
	ResetInervalValues();

	sAppSession.CreateWorkerThread(ServerHealthReportingManager_AsyncExecuteThread, "ServerHealthReporting", SHCK_THREAD_SLEEP_PERIOD);
}

void ServerHealthReportingManager::ResetInervalValues()
{
	m_dIntervalEndStamp = GetTickCount64() + SHCK_TIME_INTERVAL;
	m_dIntervalValuesAdded = 0;
	m_dIntervalBytesRx = 0;
	m_dIntervalBytesTx = 0;
	m_dIntervalPktRx = 0;
	m_dIntervalPktTx = 0;
	m_fIntervalCPUUsage = 0.0f;
}

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <tlhelp32.h>
float GetCPUUsage()
{
	HANDLE hProcess = GetCurrentProcess();
	FILETIME createTime, exitTime, kernelTime, userTime;
	GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime);
	ULARGE_INTEGER li;
	li.LowPart = userTime.dwLowDateTime;
	li.HighPart = userTime.dwHighDateTime;
	return (float)(li.QuadPart / 10e6); // CPU time in seconds
}

void getProcessInfo(unsigned long &PID, size_t &memUsage, unsigned long &threadCount)
{
	// Get current process handle
	HANDLE hProcess = GetCurrentProcess();
	PID = GetCurrentProcessId();

	// Get memory information
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	memUsage = pmc.PrivateUsage;

	// Get thread count
	threadCount = 0;
	HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnapshot != INVALID_HANDLE_VALUE) 
	{
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);
		if (Thread32First(hThreadSnapshot, &te32)) 
		{
			do {
				if (te32.th32OwnerProcessID == GetCurrentProcessId()) 
				{
					threadCount++;
				}
			} while (Thread32Next(hThreadSnapshot, &te32));
		}
		CloseHandle(hThreadSnapshot);
	}
}
#else
float GetCPUUsage()
{
	// Get CPU usage
	double cpuUsage = 0.0;
	std::ifstream statFile("/proc/self/stat");
	if (statFile) {
		std::string line;
		std::getline(statFile, line);
		std::istringstream linestream(line);
		for (int i = 0; i < 13; ++i) {
			linestream.ignore();
		}
		unsigned long utime, stime;
		linestream >> utime >> stime;
		cpuUsage = (utime + stime) / sysconf(_SC_CLK_TCK);
	}
	return cpuUsage;
}

void getProcessInfo(unsigned long& PID, size_t& memUsage, unsigned long& threadCount)
{
	// Get current process ID
	pid_t pid = getpid();
	PID = pid;

	// Get memory usage
	long virtualMemUsedByMe = 0;
	std::ifstream stat_stream("/proc/self/stat", std::ios_base::in);
	if (stat_stream.is_open()) 
	{
		std::string dummy;
		for (int i = 0; i < 22; ++i)
		{
			stat_stream >> dummy;
		}
		stat_stream >> virtualMemUsedByMe;
		stat_stream.close();
	}
	memUsage = virtualMemUsedByMe;

	// Get thread count
	threadCount = 0;
	std::ifstream threads_stream("/proc/self/status", std::ios_base::in);
	if (threads_stream.is_open()) 
	{
		std::string line;
		while (std::getline(threads_stream, line)) 
		{
			if (line.substr(0, 6) == "Threads:") 
			{
				std::istringstream linestream(line.substr(9));
				linestream >> threadCount;
				break;
			}
		}
		threads_stream.close();
	}
}
#endif

void ServerHealthReportingManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() &&
		wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		unsigned __int64 TickNow = GetTickCount64();

		float CPUUsageNow = GetCPUUsage();
		sServerHealthReportingManager.m_fIntervalCPUUsage += CPUUsageNow;
		sServerHealthReportingManager.m_dIntervalValuesAdded++;
		if (sServerHealthReportingManager.m_fPeakCPUUsage < CPUUsageNow)
		{
			sServerHealthReportingManager.m_fPeakCPUUsage = CPUUsageNow;
		}

		if (TickNow > sServerHealthReportingManager.m_dIntervalEndStamp)
		{
			unsigned long PID;
			size_t memUsage;
			unsigned long threadCount;
			getProcessInfo(PID, memUsage, threadCount);
			float CPUUsage = sServerHealthReportingManager.m_fIntervalCPUUsage / sServerHealthReportingManager.m_dIntervalValuesAdded;
			// report values
			sDBManager.Update_ServerHealthReport(sServerHealthReportingManager.m_dDPSId,
				threadCount,
				CPUUsage,
				sServerHealthReportingManager.m_fPeakCPUUsage,
				memUsage,
				sServerHealthReportingManager.m_dIntervalBytesRx,
				sServerHealthReportingManager.m_dIntervalBytesTx,
				sServerHealthReportingManager.m_dIntervalPktRx,
				sServerHealthReportingManager.m_dIntervalPktTx,
				sServerHealthReportingManager.m_dStartupStamp,
				PID,
				(int)sWSServer.GetClientConnectedCount()
				);
			// reset values
			sServerHealthReportingManager.ResetInervalValues();
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}