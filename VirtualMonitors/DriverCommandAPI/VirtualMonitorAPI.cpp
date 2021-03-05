#include <Windows.h>
#include <wrl.h>
#include <algorithm>
#include <cctype>
#include <process.h>
#include <sddl.h>
#include "VirtualMonitorAPI.h"
#include "Logger.h"

JumpDisplayDriverControl::~JumpDisplayDriverControl()
{
	// Make sure we properly unload the driver
	Uninitialize();
}

bool JumpDisplayDriverControl::InitWithMonitors(const std::vector<MonitorInfo>& monitors, bool ignoreDriverSetup)
{
	// We do not support changing the driver. We unload it and reload it with new settings
    if (hSwDevice != NULL)
    {
        LOG_INFO(NULL, "Device has already been initialized. Unloading it.\n");
        Uninitialize();
    }

	// Check if driver has been registered on the system. If not, make sure we register it
    // The driver was probably already registered using the software installer
    if (ignoreDriverSetup == false && DriverInstall() == false)
    {
        LOG_ERROR(NULL, "Exiting. Could not intall the driver.\n");
        return false;
    }

    // Sanity checks
    if (monitors.size() == 0)
    {
        LOG_ERROR(NULL, "Invalid settings.There is no point to create a device with 0 monitors\n");
        return false;
    }
    // Sanity checks
    bool foundAValidMonitor = false;
    for (unsigned int i = 0; i < monitors.size(); i++)
    {
        if (MonitorInfo::IsValid(monitors[i]) == true)
        {
            foundAValidMonitor = true;
        }
    }
    if (foundAValidMonitor == false)
    {
        LOG_ERROR(NULL, "Error : Could not find any valid monitor settings\n");
        return false;
    }

    // Update registry with the fixed size structure containing all the settings
    if (RegistryStoreSettings(monitors) == false)
    {
        LOG_ERROR(NULL, "Exiting. Could not set driver settings in registry.\n");
        return false;
    }

    // Create the device ( will handle the virtual monitors )
    int errorCodeCreateDevice = SoftwareDeviceCreate();
    if (errorCodeCreateDevice)
    {
        LOG_ERROR(NULL, "Exiting. Could not create software device for monitors {}.\n", errorCodeCreateDevice);
        return false;
    }

    // If we got here, all went great
    LOG_TRACE(NULL, "Exiting : New device with requested monotors.\n");
    return true;
}

void JumpDisplayDriverControl::Uninitialize()
{
#ifndef DisablePipeImplementation
    // Nothing to do. Driver is not loaded
    if (mDriverHasBeenLoaded == false)
    {
        LOG_TRACE(NULL, "Tried to remove software device, but it was not initialized. Maybe double remove\n");
        return;
    }

    // Mark the function as not required to be called multiple times
    mDriverHasBeenLoaded = false;

    // Most probably this does nothing. Better be safe than sorry
    SettingsServiceClose();
#endif
    // Stop the device, this will cause the sample to be unloaded
    if (hSwDevice != NULL)
    {
        SwDeviceClose(hSwDevice);
        hSwDevice = NULL;
    }
}

VOID WINAPI CreationCallback(_In_ HSWDEVICE hSwDevice, _In_ HRESULT hrCreateResult, _In_opt_ PVOID pContext, _In_opt_ PCWSTR pszDeviceInstanceId)
{
	HANDLE hEvent = *(HANDLE*)pContext;
//    LOG_INFO(NULL, "Software device has been created with result code {} and InstanceId {:S}", hrCreateResult, pszDeviceInstanceId);
    LOG_INFO(NULL, "Software device has been created with result code {}\n", hrCreateResult);
    SetEvent(hEvent);
	UNREFERENCED_PARAMETER(hSwDevice);
	UNREFERENCED_PARAMETER(hrCreateResult);
	UNREFERENCED_PARAMETER(pszDeviceInstanceId);
}

int JumpDisplayDriverControl::SoftwareDeviceCreate()
{
    HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    SW_DEVICE_CREATE_INFO createInfo = { 0 };
    PCWSTR description = SoftwareDeviceDescription;

    // These match the Pnp id's in the inf file so OS will load the driver when the device is created    
    PCWSTR instanceId = InstanceId;
    PCWSTR hardwareIds = HardwareId;
    PCWSTR compatibleIds = HardwareId;

    createInfo.cbSize = sizeof(createInfo);
    createInfo.pszzCompatibleIds = compatibleIds;
    createInfo.pszInstanceId = instanceId;
    createInfo.pszzHardwareIds = hardwareIds;
    createInfo.pszDeviceDescription = description;

    createInfo.CapabilityFlags = SWDeviceCapabilitiesRemovable | SWDeviceCapabilitiesSilentInstall | SWDeviceCapabilitiesDriverRequired;

    // Create the device
    HRESULT hr = SwDeviceCreate(InstanceId, L"HTREE\\ROOT\\0", &createInfo, 0, nullptr, CreationCallback, &hEvent, &hSwDevice);
    if (FAILED(hr))
    {
        LOG_ERROR(NULL, "SwDeviceCreate failed with 0x{:X}\n", hr);
        CloseHandle(hEvent);
        return hr;
    }

    // Wait for callback to signal that the device has been created
    LOG_TRACE(NULL, "Waiting for device to be created....\n");
    DWORD waitResult = WaitForSingleObject(hEvent, WaitTimeOutMSDeviceLoad);
    if (waitResult != WAIT_OBJECT_0)
    {
        LOG_ERROR(NULL, "Wait for device creation failed {}\n", waitResult);
        CloseHandle(hEvent);
        return waitResult;
    }

    CloseHandle(hEvent);

    LOG_TRACE(NULL, "Software device created successfully\n");
    return 0;
}

std::string RunSystemCommandGetResult(const std::string &cmd)
{
    std::string result = "";
#if defined(_x86) && 0 // This block of code is non functional due to system unable to execute pnputil.exe
    std::string redirectToFile = "cmdOut.txt";
    std::string redirectedCmd = cmd + " > " + redirectToFile;

    int cmdStatusCode = system(redirectedCmd.c_str());
    // The return code is not reliable, but it might help us at some point
    if (cmdStatusCode != 0)
    {
        LOG_INFO(NULL, "Return code {} for {}\n", cmdStatusCode, redirectedCmd);
    }

    // Get the content of the file
    FILE* outputFile;
    errno_t openErr = fopen_s(&outputFile, redirectToFile.c_str(), "rt");
    if (outputFile != NULL)
    {
        size_t bytesRead;
        char tempBuffer[1024];
        
        // Make sure there is alway at least 1 NULL terminator for the string
        tempBuffer[sizeof(tempBuffer) - 1] = 0;
        
        do {
            bytesRead = fread(tempBuffer, 1, sizeof(tempBuffer) - 1, outputFile);
            // Append this chunk to the return string
            result += tempBuffer;
        } while (bytesRead > 0);

        // We are done with this file
        fclose(outputFile);
    }
    else
    {
        LOG_ERROR(NULL, "FIle open error {}\n", openErr);
    }

    // Get rid of the temporary files
//    cmdStatusCode = _unlink(redirectToFile.c_str());
#else
    char buffer[128];
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe)
    {
        LOG_ERROR(NULL, "Could not open pipe to fetch system command output\n");
        return result;
    }
    try
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
        {
            result += buffer;
        }
    }
    catch (...)
    {
        _pclose(pipe);
        throw;
    }

    _pclose(pipe);
#endif
    return result;
}

bool JumpDisplayDriverControl::DriverInstall()
{
    //no reason to install the driver multiple times ( you can do it if you want to )
    if (DriverIsInstalled() == true)
    {
        LOG_TRACE(NULL, "Driver is installed\n");
        return true;
    }

    LOG_TRACE(NULL, "Installing driver\n");
    // Read that dpinst32.exe is a more proper way to install drivers. But that is an external program that needs installing
    std::string systemCommand = "%SystemRoot%\\System32\\pnputil.exe /add-driver ";
    systemCommand += DriverFileName;
    std::string result = RunSystemCommandGetResult(systemCommand.c_str());

    // Check if we indeed managed to install it
    if (result.find("Added driver packages:  1") == string::npos)
    {
        LOG_ERROR(NULL, "Could not install driver\n");
        return false;
    }

    LOG_TRACE(NULL, "Driver installed successfully\n");
    return true;
}

#ifndef DisablePipeImplementation
bool JumpDisplayDriverControl::InitWithMonitors_Piped(const std::vector<MonitorInfo>& monitors)
{
    // We do not support changing the driver. We unload it and reload it with new settings
    if (mDriverHasBeenLoaded == true)
    {
        LOG_TRACE(NULL, "Driver has already been initialized. Unloading it.\n");
        Uninitialize();
    }

    // Check if driver has been registered on the system. If not, make sure we register it
    // The driver was probably already registered using the software installer
    if (DriverInstall() == false)
    {
        LOG_ERROR(NULL, "Exiting. Could not intall the driver.\n");
        return false;
    }

    // Sanity checks
    if (monitors.size() == 0)
    {
        LOG_ERROR(NULL, "Invalid settings.There is no point to create a device with 0 monitors\n");
        return false;
    }
    // Sanity checks
    for (unsigned int i = 0; i < monitors.size(); i++)
    {
        if (MonitorInfo::IsValid(monitors[i]) == false)
        {
            LOG_ERROR(NULL, "Monitor {} setting is invalid {}x{}, {} Hz\n", i, monitors[i].width, monitors[i].height, monitors[i].verticalSync);
            return false;
        }
    }

    if (RegistryStoreSettings(monitors) == false)
    {
        LOG_ERROR(NULL, "Exiting. Could not set driver settings in registry.\n");
        return false;
    }

    // Copy all the data
    mMonitors = monitors;

    // Create a service to provide settings for the driver
    if (SettingsServiceCreate() == false)
    {
        LOG_ERROR(NULL, "Exiting. Could not create settings provider service.\n");
        return false;
    }

    // Create the device ( will handle the virtual monitors )
    int errorCodeCreateDevice = SoftwareDeviceCreate();
    if (errorCodeCreateDevice)
    {
        LOG_ERROR(NULL, "Exiting. Could not create software device for monitors {}.\n", errorCodeCreateDevice);
        // No longer need the settings service
        SettingsServiceClose();
        // No reason to keep the monitor settings
        mMonitors.clear();
        return false;
    }

    // Wait for the driver to fetch settings from us
    LOG_TRACE(NULL, "Wait for settings to be fetched by the driver\n");
    DWORD waitResult = WAIT_OBJECT_0;
    if (mServiceThread != NULL)
    {
        waitResult = WaitForSingleObject(mServiceThread, WaitTimeOutMSDeviceLoad);
    }
    else
    {
        LOG_ERROR(NULL, "Unexpected : Settings service thread handle is NULL\n");
    }

    // Close the settings provider service
    if (waitResult != WAIT_OBJECT_0)
    {
        LOG_ERROR(NULL, "Settings were not fetched by the driver. Wait result {} ( timeout = {} ), service running {}\n", waitResult, waitResult == WAIT_TIMEOUT, ServiceIsRunning());
        SettingsServiceClose();
    }

    mDriverHasBeenLoaded = true;

    // If we got here, all went great
    LOG_TRACE(NULL, "Exiting : New device with requested monotors.\n");
    return true;
}

void JumpDisplayDriverControl::SettingsServiceRun(void* callerClass)
{
    JumpDisplayDriverControl* owner = (JumpDisplayDriverControl*)callerClass;
    if (owner == NULL)
    {
        LOG_ERROR(NULL, "Thread created with invalid parameter\n");
        owner->ServiceFinishedRaiseEvent(false);
        return;
    }

    // The length of our serialized settings vector. Around 50 bytes / monitor
    int maxBufferToSend = 2 * ( MaxMonitorCount * sizeof(JumpDisplayDriverControl::MonitorInfo) + sizeof(JumpDisplayDriverControl::PipeMessageHeaderSettings));

    BYTE secdesc[SECURITY_DESCRIPTOR_MIN_LENGTH];
    SECURITY_ATTRIBUTES secattr;

    secattr.nLength = sizeof(secattr);
    secattr.bInheritHandle = TRUE;
    secattr.lpSecurityDescriptor = &secdesc;

    InitializeSecurityDescriptor(&secdesc, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&secdesc, TRUE, (PACL)0, FALSE);

    // Create the pipe
    HANDLE hPipe;
    hPipe = CreateNamedPipe(NamedPipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
        1,
        maxBufferToSend,
        maxBufferToSend,
        NMPWAIT_USE_DEFAULT_WAIT, // default timeout is 50 ms
        &secattr);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR(NULL, "Could not create pipe to send settings to driver\n");
        owner->ServiceFinishedRaiseEvent(false);
        return;
    }

    LOG_TRACE(NULL, "Waiting for driver to connect to us\n");
    if (ConnectNamedPipe(hPipe, NULL) == FALSE)   
    {
        LOG_ERROR(NULL, "Driver did not connect to the pipe\n");
        owner->ServiceFinishedRaiseEvent(false);
        DisconnectNamedPipe(hPipe);
        return;
    }

    // Maybe something changed in the background thread and we should no longer try to send data to the driver
    if (owner->ServiceIsRunning() == false)
    {
        LOG_ERROR(NULL, "SettingsService closed before sending settings work\n");
        owner->ServiceFinishedRaiseEvent(false);
        DisconnectNamedPipe(hPipe);
        return;
    }

    // Calculate the amount of bytes we want to send
    int headerSize = sizeof(JumpDisplayDriverControl::PipeMessageHeaderSettings);
    int messageSize = (int)(owner->MonitorsGet().size()) * (int)(sizeof(JumpDisplayDriverControl::MonitorInfo));
    int bytesToSend = headerSize + messageSize;
    
    // Reserver memory for the full message. (Could send data in chunks also )
    char* message = (char*)malloc(bytesToSend);
    if (message == NULL)
    {
        LOG_ERROR(NULL, "Failed to allocate {} bytes\n", bytesToSend);
        DisconnectNamedPipe(hPipe);
        return;
    }

    // Not required. Trying to make good habbits
    memset(message, 0, bytesToSend);
    
    // Set values for header. Map the header structure to the beginning of the buffer we allocated
    JumpDisplayDriverControl::PipeMessageHeaderSettings*header;
    header = (JumpDisplayDriverControl::PipeMessageHeaderSettings*)message;
    header->messageSize = header->elementCount * sizeof(JumpDisplayDriverControl::MonitorInfo);
    header->elementCount = (unsigned char)(owner->MonitorsGet().size());
    header->elementSize = sizeof(JumpDisplayDriverControl::MonitorInfo);

    // Map a vector of strutures in the allocated memory
    JumpDisplayDriverControl::MonitorInfo* monitors = (JumpDisplayDriverControl::MonitorInfo*)&message[sizeof(JumpDisplayDriverControl::PipeMessageHeaderSettings)];

    // Actual values we want to send
    for (int i = 0; i < header->elementCount; i++)
        memcpy(&monitors[i], &owner->MonitorsGet().at(i), sizeof(JumpDisplayDriverControl::MonitorInfo));
        
    // Actual sending of the data
    DWORD dwWrite;
    if (WriteFile(hPipe, message, (DWORD)bytesToSend, &dwWrite, NULL) == TRUE && dwWrite == bytesToSend)
    {
        LOG_TRACE(NULL, "Sent settings header to driver. Sent {}/{} bytes\n", dwWrite, bytesToSend);
    }
    else
    {
        LOG_ERROR(NULL, "Could not send header to driver\n");
        DisconnectNamedPipe(hPipe);
        return;
    }

    // We are done communitcating
    DisconnectNamedPipe(hPipe);

    free(message);
    message = NULL; // For good habbits. Will be removed by the compiler anyway

    // Let the owner thread know we are done sending data
    owner->ServiceFinishedRaiseEvent(true);

    LOG_TRACE(NULL, "Exiting settings service thread\n");
}

void JumpDisplayDriverControl::ServiceFinishedRaiseEvent(bool Success)
{
    // CloseHandle is called automatically
    mServiceThread = NULL; 
}

bool JumpDisplayDriverControl::SettingsServiceCreate()
{
    uintptr_t thread = _beginthread(SettingsServiceRun, 0, this);
    if (thread == -1)
    {
        LOG_ERROR(NULL, "Could not create thread. Error {}", errno);
        return false;
    }
    mServiceThread = (HANDLE)thread;

    return true;
}

void JumpDisplayDriverControl::SettingsServiceClose()
{
    LOG_TRACE(NULL, "Signal settings service to close\n");
    mServiceThread = NULL;
}
#endif

/*
* The issue is that the old API to get driver list is depracated
* The new API filter by driver deos not seem to work even if copy pasted values from the "found" version
* If we do the manual filtering, it takes 10 seconds to do it.
* Multiple project simply reuse "pnputil.exe /enum-drivers" and parse output
* There needs to be a better way to check if the driver is installed
*/
bool JumpDisplayDriverControl::DriverIsInstalled()
{
    std::string result = RunSystemCommandGetResult("%SystemRoot%\\System32\\pnputil.exe /enum-drivers");

    // Convert the output to lower case
    std::transform(result.begin(), result.end(), result.begin(),[](unsigned char c) { return std::tolower(c); });

    // Search lowercase driver filename in the output
    if (result.find(DriverFileName) == string::npos)
    {
        LOG_TRACE(NULL, "Diver is not installed\n");
        return false;
    }

    LOG_TRACE(NULL, "Driver is installed\n");

    return true;
}

bool JumpDisplayDriverControl::RegistryStoreSettings(const std::vector<MonitorInfo>& monitors)
{
    HKEY hKey;

    // Open path to the value
    LONG nError = RegCreateKeyExA(REGISTRY_ROOT, REGISTRY_SETTINGS_PATH, 0, NULL, REG_OPTION_VOLATILE, STANDARD_RIGHTS_REQUIRED | KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL);
    if (nError != ERROR_SUCCESS)
    {
        LOG_ERROR(NULL, "ERROR {} : while opening registry path\n", nError);
        return false;
    }

    // Prepare data we will set to the registry
    DriverSettings settings;
    // Make sure driver will not read random values. Zero everything
    memset(&settings, 0, sizeof(DriverSettings));
    // Values used for sanity checks on driver side
    settings.size = sizeof(DriverSettings);
    settings.versionNumber = VersionNumber;

    // The actual settings
    for (unsigned int i = 0; i < monitors.size() && i < MaxMonitorCount; i++)
        settings.monitorInfos[i] = monitors[i];
        //memcpy(&settings.monitorInfos[i], &monitors[i], sizeof(MonitorInfo));

    // Driver will not produce logs unless we set a log file path for him
    errno_t copyRes = strcpy_s(settings.driverLogFilePath, sizeof(settings.driverLogFilePath), mDriverLofFilePath.c_str());
    if (copyRes != NO_ERROR)
    {
        LOG_ERROR(NULL, "Failed to copy log dir path {}\n", mDriverLofFilePath);
        settings.driverLogFilePath[0] = 0;
    }

    // Set the value in the registry
    DWORD dwSize = settings.size;
    nError = RegSetValueEx(hKey, REGISTRY_KEY_NAME, 0, REG_BINARY, (unsigned char*)&settings, dwSize);
    if (nError != ERROR_SUCCESS)
    {
        LOG_ERROR(NULL, "ERROR {} : while setting registry value\n", nError);
        RegCloseKey(hKey);
        return false;
    }

    // Give full rights to authentificated users and administrator. Required because driver will run in user mode
    const TCHAR* rights = TEXT("D:(A;OICI;GA;;;AU)(A;OICI;GA;;;BA)");
    PSECURITY_DESCRIPTOR secdesc = 0;
    DWORD size = 0;
    if (ConvertStringSecurityDescriptorToSecurityDescriptor(rights, SDDL_REVISION_1, &secdesc, &size) == FALSE)
    {
        LOG_ERROR(NULL, "Could not convert registry access rights from string to struct\n");
    }
    else
    {
        nError = RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION, secdesc);
        if(nError != ERROR_SUCCESS)
        {
            LOG_ERROR(NULL, "Could not set registry access rights for driver {}\n", nError);
        }
    }

    // No longer need the key
    nError = RegCloseKey(hKey);
    if (nError != ERROR_SUCCESS)
    {
        LOG_ERROR(NULL, "Could not close registry key {}\n", nError);
    }

    return true;
}