*********************************************
* 			Description
*********************************************
Class that enables the creation of virtual monitors


*********************************************
* 			Usage
*********************************************
Include "VirtualMonitorAPI.h", "VirtualMonitorAPI.cpp" into your project.
Make sure you replace "Logger.h" and it's logging functions with your logging implementation

*********************************************
* 			Usage example
*********************************************
See "main.cpp" for full functioning example.


*********************************************
* 			Internal process
*********************************************
Before creating virtual monitors, the class will check if the driver is installed in the Microsoft driver store.
If the driver is not installed, it will install the driver foundbeside the application. 
It is best to install the driver when application is installed to fully control the process of instalation details.
You can specify up to 8 ( can be changed with recompilation ) virtual monitors and their resolution settings.
The class will create a named pipe ( run in a background thread ) that will expose the settings for the monitors.
The class will create a software device. PnP manager will look for a mathing driver to be loaded.
When the driver loads, it will connect to the named pipe and fetch the monitor settings : number of monitors and their resolutions
After the settings are fetched, or the pipe times out, the background thread is stopped.
When the object is destroyed, the software device is removed and the driver is unloaded.