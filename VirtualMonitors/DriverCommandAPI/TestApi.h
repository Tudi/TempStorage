#pragma once

/*
* Create a device without any monitors
*/
bool TestingCreateDeviceTwoMonitors();
/*
* Destroy the device session store. Unload the device also
*/
void TestingUnloadDevice();
/*
* Wait time until a key is pressed is pressed
*/
void TestingWaitForKeypress(char key);
/*
* Test if adding a couple of monitors work
*/
void TestingAddMonitors();
/*
* Test if removing monitors work
*/
void TestingRemoveAMonitor();
/*
* Test if readding the monitor works
*/
void TestingReAddSameMonitor();
/*
* Test if changing existing monitor resolution works
*/
void TestChangeMonitorResolution();
/*
* Test if remove A monitor and reAdd it make it special
*/
void TestingRemoveAddNewMonitor();
/*
* Test if remove A monitor and reAdd it make it special
*/
void TestingReplaceMonitor();
