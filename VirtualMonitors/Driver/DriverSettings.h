#pragma once

/*
* Driver needs to be configurable to change the number of monitors presented dynamically
* Feel free to change the implementation to whatever you like : static file, registry setting....
*/
#include <vector>
#include <map>
#include <string>
#include "../DriverCommandAPI/VirtualMonitorAPI.h"

// After monitor has been plugged in, how much time should we wait to enable HW mouse support
#define TIME_TOGGLE_HW_MOUSE_SUPPORT_MS	2000
#define MOUSE_ENABLE_RETRY_COUNT		3		// In my test I rarely seen the API fail more than once
#define MOUSE_CURSOR_PIXEL_SIZE			32
#define SHUTDOWN_TIMEOUT_MS				10000 // Must be larget than DriverSettingsUpdateIntervalMS

class AutoLock
{
public:
	AutoLock(CRITICAL_SECTION* cs);
	~AutoLock();
private:
	CRITICAL_SECTION* SectionLock;
};
/*
* Driver should have one object for settings
* Settings should be initialized when the driver gets loaded
* Right now the driver does not support settings changes after it has been loaded
*/
class DriverSettingManager
{
public:
	// Keep track which monitor is pluged into what
	struct AdapterMonitorInfo
	{
		// Monitors are plugged into adapters into ports
		unsigned short		adapterPort = 0xFFFF;
		// An ID so we may know which monitor is plugged in
		unsigned int		monitorUID = 0;
		// This plugged in monitor is using settings located at this index
		unsigned int		settingsIndex = 0xFFFF;
		// Monitor pointer is used if we wish to unplug only 1 monitor
		const void* pMonitor = NULL;
		// After monitor has been plugged in and setup finalized, we enable mouse hardware rendering 
		unsigned long long	enableHWMouseStamp = 0;
		// Do not try too many times to enable HW mouse
		unsigned char		mouseEnableRetries = 0;
	};

	enum ThreadStates
	{
		TS_OFFLINE = 0,
		TS_RUNNING,
		TS_STOPPING,
		TS_STOPPED
	};

	DriverSettingManager();
	~DriverSettingManager();

	/*
	* Block until the update thread exits
	*/
	void UnInitialize();
	/*
	* Fetch settings info from parent application
	* Disadvanatages of using a pipe is that a device might load up slower than the wait timout
	*/
	//	bool GetSettingsFromPipe();

	/*
		* Fetch settings from registry entry
		*/
	bool GetSettingsFromRegistry(bool firstCall);

	/*
	* Number of monitors we should create
	*/
	unsigned int GetMonitorCount()
	{
		return JumpDisplayDriverControl::DriverSettings::MonitorCountGet(mSettingsApplied);
	}

	/*
	* Inverse lookup table to get connector index from monitor object
	*/
	void MonitorIndexSet(const void* pMonitor, unsigned int connectorIndex);

	/*
	* From the list of all possible resolutions, which one do we wish to use at startup ?
	*/
	unsigned int MonitorIndexGet(const void* pMonitor);

	/*
	* All supported resolutions and refresh rates supported by default + settings requested by app
	*/
	const JumpDisplayDriverControl::MonitorInfo* MonitorSettingsGet(const void* pMonitor)
	{
		unsigned int index = MonitorIndexGet(pMonitor);
		// Sanity check
		if (index >= MaxMonitorCount)
		{
			return NULL;
		}
		return &mSettingsApplied.monitorInfos[index];
	};

	/*
	* All supported resolutions and refresh rates supported by default + settings requested by app
	*/
	unsigned int MonitorSettingsCountGet(const void* pMonitor)
	{
		unsigned int index = MonitorIndexGet(pMonitor);
		// Sanity check
		if (index >= MaxMonitorCount)
		{
			return NULL;
		}
		return JumpDisplayDriverControl::MonitorInfo::ResolutionCountGet(mSettingsApplied.monitorInfos[index]);
	};

	/*
	* Run a background thread to periodically check for settings changes
	*/
	bool SettingsUpdaterThreadRun();

	/*
	* Maybe the "app" can signal the unloading of the driver
	*/
	bool IsUpdateThreadStopQueued() { return mUpdateThreadState != ThreadStates::TS_RUNNING; }

	/*
	* Set thread state
	*/
	void SetUpdateThreadState(ThreadStates ts) { mUpdateThreadState = ts; }

	/*
	* After a monitors finished setting up, we enable hardware mouse cursor
	*/
	void HardwareMouseCursorTryEnable();
private:
	/*
	* When we unplug a monitor, we remove it from the list of active monitors
	*/
	void MonitorIndexRemove(const void* pMonitor);

	/*
	* Unplug a monitor from the adapter
	*/
	bool MonitorUnplug(const unsigned int UID);

	/*
	* Plug a new monitor into the adapter
	* The settings of this new monitor, will be the first non used settings
	*/
	bool MonitorPlugin();

	// The settings as sent by the "APP". Store them so we may compare changes from 1 version to another
	JumpDisplayDriverControl::DriverSettings mSettingsApplied = {};

	// Only one instance of the update function should run at a time
	CRITICAL_SECTION mSettinsAppliedLock;

	// Break the update thread cycle ?
	volatile ThreadStates mUpdateThreadState = ThreadStates::TS_OFFLINE;

	// Adapter will have it's own ports where we can plugin/unplug monitors. Need to keep track where we can plug in a new one
	// The index of the array matches the port of the adapter
	AdapterMonitorInfo mAdapterStatus[MaxMonitorCount] = {};

	// Make sure there is no parallel access on adapter status
	CRITICAL_SECTION mAdapterStatusLock;

	// Hardware cursor requires these events
	HANDLE mCursorEvents[MaxMonitorCount] = {};

	// The background thread that periodically checks for monitor setting changes
	HANDLE mUpdaterThread = NULL;
};