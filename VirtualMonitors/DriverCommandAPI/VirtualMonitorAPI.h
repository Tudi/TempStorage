#pragma once

#include <vector>
#include <string>
#include <swdevice.h>

// This is how you will see the device in the device manager
#define SoftwareDeviceDescription       L"Jump Desktop Virtual Monitor"
// These match the Pnp id's in the inf file so OS will load the driver when the device is created
// Should be lower case
#define DriverFileName                  "virtualmonitordriver.inf"
#define InstanceId                      L"VirtualMonitors"
#define HardwareId                      L"VirtualMonitors\0\0"
// Check for reasonable requested settings for a monitor
#define SANITY_CHECK_MAX_RESOLUTION		20000
#define SANITY_CHECK_MAX_REFRESH_RATE	500
#define VersionNumber                   1
// Settings if we use registry to store driver setup parameters
#define REGISTRY_ROOT                   HKEY_LOCAL_MACHINE
#define REGISTRY_SETTINGS_PATH          "Software\\Jump Desktop"
#define REGISTRY_KEY_NAME               TEXT("MonitorSettings")
// Used only for sanity checks. You can change this value
#define MaxMonitorCount                 8
#define MaxMonitorResolutions           32
#define WaitTimeOutMSDeviceLoad         10 * 1000
#define DriverSettingsUpdateIntervalMS  1000        // every X ms the driver will check if settings changed

// Purpuse of the ifdef is to practically test the separation of different implementations. Make sure one can exist without the other
#define DisablePipeImplementation  
#ifndef DisablePipeImplementation
    // Amount of time the application will wait for the driver to get online and request settings
    #define NamedPipeName                   TEXT("\\\\.\\pipe\\VirtualMonitorSettingsChannel")
#endif

/*
Create virtual monitors with specific resolution
*/

class JumpDisplayDriverControl
{
public:
    /*
    * Structure you need to initialize for the driver to create a monitor. All properties are required
    */
#pragma pack(push,1) // Just in case this will be shared between projects. Make sure allignment will not be an issue.
    struct ResolutionInfo
    {
        unsigned short width = 0;
        unsigned short height = 0;
        unsigned short verticalSync = 60; // Refresh rate

        /*
        * Basic check to make sure settings are not bad in an obvious way
        * Note that some adapters might be sensible about the amount of memory a resolution requires. This means that not every resolution is supported
        */
        static bool IsValid(const ResolutionInfo& self)
        {
            if (self.width <= 0 || self.width > SANITY_CHECK_MAX_RESOLUTION)
                return false;
            if (self.height <= 0 || self.height > SANITY_CHECK_MAX_RESOLUTION)
                return false;
            if (self.verticalSync <= 0 || self.verticalSync > SANITY_CHECK_MAX_REFRESH_RATE)
                return false;
            return true;
        }
    };
    // For a new monitor, the default resolution will be on index 0. New monitor is something the OS have not seen yet
    // Do not leave gaps between valid resolutions. First invalid resolution will be considered end of the list
    struct MonitorInfo
    {
        unsigned int    UID = 0; // You should not reuse IDs for monitors when creating new monitors or else driver will not know if it's a resolution change or new monitor
        ResolutionInfo  resolutions[MaxMonitorResolutions] = {}; // Could have been a vector, but I was advised to be static length

        /*
        * Helper function to save settings in the first available resolution slot
        */
        static bool ResolutionAdd(MonitorInfo& mi, unsigned short width, unsigned short height, unsigned short verticalSync)
        {
            for (int i = 0; i < MaxMonitorResolutions; i++)
            {
                if (ResolutionInfo::IsValid(mi.resolutions[i]) == false)
                {
                    mi.resolutions[i].width = width;
                    mi.resolutions[i].height = height;
                    mi.resolutions[i].verticalSync = verticalSync;
                    return true;
                }
            }
            return false;
        }

        /*
        * Check to see if monitor has mandatory settings configured
        */
        static bool IsValid(const MonitorInfo& mi)
        {
            if (mi.UID == 0)
            {
                return false;
            }
            // We need at least 1 valid resolution for the monitor to function
            for (int i = 0; i < MaxMonitorResolutions; i++)
            {
                if (ResolutionInfo::IsValid(mi.resolutions[i]) == true)
                {
                    return true;
                }
            }
            return false;
        }

        /*
        * Get the number of resolutions a monitor has that passes the minimal checks
        */
        static unsigned int ResolutionCountGet(const MonitorInfo& mi)
        {
            unsigned int validResolutionCount = 0;
            for (int i = 0; i < MaxMonitorResolutions; i++)
            {
                if (JumpDisplayDriverControl::ResolutionInfo::IsValid(mi.resolutions[i]) == true)
                {
                    validResolutionCount++;
                }
                else
                {
                    // Invalid monitor is considered end of the list
                    break;
                }
            }
            return validResolutionCount;
        }

        /*
        * Monitors need a unique identifier so we may reference them if they get unplugged/replugged
        */
        static unsigned int UIDGenerate()
        {
            static unsigned int nextFreeUID = 1;
            unsigned int ret = nextFreeUID;
            nextFreeUID++;
            return ret;
        }

        /*
        * Compare 2 monitor settings to see if they are equal
        */
        static bool ResolutionsChanged(const MonitorInfo& mi1, const MonitorInfo& mi2)
        {
            return memcmp(mi1.resolutions, mi2.resolutions, sizeof(mi2.resolutions)) != 0;
        }
    };

#ifndef DisablePipeImplementation
    // Client will request a header to know how much memory to prepare to be able to receive the sent data
    struct PipeMessageHeaderSettings
    {
        // To know how much buffer the client needs
        unsigned short messageSize = sizeof(PipeMessageHeaderSettings) + MaxMonitorCount * sizeof(MonitorInfo);
        // We are only expecting less than MaxMonitorCount
        unsigned char elementCount = MaxMonitorCount;
        // Make sure there are no allignment or structure version issues
        unsigned short elementSize = sizeof(MonitorInfo);
    };
#endif

    struct DriverSettings
    {
        unsigned int    size = sizeof(DriverSettings);
        unsigned int    versionNumber = VersionNumber;
        MonitorInfo     monitorInfos[MaxMonitorCount] = {};
        char            driverLogFilePath[MAX_PATH] = {};

        /*
        * Get the number of monitors that contain valid settings
        */
        static unsigned int MonitorCountGet(const DriverSettings& ds)
        {
            unsigned int validMonitorCount = 0;
            for (int i = 0; i < MaxMonitorCount; i++)
            {
                if (JumpDisplayDriverControl::MonitorInfo::IsValid(ds.monitorInfos[i]) == true)
                {
                    validMonitorCount++;
                }
                else
                {
                    // Invalid monitor is considered end of the list
                    break;
                }
            }
            return validMonitorCount;
        }
    };
#pragma pack(pop)

    /*
    * Generic destructor
    */
    ~JumpDisplayDriverControl();

    /*
    * Create virtual monitors based on the provided monitor settings ( count, resolution, refresh rate .. )
    * x86 version of this function is unable to check if driver is installed and can not install the driver ( x64 build )
    * Main API call
    */
    bool InitWithMonitors(const std::vector<MonitorInfo>& monitors, bool ignoreDriverSetup);

    /*
    * Tell the driver that we wish to make changes to the monitor count or resolutions
    * Not advised to be used only for a resolution change that a monitor already supports
    */
    bool DriverUpdateSettings(const std::vector<MonitorInfo>& monitors) { return RegistryStoreSettings(monitors); }

#ifndef DisablePipeImplementation
    bool InitWithMonitors_Piped(const std::vector<MonitorInfo>& monitors); // Probably depracated

    /*
    * Check if settings provider thread is still running
    */
    bool ServiceIsRunning() { return mServiceThread != NULL; }

    /*
    * Signal service finished. Called by the settings provider thread to let know the main application that driver
    * either timed out or successfully requested the settings
    */
    void ServiceFinishedRaiseEvent(bool Success);

    /*
    * Reference to the list of desired monitors
    */
    const std::vector<MonitorInfo>& MonitorsGet() { return mMonitors; }
#endif
    /*
    * Destroy all virtual monitors. Remove the device. Unload the driver
    * The function should not fail
    * Use this function if you wish to change the monitor setup
    */
    void Uninitialize();

    /*
    * If you wish for the driver to output logs to a specific file
    * This needs to be set before device driver is loaded
    * If not set, driver will not generate logs by default
    */
    void DriverLogFilePathSet(std::string newPath) { mDriverLofFilePath = newPath; }

private:
    /*
    * Use the PnP manager to check if we need to install the driver
    */
    bool DriverIsInstalled();

    /*
    * Use PnP manager to install the driver
    * This is just a fail safe. We probably installed the driver when the software was installed
    */
    bool DriverInstall();

#ifndef DisablePipeImplementation
    /*
    * Create a "method" for the driver to fetch settings from
    * Multiple methods could be used here, I have chosen name pipes as it seems to have the least requirements
    */
    bool SettingsServiceCreate();

    /*
    * Run in a background thread by SettingsServiceCreate
    */
    static void SettingsServiceRun(void *CallerClass);

    /*
    * Close the settings service
    */
    void SettingsServiceClose();
#endif
    /*
    * Create software device that will load the driver
    * Make sure you initialized the class before you call this function
    */
    int SoftwareDeviceCreate();

    /*
    * Create registry key/value and store driver settings
    */
    bool RegistryStoreSettings(const std::vector<MonitorInfo>& monitors);

    // Section for member variables

    // Pointer to the device we created to show the monitors
    HSWDEVICE hSwDevice = NULL;
    std::string mDriverLofFilePath;
#ifndef DisablePipeImplementation
    // Avoid some functions to be called multiple times
    bool mDriverHasBeenLoaded = false; 
    // Store the monitor info so that the settings service may use it later
    std::vector<MonitorInfo> mMonitors; 
    // The thread for serving settings
    HANDLE mServiceThread = NULL;
#endif
};