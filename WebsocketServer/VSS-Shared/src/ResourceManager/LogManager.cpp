#include <time.h>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <filesystem>
#include "Util/Allocator.h"
#include "ResourceManager/LogManager.h"
#include "Session/ApplicationSession.h"
#include "Util/InitFileHandler.h"
#include "ResourceManager/ConfigManager.h"
#include "Util/VariousFuncs.h"
#include <assert.h>

#ifdef BUILD_CLIENT_PACKETS
    #include "Web/CurlInterface.h"
    #include "Web/WebApiInterface.h"
#endif

#pragma pack(push, 1)
typedef struct LogFileFileHeader
{
    unsigned int version;
}LogFileFileHeader;
typedef struct LogFileEntryHeader
{
    enum LogFileEntryFlags : unsigned char
    {
        LFEF_NO_VALUES = 0,
        LFEF_PENDING_SEND_TO_SERVER = 1 << 0,
    };
    static bool IsHeaderValid(LogFileEntryHeader& h)
    {
        // contains unrecognized flags
        if ((h.cFlags & (~(LFEF_PENDING_SEND_TO_SERVER))) != 0)
        {
            return false;
        }
        // strange format ?
        if (h.uMsgSize > MAX_LOG_MESSAGE_SIZE)
        {
            return false;
        }
        // message comes from the future
        if (h.lldTimeStamp == 0 || h.lldTimeStamp > (unsigned __int64)time(NULL))
        {
            return false;
        }

        return true;
    }
    unsigned char cFlags;
    unsigned int uMsgSize;
    unsigned __int64 lldTimeStamp;
}LogFileEntryHeader;
typedef struct LogFileEntry
{
    LogFileEntryHeader header;
    char data[1]; //fake placeholder
}LogFileEntry;
#pragma pack(pop)

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

// forward declaration
void LogManager_AsyncExecuteThread(WatchdogThreadData* wtd);
HANDLE LogManager_WakeupEvent = NULL;

static std::string GetCurrentDate()
{
    std::time_t now = std::time(nullptr);
    std::tm timeInfo;
    if (localtime_s(&timeInfo, &now) == 0) {
        std::ostringstream oss;
        oss << std::put_time(&timeInfo, "%Y-%m-%d");
        return oss.str();
    }
    else
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceLogging, 0, 0, 
            "Failed to get local time");
    }
    return "";
}

static bool IsDateInFileName(const std::string& filename, std::tm& date) {
    std::istringstream iss(filename);
    char delimiter;
    int year, month, day;
    if (iss >> year >> delimiter >> month >> delimiter >> day) {
        date.tm_year = year - 1900;
        date.tm_mon = month - 1;
        date.tm_mday = day;
        return true;
    }
    return false;
}

static void DeleteOldFiles(const std::string& directoryPath, int daysThreshold) 
{
    std::filesystem::path dirPath(directoryPath);
    std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (std::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".log.txt") {
                std::string filename = entry.path().filename().string();
                std::tm fileDate = {};
                if (IsDateInFileName(filename, fileDate)) {
                    std::time_t fileTime = std::mktime(&fileDate);
                    if (fileTime != -1) {
                        // Calculate the age of the file in days
                        int daysOld = static_cast<int>((currentTime - fileTime) / (60 * 60 * 24));
                        if (daysOld > daysThreshold) {
                            std::filesystem::remove(entry.path());
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& e) 
    {  
        e;
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceLogging, 0, 0,
            "Failed to delete old logs");
    }
}

static FILE *CreateNewLogFile()
{
    std::string current_date = GetCurrentDate();
    char szFileName[MAX_PATH];
    sprintf_s(szFileName, "./Logs/log_%s.log.txt", current_date.c_str());

    FILE* retf;
    errno_t openErr = fopen_s(&retf, szFileName, "at");

    if (retf != NULL && openErr == NO_ERROR)
    {
        return retf;
    }

    return NULL;
}

LogManager::LogManager()
{
    m_bIsInitialized = false;
    m_bServerLogBanned = false;

    m_SeverityFilterFile = LogSeverityValue::LogSeverityAll;
    m_SeverityFilterServer = LogSeverityValue::LogSeverityAll;
    m_DestinationFilter = LogDestinationFlags::LDF_ALL;

    m_fOutFile = NULL;
    m_fServerQueueFile = NULL;
}

void LogManager::DestructorCheckMemLeaks()
{
    if (m_fOutFile != NULL)
    {
        fclose(m_fOutFile);
        m_fOutFile = NULL;
    }
    if (m_fServerQueueFile != NULL)
    {
        fclose(m_fServerQueueFile);
        m_fServerQueueFile = NULL;
    }
    while (!m_ServerLogsQueue.empty())
    {
        LogFileEntry* job = std::move(m_ServerLogsQueue.front());
        m_ServerLogsQueue.pop();
        InternalFree(job);
    }

#ifdef _DEBUG
    delete& sLog;
#endif

}

void LogManager::Init()
{
    if (m_bIsInitialized == true)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceLogging, 0, 0,
            "Trying to initialize log manager more than once. Unexpected behavior");
        return;
    }
    m_bIsInitialized = true;

    m_SeverityFilterFile = (LogSeverityValue)0;
    m_SeverityFilterServer = (LogSeverityValue)0;
    m_DestinationFilter = (LogDestinationFlags)0;

    std::string szFileFilterStrings = sConfigManager.GetString(ConfigOptionIds::LogFileMessages);
    if (szFileFilterStrings.find("Debug") != std::string::npos)
        m_SeverityFilterFile |= LogSeverityValue::LogSeverityDebug;
    if (szFileFilterStrings.find("KPI") != std::string::npos)
        m_SeverityFilterFile |= LogSeverityValue::LogSeverityKPI;
    if (szFileFilterStrings.find("Normal") != std::string::npos)
        m_SeverityFilterFile |= LogSeverityValue::LogSeverityNormal;
    if (szFileFilterStrings.find("Unexpected") != std::string::npos)
        m_SeverityFilterFile |= LogSeverityValue::LogSeverityUnexpected;
    if (szFileFilterStrings.find("Sever") != std::string::npos)
        m_SeverityFilterFile |= LogSeverityValue::LogSeveritySever;
    if (szFileFilterStrings.find("Critical") != std::string::npos)
        m_SeverityFilterFile |= LogSeverityValue::LogSeverityCritical;

    szFileFilterStrings = sConfigManager.GetString(ConfigOptionIds::LogServerMessages);
    if (szFileFilterStrings.find("Debug") != std::string::npos)
        m_SeverityFilterServer |= LogSeverityValue::LogSeverityDebug;
    if (szFileFilterStrings.find("KPI") != std::string::npos)
        m_SeverityFilterServer |= LogSeverityValue::LogSeverityKPI;
    if (szFileFilterStrings.find("Normal") != std::string::npos)
        m_SeverityFilterServer |= LogSeverityValue::LogSeverityNormal;
    if (szFileFilterStrings.find("Unexpected") != std::string::npos)
        m_SeverityFilterServer |= LogSeverityValue::LogSeverityUnexpected;
    if (szFileFilterStrings.find("Sever") != std::string::npos)
        m_SeverityFilterServer |= LogSeverityValue::LogSeveritySever;
    if (szFileFilterStrings.find("Critical") != std::string::npos)
        m_SeverityFilterServer |= LogSeverityValue::LogSeverityCritical;

    szFileFilterStrings = sConfigManager.GetString(ConfigOptionIds::LogDestinationEnable);
    if (szFileFilterStrings.find("Console") != std::string::npos)
        m_DestinationFilter |= LogDestinationFlags::LDF_CONSOLE;
    if (szFileFilterStrings.find("File") != std::string::npos)
        m_DestinationFilter |= LogDestinationFlags::LDF_FILE;
    if (szFileFilterStrings.find("Server") != std::string::npos)
        m_DestinationFilter |= LogDestinationFlags::LDF_SERVER;
    if (szFileFilterStrings.find("ServerQueued") != std::string::npos)
        m_DestinationFilter |= LogDestinationFlags::LDF_SERVER_FILE;

    // Clean up old log files
    DeleteOldFiles("./Logs", sConfigManager.GetInt(ConfigOptionIds::DeleteOlderLogFiles));

    // Create or Apped to "today" log file
    m_fOutFile = CreateNewLogFile();

    // Create a temp persistent DB on HDD
    errno_t ferr = fopen_s(&m_fServerQueueFile, LOG_PERSISTENT_DB_NAME, "ab+");
    if (ferr != NO_ERROR || m_fServerQueueFile == NULL)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceLogging, 0, 0,
            "Failed to create pesistent Log queue");
    }
    // if this file got created just now, add the file header to it
    if(m_fServerQueueFile != NULL && ftell(m_fServerQueueFile) == 0)
    {
        LogFileFileHeader lfh2;
        lfh2.version = LOG_FILE_VERSION_NUM;
        fwrite(&lfh2, 1, sizeof(LogFileFileHeader), sLog.m_fServerQueueFile);
    }

    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceLogging, 0, 0,
        "Using Log format : %s", LOG_FORMAT_VERSION_INFO);

#ifdef BUILD_CLIENT_PACKETS
    // create watchdog data
    WatchdogThreadData* wtd = sAppSession.CreateWorkerThread(LogManager_AsyncExecuteThread, "LogManager", LOG_MANAGER_THREAD_SLEEP);
    LogManager_WakeupEvent = wtd->GetEventHandle();
#endif
}

char* LogManager::FormatLogMessage(const LogSeverityValue severity, const LogSourceGroups source, 
    const __int64 filter1, const double filter2, const char* msg, char* out_msg, const size_t out_msg_size)
{
    auto currentTime = std::chrono::system_clock::now();
    unsigned __int64 seconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();
    unsigned __int64 nanoseconds = ( GetNanoStamp() / 100 ) % 1000000;

    DWORD threadId = GetCurrentThreadId();
    
    char* ret;
    size_t size;
    if (out_msg == NULL)
    {
        ret = (char*)InternalMalloc(MAX_LOG_MESSAGE_SIZE);
        size = MAX_LOG_MESSAGE_SIZE;
        if (ret == NULL)
        {
            return NULL;
        }
    }
    else
    {
        ret = out_msg;
        size = out_msg_size;
    }

    if (filter2 == (int)filter2)
    {
        snprintf(ret, size, "%llu:%06llu:%lu:%d:%d:%lld:%d:%s",
            seconds, nanoseconds,
            threadId, (int)severity, (int)source, filter1, (int)filter2, msg);
    }
    else
    {
        snprintf(ret, size, "%llu:%llu:%lu:%d:%d:%lld:%.2e:%s",
            seconds, nanoseconds,
            threadId, (int)severity, (int)source, filter1, filter2, msg);
    }
    ret[size - 1] = 0; // make sure always 0 terminated

    return ret;
}

inline bool LogManager::ShouldLog(const LogDestinationFlags destinationFlags, const LogSeverityValue severity)
{
    if ((severity & (m_SeverityFilterFile | m_SeverityFilterServer)) == 0 ||
        (destinationFlags & m_DestinationFilter) == 0)
    {
        return false;
    }
    return true;
}

thread_local bool isLogging = false;
void LogManager::AddLogEntryS(const LogDestinationFlags destinationFlags, const LogSeverityValue severity, 
    const LogSourceGroups source, const __int64 filter1, const double filter2, const char* msg)
{
    if ((severity & (m_SeverityFilterFile | m_SeverityFilterServer)) == 0 ||
        (destinationFlags & m_DestinationFilter) == 0)
    {
        return;
    }

    char sFormattedMsg[MAX_LOG_MESSAGE_SIZE];
    FormatLogMessage(severity, source, filter1, filter2, msg, sFormattedMsg, sizeof(sFormattedMsg));

    if (destinationFlags & LogDestinationFlags::LDF_CONSOLE)
    {
        struct tm timeinfo;
        char sReadableTime[90];
        time_t nVal = time(NULL);
        localtime_s(&timeinfo, &nVal); // Convert to local time
        strftime(sReadableTime, sizeof(sReadableTime), "%m/%d/%Y %I:%M:%S %p:", &timeinfo);
        if (isLogging == true)
        {
            if (severity == LogSeverityValue::LogSeverityCritical) 
            {
                printf(RED "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else if (severity == LogSeverityValue::LogSeveritySever) 
            {
                printf(YELLOW "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else if (severity == LogSeverityValue::LogSeverityUnexpected) 
            {
                printf(BLUE "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else if (severity == LogSeverityValue::LogSeverityKPI) 
            {
                printf(GREEN "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else 
            {
                printf("%s%s\n", sReadableTime, sFormattedMsg);
            }
        }
        else
        {
            isLogging = true; // handle recursive calls
            // console is not thread safe. Avoid mixing messages from multiple threads
            std::unique_lock<std::mutex> lock(sLog.m_LogConsoleLock);

            if (severity == LogSeverityValue::LogSeverityCritical)
            {
                printf(RED "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else if (severity == LogSeverityValue::LogSeveritySever)
            {
                printf(YELLOW "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else if (severity == LogSeverityValue::LogSeverityUnexpected)
            {
                printf(BLUE "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else if (severity == LogSeverityValue::LogSeverityKPI)
            {
                printf(GREEN "%s%s" RESET "\n", sReadableTime, sFormattedMsg);
            }
            else
            {
                printf("%s%s\n", sReadableTime, sFormattedMsg);
            }

            lock.unlock();
            isLogging = false;
        }
    }
    if ((destinationFlags & LogDestinationFlags::LDF_FILE) && m_fOutFile != NULL)
    {
        fprintf(m_fOutFile, "%s\n", sFormattedMsg);
    }
    if (destinationFlags & LogDestinationFlags::LDF_SERVER)
    {
        PushLogToServerMessageQueue(sFormattedMsg, false);
    }
    // probably only used by crash handler
    if (destinationFlags & LogDestinationFlags::LDF_SERVER_FILE)
    {
        PushLogToServerMessageQueue(sFormattedMsg, true);
    }
}

void LogManager::AddLogEntryV(const LogDestinationFlags destinationFlags, const LogSeverityValue severity, 
    const LogSourceGroups source, const __int64 filter1, const double filter2, const char* msgFormat, ...)
{
    char buffer[MAX_LOG_MESSAGE_SIZE];

    if ((severity & (m_SeverityFilterFile | m_SeverityFilterServer)) == 0 ||
        (destinationFlags & m_DestinationFilter) == 0)
    {
        return;
    }

#ifdef _DEBUG
    bool FoundVarArg = false;
    for (size_t i = 0; msgFormat[i] != 0; i++)
    {
        if (msgFormat[i] == '%')
        {
            FoundVarArg = true;
            break;
        }
    }
    if (FoundVarArg == false)
    {
        // use a simple AddLogEntry instead AddLogEntryV
        assert(false);
    }
#endif

    va_list args;
    va_start(args, msgFormat);

    int length = std::vsnprintf(buffer, sizeof(buffer), msgFormat, args);

    va_end(args);

    // !! message was too large to fit into max size. Truncate it
    if (!(length >= 0 && length < sizeof(buffer)))
    {
        buffer[sizeof(buffer) - 1] = 0;
    }

    AddLogEntry(destinationFlags, severity, source, filter1, filter2, buffer);
}

//////////////////////////////////////////////////////////////
// Code for sending messages to the server
//////////////////////////////////////////////////////////////

void LogManager::PushLogToServerMessageQueue(const char* msg, bool bFileOnly)
{
    size_t msgSize = strlen(msg) + 1;
    LogFileEntry* lfe = (LogFileEntry *)InternalMalloc(sizeof(LogFileEntry) + msgSize);
    if (lfe == NULL)
    {
        return; // when a malloc fails, we won't be able to even make a log of it
    }

    lfe->header.cFlags = LogFileEntryHeader::LogFileEntryFlags::LFEF_PENDING_SEND_TO_SERVER;
    lfe->header.uMsgSize = (unsigned int)msgSize;
    lfe->header.lldTimeStamp = time(NULL);
    memcpy(lfe->data, msg, msgSize);

    // in case of crash, we are very limited in the type of functions we can call
    if ((bFileOnly == true && sLog.m_fServerQueueFile != NULL) || 
        sAppSession.IsUserLoggedIn() == false ||
        sAppSession.IsApplicationRunning() == false)
    {
        if (m_fServerQueueFile != NULL)
        {
            fwrite(lfe, 1, sizeof(LogFileEntryHeader) + lfe->header.uMsgSize, m_fServerQueueFile);
            fflush(m_fServerQueueFile);
        }
        InternalFree(lfe);
    }
    else
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceLogging, 0, 0,
            "Server Log Message queue len now : %d", sLog.m_ServerLogsQueue.size());

        std::unique_lock<std::mutex> lock(sLog.m_LogListLock);
        sLog.m_ServerLogsQueue.push(lfe);
		lock.unlock();

        SetEvent(LogManager_WakeupEvent);
    }
}

void SendCrashLogsToServer()
{
#ifdef BUILD_CLIENT_PACKETS
    std::filesystem::path dirPath("./Exceptions");

    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) 
    {
        if (std::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".log") 
        {
            std::string filename = entry.path().string();
            FILE* f;
            errno_t openErr = fopen_s(&f, filename.c_str(), "rt");
            if (f && openErr == 0)
            {
                char fcontent[15000];
                size_t readCount = fread_s(fcontent, sizeof(fcontent), 1, sizeof(fcontent), f);
                fclose(f);
                if (readCount > 0)
                {
                    fcontent[readCount] = 0; // make sure it's 0 terminated
                    WebApi_SaveCrashLogAsync(fcontent, NULL, NULL);
                }
                std::filesystem::remove(entry.path());
            }
        }
    }
#endif
}

void LogManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
    static bool OnlyOneInstace = false;
    if (OnlyOneInstace == true)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLogging, 0, 0,
            "LogManager:Async:Trying to start already running worker Log thread. Unexpected behavior");
        return;
    }
    OnlyOneInstace = true;

    // wait for the user to log in
    while (sAppSession.IsApplicationRunning() &&
        sAppSession.IsUserLoggedIn() == false &&
        wtd->ShouldShutDown() == false)
    {
        wtd->BlockThreadUntil(LOG_MANAGER_THREAD_SLEEP_WAIT_LOGIN);
    }
    // maybe we waited too much ? Check if app got killed since we started waiting
    if (sAppSession.IsApplicationRunning() == false ||
        wtd->ShouldShutDown() == true)
    {
        wtd->MarkDead();
        return;
    }

    // Send crashlogs before sending offline messages
    if (sAppSession.IsApplicationRunning() && sAppSession.IsUserLoggedIn() == true)
    {
        SendCrashLogsToServer();
    }

    // jump to the beginning of the DB file and try to send all unsent messages
#ifdef BUILD_CLIENT_PACKETS
    if(sAppSession.IsApplicationRunning() && sAppSession.IsUserLoggedIn() == true)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceLogging, 0, 0,
            "LogManager:Async:Start sending unsent messages to server");

        bool sentAllMessagesToServer = true;
        int messagesSkipped = 0;
        int messagesSent = 0;
        int messagesFailedToSend = 0;
        bool fileHasInvalidData = false;

        if (sLog.m_fServerQueueFile != NULL)
        {
            std::unique_lock<std::mutex> lock(sLog.m_LogListLock);

            fseek(sLog.m_fServerQueueFile, 0, SEEK_SET);
            LogFileFileHeader lfh;

            size_t bytesRead = fread(&lfh, 1, sizeof(LogFileFileHeader), sLog.m_fServerQueueFile);
            if (bytesRead == sizeof(LogFileFileHeader) && lfh.version == LOG_FILE_VERSION_NUM)
            {
                do {
//                    size_t startLoc = ftell(sLog.m_fServerQueueFile);
                    LogFileEntryHeader msgHeader;

                    // read the header of a persisted message
                    bytesRead = fread(&msgHeader, 1, sizeof(LogFileEntryHeader), sLog.m_fServerQueueFile);
                    if (bytesRead < sizeof(LogFileEntryHeader))
                    {
                        if (bytesRead != 0)
                        {
                            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLogging, 0, 0,
                                "LogManager:Async:Unexpected remaining of %lld bytes in log file", bytesRead);
                        }
                        break;
                    }

                    // check if message header is valid
                    if (LogFileEntryHeader::IsHeaderValid(msgHeader) == false)
                    {
                        fileHasInvalidData = true;
                        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLogging, 0, 0,
                            "LogManager:Async:Unexpected message header in log file");
                        break;
                    }

                    // make sure it fits into our new limitations
                    char msg[MAX_LOG_MESSAGE_SIZE];
                    size_t maxRead = MIN(msgHeader.uMsgSize, MAX_LOG_MESSAGE_SIZE);
                    // read the message from the file
                    bytesRead = fread_s(msg, sizeof(msg), 1, maxRead, sLog.m_fServerQueueFile);
                    // make sure it's null terminated
                    msg[MAX_LOG_MESSAGE_SIZE - 1] = 0;

                    // do we need to send this message to the server ?
                    if (msgHeader.cFlags & LogFileEntryHeader::LogFileEntryFlags::LFEF_PENDING_SEND_TO_SERVER)
                    {
                        // send it to the server. This is a blocking operation, but we are in a worker thread
                        WebApiErrorCodes resp = WebApi_SendLogMessage(msgHeader.lldTimeStamp, msg);
                        if (resp == WebApiErrorCodes::WAE_LogClientBanned)
                        {
                            sLog.m_bServerLogBanned = true;
                            sentAllMessagesToServer = true;
                            break;
                        }
                        if (resp == WebApiErrorCodes::WAE_NoError)
                        {
                            // clear the flag
//                            msgHeader.cFlags = msgHeader.cFlags & (~LogFileEntryHeader::LogFileEntryFlags::LFEF_PENDING_SEND_TO_SERVER);
//                            fseek(sLog.m_fServerQueueFile, (long)startLoc, SEEK_SET);
//                            fwrite(&msgHeader, 1, sizeof(LogFileEntryHeader), sLog.m_fServerQueueFile);
                            messagesSent++;
                        }
                        else
                        {
                            sentAllMessagesToServer = false;
                            messagesFailedToSend++;
                        }
                        // maybe only read part of the message, maybe we jumped backwards for writing ..
//                        fseek(sLog.m_fServerQueueFile, (long)(startLoc + sizeof(LogFileEntryHeader) + msgHeader.uMsgSize), SEEK_SET);
                    }
                    // we do not need to send this message, jump to the next one
                    else
                    {
                        messagesSkipped++;
//                        fseek(sLog.m_fServerQueueFile, (long)(startLoc + sizeof(LogFileEntryHeader) + msgHeader.uMsgSize), SEEK_SET);
                    }
                } while (1);
            }
			lock.unlock();
        }

        // if there are no pending messages, reset file to 0 size
        if (sentAllMessagesToServer == true || fileHasInvalidData == true)
        {
            if (sLog.m_fServerQueueFile != NULL)
            {
                fclose(sLog.m_fServerQueueFile);
            }
            fopen_s(&sLog.m_fServerQueueFile, LOG_PERSISTENT_DB_NAME, "wb");
            if (sLog.m_fServerQueueFile != NULL)
            {
                LogFileFileHeader lfh2;
                lfh2.version = LOG_FILE_VERSION_NUM;
                fwrite(&lfh2, 1, sizeof(LogFileFileHeader), sLog.m_fServerQueueFile);
            }
        }

        if (messagesSkipped != 0 || messagesFailedToSend != 0 || messagesSent != 0)
        {
            if(messagesFailedToSend > 0)
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceLogging, 0, 0,
                    "LogManager:Async:Failed to send logs to server");
            }
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceLogging, 0, 0,
                "LogManager:Async:Log message send result : Skipped %d, Sent %d, Failed %d", messagesSkipped, messagesSent, messagesFailedToSend);
        }
    }

    while (sAppSession.IsApplicationRunning() && sLog.m_bServerLogBanned == false && wtd->ShouldShutDown() == false)
    {
        // let watchdog know this thread is functioning as expected
        wtd->SignalHeartbeat();

        std::unique_lock<std::mutex> lock(sLog.m_LogListLock);
        // Check if the queue is empty
        if (sLog.m_ServerLogsQueue.empty()) {
            // Release the mutex and wait for a job to be added
            lock.unlock();
            wtd->BlockThreadUntil(LOG_MANAGER_THREAD_SLEEP);
        }
        else {
            // Pop a job from the queue
            LogFileEntry* job = std::move(sLog.m_ServerLogsQueue.front());
            sLog.m_ServerLogsQueue.pop();

            // Release the mutex so other threads may push new work
            lock.unlock();

            // try to send it to the server
            WebApiErrorCodes resp = WebApi_SendLogMessage(job->header.lldTimeStamp, job->data);
            if (resp == WebApiErrorCodes::WAE_LogClientBanned)
            {
                sLog.m_bServerLogBanned = true;
            }
            else if (resp != WebApiErrorCodes::WAE_NoError && sLog.m_fServerQueueFile)
            {
                // store the message to persistent DB until we can actually send the message
                fwrite(&job, 1, sizeof(LogFileEntryHeader) + job->header.uMsgSize, sLog.m_fServerQueueFile);
                fflush(sLog.m_fServerQueueFile);
            }

            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceCURL, 0, 0,
                "LogManager:Async:Log manager sent log %s to server. Result : %d", job->data, resp);

            // Free up queue resources
            InternalFree(job);
        }
    }
#endif

    // Let watchdog know we exited
    wtd->MarkDead();
}