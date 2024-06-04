#include "StdAfx.h"
#include "ResourceManager/AsyncTaskManager.h"

void AsyncTask_Init(void* params)
{
    params;
#if defined(WINDOWS) && defined(_DEBUG)
    // split the screen between UI and DPS
    AutoSizeConsoleWindow(1, 2);
#endif

    // Init exception handler. On crash, dump the stack to a humanly readable file
    ExceptionHandlerInit();
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceMain, 0, 0,
        "Exception handler initialized");

    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceMain, 0, 0,
        "Application is starting. App Id %lu", GetCurrentProcessId());

    // Load configs
    // load application settings
    sConfigManager.LoadStrings("config.ini");

    // Load UI component colors and other styles
    sStyles.LoadStyles("data/styles.ini");

    // Load static strings shown on UI. Chance to support localization
    sLocalization.LoadStrings("data/localizations.ini");

    // Start Logging module
    sLog.Init();

    // Init CURL
    CURL_Init();

    // start monitoring application and user performance metrics
    sKPI.Init();

    // start worker thread that will query for alerts as soon as possible
    sAlertsCache.Init();

    // Manage connections to Modules for logged in user
    sDataSourceManager.Init();

    // Check if WebAPI has load balancing functionality
    // This check will be performed async to not delay UI startup
    WebApi_PerformAPIRedirect();

    AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
        LogSourceGroups::LogSourceMain, 0, 0, "Application started");

    // let all other threads know we loaded statis configs, images ...
    sAppSession.SetStartupPhaseStatus(true);
}

// Main code
#ifndef _DEBUG
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)   // hides consoles
{
    hInstance; hPrevInstance; lpCmdLine; nCmdShow;
#else
int main(int, char**)   // allow debug console to be shown to be able to spam debug messages
{
#endif
    // Tell windows to dump memory leaks on application exit
#if defined(WINDOWS) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CRT_BLOCK;
#endif

    // you can queue async tasks from this point on
    sAsyncTaskManager.Init(sConfigManager.GetInt(ConfigOptionIds::AsyncTasksThreadPoolSize, 5));

    AddAsyncTask(AsyncTask_Init, NULL);

    // test if log messages get stored / resumed when sent to the server
//    AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceMain, 0, 0, "a");
//    AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceMain, 0, 0, "b");
//    AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceMain, 0, 0, "Testing server persistent messages");

    // start up the Vulkan / GL UI rendering pipeline. Also startup ImGUI graphics loop
    ImGUI_Backend_Init();
    // At this point the main thread will block. Make sure you started worker threads before this line
    ImGUI_Backend_RunInfiniteFrameLoop();
    ImGUI_Backend_Unload();

    sWindowManager.OnUserLoggeOut();

    AddLogEntry(LogDestinationFlags::LDF_SERVER, LogSeverityValue::LogSeverityNormal,
        LogSourceGroups::LogSourceMain, 0, 0, "Application closed");

    // Main thread terminated. Wait for module threads exit
    sAppSession.WaitAllModuleThreadShutdown();

    // free resources allocated
    CURL_Shutdown();

    // manually destroy dynamically allocated data so we can check for memory leaks
    // Mem leak reporting will not be 100% accurate. But you can compare relative one build to another
    // Deleting singletons will lead to crash unless you do it as a last step
    sDataSourceManager.DestructorCheckMemLeaks();
    sConfigManager.DestructorCheckMemLeaks();
    sLocalization.DestructorCheckMemLeaks();
    sWindowManager.DestructorCheckMemLeaks();
    sUserSession.DestructorCheckMemLeaks();
    sKPI.DestructorCheckMemLeaks();
    sAsyncTaskManager.DestructorCheckMemLeaks();
//    sImageManager.DestructorCheckMemLeaks(); // called within ImGui shutdown
    sFontManager.DestructorCheckMemLeaks();
    sStyles.DestructorCheckMemLeaks();
    sAppSession.DestructorCheckMemLeaks();
    sAlertsCache.DestructorCheckMemLeaks();
    sLog.DestructorCheckMemLeaks();
#if defined(WINDOWS) && defined(_DEBUG)
    ConsoleDumpAllocatedRegions();
    AllocatorFreeInternalMem();
    // dump to Visual studio console memory leaks
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
