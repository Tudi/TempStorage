#include <thread>
#include <mutex>
#include <list>
#include <Windows.h>
#include "Util/Allocator.h"
#include "ResourceManager/LogManager.h"
#include "Session/ApplicationSession.h"

WatchdogThreadData::WatchdogThreadData()
{
    m_Thread = NULL;
    m_HeartbeatPeriod = 0;
    m_IsAlive = true;
    m_IsAskedToShutdown = false;
    m_bIsSleeping = false;
    m_LastHeartbeatStamp = GetTickCount64();
    m_GraceStartedStamp = 0;
    m_UpdateLoopCount = 0;
    m_bShouldSkipNextSleep = false;
    m_pThreadGroup = NULL;
    m_WakeupEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_WakeupEvent == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceApplicationSession, 0, 0,
            "Watchdog:Failed to create wakeup event");
    }
}

WatchdogThreadData::~WatchdogThreadData()
{
    CloseHandle(m_WakeupEvent);
    m_WakeupEvent = NULL;
}

void WatchdogThreadData::Init(std::thread* pThread, const uint64_t heartbeatPeriod, const char* threadName)
{
    m_Thread = pThread;
    m_HeartbeatPeriod = heartbeatPeriod;
    if (threadName != NULL)
    {
        m_sThreadName = threadName;
    }
}

int WatchdogThreadData::GetGroupId() 
{ 
    if (m_pThreadGroup == NULL)
    {
        return -1;
    }
    else
    {
        return m_pThreadGroup->GetGroupId();
    }
}

void WatchdogThreadData::BlockThreadUntil(uint64_t TimeoutMS)
{
    if (m_bIsSleeping == false)
    {
        if (m_bShouldSkipNextSleep)
        {
            m_bShouldSkipNextSleep = false;
        }
        else
        {
            m_bIsSleeping = true;
            WaitForSingleObject(m_WakeupEvent, (DWORD)TimeoutMS);
            m_bIsSleeping = false;
        }
    }
}

void WatchdogThreadData::BlockThreadUntilNextCycle()
{
    uint64_t sleepDuration = 0;
    if (m_bShouldSkipNextSleep)
    {
        m_bShouldSkipNextSleep = false;
    }
    else
    {
        ULONGLONG endStamp = GetTickCount64();
        if (endStamp < m_LastHeartbeatStamp + UpdatePeriod())
        {
            uint64_t loopDuration = endStamp - m_LastHeartbeatStamp;
            sleepDuration = UpdatePeriod() - loopDuration;
        }
    }

    // check and report thread busy status
    if (m_pThreadGroup != NULL)
    {
        m_pThreadGroup->OnThreadStartSleep(sleepDuration);
    }
    if (sleepDuration > 0)
    {
        BlockThreadUntil(sleepDuration);
    }
}

void WatchdogThreadData::WakeupThread()
{
    if (m_bIsSleeping == false)
    {
        if (m_pThreadGroup != NULL)
        {
            m_pThreadGroup->OnFailedWakeup();
        }
        m_bShouldSkipNextSleep = true;
    }
//    else // going to always wake the tread up since m_bIsSleeping is not thread safe
    {
        SetEvent(m_WakeupEvent);
    }
}

void WatchdogThreadData::SignalHeartbeat()
{
    m_LastHeartbeatStamp = GetTickCount64();
    m_UpdateLoopCount++;
    m_bIsSleeping = false;
    m_bShouldSkipNextSleep = false;
}

void WatchdogThreadData::SignalShutdown() 
{ 
    if (m_IsAskedToShutdown == false)
    {
        m_IsAskedToShutdown = true;
        m_GraceStartedStamp = GetTickCount64();
    }
    WakeupThread();
}

bool WatchdogThreadData::ShouldBeKilled()
{
    uint64_t timeSinceLastLoop = GetTickCount64() - m_LastHeartbeatStamp;
    if (timeSinceLastLoop > m_HeartbeatPeriod + WATCHDOG_WAIT_GRACE_PERIOD)
    {
        return true;
    }
    if (m_GraceStartedStamp != 0)
    {
        uint64_t timeSinceGraceInitiated = GetTickCount64() - m_GraceStartedStamp;
        if (timeSinceGraceInitiated > WATCHDOG_WAIT_GRACE_PERIOD)
        {
            return true;
        }
    }
    return false;
}

bool WatchdogThreadData::IsAlive()
{
    if (m_IsAlive == false)
    {
        return false;
    }
    if (m_Thread == NULL)
    {
        return false;
    }
    if (m_Thread->joinable() == false)
    {
        m_IsAlive = false;
        m_Thread = NULL;
        return false;
    }
    return true;
}

void WatchdogThreadData::UngracefullKillThread()
{
    try 
    {
        if (m_Thread && m_Thread->joinable())
        {
            m_Thread->detach();
        }
    }
    catch (...)
    {
    }
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceApplicationSession, 0, 0,
        "AppSess:Ungracefully killed worker thread %s", m_sThreadName.c_str());
}

void ApplicationSession::AddModuleThread(WatchdogThreadData *wtd)
{
    std::lock_guard<std::mutex> lock(m_mThreadListLock);
    m_ThreadsToWaitForExit.push_front(wtd);

    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceApplicationSession, 0, 0,
        "AppSess:Added worker thread %s. Update period %d. Number of threads monitored %d", wtd->GetName(), wtd->UpdatePeriod(), (int)m_ThreadsToWaitForExit.size());
}

void ApplicationSession::WaitAllModuleThreadShutdown()
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceApplicationSession, 0, 0,
        "AppSess:Started waiting on worker threads to exit. Number of threads monitored %d", (int)m_ThreadsToWaitForExit.size());

    std::lock_guard<std::mutex> lock(m_mThreadListLock);

    // shut down worker threads
    for (int i = 0; i < 100; i++)
    {
        bool hasActiveThreads = false;
        // iterate through all the registered threads
        for (auto itr = m_ThreadsToWaitForExit.begin(); itr != m_ThreadsToWaitForExit.end(); itr++)
        {
            // in case one module killed another one, we only set values to NULL in this list. Never remove nodes
            if ((*itr) == NULL)
            {
                continue;
            }

            // dead threads can be 
            if ((*itr)->IsAlive() == false)
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceApplicationSession, 0, 0,
                    "AppSess:Exited worker thread %s", (*itr)->GetName());

                InternalDelete((*itr));

                (*itr) = NULL;
                continue;
            }

            // we can call this many times
            (*itr)->SignalShutdown();

            // if it refuses to die, we kill it
            if ((*itr)->ShouldBeKilled())
            {
                (*itr)->UngracefullKillThread();
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceApplicationSession, 0, 0,
                    "AppSess:Killed worker thread %s", (*itr)->GetName());
            }

            // looks like we should run the loop again
            hasActiveThreads = true;
        }

        if (hasActiveThreads == false)
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceApplicationSession, 0, 0,
                "AppSess:All worker threads exited");
            break;
        }
        else
        {
            // give some time for worker threads to exit
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // we deleted all thread stores
    m_ThreadsToWaitForExit.clear();
}

void ApplicationSession::DestructorCheckMemLeaks()
{
#ifdef _DEBUG
    delete &sAppSession;
#endif
}

void ApplicationSession::CreateWorkerThreadGroup(WorkerThreadGroups wtg, int MaxThreads, const char* ThreadName, WorkerThreadFnc FNC, unsigned __int64 DefaultSleep)
{
    if (m_ThreadGroups[wtg].IsInitialized())
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceApplicationSession, 0, 0,
            "AppSess:WorkerThread group %d already initialized", (int)wtg);
        return;
    }
    if(FNC == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceApplicationSession, 0, 0,
            "AppSess:WorkerThread group %d can't have NULL worker function", (int)wtg);
        return;
    }
    if (DefaultSleep < ThreadSleepAcceptedErrorTime)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceApplicationSession, 0, 0,
            "AppSess:WorkerThread group %d default sleep is too small. High chance of always busy", (int)DefaultSleep);
        return;
    }
    m_ThreadGroups[wtg].Init(wtg, MaxThreads, ThreadName, FNC, DefaultSleep);
    m_ThreadGroups[wtg].CloneNewWorkerThread();
}

void ApplicationSession::WakeUpWorkerThread(WorkerThreadGroups wtg, bool bAll)
{
    std::lock_guard<std::mutex> lock(m_mThreadListLock);
    for (auto itr = m_ThreadsToWaitForExit.begin(); itr != m_ThreadsToWaitForExit.end(); itr++)
    {
        if ((*itr)->IsSleeping() == true && (*itr)->GetGroupId() == (int)wtg)
        {
            (*itr)->WakeupThread();
            if (bAll == false)
            {
                return;
            }
        }
    }
    if (bAll == false)
    {
        m_ThreadGroups[wtg].OnFailedWakeup();
    }
}

WatchdogThreadData* ApplicationSession::CreateWorkerThread(WorkerThreadFnc FNC, const char* ThreadName, unsigned __int64 DefaultSleep, WorkerThreadGroups wtg)
{
    WatchdogThreadData* wtd;
    InternalNew(wtd, WatchdogThreadData);
    if (wtd == NULL)
    {
        return NULL;
    }

    // busy stats are generated for this group
    if (wtg > WTG_INVALID && wtg < WTG_MAX_WTG)
    {
        wtd->SetThreadGroup(&m_ThreadGroups[wtg]);
    }

    // start worker thread to push log messages to server
    std::thread* myThread = new std::thread(FNC, wtd);
    wtd->Init(myThread, DefaultSleep, ThreadName);

    // Make the application wait until this thread also exits
    sAppSession.AddModuleThread(wtd);

    return wtd;
}

#ifdef _WIN32
#include <stdio.h>
#include <windows.h>
int GetPhisicalThreadCount()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}
#else
#include <stdio.h>
#include <unistd.h>
int GetPhisicalThreadCount()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}
#endif

WorkerThreadGroupData::WorkerThreadGroupData()
{
    m_dGroupId = (WorkerThreadGroups)-1;
    m_dThreadsCreated = 0;
    m_dMaxThreads = GetPhisicalThreadCount() * 2;
    m_sBaseName[0] = 0;
    memset(m_Sleeps, 10000, sizeof(m_Sleeps));
    m_dSleepWriteIndex = 0;
    m_dFailedWakeups = 0;
    m_pWorkerFnc = NULL;
    m_dSleepUntilHeartbeat = 0;
}

void WorkerThreadGroupData::Init(WorkerThreadGroups wtg, int maxThreads, const char* ThreadName, WorkerThreadFnc FNC, unsigned __int64 DefaultSleep)
{
    m_dGroupId = wtg;
    if (maxThreads != 0)
    {
        m_dMaxThreads = maxThreads;
    }
    if (ThreadName != NULL)
    {
        snprintf(m_sBaseName, sizeof(m_sBaseName), "%s", ThreadName);
    }
    m_pWorkerFnc = FNC;
    m_dSleepUntilHeartbeat = DefaultSleep;

    memset(m_Sleeps, 10000, sizeof(m_Sleeps));
    m_dSleepWriteIndex = 0;
    m_dFailedWakeups = 0;
}

void WorkerThreadGroupData::OnThreadStartSleep(unsigned __int64 dSleep)
{
    m_Sleeps[m_dSleepWriteIndex] = dSleep;
    m_dSleepWriteIndex = (m_dSleepWriteIndex + 1) & ThreadMonitoredSleepCount;
    if (ShouldCloneNewWorkerThread())
    {
        CloneNewWorkerThread();
        //reset busy stats
        memset(m_Sleeps, 10000, sizeof(m_Sleeps));
        m_dSleepWriteIndex = 0;
        m_dFailedWakeups = 0;
    }
}

void WorkerThreadGroupData::OnFailedWakeup()
{
    m_dFailedWakeups++;
}

bool WorkerThreadGroupData::ShouldCloneNewWorkerThread()
{
    if (m_dFailedWakeups > ThreadRequiredFailedWakeups)
    {
        return true;
    }
    size_t sleepCount = 0;
    for (size_t i = 0; i < _countof(m_Sleeps); i++)
    {
        if (m_Sleeps[i] > ThreadSleepAcceptedErrorTime)
        {
            sleepCount++;
        }
    }
    if (sleepCount < ThreadRequiredSleeps)
    {
        return true;
    }
    // if worker thread found time to sleep, it's all good
    return false;
}

bool WorkerThreadGroupData::CloneNewWorkerThread()
{
    if (sAppSession.IsApplicationRunning() == false)
    {
        return false;
    }

    std::unique_lock<std::mutex> lock(m_ThreadCountProtect, std::defer_lock);
    lock.lock();
    if (m_dThreadsCreated >= m_dMaxThreads)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceApplicationSession, 0, 0,
            "WorkerThreadGroupData:Cloning failed. Reached max %d allowed worker threads in %s group", m_dMaxThreads, m_sBaseName);
        return false;
    }
    m_dThreadsCreated++;
    lock.unlock();

    char szThreadName[500];
    sprintf_s(szThreadName, "%s %d / %d", m_sBaseName, m_dThreadsCreated, m_dMaxThreads);

    // Make the application wait until this thread also exits
    sAppSession.CreateWorkerThread(m_pWorkerFnc, szThreadName, m_dSleepUntilHeartbeat, m_dGroupId);

    return true;
}