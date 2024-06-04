#include <thread>
#include <string>
#include <iostream>
#include <mutex>

#include "ConsoleCommandManager.h"
#include "Session/ApplicationSession.h"
#include "Util/Allocator.h"
#include "LogManager.h"
#include "DataSourceManager.h"
#include "DB/MysqlManager.h"

VSSConsoleCommandsManager::VSSConsoleCommandsManager(){}
VSSConsoleCommandsManager::~VSSConsoleCommandsManager() 
{
	DestructorCheckMemLeaks();
}

void VSSConsoleCommandsManager::DestructorCheckMemLeaks() {}

static void SimulateDummyAlert()
{
    DSModuleAlert ma;
    ma.ModuleID = 1;
    ma.AlertType = VSS_N_ModuleAlertState::AlertTypes::MultiPersonDetected;
    ma.TriggerStamp = time(NULL);
    ma.OrganizationId = 1;
    ma.UserId = 9;

    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceConsoleReader, 0, 0,
        "CCParser:Simulating a dummy alert from ModuleId %d, Type %d", ma.ModuleID, ma.AlertType);

    sDSManager.OnModuleAlertArrived(ma.ModuleID, &ma);
}

void ConsoleCommandParser_AsyncExecuteThread(WatchdogThreadData* wtd)
{
    std::string input;
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceConsoleReader, 0, 0,
        "CCParser:Reading console input until exited with command 'x'");

    while (sAppSession.IsApplicationRunning() && wtd->ShouldShutDown() == false)
    {
        std::getline(std::cin, input);
        if (input.length() == 1 && input.c_str()[0] == 'x')
        {
            sAppSession.SetApplicationRunning(false);
            break;
        }
        else if (input == "alert")
        {
            SimulateDummyAlert();
        }
        else if (!input.empty())
        {
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceConsoleReader, 0, 0,
                "CCParser:Unhandled command '%s'", input.c_str());
        }
    }

    // Let watchdog know we exited
    wtd->MarkDead();
}

void VSSConsoleCommandsManager::Init() 
{
    sAppSession.CreateWorkerThread(ConsoleCommandParser_AsyncExecuteThread, "ConsoleCommandParser", ConsoleCommandThreadSleep);
}
