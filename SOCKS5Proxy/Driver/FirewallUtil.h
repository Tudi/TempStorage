#pragma once

/*********************************************
* When the program starts, will add firewall rule to allow this program to send out/listen to network connecitons
* If rule already exists, nothing will happen ( even if it is a disabled rule )
*********************************************/

void AddFirewallRule();