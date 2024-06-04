#pragma once

#define APPLICATION_VERSION "1.0.0.1"

#define WATCHDOG_WAIT_GRACE_PERIOD 60000

#include <list>
#include <array>

class WorkerThreadGroupData;

// We are not expecting any thread to hang. But let's plan for LUA integration
class WatchdogThreadData
{
public:
    WatchdogThreadData();
    ~WatchdogThreadData();
    void Init(std::thread* pThread, const uint64_t heartbeatPeriod, const char* threadName);
    // worker thread updates hearbeat
    void SignalHeartbeat();
    // watchdog is asking worker thread to exit
    void SignalShutdown();
    // worker thread can query if watchdog is trying to kill it
    bool ShouldShutDown() { return m_IsAskedToShutdown; }
    // watchdog checks if worker thread has not been updated over long time
    bool ShouldBeKilled();
    void MarkDead() { m_IsAlive = 0; m_Thread = NULL; }
    // watchod to check if this thread still exists
    bool IsAlive();
    // hope we never get to kill threads like this
    void UngracefullKillThread();
    // Name of the worker thread. For the sake of logs
    const char* GetName() const { return m_sThreadName.c_str(); }
    // Update period. For the sake of logs
    uint64_t UpdatePeriod() const { return m_HeartbeatPeriod; }
    // block the thread until wakeupevent or timeout
    void BlockThreadUntilNextCycle();
    void BlockThreadUntil(uint64_t TimeoutMS);
    bool IsSleeping() const { return m_bIsSleeping; }
    // trigger the event to wake up worker thread right now
    void WakeupThread();
    void* GetEventHandle() { return m_WakeupEvent; }
    // when we want to wake up an already active worker thread because we queued up work for him
    // we will skip sleeping 1 round to make sure the queued up work gets processed before the sleep
    void SkipNextSleep() { m_bShouldSkipNextSleep = true; }
    void SetThreadGroup(WorkerThreadGroupData* tg) { m_pThreadGroup = tg; }
    int GetGroupId();
    WorkerThreadGroupData* GetThreadGroupInfo() { return m_pThreadGroup; }
private:
    std::thread* m_Thread;  // the actual worker thread
    uint64_t m_HeartbeatPeriod; // worker thread is not expected to give a heartbeat sooner
    bool m_IsAlive;        // worker thread should set this 
    bool m_IsAskedToShutdown;
    bool m_bIsSleeping;     // set when BlockThreadUntil is called
    uint64_t m_LastHeartbeatStamp; // last time the worker thread was alive
    uint64_t m_UpdateLoopCount;
    uint64_t m_GraceStartedStamp;
    std::string m_sThreadName;
    void* m_WakeupEvent;
    bool m_bShouldSkipNextSleep;
    WorkerThreadGroupData * m_pThreadGroup;
};

enum WorkerThreadGroups
{
    WTG_INVALID = -1,

    // DPS threadgroups
    WTG_Alert_Lifecylce = 0,
    WTG_UDP_Packet_Parser = 1,
    WTG_WS_Packet_Parser = 2,
    WTG_DBTableCache = 3,
    WTG_AsyncTasks = 4,

    // UI thread pools
    WTG_MAX_WTG = 10
};

typedef void (*WorkerThreadFnc)(WatchdogThreadData *wtd);

#define ThreadMonitoredSleepCount 7
#define ThreadRequiredSleeps 2
#define ThreadRequiredFailedWakeups 2
#define ThreadSleepAcceptedErrorTime 50 // because even getting the time takes time

class WorkerThreadGroupData
{
public:
    WorkerThreadGroupData();
    bool IsInitialized() { return m_pWorkerFnc != NULL; }
    void Init(WorkerThreadGroups wtg, int MaxThreads, const char* ThreadName, WorkerThreadFnc FNC, unsigned __int64 DefaultSleep);
    void OnThreadStartSleep(unsigned __int64 dSleep);
    // work has been queued to this group but all threads were busy
    void OnFailedWakeup(); 
    // if he manages to sleep at least Y times every X intervals, we call it good enough
    bool ShouldCloneNewWorkerThread();
    bool CloneNewWorkerThread();
    int GetGroupId() { return (int)m_dGroupId; }
private:
    WorkerThreadGroups m_dGroupId;
    char m_sBaseName[512];
    unsigned __int64 m_dSleepUntilHeartbeat;
    WorkerThreadFnc m_pWorkerFnc; // newly cloned worker threads will launch this function
    unsigned __int64 m_Sleeps[ThreadMonitoredSleepCount];
    unsigned int m_dSleepWriteIndex; // every time this worker thread sleeps, it will register how busy the thread is
    int m_dFailedWakeups;
    std::mutex m_ThreadCountProtect; // rest is not thread safe. sad, but fast
    int m_dThreadsCreated;
    int m_dMaxThreads;
};

/// <summary>
/// Application related status storage
/// - keeps track if application should start shutting down
/// - keeps track of worker threads. Maybe later can use watchdog to check for deadlocks
/// </summary>
class ApplicationSession
{
public:
    inline static ApplicationSession& getInstance() {
#ifdef _DEBUG
        static ApplicationSession* instance = new ApplicationSession;
        return *instance;
#else
        static ApplicationSession instance;
        return instance;
#endif
    }
    /// <summary>
    /// Mark the application as running or shutting down
    /// </summary>
    /// <param name="bNewState"></param>
    void SetApplicationRunning(bool bNewState) { m_bAppIsRunning = bNewState; }

    /// <summary>
    /// 3rd party module threads should only run as long as application is running
    /// </summary>
    /// <returns></returns>
    bool IsApplicationRunning() { return m_bAppIsRunning; }

    /// <summary>
    /// Add a worker thread that should be waited for shutdown when the application is shutting down
    /// Some modules might want to save data to file or send to internet resource
    /// </summary>
    /// <param name="pThread"></param>
    void AddModuleThread(WatchdogThreadData* wtd);

    /// <summary>
    /// Main thread will call this function to wait for all 3rd party modules to shut down properly
    /// </summary>
    void WaitAllModuleThreadShutdown();

    /// <summary>
    /// Used for logging and crash reporting
    /// </summary>
    /// <returns></returns>
    const char* GetApplicationVersion() { return APPLICATION_VERSION; }

    /// <summary>
    /// Most API calls will require a session ID in order to reply back
    /// This function is called after a successfull login
    /// </summary>
    void SessionIdInit(const unsigned __int64 newSessionId, bool bResetCallCounter = true)
    {
        m_lluSessionId = newSessionId;
        if (bResetCallCounter == true)
        {
            m_lluSessionSalt = 0;
        }
    }

    /// <summary>
    /// Session ID used for most API calls
    /// </summary>
    unsigned __int64 SessionIdGet() { return m_lluSessionId; }

    /// <summary>
    /// Dynamic part of a session. User needs to know and keep track of this number of server will log him out
    /// </summary>
    /// <param name="saltChange"></param>
    void SessionSaltUpdate(const __int64 saltValue)
    {
        m_lluSessionSalt = saltValue;
    }

    /// <summary>
    /// Value that we should be able to guess in order for our query to succeed
    /// </summary>
    /// <returns></returns>
    unsigned __int64 SessionSaltGet()  
    { 
        __int64 appTime = time(NULL);
        __int64 serverTime = appTime / 3 + m_lluSessionSalt;
        return serverTime % 37;
    }

    /// <summary>
    /// Interfaced way of checking if the user has been successfully logged in
    /// </summary>
    bool IsUserLoggedIn() { return m_lluSessionId != 0; }

    /// <summary>
    /// Only when checking if Application has memory leaks
    /// </summary>
    void DestructorCheckMemLeaks();

    /// <summary>
    /// Create a thread group that will monitor if new threads are required and lauch them automatically
    /// </summary>    
    void CreateWorkerThreadGroup(WorkerThreadGroups wtg, int MaxThreads, const char* ThreadName, WorkerThreadFnc FNC, unsigned __int64 DefaultSleep);
    void WakeUpWorkerThread(WorkerThreadGroups wtg, bool bAll = false);
    WatchdogThreadData* CreateWorkerThread(WorkerThreadFnc FNC, const char* ThreadName, unsigned __int64 DefaultSleep, WorkerThreadGroups wtg = WTG_INVALID);
    size_t GetMonitoredThreadCount() { return m_ThreadsToWaitForExit.size(); };
    // application logic should wait until startup phase finished
    bool IsStartupPhaseDone() { return m_bFinishedStartupPhase; }
    void SetStartupPhaseStatus(bool newVal) { m_bFinishedStartupPhase = newVal; }
private:
    ApplicationSession() 
    {
        m_bAppIsRunning = true;
        m_lluSessionId = 0;
        m_lluSessionSalt = 0;
        m_bFinishedStartupPhase = false;
    };
    ApplicationSession(const ApplicationSession&) = delete;
    ApplicationSession& operator=(const ApplicationSession&) = delete;

    // global aggregated value that marks if the application should be running
    bool m_bAppIsRunning;
    bool m_bFinishedStartupPhase;

    // threads used by modules that need to be waited to be shut down on application exit
    // todo : convert from simple pointer to a storage to be able to monitor status
    std::list<WatchdogThreadData*> m_ThreadsToWaitForExit;
    // avoid corrupting the thread list due to multi threaded access
    std::mutex m_mThreadListLock;
    unsigned __int64 m_lluSessionId;
    __int64 m_lluSessionSalt; // part of session ID

    std::array<WorkerThreadGroupData, WTG_MAX_WTG> m_ThreadGroups;
};

#define sAppSession ApplicationSession::getInstance()