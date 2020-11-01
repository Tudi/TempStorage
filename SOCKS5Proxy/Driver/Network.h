#ifndef NETWORK_H_
#define NETWORK_H_

/*********************************************
* Header to include async socket related headers
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
#include <winsock2.h>
#include <Windows.h>
#include <assert.h>

#include "StraightBuffer.h"
#include "SocketDefines.h"
#include "SocketOps.h"
#include "Socket.h"

#include "SocketMgrWin32.h"
#include "ListenSocketWin32.h"

#ifndef ASSERT
	#define ASSERT(x) assert(x)
#endif

#endif
#endif
