*********************************************
* 			Description
*********************************************
Device driver that will create virtual monitors

*********************************************
* 			Internal process
*********************************************
At startup the driver will try to connect, through a named pipe, to an application to fetch settings
If the driver is not able to fetch monitor settings, he will not create any virtual monitors
After fetching settings, the named pipe is closed and the driver no longer tries to communicate with the outside world
Based on given settings, number of monitors and their resolutions will be created
When a device is unplugged( no longer enumerated by PnP manager ), the driver will unload and "remove" the virtual monitors
Right now the settings can be changed only when the driver is loaded for the device