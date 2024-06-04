#pragma once

#include <queue>
#include <mutex>

#define MAX_LOG_MESSAGE_SIZE            ((int)31000)
#define LOG_PERSISTENT_DB_NAME          "./Logs/log.db"
#define LOG_FORMAT_VERSION_INFO         "1.0.0.1"
#define LOG_FILE_VERSION_NUM            0x01000001
#define LOG_MANAGER_THREAD_SLEEP        100
#define LOG_MANAGER_THREAD_SLEEP_WAIT_LOGIN    1000

// Never rename/reuse. Always add new values instead
enum LogSeverityValue : int
{
    LogSeverityInvalidValue = 0,
    LogSeverityDebug = 1 << 0,
    LogSeverityKPI = 1 << 1,
    LogSeverityNormal = 1 << 2,
    LogSeverityUnexpected = 1 << 3,
    LogSeveritySever = 1 << 4, // bad, but can recover
    LogSeverityCritical = 1 << 5, // shutting down, can't recover
    LogSeverityAll = LogSeverityDebug | LogSeverityKPI | LogSeverityNormal | LogSeverityUnexpected | LogSeveritySever | LogSeverityCritical,
};

inline LogSeverityValue& operator|=(LogSeverityValue& lhs, LogSeverityValue rhs) {
    lhs = static_cast<LogSeverityValue>(static_cast<unsigned __int64>(lhs) | static_cast<unsigned __int64>(rhs));
    return lhs;
}

// Never rename/reuse. Always add new values instead
enum LogSourceGroups : int
{
    LogSourceInvalidValue = 0,
    LogSourceLogging = 1,
    LogSourceConfig = 2,
    LogSourceWindowManager = 3,
    LogSourceUserSession = 4,
    LogSourceUserGroup = 5,
    LogSourceLicensing = 6,
    LogSourceEncryption = 7,
    LogSourceUserManagement = 8,
    LogSourceHacking = 9,
    LogSourceDataAccessViolation = 10,
    LogSourceModule = 11,
    LogSourceAlert = 12,
    LogSourceReportGeneration = 13,
    LogSourceLib = 14,
    LogSourceMain = 15,
    LogSourceApplicationSession = 16,
    LogSourceCURL = 17,
    LogSourceWebAPI = 18,
    LogSourceLoginWindow = 19,
    LogSourceMainWindow = 20,
    LogSourceCrashHandler = 21,
    LogSourceKPIWorkerThread = 22,
    LogSourceUI = 23,
    LogSourceFontManager = 24,
    LogSourceLocalStrings = 25,
    LogSourceLocationsWindow = 26,
    LogSourceDataGrids = 27,
    LogSourceLocationEditWindow = 28,
    LogSourceAlertsWindow = 29,
    LogSourceKPI = 30,
    LogSourceLocationViewWindow = 31,
    LogSourceAlertsCacheManager = 32,
    LogSourceActivityLogWindow = 33,
    LogSourceModulesWindow = 34,
    LogSourceDropDown = 35,
    LogSourceModulesBuyWindow = 36,
    LogSourceModuleDataSourceManager = 37,
    LogSourceDatabaseManager = 38,
    LogSourceNetworkManager = 39,
    LogSourceWSServer = 40,
    LogSourcePacketParser = 41,
    LogSourceWSClient = 42,
    LogSourceDPSDataSource = 43,
    LogSourceDPSCommandParser = 44,
    LogSourceObjectCast = 45,
    LogSourceConsoleReader = 46,
    LogSourceUDPServer = 47,
    LogSourceAlertLifeCycle = 48,
    LogSourceImageManager = 49,
    LogSourceTransparentButton = 50,
    LogSourceAsyncTaskManager = 51,
    LogSourceDataGridFilter = 52,
};

// used when sending log messages to backend
enum LogSourceTypes : int
{
    LogSourceTypeInvalidValue = 0,
    LogSourceTypeVSSUI = 1,
    LogSourceTypeVSSBackend = 2,
    LogSourceTypeVSSModule = 3,
    LogSourceTypeVSSAI = 4,
    LogSourceTypeVSSMessageBroaker = 5,
};

enum LogDestinationFlags : int
{
    LDF_INVALID_VALUE = 0,
    LDF_CONSOLE = 1 << 0,
    LDF_FILE = 1 << 1,
    LDF_LOCAL = LDF_CONSOLE | LDF_FILE,
    LDF_SERVER = 1 << 2,
    LDF_SERVER_FILE = 1 << 3,
    LDF_ALL = LDF_LOCAL | LDF_SERVER,
};
inline LogDestinationFlags& operator|=(LogDestinationFlags& lhs, LogDestinationFlags rhs) {
    lhs = static_cast<LogDestinationFlags>(static_cast<unsigned __int64>(lhs) | static_cast<unsigned __int64>(rhs));
    return lhs;
}

struct LogFileEntry;
class WatchdogThreadData;

constexpr bool HasVarArgValues() {
    return false;
}

#pragma warning(push)
#pragma warning(disable: 4100)
template<typename... Args>
constexpr bool HasVarArgValues(Args&&... args) {
    return true;
}
#pragma warning(pop)

class LogManager
{
public:
    inline static LogManager& getInstance() {
#ifdef _DEBUG
        static LogManager* instance = new LogManager;
        return *instance;
#else
        static LogManager instance;
        return instance;
#endif
    }
    void Init();
#define AddLogEntryCheckPoint(destinationFlags, severity, source, filter1, filter2, msg) AddLogEntryV(destinationFlags, severity, source, filter1, filter2, "%s:%d:%s", __func__, __LINE__, msg);
#define AddLogEntry(destinationFlags,severity,source,filter1,filter2,msgFormat,...) { if(sLog.ShouldLog(destinationFlags,severity)) {if (HasVarArgValues(__VA_ARGS__) == false) {\
        sLog.AddLogEntryS(destinationFlags,severity,source,filter1,filter2,msgFormat); } else { \
        sLog.AddLogEntryV(destinationFlags,severity,source,filter1,filter2,msgFormat,__VA_ARGS__);}}}
    void AddLogEntryS(const LogDestinationFlags destinationFlags, const LogSeverityValue severity, const LogSourceGroups source, const __int64 filter1, const double filter2, const char *msg);
    void AddLogEntryV(const LogDestinationFlags destinationFlags, const LogSeverityValue severity, const LogSourceGroups source, const __int64 filter1, const double filter2, const char* msgFormat, ...);
    void DestructorCheckMemLeaks();
    inline bool ShouldLog(const LogDestinationFlags destinationFlags, const LogSeverityValue severity);
private:
    char *FormatLogMessage(const LogSeverityValue severity, const LogSourceGroups source, const __int64 filter1, const double filter2, const char* msg, char* out_msg, const size_t out_msg_size);
    LogManager();
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;
    void PushLogToServerMessageQueue(const char* msg, bool bFileOnly);

    bool m_bIsInitialized;
    bool m_bServerLogBanned;
    FILE* m_fOutFile;   // dump filtered strings to file
    LogSeverityValue m_SeverityFilterFile;      // based on config file, only dump these logs to file
    LogSeverityValue m_SeverityFilterServer;    // based on config file, only send these to the VSS server
    LogDestinationFlags m_DestinationFilter;    // based on config file, only store messages destined here
   
    friend void LogManager_AsyncExecuteThread(WatchdogThreadData* wtd);
    std::mutex m_LogConsoleLock;
    std::mutex m_LogListLock;
    std::queue<LogFileEntry*> m_ServerLogsQueue;
    FILE* m_fServerQueueFile;
};

#define sLog LogManager::getInstance()