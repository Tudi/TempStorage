#pragma once

/*********************************************
* Catch outgoung packets. Based on rules, forward them to our proxy, or let them follow their own path
*********************************************/

//Create interface to the driver
void SetupDriver();
//start redirecting packets until we say to stop
void StartRedirectLoop();