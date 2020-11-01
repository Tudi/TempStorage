#pragma once

/*********************************************
* Whatchdog to monitor the status of the SOCKS 5 tunnel
* There is no reason to redirect traffic to an inexistent tunnel
*********************************************/

//start watchdog thread
void StartTunnelCheckerThread();
