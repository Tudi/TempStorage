#pragma once

/*********************************************
* Implementation to restart active connections on the system
* The more I read, the more I'm convinced that there should be a lot of versions how to restart connections
* We will start with a simple solution, and in case connections have not been restarted, try another one
*********************************************/

void RestartNetworkConnections(int StartRedirection);