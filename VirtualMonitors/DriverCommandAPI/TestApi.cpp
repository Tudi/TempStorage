#include <Windows.h>
#include <conio.h>
#include <assert.h>

#include "TestApi.h"
#include "Logger.h"
#include "VirtualMonitorAPI.h"

JumpDisplayDriverControl sDriverControlSession;
std::vector<JumpDisplayDriverControl::MonitorInfo> sMonitors;
JumpDisplayDriverControl::MonitorInfo sMonitorBackup;

void TestingWaitForKeypress(char key)
{
    LOG_INFO(NULL, "Press '{}' to continue\n", key);
    bool bExit = false;
    do
    {
        // Wait for key press
        int key = _getch();

        if (key == tolower(key) || key == toupper(key))
        {
            bExit = true;
        }
    } while (!bExit);
}

bool TestingCreateDeviceTwoMonitors()
{
    LOG_TRACE(NULL, "Testing : Create device with 2 monitors attached to it :\n");

    // Could set to application path or some specific log directory
    char curDir[MAX_PATH];
    DWORD dirRes = GetCurrentDirectoryA(sizeof(curDir), curDir);
    std::string driverLogFilePath = std::string(curDir) + std::string("\\logD.txt");
    sDriverControlSession.DriverLogFilePathSet(driverLogFilePath);
    LOG_INFO(NULL, "Set driver log file to : {}\n", driverLogFilePath);

    // List of monitors with specific resolutions that we need to get created
    sMonitors.reserve(MaxMonitorCount);
    sMonitors.resize(2);

    // Settings for monitor 1
    sMonitors[0].UID = JumpDisplayDriverControl::MonitorInfo::UIDGenerate();
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[0], 800, 600, 60);
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[0], 1024, 800, 60);

    // Settings for monitor 2
    sMonitors[1].UID = JumpDisplayDriverControl::MonitorInfo::UIDGenerate();
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[1], 320, 200, 60);
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[1], 640, 480, 60);

    // Start creating the monitors
#ifdef _x86
    bool monitorCreateResult = sDriverControlSession.InitWithMonitors(sMonitors, true);
#else
    bool monitorCreateResult = sDriverControlSession.InitWithMonitors(sMonitors, false);
#endif
    LOG_TRACE(NULL, "Device created : {}\n", monitorCreateResult);

    return monitorCreateResult;
}

void TestingUnloadDevice()
{
    sDriverControlSession.Uninitialize();
}

void TestingAddMonitors()
{
    LOG_TRACE(NULL, "********************************************************\n");
    LOG_TRACE(NULL, "Testing : Add 1 monitor to existing device :\n");
    assert(sMonitors.size() == 2);

    sMonitors.resize(3);

    // Settings for monitor 1
    sMonitors[2].UID = JumpDisplayDriverControl::MonitorInfo::UIDGenerate();
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[2], 900, 900, 60);
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[2], 901, 901, 60);

    // Start creating the monitors
    bool monitorCreateResult = sDriverControlSession.DriverUpdateSettings(sMonitors);

}

void TestingRemoveAMonitor()
{
    LOG_TRACE(NULL, "********************************************************\n");
    LOG_TRACE(NULL, "Testing : Remove 1 monitor(previously added) from existing device :\n");
    assert(sMonitors.size() == 3);

    // Remove one monitor
    sMonitorBackup = sMonitors[2];
    sMonitors.resize(sMonitors.size() - 1);

    bool monitorCreateResult = sDriverControlSession.DriverUpdateSettings(sMonitors);
}

void TestingReAddSameMonitor()
{
    LOG_TRACE(NULL, "********************************************************\n");
    LOG_TRACE(NULL, "Testing : Add 1 monitor(we removed previously) to existing device :\n");
    assert(sMonitors.size() == 2);

    // Add back the same monitor
    sMonitors.resize(sMonitors.size() + 1);
    sMonitors[2] = sMonitorBackup;
//    LOG_TRACE(NULL, "Restored monitor UID {} :\n", sMonitors[2].UID);

    bool monitorCreateResult = sDriverControlSession.DriverUpdateSettings(sMonitors);
}

void TestChangeMonitorResolution()
{
    LOG_TRACE(NULL, "********************************************************\n");
    LOG_TRACE(NULL, "Testing : Change resolution of monitor 1 :\n");
    assert(sMonitors.size() > 0);

    // Change the resolution for monitor 1
    sMonitors[0].resolutions[0].width = sMonitors[0].resolutions[0].width + 100;
    sMonitors[0].resolutions[0].height = sMonitors[0].resolutions[0].height + 100;

    sMonitors[0].resolutions[1].width = sMonitors[0].resolutions[1].width + 100;
    sMonitors[0].resolutions[1].height = sMonitors[0].resolutions[1].height + 100;

    bool monitorCreateResult = sDriverControlSession.DriverUpdateSettings(sMonitors);
}

void TestingRemoveAddNewMonitor()
{
    LOG_TRACE(NULL, "********************************************************\n");
    LOG_TRACE(NULL, "Testing : Add and remove a monitor(nr 1, nr 4) in 1 cycle:\n");
    assert(sMonitors.size() == 3);

    // A bad UI should be enough to remove a monitor
    sMonitors[0].UID = 0;

    // Add a new monitor
    sMonitors.resize(sMonitors.size() + 1);
    sMonitors[3].UID = JumpDisplayDriverControl::MonitorInfo::UIDGenerate();
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[3], 900, 900, 60);
    JumpDisplayDriverControl::MonitorInfo::ResolutionAdd(sMonitors[3], 901, 901, 60);

    bool monitorCreateResult = sDriverControlSession.DriverUpdateSettings(sMonitors);

}

void TestingReplaceMonitor()
{
    LOG_TRACE(NULL, "********************************************************\n");
    LOG_TRACE(NULL, "Testing : Remove and add a monitor from same connector in 1 cycle. Also no hardware cursor enabling for the new monitor:\n");
    assert(sMonitors.size() == 4);

    // Add a new monitor
    sMonitors[1].UID = JumpDisplayDriverControl::MonitorInfo::UIDGenerate();
    sMonitors[1].enableHardwareCursor = 0;

    bool monitorCreateResult = sDriverControlSession.DriverUpdateSettings(sMonitors);
}