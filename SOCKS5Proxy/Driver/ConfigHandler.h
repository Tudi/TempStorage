#pragma once

/*********************************************
* Load / Store static values that will help adjust the behavior of our program
*********************************************/


//the given path should contain a JSON encoded config file
//see the given example for hardcoded required values
void LoadConfigValues(const char *);
//based on the last loaded config file, did we load enough required values ?
int ConfigFileContainedSetupValues();
//our proxy will create a bridge compatible with SOCKS 5 tunnel
unsigned short GetOurProxyPort();
// commands port should not be redirected
unsigned short GetOurCommandsPort();
//keep this program running until we get a command to shut down
int IsWaitingForUserExitProgram();
//maybe due to an error, maybe some external command
void SetProgramTerminated();
//we will need to wait for all threads to exit if we want to properly shut down
void IncreaseThreadsRunningCount();
//when a thread exited, decrease the counter. TRequired for graceful shutdown
void DecreaseThreadsRunningCount();
//when our thread counter gets to 0, we presume a proper shutdown was achieved
int GetRunningThreadCount();
//How much should the watchdog thread sleep before rechecking the tunnel status ?
int GetWatchdogTunnelSleepMs();
//We might not want to redirect packets to dead tunnels
int RedirectPacketsToDeadTunnels();
//Port number where we listen for external commands
unsigned short GetCommandsPort();
//we can disable traffic redirection with external commands
int IsRedirectionEnabled();
//can enable or disable packet redirection using config file or external commands
void SetPacketRedirectionStatus(int EnableRedirection);
//when running as service, need to give full path to config file
char* GetFullPath();


//monitor when a thread exited or started. If we want a graceful shutdown, we should wait for threads to exit
class AutoMonitorThreadExit
{
public:
	AutoMonitorThreadExit()
	{
		IncreaseThreadsRunningCount();
	}
	~AutoMonitorThreadExit()
	{
		DecreaseThreadsRunningCount();
	}
};