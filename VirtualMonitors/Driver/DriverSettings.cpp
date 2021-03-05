#include <Windows.h>
#include <wdf.h>
#include <iddcx.h>
#include <process.h>
#include "DriverSettings.h"
#include "Logger.h"

DriverSettingManager::DriverSettingManager()
{
    InitializeCriticalSection(&mSettinsAppliedLock);
    InitializeCriticalSection(&mAdapterStatusLock);

    for (int i = 0; i < MaxMonitorCount; i++)
    {
        mCursorEvents[i] = CreateEvent(NULL, false, false, NULL);
    }
}

DriverSettingManager::~DriverSettingManager()
{
    DeleteCriticalSection(&mSettinsAppliedLock);
    DeleteCriticalSection(&mAdapterStatusLock);

    for (int i = 0; i < MaxMonitorCount; i++)
    {
        CloseHandle(mCursorEvents[i]);
        mCursorEvents[i] = NULL;
    }
}

#ifndef DisablePipeImplementation
bool DriverSettingManager::GetSettingsFromPipe()
{
	std::vector< JumpDisplayDriverControl::MonitorInfo> ret;

    HANDLE hPipe;

    // Create a communication channel to the program that created us
    hPipe = CreateFile(NamedPipeName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING, // Unless the application is waiting for us to connect to, this function will fail instantly
        0,
        NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR(NULL, "Could not connect to pipe to fetch settings {}\n", GetLastError());
        return false;
    }

    JumpDisplayDriverControl::PipeMessageHeaderSettings header;
    // Wait for the driver to send us the encrypt key
    DWORD dwRead;
    if (ReadFile(hPipe, &header, sizeof(header), &dwRead, NULL) == false || dwRead != sizeof(header))
    {
        LOG_ERROR(NULL, "Could not read data from pipe. Requested {}, got {} bytes\n", sizeof(header), dwRead);
        CloseHandle(hPipe);
        return false;
    }

    // There is no real restriction for monitor count, but we want to check for realistic values
    if (header.elementCount >= MaxMonitorCount)
    {
        LOG_ERROR(NULL, "Was expecting {} elements, received {}\n", MaxMonitorCount, header.elementCount);
        CloseHandle(hPipe);
        return false;
    }
    // Make sure driver was compiled with same header as the application
    if (header.elementSize != sizeof(JumpDisplayDriverControl::MonitorInfo))
    {
        LOG_ERROR(NULL, "Was expecting {} element size, received {}\n", sizeof(JumpDisplayDriverControl::MonitorInfo), header.elementSize);
        CloseHandle(hPipe);
        return false;
    }

    if (header.elementCount > 0)
    {
        mMonitors.reserve(header.elementCount);
        mMonitors.resize(header.elementCount);
        for (int i = 0; i < header.elementCount; i++)
        {
            if (ReadFile(hPipe, &mMonitors.at(i), sizeof(JumpDisplayDriverControl::MonitorInfo), &dwRead, NULL) == false || dwRead != sizeof(JumpDisplayDriverControl::MonitorInfo))
            {
                LOG_ERROR(NULL, "Could not read data from pipe. Index {}, received {}, expected {}\n", i, dwRead, sizeof(JumpDisplayDriverControl::MonitorInfo));
                CloseHandle(hPipe);
                return false;
            }
            // Check if received values have realistic ranges
            if(mMonitors[i].width <= 0 || mMonitors[i].width > SANITY_CHECK_MAX_RESOLUTION || mMonitors[i].height <= 0 || mMonitors[i].height > SANITY_CHECK_MAX_RESOLUTION)
            {
                LOG_ERROR(NULL, "Monitor {} setting is invalid {}x{}, {} Hz\n", i, mMonitors[i].width, mMonitors[i].height, mMonitors[i].verticalSync);
                CloseHandle(hPipe);
                return false;
            }
            if (mMonitors[i].verticalSync <= 0 || mMonitors[i].verticalSync > SANITY_CHECK_MAX_REFRESH_RATE)
            {
                LOG_ERROR(NULL, "Monitor {} setting is invalid {}x{}, {} Hz\n", i, mMonitors[i].width, mMonitors[i].height, mMonitors[i].verticalSync);
                CloseHandle(hPipe);
                return false;
            }
        }
    }
    else
    {
        // It should already be empty. Maybe someone makes a mistakes in the future. Better be safe than sorry
        mMonitors.clear();
    }

    // We got the settings. We no longer need the pipe
    CloseHandle(hPipe);

    LOG_TRACE(NULL, "Finished fetching settings for driver\n");
    return true;
}
#endif

void DriverSettingManager::MonitorIndexSet(const void* pMonitor, unsigned int connectorIndex)
{
    // Sanity check
    if (connectorIndex >= MaxMonitorCount)
    {
        LOG_ERROR(NULL, "Driver managet to create more monitors than possible\n");
        return;
    }

    AutoLock adapterSettingsLock(&mAdapterStatusLock);
    AutoLock settingsAppliedLock(&mSettinsAppliedLock);

    // Search for a monitor that has not yet been plugged in
    for (int i = 0; i < MaxMonitorCount; i++)
    {
        // Only inspect valid info
        if (JumpDisplayDriverControl::MonitorInfo::IsValid(mSettingsApplied.monitorInfos[i]) == false)
        {
            continue;
        }

        // Do we have this monitor plugged in ?
        bool monitorInUse = false;
        for (int j = 0; j < MaxMonitorCount; j++)
        {
            if (mAdapterStatus[j].monitorUID == mSettingsApplied.monitorInfos[i].UID)
            {
                monitorInUse = true;
                break;
            }
        }

        // if it's plugged in, search for a monitor that is free
        if (monitorInUse == true)
        {
            continue;
        }

        // Need to keep track where we can insert new monitors
        mAdapterStatus[connectorIndex].pMonitor = pMonitor;
        mAdapterStatus[connectorIndex].adapterPort = (unsigned short)connectorIndex;
        mAdapterStatus[connectorIndex].monitorUID = mSettingsApplied.monitorInfos[i].UID;
        mAdapterStatus[connectorIndex].settingsIndex = i;
        mAdapterStatus[connectorIndex].enableHWMouseStamp = GetTickCount64() + TIME_TOGGLE_HW_MOUSE_SUPPORT_MS;

        LOG_INFO(NULL, "Set monitor UID {}, monitor {}, adapter index {}\n", mSettingsApplied.monitorInfos[i].UID, pMonitor, connectorIndex);
        return;
    }

    LOG_ERROR(NULL, "Adapter asked for a free monitor to be plugged in, but we have none\n");
}

bool DriverSettingManager::GetSettingsFromRegistry(bool firstCall=true)
{
    HKEY hKey;

    // Open path to the value
    LONG nError = RegOpenKeyEx(REGISTRY_ROOT, TEXT(REGISTRY_SETTINGS_PATH), NULL, KEY_READ | KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey);
    if (nError != ERROR_SUCCESS)
    {
        LOG_ERROR(NULL, "ERROR {} : while opening registry path\n", nError);
        return false;
    }

    // Prepare data we will set to the registry
    JumpDisplayDriverControl::DriverSettings settings;
    DWORD dwSize = sizeof(settings);
    DWORD dwType = REG_BINARY;

    // Actually set the value in the registry
    nError = RegQueryValueEx(hKey, REGISTRY_KEY_NAME, 0, &dwType, (LPBYTE)&settings, &dwSize);
    if (nError != ERROR_SUCCESS)
    {
        LOG_ERROR(NULL, "ERROR {} : while setting registry value\n", nError);
        return false;
    }

    // No longer need this
    RegCloseKey(hKey);
    hKey = NULL;

    // Sanity check
    if (settings.versionNumber != VersionNumber)
    {
        LOG_ERROR(NULL, "Settings version number mismatch. Got {}, expected {}\n", settings.versionNumber, VersionNumber);
        return false;
    }

    if(settings.size != sizeof(settings))
    {
        LOG_ERROR(NULL, "Structure size mismatch. Got {}, expected {}\n", settings.size, sizeof(settings));
        return false;
    }

    if (sizeof(settings) != dwSize)
    {
        LOG_ERROR(NULL, "Read size mismatch. Got {}, expected {}\n", dwSize, sizeof(settings));
        return false;
    }

    // If we received a new path to start logging data, reinitialize the logger
    std::string sLogFilePath = settings.driverLogFilePath;
    if (sLogFilePath.size() > 0 && memcmp(settings.driverLogFilePath,mSettingsApplied.driverLogFilePath,sizeof(settings.driverLogFilePath)) != 0)
    {
        sLogger.Init(sLogFilePath.c_str());
    }

    // First call does not need to detect changes from one version to another
    if (firstCall == true)
    {
        AutoLock adapterSettingsLock(&mSettinsAppliedLock);

        mSettingsApplied = settings;

        // For the sake of debugging, check what we received
        unsigned int monitorCount = GetMonitorCount();
        LOG_INFO(NULL, "Number of monitors to create : {}\n", monitorCount);
        for (unsigned int i = 0; i < monitorCount; i++)
        {
            unsigned int resolutionCount = JumpDisplayDriverControl::MonitorInfo::ResolutionCountGet(mSettingsApplied.monitorInfos[i]);
            for (unsigned int j = 0; j < resolutionCount; j++)
            {
                LOG_INFO(NULL, "\t Monitor {} : {}x{}, {} Hz\n",
                    i,
                    mSettingsApplied.monitorInfos[i].resolutions[j].width,
                    mSettingsApplied.monitorInfos[i].resolutions[j].height,
                    mSettingsApplied.monitorInfos[i].resolutions[j].verticalSync);
            }
        }
    }
    // check for changes
    else
    {
        AutoLock adapterSettingsLock(&mSettinsAppliedLock);

        // Bulk compare
        if (memcmp(&mSettingsApplied, &settings, sizeof(mSettingsApplied)) == 0)
        {
            // Nothing changed, no need to make updates
            return true;
        }

        // Update current status to represent the requested status
        // This is just to make sure, if updates run in the background, we do not try to access outdated settings
        JumpDisplayDriverControl::DriverSettings oldSettings = mSettingsApplied;
        mSettingsApplied = settings;

        // Check if monitors got removed
        for (unsigned int oldInd = 0; oldInd < MaxMonitorCount; oldInd++)
        {
            // Only inspect valid info
            if (JumpDisplayDriverControl::MonitorInfo::IsValid(oldSettings.monitorInfos[oldInd]) == false)
            {
                continue;
            }
            bool monitorPresent = false;
            for (unsigned int newInd = 0; newInd < MaxMonitorCount; newInd++)
            {
                if (settings.monitorInfos[newInd].UID == oldSettings.monitorInfos[oldInd].UID)
                {
                    monitorPresent = true;
                    break;
                }
            }
            // Need to remove this monitor
            if (monitorPresent == false)
            {
                LOG_TRACE(NULL, "Driver should remove monitor UID {}\n", oldSettings.monitorInfos[oldInd].UID);
                MonitorUnplug(oldSettings.monitorInfos[oldInd].UID);
            }
        }

        // Check if monitors got added
        for (unsigned int newInd = 0; newInd < MaxMonitorCount; newInd++)
        {
            // Only inspect valid info
            if (JumpDisplayDriverControl::MonitorInfo::IsValid(settings.monitorInfos[newInd]) == false)
            {
                continue;
            }
            bool monitorPresent = false;
            for (unsigned int oldInd = 0; oldInd < MaxMonitorCount; oldInd++)
            {
                if (settings.monitorInfos[newInd].UID == oldSettings.monitorInfos[oldInd].UID)
                {
                    monitorPresent = true;
                    break;
                }
            }
            // Need to add this monitor
            if (monitorPresent == false)
            {
                LOG_TRACE(NULL, "Driver should add monitor UID {}\n", settings.monitorInfos[newInd].UID);
                MonitorPlugin();
            }
        }

        // Check for same monitors if their resolution changed
        for (unsigned int newInd = 0; newInd < MaxMonitorCount; newInd++)
        {
            // Only inspect valid info
            if (JumpDisplayDriverControl::MonitorInfo::IsValid(settings.monitorInfos[newInd]) == false)
            {
                continue;
            }
            for (unsigned int oldInd = 0; oldInd < MaxMonitorCount; oldInd++)
            {
                if (settings.monitorInfos[newInd].UID == oldSettings.monitorInfos[oldInd].UID)
                {
                    if (JumpDisplayDriverControl::MonitorInfo::ResolutionsChanged(settings.monitorInfos[newInd], oldSettings.monitorInfos[oldInd]) == true)
                    {
                        LOG_TRACE(NULL, "Driver should update monitor UID {} resolution list\n", settings.monitorInfos[newInd].UID);
                        MonitorUnplug(oldSettings.monitorInfos[oldInd].UID);
                        MonitorPlugin();
                    }
                }
            }
        }
    }

    // All done well
    return true;

}

unsigned int DriverSettingManager::MonitorIndexGet(const void* pMonitor)
{
    AutoLock adapterSettingsLock(&mAdapterStatusLock);

    // Find the index in the list for this setting
    for (int j = 0; j < MaxMonitorCount; j++)
    {
        if (mAdapterStatus[j].pMonitor == pMonitor)
        {
            return mAdapterStatus[j].settingsIndex;
        }
    }

    LOG_TRACE(NULL, "Could not find default settings for monitor {}\n", pMonitor);
    return MaxMonitorCount;
}

void DriverSettingManager::MonitorIndexRemove(const void* pMonitor)
{
    AutoLock adapterSettingsLock(&mAdapterStatusLock);

    for (int j = 0; j < MaxMonitorCount; j++)
    {
        if (mAdapterStatus[j].pMonitor == pMonitor)
        {
            AdapterMonitorInfo mi;
            mAdapterStatus[j] = mi; // Restore to default settings
            return;
        }
    }
    LOG_ERROR(NULL, "Tried to remove monitor {}, but we do not know about it", pMonitor);
}

void DriverSettingManager::HardwareMouseCursorTryEnable()
{
    AutoLock adapterSettingsLock(&mAdapterStatusLock);

    for (int j = 0; j < MaxMonitorCount; j++)
    {
        // This monitor no longer requires it's HW cursor support to be toggled
        if (mAdapterStatus[j].enableHWMouseStamp == 0)
        {
            continue;
        }

        // Disable Hardware mouse support for this monitor
        // Right now this code returns : STATUS_INVALID_PARAMETER
        IDARG_IN_SETUP_HWCURSOR hwCursor;
        memset(&hwCursor, 0, sizeof(hwCursor));

        hwCursor.hNewCursorDataAvailable = mCursorEvents[j];
        hwCursor.CursorInfo.Size = sizeof(IDDCX_CURSOR_CAPS);
        hwCursor.CursorInfo.AlphaCursorSupport = true;
        hwCursor.CursorInfo.ColorXorCursorSupport = IDDCX_XOR_CURSOR_SUPPORT_FULL;
        hwCursor.CursorInfo.MaxX = MOUSE_CURSOR_PIXEL_SIZE;
        hwCursor.CursorInfo.MaxY = MOUSE_CURSOR_PIXEL_SIZE;
        NTSTATUS Status = IddCxMonitorSetupHardwareCursor((IDDCX_MONITOR)mAdapterStatus[j].pMonitor, &hwCursor);
        if (!NT_SUCCESS(Status))
        {
            LOG_ERROR(NULL, "Error {}. Could not enable monitor hardware mouse rendering for monitor {}\n", Status, mAdapterStatus[j].pMonitor);
            //wait a bit more and try again
            mAdapterStatus[j].enableHWMouseStamp = GetTickCount64() + TIME_TOGGLE_HW_MOUSE_SUPPORT_MS;
        }
        else
        {
            mAdapterStatus[j].enableHWMouseStamp = 0;
            LOG_TRACE(NULL, "Enabled hardware mouse rendering for monitor {}\n", mAdapterStatus[j].pMonitor);
        }

    }
}

DWORD ThreadPeriodicSettingsChangeCheck(void* callerClass)
{
    DriverSettingManager* owner = (DriverSettingManager*)callerClass;
    if (owner == NULL)
    {
        LOG_ERROR(NULL, "Thread created with invalid parameter\n");
        return 1;
    }

    // For the curious minds
    owner->SetUpdateThreadState(DriverSettingManager::ThreadStates::TS_RUNNING);

    // Run the loop until someone externally will try to shut us down
    while (owner->IsUpdateThreadStopQueued() == false)
    {
        // Sleep first since we updated the settings before we created the thread
        Sleep(DriverSettingsUpdateIntervalMS);

        // Maybe driver unload started while we were waiting
        if (owner->IsUpdateThreadStopQueued() == true)
        {
            break;
        }

        // This will also do the "unplug","replug" of monitors
        owner->GetSettingsFromRegistry(false);

        // Check if any of the monitors need their harware mouse enabled
        owner->HardwareMouseCursorTryEnable();
    }

    // For the curious minds
    owner->SetUpdateThreadState(DriverSettingManager::ThreadStates::TS_STOPPED);

    LOG_TRACE(NULL, "Exiting settings updater thread\n");

    return 0;
}

bool DriverSettingManager::SettingsUpdaterThreadRun()
{
    // Sanity check
    if (mUpdateThreadState != ThreadStates::TS_STOPPED && mUpdateThreadState != ThreadStates::TS_OFFLINE)
    {
        LOG_ERROR(NULL, "There should be only 1 thread running at a time. State {}\n", mUpdateThreadState);
        return false;
    }

    // Start the thread
    mUpdaterThread = CreateThread(NULL, 0, ThreadPeriodicSettingsChangeCheck, this, NULL, NULL);
    if (mUpdaterThread == NULL)
    {
        LOG_ERROR(NULL, "Could not create thread. Error {}", errno);
        return false;
    }

    // Thread got created successfully
    LOG_TRACE(NULL, "Created thread to monitor setting changes\n");

    return true;

}

bool DriverSettingManager::MonitorUnplug(const unsigned int UID)
{
    AutoLock adapterSettingsLock(&mAdapterStatusLock);

    int atIndex = -1;
    for (int j = 0; j < MaxMonitorCount; j++)
    {
        if (mAdapterStatus[j].monitorUID == UID)
        {
            atIndex = j;
        }
    }
    if (atIndex == -1)
    {
        LOG_ERROR(NULL, "Tried to remove monitor with UID {}, but there is no monitor instance for it\n", UID);
        return false;
    }
    // Unplug the monitor
    IDDCX_MONITOR pMonitor = (IDDCX_MONITOR)mAdapterStatus[atIndex].pMonitor;

    // Remove from the lookup list
    AdapterMonitorInfo mi;
    mAdapterStatus[atIndex] = mi; // Restore to default settings

    NTSTATUS status = IddCxMonitorDeparture(pMonitor);
    if (status != STATUS_SUCCESS)
    {
        LOG_ERROR(NULL, "Failed to unplug monitor UID {} {}\n", UID, (void*)pMonitor);
        return false;
    }

    return true;
}

void PluginNewMonitor(const unsigned int connectorIndex);
bool DriverSettingManager::MonitorPlugin()
{
    AutoLock adapterSettingsLock(&mAdapterStatusLock);

    // Find a free adapter port we could use
    int atIndex = -1;
    for (int j = 0; j < MaxMonitorCount; j++)
    {
        if (mAdapterStatus[j].monitorUID == 0)
        {
            atIndex = j;
            break;
        }
    }

    if (atIndex == -1)
    {
        LOG_ERROR(NULL, "Adapter has no more free ports to plug in new monitors into it\n");
        return false;
    }

    // Initialization might run in the background
    PluginNewMonitor(atIndex);

    return true;
}

void DriverSettingManager::UnInitialize()
{
    // Singal the background thread that we want it to stop
    if (mUpdateThreadState != ThreadStates::TS_OFFLINE)
    {
        // Queue thread to exit as soon as possible
        mUpdateThreadState = ThreadStates::TS_STOPPING;

        // Wait for the thread to exit. 
        LOG_TRACE(NULL, "Waiting for updater thread to exit\n");
        WaitForSingleObject(mUpdaterThread, SHUTDOWN_TIMEOUT_MS);
        LOG_TRACE(NULL, "Updater thread exited\n");
    }
}

AutoLock::AutoLock(CRITICAL_SECTION* cs)
{
    SectionLock = NULL;
    if (cs == NULL)
    {
        LOG_ERROR(NULL, "Atempted to lock section without an actual lock\n");
        return;
    }
    EnterCriticalSection(cs);
    SectionLock = cs;
}

AutoLock::~AutoLock()
{
    if (SectionLock)
    {
        LeaveCriticalSection(SectionLock);
        SectionLock = NULL;
    }
}
