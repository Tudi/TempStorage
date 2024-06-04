#include <winsock2.h>
#include "Util/InitFileHandler.h"
#include "Network/WSServer.h"
#include "Network/NetworkSessionManager.h"
#include "ApplicationSession.h"
#include "LogManager.h"
#include "Allocator.h"
#include "ConfigManager.h"
#include "ResourceManager/DataSourceManager.h"
#include "ResourceManager/ConsoleCommandManager.h"
#include "ResourceManager/AlertLifeCycleManager.h"
#include "ResourceManager/ServerHealthReportingManager.h"
#include "DB/TableCacheManager.h"
#include "DB/MysqlManager.h"
#include "DummyRadar.h"
#include "Network/UDPServer.h"
#include "Util/VariousFuncs.h"
#include "curl/include/curl/curl.h"
#include "Web/CurlInterface.h"

// Main code
int main(int, char**)   // allow debug console to be shown to be able to spam debug messages
{
    // Tell windows to dump memory leaks on application exit
#if defined(WINDOWS) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CRT_BLOCK;

    // split the screen between UI and DPS
    AutoSizeConsoleWindow(0, 2);
#endif

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeverityNormal,
            LogSourceGroups::LogSourceMain, 0, 0, "Failed to WSAStartup");
    }

    sConfigManager.LoadStrings("config.ini");
    sLog.Init();

    sDBManager.StartServer(sConfigManager.GetInt(ConfigOptionIds::DB_ConnectionsPooled),
        sConfigManager.GetString(ConfigOptionIds::DB_URL),
        sConfigManager.GetString(ConfigOptionIds::DB_Port),
        sConfigManager.GetString(ConfigOptionIds::DB_User),
        sConfigManager.GetString(ConfigOptionIds::DB_Passw),
        sConfigManager.GetString(ConfigOptionIds::DB_Database));

    // Init CURL to be able to make API calls to web server
    CURL_Init();

    // Fetch a list of modules we should be listenting to
    sDSManager.Init(sConfigManager.GetInt(ConfigOptionIds::DPS_Id));

    // cache very very often accesed DB tables. Periodically refresh content
    sTableCacheManager.Init();

    // process incomming packets
    sNetworkSessionManager.Init(sConfigManager.GetInt(ConfigOptionIds::DPS_MaxNetworkThreads));

    // Web socket server
    sWSServer.StartServer((unsigned short)sConfigManager.GetInt(ConfigOptionIds::DPS_WSListenPort));

    // server reporting. Let the backend / load balancer know we are alive
    sServerHealthReportingManager.Init(sConfigManager.GetInt(ConfigOptionIds::DPS_Id));

    sAlertManager.Init(sConfigManager.GetInt(ConfigOptionIds::DPS_Id), 
        sConfigManager.GetInt(ConfigOptionIds::DPS_MaxAlertProcessingThreads));

    // listen to Radar modules. In fact any other sensor
    sUDPServerManager.Init(sConfigManager.GetInt(ConfigOptionIds::DPS_UDPMaxListenThreads),
        (unsigned short)sConfigManager.GetInt(ConfigOptionIds::DPS_UDPListenPort));

    // probably should do something better
    DummyRadar dr(1, 200, 3);

    // listen to console commands
    sCCManager.Init();

    while(sAppSession.IsApplicationRunning())
    {
        Sleep(1000);
    }

    // can start shutting down
    AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeverityNormal,
        LogSourceGroups::LogSourceMain, 0, 0, "Application shutting down");

    // first close data sources
    sUDPServerManager.ShutDownServer();
    sWSServer.StopServer();
    dr.DisconnectRadar();
    CURL_Shutdown();
    // Main thread terminated. Wait for module threads exit
    sNetworkSessionManager.WakeUpAllWorkerThreads(); // allow all worker threads to shut down properly

    sAppSession.WaitAllModuleThreadShutdown();

    sServerHealthReportingManager.DestructorCheckMemLeaks();
    sAlertManager.DestructorCheckMemLeaks();
    sDSManager.DestructorCheckMemLeaks();
    sNetworkSessionManager.DestructorCheckMemLeaks();
    sDBManager.DestructorCheckMemLeaks();
    sConfigManager.DestructorCheckMemLeaks();
    sAppSession.DestructorCheckMemLeaks();
    sTableCacheManager.DestructorCheckMemLeaks();
    sLog.DestructorCheckMemLeaks();
    WSACleanup();
#if defined(WINDOWS) && defined(_DEBUG)
    ConsoleDumpAllocatedRegions();
    AllocatorFreeInternalMem();
    // dump to Visual studio console memory leaks
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
