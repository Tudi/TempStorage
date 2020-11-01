#ifndef _THREADING_H
#define _THREADING_H

/*********************************************
* Header to include thread related header files
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
// Platform Specific Thread Starter
#include "ThreadStarter.h"

// Thread Pool
#include "ThreadPool.h"
#endif
#endif

